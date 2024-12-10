/** @file
  PCI memory configuration 
  Copyright (c) 2017, Linaro Ltd. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PLATFORM_PCIE_H_
#define _PLATFORM_PCIE_H_

#define PCI_HOST_MAX_COUNT        1
#define PCI_SEG_CONFIG_BASE       0x40000000

#define FVH_SIGNATURE             SIGNATURE_32('_', 'F', 'V', 'H')
#define SPI_FLASH_BASE_ADDRESS    0x0
#define PBF_HEADER_SIZE           0x200000 
#define PBF_HEADER1_SIZE          0x30000 
#define BIOS_OFFSET_ADDRESS       PBF_HEADER_SIZE + PBF_HEADER1_SIZE
#define BIOS_START_ADDRESS        SPI_FLASH_BASE_ADDRESS + BIOS_OFFSET_ADDRESS

#endif
