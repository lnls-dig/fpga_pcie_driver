###############################################################
#
# Change History:
#
# Revision 2.0 2013/03/28 abyszuk
# Moved to OHWR repo
#
# Revision 1.4  2006/11/17 18:56:44  marcus
# Added testCompatInterface.
#
# Revision 1.3  2006/10/30 19:40:32  marcus
# Added testPCIeBoard.
#
# Revision 1.2  2006/10/16 16:57:17  marcus
# Added testCinterface.
#
# Revision 1.1  2006/10/13 17:18:35  marcus
# Implemented and tested most of C++ interface.
#
###############################################################

# Makefile for the pciDriver test programs
CC = gcc
CXX = g++

CFLAGS += -O2 -g
CXXFLAGS += -O2 -g

INCDIR = ../../include/pcie
LIBDIR = /usr/lib
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -lpcidriver -lpthread
LDFLAGS_BOOST = -lboost_timer -lboost_system

BINDIR = bin

CFLAGS += $(addprefix -I, $(INCDIR))
CXXFLAGS += $(addprefix -I,i $(INCDIR))

BINARIES = testCppInterface \
	testDMA \
	testPciDriver \
	testPciDriverMod \
	testCinterface \
	benchmarkDevice \
        regAccess

###############################################################
# Target definitions

.PHONY: all dirs clean

all: dirs $(BINARIES)

dirs:
	$(Q)mkdir -p $(BINDIR)

# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: %.cpp dirs
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/benchmarkDevice: benchmarkDevice.cpp dirs
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS_BOOST) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/testCinterface: testCinterface.c dirs
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/testPciDriverMod: testPciDriverMod.c dirs
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/testPciDriver: testPciDriver.c dirs
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BINDIR)/regAccess: regAccess.c dirs
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(BINDIR)/testCinterface
	-$(Q)rm -f $(BINDIR)/testPciDriver
