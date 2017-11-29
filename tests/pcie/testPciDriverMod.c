/*******************************************************************
   This is a test program for the IOctl interface of the
 * pciDriver.
 *
 * $Revision: 1.3 $
 * $Date: 2006-11-17 18:49:01 $
 *
 *******************************************************************/

/*******************************************************************
 * Change History:
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/10/16 16:56:09  marcus
 * Added nice comment at the start.
 *
 *******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "driver/pciDriver.h"

/* defines */
#define MAX_KBUF 14
//#define BIGBUFSIZE (512*1024*1024)
#define BIGBUFSIZE (1024*1024)

/* prototypes */
void testMmapMode(int handle);
void testMmapArea(int handle);
void testKbuf(int handle);
void testUmap(int handle);
void testPCIinfo(int handle);
void testPCIconfig(int handle);
void testPCImmap(int handle);
void testKmmap(int handle);

/* Big buffer used by the Umap test */
char bigbuffer[ BIGBUFSIZE ];

/* Global error counter */
int __err = 0;

/* Main Program */
int main(int argc, char *argv[]) {
    int handle;
    char temp[50];
    int i,ret;
    int dev_number;

    if (argc < 2) {
        printf(" Missing device number\n");
        return 0;
    }

    dev_number = strtol(argv[1], NULL, 10);
    printf(" Testing OPEN DEVICE...");
    sprintf( temp, "/dev/fpga-%d", dev_number );
    handle = open( temp, O_RDWR );
 
    if (handle < 0) {
        printf( "FAILED\n" );
        __err++;
        goto exit;
    }
 
    printf(" PASSED!\n");
    //printf(" Handle: %x\n", handle );
 
    testMmapMode(handle);
    testMmapArea(handle);
 
    testPCIinfo(handle);
    testPCIconfig(handle);
 
    testPCImmap(handle);
 
    testKbuf(handle);
    testKmmap(handle);
    testUmap(handle);
 
 
exit:
    /* Check tthe global error counter */
    if (__err == 0) {
        printf ("\n-------------------------------------\n");
        printf (  "|        All tests PASSED!          |\n");
        printf (  "-------------------------------------\n");
    }
    else {
        printf ("\n-------------------------------------\n");
        printf (  "|%d        tests FAILED!            |\n", __err);
        printf (  "-------------------------------------\n");
    }
 
    printf("\n");
    close(handle);
}

/* Test Memory Map ioctl ONLY (mmap is tested separately) */
void testMmapMode(int handle) {
    int ret;

    printf(" Testing PCIDRIVER_IOC_MMAP_MODE..." );

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_MODE, PCIDRIVER_MMAP_PCI );
    if (ret)
        goto mmap_mode_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_MODE, PCIDRIVER_MMAP_KMEM );
    if (ret)
        goto mmap_mode_fail;

    printf(" PASSED!\n");
    return;

mmap_mode_fail:
    printf(" FAILED!\n");
    __err++;
    return;
}

/* Test Memory Map Area Select ioctl ONLY (mmap is tested separately) */
void testMmapArea(int handle) {
    int ret;

    printf(" Testing PCIDRIVER_IOC_MMAP_AREA..." );

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR0 );
    if (ret)
        goto mmap_area_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR1 );
    if (ret)
        goto mmap_area_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR2 );
    if (ret)
        goto mmap_area_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR3 );
    if (ret)
        goto mmap_area_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR4 );
    if (ret)
        goto mmap_area_fail;

    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR5 );
    if (ret)
        goto mmap_area_fail;

    printf(" PASSED!\n");
    return;

mmap_area_fail:
    printf(" FAILED!\n");
    __err++;
    return;
}

/* Test Kernel buffer IOctls (ALLOC,FREE,SYNC) */
void testKbuf(int handle) {
    kmem_handle_t kh[MAX_KBUF];
    kmem_sync_t ks;
    int i,s,ret,max;

    printf(" Testing PCIDRIVER_IOC_KMEM_ALLOC...\n");

    for(i=0,s=1024;i<MAX_KBUF;i++,s*=2) {
        kh[i].size = s;

        printf( "   alloc size %7d : ", s );
        ret = ioctl(handle, PCIDRIVER_IOC_KMEM_ALLOC, &kh[i] );
        if (ret == 0)
            printf(" PASSED!\n");
        else {
            printf(" FAILED (maybe size is just too big)!\n");
            //__err++;
            break;
        }
    }
    max = i;

    printf(" Testing PCIDRIVER_IOC_KMEM_SYNC...\n");

    ks.handle = kh[max-1];
    ks.dir = PCIDRIVER_DMA_TODEVICE;
    ret = ioctl(handle, PCIDRIVER_IOC_KMEM_SYNC, &ks );
    printf("   Setting KMEM SYNC to write mode...");

    if (ret == 0) {
        //printf("W:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("W:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    ks.dir = PCIDRIVER_DMA_FROMDEVICE;
    ret = ioctl(handle, PCIDRIVER_IOC_KMEM_SYNC, &ks );
    printf("   Setting KMEM SYNC to read mode...");

    if (ret == 0) {
        //printf("W:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("W:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    ks.dir = PCIDRIVER_DMA_BIDIRECTIONAL;
    ret = ioctl(handle, PCIDRIVER_IOC_KMEM_SYNC, &ks );
    printf("   Setting KMEM SYNC to read/write mode...");

    if (ret == 0) {
        //printf("W:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("W:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    int err = 0;

    printf(" Testing PCIDRIVER_IOC_KMEM_FREE...");
    for(i=0;i<max;i++) {
        ret = ioctl(handle, PCIDRIVER_IOC_KMEM_FREE, &kh[i] );
        if (ret == 0) {
            //printf("W:ok ");
            //printf(" PASSED!\n");
        }
        else {
            //printf("W:fail ");
            err++;
            //printf(" FAILED!\n");
        }
    }

    if (err == 0) {
        printf(" PASSED!\n");
    }
    else {
        printf(" FAILED!\n");
        __err++;
    }
}

/* Test Usem Memory IOctls (SGMAP,SGGET,SGSYNC,SGUNMAP) */
void testUmap(int handle) {
    umem_handle_t uh;
    umem_sglist_t sgl;
    int i,s,ret,max;
    unsigned int pagesize;

    pagesize = getpagesize();

    /*
       printf(" Locking User memory..." );
       ret = mlock( bigbuffer, BIGBUFSIZE );
       if (ret != 0)
       goto test_umap_fail;
       else
       printf("ok.\n");
       */
    printf(" Testing PCIDRIVER_IOC_UMEM_SGMAP ...");

    uh.vma = (unsigned long)bigbuffer;
    uh.size = BIGBUFSIZE;

    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SGMAP, &uh );
    if (ret != 0) {
        printf(" FAILED!\n");
        __err++;
        goto test_umap_sgmap_fail;
    }
    else {
        //printf("ok.\n");
        printf(" PASSED!\n");
    }
    //printf("  handle_id: %d\n", uh.handle_id );

    //printf("Press any key...\n");
    //getchar();

    printf(" Testing PCIDRIVER_IOC_UMEM_SGGET ...");
    sgl.handle_id = uh.handle_id;
    sgl.type = PCIDRIVER_SG_MERGED;
    sgl.nents = (BIGBUFSIZE / pagesize) + 1;
    sgl.sg = malloc( sgl.nents*sizeof(umem_sgentry_t) );

    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SGGET, &sgl );
    if (ret != 0) {
        printf(" FAILED!\n");
        __err++;
        goto test_umap_sgget_fail;
    }
    else {
        //printf("ok.\n");
        printf(" PASSED!\n");
    }

    //printf("  Scatter/Gather list of the bigbuffer:\n");
    //for(i=0;i<sgl.nents;i++) {
    //    printf("  %d: %lx - %lx\n",i, sgl.sg[i].addr, sgl.sg[i].size );
    //}

    printf(" Testing PCIDRIVER_IOC_UMEM_SYNC ...\n");

    printf ("   Setting UMEM SYNC to write mode...");
    uh.dir = PCIDRIVER_DMA_TODEVICE;
    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SYNC, &uh );
    if (ret == 0) {
        //printf("W:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("W:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    printf ("   Setting UMEM SYNC to read mode...");
    uh.dir = PCIDRIVER_DMA_FROMDEVICE;
    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SYNC, &uh );
    if (ret == 0) {
        //printf("R:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("R:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    printf ("   Setting UMEM SYNC to read/write mode...");
    uh.dir = PCIDRIVER_DMA_BIDIRECTIONAL;
    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SYNC, &uh );
    if (ret == 0) {
        //printf("RW:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("RW:fail ");
        printf(" FAILED!\n");
        __err++;
    }

test_umap_sgget_fail:
    printf(" Testing PCIDRIVER_IOC_UMEM_SGUNMAP ...");
    ret = ioctl(handle, PCIDRIVER_IOC_UMEM_SGUNMAP, &uh );
    if (ret == 0) {
        //printf("W:ok ");
        printf(" PASSED!\n");
    }
    else {
        //printf("W:fail ");
        printf(" FAILED!\n");
        __err++;
    }

    free( sgl.sg );
    //	munlock( bigbuffer, BIGBUFSIZE );
    return;

test_umap_sgmap_fail:
    //	munlock( bigbuffer, BIGBUFSIZE );
test_umap_fail:
    //printf( " failed! (%d)\n", ret );
    printf (" FAILED!\n");
    __err++;
    return;
}

void testPCIinfo(int handle)
{
    int ret,i;
    pci_board_info info;

    printf(" Testing PCIDRIVER_IOC_PCI_INFO... ");
    ret = ioctl( handle, PCIDRIVER_IOC_PCI_INFO, &info );
    if (ret != 0) {
        printf( " FAILED! (%d)\n", ret );
        __err++;
    }
    else {
        printf(" PASSED!\n");
        //printf("  Vendor ID: %x\n", info.vendor_id );
        //printf("  Device ID: %x\n", info.device_id );
        //printf("  Interrupt Pin: %i\n", info.interrupt_pin );
        //printf("  Interrupt Line: %i\n", info.interrupt_line );
        //for(i=0;i<6;i++) {
        //	printf( "  BAR: %d  - Start: %x Length: %x\n", i, info.bar_start[i], info.bar_length[i] );
        //}
    }
    return;
}

void testPCIconfig(int handle)
{
    int i,j,ret;
    int err = 0;
    pci_cfg_cmd cmd;

    printf(" Testing PCI CONFIG... \n");
    printf("   Reading PCI config area in byte mode ...");
    for(i=0;i<32;i++) {
        //printf("   %03d: ", i*8 );
        for(j=0;j<8;j++) {
            cmd.addr = (i*8)+j;
            cmd.size = PCIDRIVER_PCI_CFG_SZ_BYTE;
            ret = ioctl( handle, PCIDRIVER_IOC_PCI_CFG_RD, &cmd );
            if (ret != 0) {
                //printf( " FAILED!\n" );
                err++;
            }
        }
    }

    if (err != 0) {
        printf( " FAILED!\n" );
        __err++;
        goto test_failed;
    }
    else {
        printf( " PASSED!\n" );
    }

    err = 0;

    printf("   Reading PCI config area in word mode ...");
    for(i=0;i<32;i++) {
        //printf("   %03d: ", i*8 );
        for(j=0;j<8;j+=2) {
            cmd.addr = (i*8)+j;
            cmd.size = PCIDRIVER_PCI_CFG_SZ_WORD;
            ret = ioctl( handle, PCIDRIVER_IOC_PCI_CFG_RD, &cmd );
            if (ret != 0) {
                //printf( " FAILED!\n" );
                err++;
            }
        }
    }

    if (err != 0) {
        printf( " FAILED!\n" );
        __err++;
        goto test_failed;
    }
    else {
        printf( " PASSED!\n" );
    }

    err = 0;

    printf("   Reading PCI config area in double-word mode ...");
    for(i=0;i<32;i++) {
        //printf("   %03d: ", i*8 );
        for(j=0;j<8;j+=4) {
            cmd.addr = (i*8)+j;
            cmd.size = PCIDRIVER_PCI_CFG_SZ_DWORD;
            ret = ioctl( handle, PCIDRIVER_IOC_PCI_CFG_RD, &cmd );
            if (ret != 0) {
                //printf( " FAILED!\n" );
                err++;
            }
        }
    }

    if (err != 0) {
        printf( " FAILED!\n" );
        __err++;
        goto test_failed;
    }
    else {
        printf( " PASSED!\n" );
    }

test_failed:
    return;
}

void testPCImmap(int handle)
{
    int ret,i;
    pci_board_info info;
    unsigned int * bar[6];

    printf(" Testing PCI mmap... \n");
    /* Get BAR info (interested in size)  */
    ret = ioctl( handle, PCIDRIVER_IOC_PCI_INFO, &info );
    printf("   Reading PCI info...");

    if (ret != 0) {
        //printf( "  Error getting PCI info (%d)\n", ret );
        printf(" FAILED!\n");
        __err++;
        return;
    }

    printf(" PASSED!\n");

    /* Set the mmap mode to PCI */
    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_MODE, PCIDRIVER_MMAP_PCI );
    printf("   Setting mmap mode...");

    if (ret != 0) {
        //printf( "  Error setting mmap mode (%d)\n", ret );
        printf(" FAILED!\n");
        __err++;
        return;
    }

    printf(" PASSED!\n");

    /* mmap the regions */
    for(i=0;i<6;i++) {
        // invalid BAR, go next
        if ((info.bar_start[i] == 0) || (info.bar_length[i] == 0)) {
            bar[i] = NULL;
            continue;
        }

        printf("   Setting mmap area...");
        ret = ioctl( handle, PCIDRIVER_IOC_MMAP_AREA, PCIDRIVER_BAR0+i );
        if (ret != 0) {
            //printf( "  Error setting mmap area %d (%d)\n", i, errno );
            printf(" FAILED!\n");
            __err++;
            return;
        }

        printf(" PASSED!\n");

        printf("   MMAP'ing BAR%d...", i);
        bar[i] = mmap( 0, info.bar_length[i], PROT_WRITE | PROT_READ, MAP_SHARED, handle, 0 );

        if ((bar[i] == MAP_FAILED) || (bar[i] == NULL)) {
            //printf( "  Error mapping BAR%d (%d - %s) \n", i, errno, strerror( errno ) );
            printf(" FAILED!\n");
            __err++;
            goto test_failed;
        }

        printf(" PASSED!\n");
    }


    /* Print the info */
    //for(i=0;i<6;i++) {
    //    if (bar[i] != NULL)
    //        printf( "  BAR: %d  - Start: %x Length: %x VMA: %x\n", i, info.bar_start[i], info.bar_length[i], bar[i] );
    //}

#if 0
    for(i=0;i<4096/4;i++) {
        printf("BAR0[%x]: %x\n", i*4, bar[0][i]);
    }
    getchar();
    for(i=0;i<4096/4;i++) {
        printf("BAR1[%d]: %x\n", i, bar[1][i]);
    }
    getchar();
    for(i=0;i<(4096/4);i++) {
        printf("BAR2[%d]: %x\n", i, bar[2][i]);
    }
    for(i=0;i<(4096/4);i++) {
        printf("BAR3[%d]: %x\n", i, bar[3][i]);
    }
#endif

    //printf("Press any key...\n");
    //getchar();

test_failed:

    /*  munmap the regions */
    for(i=0;i<6;i++) {
        if (bar[i] != NULL)
            munmap( bar[i], info.bar_length[i] );
    }

    return;
}

void testKmmap(int handle)
{
    kmem_handle_t kh[MAX_KBUF];
    void * kbufs[MAX_KBUF];
    int i,s,ret,max;

    printf(" Testing Kernel Buffer mmap... \n");

    printf("   Setting MMAP mode to KMEM...");
    /* Set the mmap mode to KMEM */
    ret = ioctl( handle, PCIDRIVER_IOC_MMAP_MODE, PCIDRIVER_MMAP_KMEM );
    if (ret != 0) {
        //printf( "  Error setting mmap mode (%d)\n", ret );
        printf(" FAILED!\n");
        __err++;
        return;
    }

    printf(" PASSED!\n");

    /* Allocate and mmap Kernel buffers */
    for(i=0,s=1024;i<MAX_KBUF;i++,s*=2) {
        kh[i].size = s;

        printf( "     Allocing size %7d :", s );
        ret = ioctl(handle, PCIDRIVER_IOC_KMEM_ALLOC, &kh[i] );
        if (ret == 0)
            printf(" PASSED!\n");
        else {
            printf(" FAILED (maybe size is just too big)!\n");
            // __err++;
            break;
        }

        printf( "     MMAPing size %8d :", s );
        kbufs[i] = mmap( 0, s, PROT_WRITE | PROT_READ, MAP_SHARED, handle, 0 );
        if ((kbufs[i] == MAP_FAILED) || (kbufs[i] == NULL)) {
            kbufs[i] = NULL;
            //printf( "  Error mapping kbuffer %i (%d - %s) \n", kh[i].handle_id, errno, strerror( errno ) );
            printf(" FAILED!\n");
            __err++;
        }
        else {
            //printf("mapped\n");
            printf(" PASSED!\n");
        }
    }
    max = i;

    /* print mapping info */
    //for(i=0;i<max;i++)
    //    printf("  mapped kmem #%i into %lx\n",kh[i].handle_id,kbufs[i]);

    printf( "   Freeing Kernel buffers...\n");
    /* Free kernel buffers */
    for(i=0;i<max;i++) {
        printf( "      Buffer index %d...", i);
        if (kbufs[i] != NULL)
            munmap( kbufs[i], kh[i].size );

        ret = ioctl(handle, PCIDRIVER_IOC_KMEM_FREE, &kh[i] );
        if (ret == 0) {
            //printf("ok ");
            printf(" PASSED!\n");
        }
        else {
            //printf("fail! ");
            printf(" FAILED!\n");
            __err++;
        }
    }

}

