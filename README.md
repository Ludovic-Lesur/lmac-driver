# Description

This repository contains **Light Medium Access Control (LMAC)** driver.

# Dependencies

The driver relies on:

* An external `types.h` header file defining the **standard C types** of the targeted MCU.
* The **embedded utility functions** defined in the [embedded-utils](https://github.com/Ludovic-Lesur/embedded-utils) repository.

Here is the versions compatibility table:

| **lmac-driver** | **embedded-utils** |
|:---:|:---:|
| [sw3.1](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw3.1) | >= [sw5.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw5.0) |
| [sw3.0](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw3.0) | >= [sw5.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw5.0) |
| [sw2.1](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw2.1) | >= [sw5.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw5.0) |
| [sw2.0](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw2.0) | >= [sw1.3](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw1.3) |
| [sw1.0](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw1.0) | >= [sw1.3](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw1.3) |

# Frame structure

The protocol is designed to **transfer 7-bits data between nodes on an RS485 bus**.

| B0 | B1 | B2 ... B(n) | B(n+1) | B(n+2) | B(n+3) |
|:---:|:---:|:---:|:---:|:---:|:---:|
| Destination @ \| 0x80 | Source @ | DATA | CKH | CKL | 0x7F |

The first byte of the frame is the **destination address** of the packet. It must have the **MSB set to 1** in order to be recognized as an address. All other bytes must have the **MSB set to 0**.

The second byte is the **source address**, in other words the address of the node which is transmitting the packet. For a slave node, this field is used to know at which address the response has to be sent. For the master node, it is used to check that the response comes from the expected slave.

Next bytes are the **data**.

Then there is a **16-bits checksum** composed of the `CKH` and `CKL` fields. It is computed over all the previous bytes using the Fletcher algorithm.

The frame finally ends with a **specific marker** `0x7F` which must not be used in any other byte.

# Node address range

Because of the MSB constraint and the specific end marker, node addresses are limited to the `0x00 - 0x7E` range (127 nodes).

# Compilation flags

| **Flag name** | **Value** | **Description** |
|:---:|:---:|:---:|
| `LMAC_DRIVER_DISABLE_FLAGS_FILE` | `defined` / `undefined` | Disable the `lmac_driver_flags.h` header file inclusion when compilation flags are given in the project settings or by command line. |
| `LMAC_DRIVER_DISABLE` | `defined` / `undefined` | Disable the LMAC driver. |
| `LMAC_DRIVER_HW_INTERFACE_ERROR_BASE_LAST` | `<value>` | Last error base of the low level bus driver. |
| `LMAC_DRIVER_NVM_ERROR_BASE_LAST` | `<value>` | Last error base of the low level NVM driver. |
| `LMAC_DRIVER_MODE_MASTER` | `defined` / `undefined` | Enable master operating mode. |
| `LMAC_DRIVER_MODE_SLAVE` | `defined` / `undefined` | Enable slave operating mode. |
| `LMAC_DRIVER_BUFFER_SIZE` | `<value>` | Size of the internal TX and RX buffers in bytes. |

# Build

A static library can be compiled by command line with `cmake`.

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="<toolchain_file_path>" \
      -DTOOLCHAIN_PATH="<arm-none-eabi-gcc_path>" \
      -DTYPES_PATH="<types_file_path>" \
      -DEMBEDDED_UTILS_PATH="<embedded-utils_path>" \
      -DLMAC_DRIVER_HW_INTERFACE_ERROR_BASE_LAST=0 \
      -DLMAC_DRIVER_NVM_ERROR_BASE_LAST=0 \
      -DLMAC_DRIVER_MODE_MASTER=ON \
      -DLMAC_DRIVER_MODE_SLAVE=OFF \
      -DLMAC_DRIVER_BUFFER_SIZE=64 \
      -G "Unix Makefiles" ..
make all
```
