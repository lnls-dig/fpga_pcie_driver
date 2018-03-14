/*******************************************************************
 * Simples Read/Write register access
 *
 *******************************************************************/


#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include "inttypes.h"
#include <pciDriver/lib/pciDriver.h>

/************************************************************/
/******************* PCIe Page constants ********************/
/************************************************************/
/* Some FPGA PCIe constants */
/* SDRAM is accesses via 32-bit BAR (32-bit addressing) */
#define PCIE_SDRAM_PG_SHIFT                 0           /* bits */
#define PCIE_SDRAM_PG_MAX                   20          /* bits */
#define PCIE_SDRAM_PG_SIZE                  (1<<PCIE_SDRAM_PG_MAX)  /* in Bytes (8-bit) */
#define PCIE_SDRAM_PG_MASK                  ((PCIE_SDRAM_PG_SIZE-1) << \
                                                PCIE_SDRAM_PG_SHIFT)

/* Wishbone is accessed via 64-bit BAR (64-bit addressed) */
#define PCIE_WB_PG_SHIFT                    0           /* bits */
#define PCIE_WB_PG_MAX                      16          /* bits */
#define PCIE_WB_PG_SIZE                     (1<<PCIE_WB_PG_MAX)  /* in Bytes (8-bit) */
#define PCIE_WB_PG_MASK                     ((PCIE_WB_PG_SIZE-1) << \
                                                PCIE_WB_PG_SHIFT)

/* PCIe SDRAM Address Page number and offset extractor */
#define PCIE_ADDR_SDRAM_PG_OFFS(addr)       ((addr & PCIE_SDRAM_PG_MASK) >> \
                                                PCIE_SDRAM_PG_SHIFT)
#define PCIE_ADDR_SDRAM_PG(addr)            ((addr & ~PCIE_SDRAM_PG_MASK) >> \
                                                PCIE_SDRAM_PG_MAX)

/* PCIe WB Address Page number and offset extractor */
#define PCIE_ADDR_WB_PG_OFFS(addr)          ((addr & PCIE_WB_PG_MASK) >> \
                                                PCIE_WB_PG_SHIFT)
#define PCIE_ADDR_WB_PG(addr)               ((addr & ~PCIE_WB_PG_MASK) >> \
                                                PCIE_WB_PG_MAX)

#define WB_QWORD_ACC                        3           /* 64-bit addressing */
#define WB_DWORD_ACC                        2           /* 32-bit addressing */
#define WB_WORD_ACC                         1           /* 16-bit addressing */
#define WB_BYTE_ACC                         0           /* 8-bit addressing */

/************************************************************/
/******************* FPGA PCIe Registers ********************/
/************************************************************/
/* FPGA PCIe registers. These are inside bar0r These must match
 * the FPGA firmware */

#define PCIE_CFG_REG_SDRAM_PG               (7 << WB_DWORD_ACC)
#define PCIE_CFG_REG_WB_PG                  (9 << WB_DWORD_ACC)

/************************************************************/
/*********************** Page Macros ************************/
/************************************************************/
#define SET_PG(bar0, which, num)                                    \
    do {                                                            \
        bar0[which >> WB_DWORD_ACC] =                               \
                num;                                                \
    } while (0)

#define SET_SDRAM_PG(bar0, num)                                     \
    SET_PG(bar0, PCIE_CFG_REG_SDRAM_PG, num)

#define SET_WB_PG(bar0, num)                                        \
    SET_PG(bar0, PCIE_CFG_REG_WB_PG, num)

#define BAR_RW_TYPE                         uint32_t

/* Read or write to BAR */
#define BAR_RW_8(barp, addr, datap, rw)                             \
    do {                                                            \
        (rw) ?                                                      \
        (*(datap) = *(BAR_RW_TYPE *)(((uint8_t *)barp) + (addr))) : \
        (*(BAR_RW_TYPE *)(((uint8_t *)barp) + (addr)) = *(datap));  \
    } while (0)

/* BAR0 is BYTE addressed for the user */
#define BAR0_RW(barp, addr, datap, rw)                              \
    BAR_RW_8(barp, addr, datap, rw)

/* BAR2 is BYTE addressed for the user */
#define BAR2_RW(barp, addr, datap, rw)                              \
    BAR_RW_8(barp, addr, datap, rw)

/* BAR4 is BYTE addresses for the user */
/* On PCIe Core FPGA firmware the wishbone address is provided with
 * only 29 bits, with the LSB zeroed:
 *
 *  bit 31        . . .      bit 3   bit 2   bit 1   bit 0
 *   A31          . . .        A3     '0'     '0'    '0'
 *
 * This is done as the BAR4 is 64-bit addressed. But, the output of the
 * PCIe wrapper are right shifted to avoid dealing with this particularity:
 *
 *  bit 31   bit 30   bit 29  bit 28    . . .      bit 3   bit 2   bit 1   bit 0
 *   '0'      '0'     '0'       A31     . . .        A6     A5      A4      A3
 *
 * */
#define BAR4_RW(barp, addr, datap, rw)                              \
    do {                                                            \
        (rw) ?                                                      \
        (*(datap) = *((barp) + (addr))) :                           \
        (*((barp) + (addr)) = *(datap));                            \
    } while (0)

static struct option long_options[] =
{
    {"help",                no_argument,         NULL, 'h'},
    {"devicefile",          required_argument,   NULL, 'b'},
    {"verbose",             no_argument,         NULL, 'v'},
    {"read",                no_argument,   	 NULL, 'r'},
    {"write",               no_argument,   	 NULL, 'w'},
    {"barno",               required_argument,   NULL, 'n'},
    {"address",             required_argument,   NULL, 'a'},
    {"data",                required_argument,   NULL, 'd'},
    {NULL, 0, NULL, 0}
};

static const char* shortopt = "hb:vrwn:a:d:";

void print_help (char *program_name)
{
    fprintf (stdout, "Simple Register Access to FPGA\n"
            "Usage: %s [options]\n"
            "\n"
            "  -h  --help                           Display this usage information\n"
            "  -b  --devicefile <Device File>       Device File\n"
            "  -v  --verbose                        Verbose output\n"
            "  -r  --read                           Perform read access\n"
            "  -w  --write                          Perform write access\n"
            "  -n  --barno <BAR number = [0|2|4]    Bar Number\n"
            "  -a  --address <FPGA Address in hex>  Address to read/write\n"
            "  -d  --data <FPGA Data in hex>        Data to write to FPGA\n",
            program_name);
}

int main(int argc, char *argv[])
{
    int verbose = 0;
    char *devicefile_str = NULL;
    int rw_fpga = -1;
    char *barno_str = NULL;
    char *address_str = NULL;
    char *data_str = NULL;
    int opt;
    pd_device_t _dev = {0};
    pd_device_t *dev = &_dev;
    uint32_t *bar0 = NULL;
    uint32_t bar0_size;
    uint32_t *bar2 = NULL;
    uint32_t bar2_size;
    uint64_t *bar4 = NULL;
    uint32_t bar4_size;
	
    while ((opt = getopt_long (argc, argv, shortopt, long_options, NULL)) != -1) {
        /* Get the user selected options */
        switch (opt) {
            /* Display Help */
            case 'h':
                print_help (argv [0]);
                exit (1);
                break;

            case 'b':
                devicefile_str = strdup (optarg);
                break;

            case 'v':
                verbose = 1;
                break;

            case 'r':
                rw_fpga = 1;
                break;

            case 'w':
                rw_fpga = 0;
                break;

            case 'n':
                barno_str = strdup (optarg);
                break;

            case 'a':
                address_str = strdup (optarg);
                break;

            case 'd':
                data_str = strdup (optarg);
                break;
            
            case '?':
                fprintf (stderr, "Option not recognized or missing argument\n");
                print_help (argv [0]);
                exit (1);
                break;

            default:
                fprintf (stderr, "Could not parse options\n");
                print_help (argv [0]);
                exit (1);
        }
    }

    /* Device file is mandatory */
    if (devicefile_str == NULL) {
         fprintf (stderr, "--devicefile option not set!\n");
         print_help (argv [0]);
         goto exit;
    } 

    /* BAR number must be set */
    int barno = 0;
    if (barno_str == NULL) {
         fprintf (stderr, "--barno option not set!\n");
         print_help (argv [0]);
         goto exit;
    } 
    else {
        barno = strtoul (barno_str, NULL, 10);

        if (barno != 0 && barno != 2 && barno != 4) {
            fprintf (stderr, "Invalid option for BAR number!\n");
            print_help (argv [0]);
            goto exit;
        }
    }

    if (rw_fpga != 0 && rw_fpga != 1) {
       fprintf (stderr, "Neither --read or --write was set!\n");
       print_help (argv [0]);
       goto exit;
    }

    /* If read access, address must be set */
    if (rw_fpga == 1 && address_str == NULL) {
         fprintf (stderr, "--read_fpga is set but no --address!\n");
         print_help (argv [0]);
         goto exit;
    } 

    if (rw_fpga == 0 && (address_str == NULL || data_str == NULL)) {
         fprintf (stderr, "--write_fpga is set but either --address or --data not set!\n");
         print_help (argv [0]);
         goto exit;
    } 

    /* Parse data/address */
    uint64_t address = 0;
    if (address_str != NULL) {
        if (sscanf (address_str, "%"PRIx64, &address) != 1) {
             fprintf (stderr, "--address format is invalid!\n");
             print_help (argv [0]);
             goto exit;
        }
        if (verbose) {
            fprintf (stdout, "Address = 0x%08X\n", address);
        }
    } 

    uint32_t data = 0;
    if (data_str != NULL) {
        if (sscanf (data_str, "%"PRIx32, &data) != 1) {
             fprintf (stderr, "--data format is invalid!\n");
             print_help (argv [0]);
             goto exit;
        }
        if (verbose) {
            fprintf (stdout, "Data = 0x%08X\n", data);
        }
    }

    /* Open device */
    int err = pd_open (0, dev, devicefile_str);
    if (err != 0) {
         fprintf (stderr, "Could not open device %s, error = %d\n", devicefile_str, err);
         exit (1);
         goto exit;
    }

    /* Map BARs */
    bar0 = pd_mapBAR (dev, 0);
    if (bar0 == NULL) {
         fprintf (stderr, "Could not map BAR 0\n");
         exit (1);
         goto exit_close;
    }

    bar2 = pd_mapBAR (dev, 2);
    if (bar2 == NULL) {
         fprintf (stderr, "Could not map BAR 2\n");
         goto exit_unmap_bar0;
    }

    bar4 = pd_mapBAR (dev, 4);
    if (bar4 == NULL) {
         fprintf (stderr, "Could not map BAR 4\n");
         goto exit_unmap_bar2;
    }

    if (verbose) {
        fprintf (stdout, "BAR 0 host address = %p\n", bar0);
        fprintf (stdout, "BAR 2 host address = %p\n", bar2);
        fprintf (stdout, "BAR 4 host address = %p\n", bar4);
    }

    /* Get BAR sizes */
    bar0_size = pd_getBARsize (dev, 0);
    if (bar0_size == -1) {
         fprintf (stderr, "Could not get BAR 0 size\n");
         goto exit_unmap_bar4;
    }

    bar2_size = pd_getBARsize (dev, 2);
    if (bar2_size == -1) {
         fprintf (stderr, "Could not get BAR 2 size\n");
         goto exit_unmap_bar4;
    }

    bar4_size = pd_getBARsize (dev, 4);
    if (bar4_size == -1) {
         fprintf (stderr, "Could not get BAR 4 size\n");
         goto exit_unmap_bar4;
    }

    if (verbose) {
        fprintf (stdout, "BAR 0 size = %u bytes\n", bar0_size);
        fprintf (stdout, "BAR 2 size = %u bytes\n", bar2_size);
        fprintf (stdout, "BAR 4 size = %u bytes\n", bar4_size);
    }

    /* Read/Write to BAR */
    int pg_num = 0;
    uint64_t pg_offs = 0;
    switch (barno) {
        case 0:
            BAR0_RW(bar0, address, &data, rw_fpga);
            if (verbose) {
                fprintf (stdout, "%s from BAR0, data = 0x%08X, addr = 0x%08X\n", 
                    (rw_fpga)? "Reading":"Writing", data, address);
            }
            break;
        case 2:
            pg_num = PCIE_ADDR_SDRAM_PG (address);
            pg_offs = PCIE_ADDR_SDRAM_PG_OFFS (address);
            SET_SDRAM_PG (bar0, pg_num); 
            BAR2_RW(bar2, pg_offs, &data, rw_fpga);
            if (verbose) {
                fprintf (stdout, "%s from BAR2, data = 0x%08X, addr = 0x%08X, page = %d\n", 
                    (rw_fpga)? "Reading":"Writing", data, address, pg_num);
            }
            break;
        case 4:
            pg_num = PCIE_ADDR_WB_PG (address);
            pg_offs = PCIE_ADDR_WB_PG_OFFS (address);
            SET_WB_PG (bar0, pg_num);
            BAR4_RW(bar4, pg_offs, &data, rw_fpga);
            if (verbose) {
                fprintf (stdout, "%s from BAR4, data = 0x%08X, addr = 0x%08X, page = %d\n", 
                    (rw_fpga)? "Reading":"Writing", data, address, pg_num);
            }
            break;

        default:
            fprintf (stderr, "Invalid BAR number, %d\n", barno);
            goto exit_unmap_bar4;
    }

    /* Output Reading value only */
    if (rw_fpga) {
        fprintf (stdout, "0x%08X\n", data);
    }

exit_unmap_bar4:
    pd_unmapBAR (dev, 4, bar4);
exit_unmap_bar2:
    pd_unmapBAR (dev, 2, bar2);
exit_unmap_bar0:
    pd_unmapBAR (dev, 0, bar0);
exit_close:
    pd_close (dev);
exit:
    free (devicefile_str);
    free (barno_str);
    free (address_str);
    free (data_str);

    return 0;
}
