/** @file
Phytium smbios helper header file.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PHYTIUM_SMBIOS_HELEPER_H_
#define PHYTIUM_SMBIOS_HELEPER_H_

#include <Uefi/UefiBaseType.h>

#define MAX_MEMORY_DEVICES_COUNT 32

#pragma pack(1)
//Phytium Smbios Cpu Info
typedef struct {
  UINTN  CpuTypeId;
  CHAR8  *CpuTypeName;
  UINT8  CpuVoltage;
  UINT16 MaxSpeed;
} SmbiosCpuInfo;

typedef struct {
  UINT8    Size;         //GB
  UINT8    TotalWidth;
  UINT8    DataWidth;
  UINT8    DimmType;
  UINT8    MemoryType;
  UINT16   MaxSpeed;
  UINT16   Manufacturer;
  UINT32   SerialNumber;
  CHAR8    PartNumber[31];
  UINT8    RankNumber;
  UINT16   ConfiguredMemoryClockSpeed;
  UINT8    MemoryTechnology;
  UINT16   ModuleManufacturerID;
} MEMORY_DEVICE_INFO;

typedef struct {
  UINT8               MaxDimmCount;
  UINT8               EccType;
  MEMORY_DEVICE_INFO  MemoryDevice[MAX_MEMORY_DEVICES_COUNT];
} PHYTIUM_MEMORY_SMBIOS_INFO;

typedef struct {
  UINT8  MfgIdLSB;
  UINT8  MfgIdMSB;
  CHAR8  *Name;
} SPD_JEDEC_MANUFACTURER;

#pragma pack()

#endif

