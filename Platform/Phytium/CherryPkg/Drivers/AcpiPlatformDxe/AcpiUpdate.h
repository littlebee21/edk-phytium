/**
  Header file of AcpiUpdate.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  _ACPI_UPDATE_H_
#define  _ACPI_UPDATE_H_

#include <ArmPlatform.h>
#include <PiDxe.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/AcpiAml.h>

#include <Library/AmlLib/AmlLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>

#define ACPI_DEBUG

#ifdef ACPI_DEBUG
#define DBG(arg...) DEBUG((DEBUG_ERROR,## arg))
#else
#define DBG(arg...)
#endif

#define DSDT_SIGNATURE                  0x54445344

// Differs from Juno, we have another affinity level beyond cluster and core
// 0x20000 is only for socket 0
#define PLATFORM_GET_MPID(ClusterId, CoreId)   (((ClusterId) << 8) | (CoreId))
#define EFI_ACPI_MAX_NUM_TABLES         20
#define CMA_SIZE                        (64 * 1024 * 1024)

/**
  Update acpi tables.

  @param[in,out]  TableHeader      A pointer to EFI_ACPI_COMMON_HEADER.

  @retval    EFI_SUCCESS    Success.
**/
EFI_STATUS
UpdateAcpiTable (
  IN OUT EFI_ACPI_COMMON_HEADER      *TableHeader
  );

#endif
