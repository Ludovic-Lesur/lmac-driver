# Description

This repository contains **Light Medium Access Control (LMAC)** driver.

# Dependencies

The driver relies on:

* An external `types.h` header file defining the **standard C types** of the targeted MCU.
* The **embedded utility functions** defined in the [embedded-utils](https://github.com/Ludovic-Lesur/embedded-utils) repository.

Here is the versions compatibility table:

| **lmac-driver** | **embedded-utils** |
|:---:|:---:|
| [sw1.0](https://github.com/Ludovic-Lesur/lmac-driver/releases/tag/sw1.0) | >= [sw1.3](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw1.3) |

# Compilation flags

| **Flag name** | **Value** | **Description** |
|:---:|:---:|:---:|
| `LMAC_DRIVER_DISABLE_FLAGS_FILE` | `defined` / `undefined` | Disable the `lmac_driver_flags.h` header file inclusion when compilation flags are given in the project settings or by command line. |
| `LMAC_DRIVER_DISABLE` | `defined` / `undefined` | Disable the LMAC driver. |
| `LMAC_DRIVER_HW_INTERFACE_ERROR_BASE_LAST` | `<value>` | Last error base of the low level bus driver. |
| `LMAC_DRIVER_NVM_ERROR_BASE_LAST` | `<value>` | Last error base of the low level NVM driver. |
| `LMAC_DRIVER_MODE_MASTER` | `defined` / `undefined` | Enable master operating mode. |
| `LMAC_DRIVER_MODE_SLAVE` | `defined` / `undefined` | Enable slave operating mode. |
| `LMAC_DRIVER_RX_TIMEOUT_SECONDS` | `<value>` | Define the frame reception timeout in seconds (slave mode only). |
