# PCIE driver for FPGA device

## Overview

This driver is based on original driver for the PCIe SG DMA project placed at
the opencores.org. It is mostly in maintenance state. Changes cover mostly
updating driver to work with newer kernels and deleting support for really
old kernels (which obfuscates the code). Also, with present makefiles it's
easier to cross-compile code (e.g. using buildroot).

It consists of two parts:

- kernel driver at `driver/pcie`
- c/c++ library at `lib/pcie`

Tests are located at tests  folder.

Include files are in `include/pcie` folder.

## Install

To compile and install kernel driver:

```bash
make kernel_driver
sudo make kernel_driver_install
```

To compile and install the library:

```bash
make kernel_lib
sudo make kernel_lib_install
```

## ToDo List

- Test interrupt support
- Make HW tests more friendly to causal user, stop them from spewing out
thousand of hex output lines
