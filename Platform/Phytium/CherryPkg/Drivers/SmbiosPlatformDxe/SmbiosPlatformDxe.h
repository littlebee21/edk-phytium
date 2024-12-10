/** @file
  Phytium E2000 SMBIOS function description.

  Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef SMBIOSPLATFORMDXE_H_
#define SMBIOSPLATFORMDXE_H_

#include <ArmPlatform.h>
#include <OEMSvc.h>
#include <SmbiosData.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SmBios.h>

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/EcLib.h>
#include <Library/IoLib.h>
#include <Library/I2CLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ParameterTable.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/Smbios.h>

#define PM_GPIO  0
#define PM_EC    1
#define PM_SE    2
#define SPD_NUM 256
#define DDR3_TYPE                 0xB
#define DDR4_TYPE                 0xC
#define LPDDR4_TYPE               0x10
#define DDR5_TYPE                 0x12
#define SERIAL_LEN                                                             \
  10 // this must be less than the buffer len allocated in the type1 structure
#define PCI_MAX_DEVICE 31
#define PCI_MAX_FUNC 7

typedef enum {
  FieldWidthUINT8,
  FieldWidthUINT16,
  FieldWidthUINT32
} PCIE_CAPREG_FIELD_WIDTH;

typedef enum {
  PcieExplainTypeCommon,
  PcieExplainTypeDevice,
  PcieExplainTypeLink,
  PcieExplainTypeSlot,
  PcieExplainTypeRoot,
  PcieExplainTypeMax
} PCIE_EXPLAIN_TYPE;

typedef enum {
  PciDevice,
  PciP2pBridge,
  PciCardBusBridge,
  PciUndefined
} PCI_HEADER_TYPE;

typedef struct {
  UINTN CpuTypeId;
  CHAR8 *CpuTypeName;
  CHAR8 *CpuFamilyName;
  UINT8 CpuVoltage;
} CPU_TYPE;

typedef union {
  PCI_DEVICE_HEADER_TYPE_REGION Device;
  PCI_BRIDGE_CONTROL_REGISTER Bridge;
  PCI_CARDBUS_CONTROL_REGISTER CardBus;
} NON_COMMON_UNION;

typedef struct {
  PCI_DEVICE_INDEPENDENT_REGION Common;
  NON_COMMON_UNION NonCommon;
  UINT32 Data[48];
} PCI_CONFIG_SPACE;

typedef struct {
  UINT64 Base;
  UINT64 Top;
  UINT64 Size;
} MEM_BLOCK;



typedef struct {
  UINTN  Size;//MB
  UINT8  FormFactor;//enum
  UINT8  ChannelNumer;//0 or 1
  UINT8  DimmNumer;//0 or 1

  UINT8  MemoryType;
  UINT8  RankNumber;//Attributes
  UINT16 DeviceSet;//0

  //CHAR8  Manufacturer[10];//send id
  UINT32  SerialNumber;
  //CHAR8  AssertTag;
  //CHAR8  PartNumber;
  UINT32 ExtendedSize;

  UINT16 Speed;//MT
  UINT16 ConfiguredMemorySpeed;//MT

  UINT16 MinVoltage;//mv
  UINT16 MaxVoltage;//mv

  UINT16 ConfiguredVoltage;//mv
  //CHAR8  FirmwareVersion;
  UINT64 ModuleManufacturerID;

  UINT16 ModuleProductID;
  UINT16 MemorSubsystemControllerManufacturerID;

  UINT16 MemorSubsystemControllerProductID;
  UINT16 Manufacturer;//send id

  UINT8  MemoryTechnology;
  UINT8  Reserve[3];

  CHAR8  PartNumber[21];
  UINT8  Reserve2[3];
}SPD_INFO;


typedef struct {
  UINT8     DevicesNumber;//actual ddr number
  UINT8     DimmCount;//toal dimm count
  UINT16    Reserve;
  SPD_INFO  SpdInfo[4];//spd information
}MEMORY_DEVICES_INFO;


typedef struct {
  UINT8  CharacteristicsUnknown;
  UINT8  Provides50Volts;
  UINT8  Provides33Volts;
  UINT8  SharedSlot;
  UINT8  PcCard16Supported;
  UINT8  CardBusSupported;
  UINT8  ZoomVideoSupported;
  UINT8  ModemRingResumeSupported;
}SLOTCHARACTERISTICS1;

typedef struct {
  UINT8  PmeSignalSupported;
  UINT8  HotPlugDevicesSupported;
  UINT8  SmbusSignalSupported;
  UINT8  BifurcationSupported;
  UINT8  Reserved[4];///< Set to 0.
}SLOTCHARACTERISTICS2;

typedef struct {
  UINT8  Enable;
  UINT8  GenNumber;
  UINT8  Xnumber;
  UINT8  SlotType;
  UINT8  SlotDataBusWidth;

  UINT8  CurrentUsage;
  UINT8  SlotLength;
  UINT16 Res;
  SLOTCHARACTERISTICS1  SlotChara1;
  SLOTCHARACTERISTICS2  SlotChara2;

  UINT16 SlotID;
  UINT16 SegmentGroupNum;

  UINT8  BusNum;
  UINT8  DevFuncNum;
  UINT8  DataBusWidth;
  UINT8  PeerGroupingCount;
}PCIE_INFO;
typedef struct {
  UINT8      SlotsNumber;
  UINT8      Res[3];
  PCIE_INFO  PcieInfo[6];
}SYSTEM_SLOTS;

typedef struct{
  UINT32 PcieMagic;
  UINT32 Version;
  UINT32 Size;
  UINT32 Res[2];
  UINT32 IndependentTree;
  UINT8 Res3[16];
  UINT32 Pcie0Controller0Conf;
  UINT32 Pcie0Controller1Conf;
  UINT32 Res4[2];
  UINT32 Pcie0Controller0Equalization;
  UINT32 Pcie0Controller1Equalization;
  UINT8 Res5[80];
  UINT32 Pcie1Controller0Conf;
  UINT32 Pcie1Controller1Conf;
  UINT32 Pcie1Controller2Conf;
  UINT32 Pcie1Controller3Conf;
  UINT32 Pcie1Controller0Equalization;
  UINT32 Pcie1Controller1Equalization;
  UINT32 Pcie1Controller2Equalization;
  UINT32 Pcie1Controller3Equalization;
}PCIE_CONFIG;

#endif
