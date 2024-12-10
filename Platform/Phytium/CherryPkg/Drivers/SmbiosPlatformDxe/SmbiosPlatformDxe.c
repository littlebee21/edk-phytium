/** @file
  This driver installs SMBIOS information for Phytium E2000.

  Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <ArmPlatform.h>
#include <OEMSvc.h>
#include <Protocol/Smbios.h>
#include <SmbiosData.h>

#include <IndustryStandard/SmBios.h>

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "SmbiosPlatformDxe.h"


PHYTIUM_MEMORY_INFOR      *mMemInfor;
MCU_DIMMS                 *mMcuDimms;
MEMORY_DEVICES_INFO       *mMemoryDevInfo;
SYSTEM_SLOTS              *mSystemSlots;

UINT32 mCoreCountEfficicency;
UINT32 mCoreCountFunc;
UINT32 mCoreFrequency;
CHAR8 mCpuTypeName[20];
CHAR8 mCpuFamilyName[20];
UINT8 mCpuVoltage;
UINT8 mForceSpd;
// SMBIOS tables often reference each other using
// fixed constants, define a list of these constants
// for our hardcoded tables


struct SPD_JEDEC_MANUFACTURER Jep106[] = {
    {    0, 0x10, "NEC"},
    {    0, 0x2c, "Micron"},
    {    0, 0x3d, "Tektronix"},
    {    0, 0x97, "TI"},
    {    0, 0xad, "Hynix"},
    {    0, 0xb3, "IDT"},
    {    0, 0xc1, "Infineon"},
    {    0, 0xce, "Samsung"},
    {    1, 0x94, "Smart"},
    {    1, 0x98, "Kingston"},
    {    2, 0xc8, "Agilent"},
    {    2, 0xfe, "Elpida"},
    {    3, 0x0b, "Nanya"},
    {    4, 0x43, "Ramaxel"},
    {    4, 0xb3, "Inphi"},
    {    5, 0x51, "Qimonda"},
    {    5, 0x57, "AENEON"},
    { 0xFF, 0xFF, ""}
};

#pragma pack()

extern ARM_TYPE0  BiosInfoType0;
extern ARM_TYPE1  SystemInfoType1;
extern ARM_TYPE2 BaseboardInfoType2;
extern ARM_TYPE3 SystemEnclosureType3;
extern ARM_TYPE32 SystemBootType32;
extern ARM_TYPE4_EFFICIENCY ProcessorInfoType4Efficiency;
extern ARM_TYPE4_FUNCTION ProcessorInfoType4Func;
extern ARM_TYPE7_L1DATA_EFFICIENCY L1DataType7Efficiency;
extern ARM_TYPE7_L1INS_EFFICIENCY L1InsType7Efficiency;
extern ARM_TYPE7_L1DATA_FUNCTION L1DataType7Function;
extern ARM_TYPE7_L1INS_FUNCTION L1InsType7Function;
extern ARM_TYPE7_L2_FUNCTION L2Type7Function;
extern ARM_TYPE7_L2_EFFICIENCY L2Type7Efficiency;
extern ARM_TYPE13 BiosLangInfoType13;
extern ARM_TYPE16 PhyMemArrayType16;
extern ARM_TYPE17 MemDevType17;
extern ARM_TYPE19 MemArrayMapAddrType19;

VOID *DefaultCommonTables[]= {
  &BiosInfoType0,
  &SystemInfoType1,
  &BaseboardInfoType2,
  &SystemEnclosureType3,
  &SystemBootType32,
  &L1DataType7Efficiency,
  &L1InsType7Efficiency,
  &L1DataType7Function,
  &L1InsType7Function,
  &L2Type7Function,
  &L2Type7Efficiency,
  &BiosLangInfoType13,
  NULL
};

/**
   Get Memory Manufacturer.

   @param[in]   MemModuleId    ManufacturerID
   @param[out]  Manufacturer    Manufacturer String

**/
VOID
SmbiosGetManufacturer (
  IN  UINT8           MemModuleId,
  OUT CHAR8           *Manufacturer
  )
{
  UINT32                  Index;

  Index = 0;

  AsciiStrCpyS (Manufacturer, (sizeof (CHAR8)) * SMBIOS_STRING_MAX_LENGTH,"Unknown");
  DEBUG((DEBUG_INFO, "%a() L%d MemModuleId = %d \n",__FUNCTION__, __LINE__, MemModuleId));
  while (Jep106[Index].MfgIdLSB != 0xFF && Jep106[Index].MfgIdMSB != 0xFF ) {
    if (Jep106[Index].MfgIdMSB == MemModuleId) {
      AsciiStrCpyS (Manufacturer, (sizeof (CHAR8)) * SMBIOS_STRING_MAX_LENGTH,Jep106[Index].Name);
      break;
    }
    Index ++;
  }
}

/**
   Get Memory Capacity In MB.

   @retval Memory Capacity

**/
UINT64
SmbiosGetMemoryCapacity (
  VOID
  )
{
  UINT32 Index;
  UINT64 Size;

  Size = 0;
  for(Index = 0; Index < mMcuDimms->MemDimmCount; Index++) {
    Size += mMcuDimms->McuDimm[Index].MemSize;
  }
  return Size;
}

/**
   Get Memory Information.
**/
VOID
SmbiosGetMemoryInformation (
  VOID
  )
{
  UINT32          Index;
  EFI_STATUS      Status;
  UINT32          I2CSlaveAddress;
  UINT8           DdrPara[0x100];
  UINT8           Buffer[SPD_NUM * 2];
  UINT8           Temp;
  UINT8           *SlaveAddress;
  I2C_INFO        I2cInfo;
  UINT16          BufferIndex;
  UINT16          PartnumIndex;
  UINT16          Serial;
  UINT16          Manufacturer;
  UINT8           SpdIndex;
  CHAR8           PartNumber[21];
  DDR_CONFIG      *DdrConfigData;

  Status = GetParameterInfo (PM_DDR, DdrPara, sizeof(DdrPara));
  if(!EFI_ERROR(Status)) {
    DdrConfigData = (DDR_CONFIG *)DdrPara;
  } else {
    DEBUG((DEBUG_ERROR, "Get DDR Parameter Fail.\n"));
    while(1);
  }
  mForceSpd = (UINT8)(DdrConfigData->MiscEnable & 0x1);
  DEBUG ((DEBUG_INFO, "mForceSpd:%x, DramType = %x\n",
              mForceSpd, DdrConfigData->DdrSpdInfo.DramType));
  if (mForceSpd == 0) {
    PartnumIndex = 0;
    BufferIndex  = 0;
    SpdIndex     = 0;
    ZeroMem (PartNumber, sizeof (PartNumber));
    ZeroMem (Buffer, sizeof (Buffer));
    SlaveAddress   = PcdGetPtr(PcdDdrI2cAddress);
    I2CSlaveAddress = SlaveAddress[SpdIndex];

    ZeroMem (&I2cInfo, sizeof (I2C_INFO));
    I2cInfo.BusAddress = PcdGet64 (PcdSpdI2cControllerBaseAddress);
    I2cInfo.Speed = PcdGet32 (PcdSpdI2cControllerSpeed);
    I2cInfo.SlaveAddress = I2CSlaveAddress;
    DEBUG ((DEBUG_INFO, "I2CBase:%x, I2CSpeed:%x, I2CSlaveAddress:%x\n",
              I2cInfo.BusAddress, I2cInfo.Speed, I2cInfo.SlaveAddress));
    //
    // Initialize I2C Bus which contain SPD
    //
    I2cInit(&I2cInfo);
    Temp = I2cRead (&I2cInfo, 0, 1, Buffer, 256);
    if (Temp != 0) {
      DEBUG((DEBUG_ERROR, "Read I2C Failed\n"));
    }

    //Get Part Number
    SpdSetpage (&I2cInfo, 1);
    I2cRead (&I2cInfo, 0, 1, &Buffer[256], 256);
    SpdSetpage (&I2cInfo, 0);

    Serial = ((UINT32)Buffer[325] << 24) + ((UINT32)Buffer[326] <<16)
    + ((UINT32)Buffer[327] << 8) + Buffer[328];
    DEBUG ((DEBUG_INFO, "/Serial:0x%x\n", Serial));
    Manufacturer = ((UINT16)Buffer[350] << 8) + Buffer[351];
    for(BufferIndex = 329; BufferIndex < 349 ;BufferIndex++) {
      PartNumber[PartnumIndex] = Buffer[BufferIndex];
      PartnumIndex++;
    }
  }

  mMemoryDevInfo->DimmCount       = mMcuDimms->MemDimmCount;
  mMemoryDevInfo->DevicesNumber   = mMcuDimms->MemDimmCount;
  for (Index = 0; Index < mMcuDimms->MemDimmCount; Index++) {
    mMemoryDevInfo->SpdInfo[Index].Size                 = mMcuDimms->McuDimm[Index].MemSize;
    mMemoryDevInfo->SpdInfo[Index].ChannelNumer         = 1;
    mMemoryDevInfo->SpdInfo[Index].DimmNumer            = 1;
    mMemoryDevInfo->SpdInfo[Index].MinVoltage = 1200;
    mMemoryDevInfo->SpdInfo[Index].MaxVoltage = 1200;
    mMemoryDevInfo->SpdInfo[Index].ConfiguredVoltage = 1200;
    switch (DdrConfigData->DdrSpdInfo.DramType) {
      case DDR4_TYPE:
        mMemoryDevInfo->SpdInfo[Index].MemoryType = MemoryTypeDdr4;
        break;
      case LPDDR4_TYPE:
        mMemoryDevInfo->SpdInfo[Index].MinVoltage = 1100;
        mMemoryDevInfo->SpdInfo[Index].MaxVoltage = 1100;
        mMemoryDevInfo->SpdInfo[Index].ConfiguredVoltage = 1100;
        mMemoryDevInfo->SpdInfo[Index].MemoryType = MemoryTypeLpddr4;
        break;
      case DDR5_TYPE:
        mMemoryDevInfo->SpdInfo[Index].MemoryType = MemoryTypeDdr5;
        break;
      default:
        mMemoryDevInfo->SpdInfo[Index].MemoryType = MemoryTypeUnknown;
        break;
    }

    if (mForceSpd == 0) {
      CopyMem (mMemoryDevInfo->SpdInfo[Index].PartNumber, PartNumber, sizeof (PartNumber));
      mMemoryDevInfo->SpdInfo[Index].SerialNumber = Serial;
      mMemoryDevInfo->SpdInfo[Index].Manufacturer = Manufacturer;
    }
  }
}

/**
   Get Cpu Information.

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
SmbiosGetCpuInfo(
  VOID
  )
{
  EFI_HOB_GUID_TYPE       *GuidHob;
  PHYTIUM_CPU_INFO        *CpuInfo;
  UINTN                   CpuMapCount;
  UINTN                   CpuMapIndex;
  UINT32                  CoreCountEfficicency;
  UINT32                  CoreCountFunc;

  CoreCountEfficicency = 0;
  CoreCountFunc = 0;
  mCpuVoltage = 8;

  GuidHob = GetFirstGuidHob (&gPlatformCpuInforGuid);
  if (GuidHob == NULL) {
  DEBUG((DEBUG_ERROR,"%a(),Get cpu info hob failed!\n",__FUNCTION__));
  return EFI_UNSUPPORTED;
  }

  CpuInfo = (PHYTIUM_CPU_INFO *) GET_GUID_HOB_DATA(GuidHob);
  CpuMapCount = CpuInfo->CpuMapInfo.CpuMapCount;
  DEBUG((DEBUG_INFO, "cpumap=%x, cpufreq=%d, cpuversion=%d\n", CpuInfo->CpuMapInfo.CpuMap[0], CpuInfo->CpuCoreInfo.CpuFreq, CpuInfo->CupVersion));


  for(CpuMapIndex = 0; CpuMapIndex < CpuMapCount; CpuMapIndex++) {
    CoreCountEfficicency += ((CpuInfo->CpuMapInfo.CpuMap[CpuMapIndex]) & 0x1);
    CoreCountEfficicency += ((CpuInfo->CpuMapInfo.CpuMap[CpuMapIndex] >> 4) & 0x1);
    CoreCountFunc += ((CpuInfo->CpuMapInfo.CpuMap[CpuMapIndex] >> 8) & 0x1);
    CoreCountFunc += ((CpuInfo->CpuMapInfo.CpuMap[CpuMapIndex] >> 9) & 0x1);
  }

  mCoreCountEfficicency = CoreCountEfficicency;
  mCoreCountFunc = CoreCountFunc;
  return EFI_SUCCESS;
}

/**
   Install Type16 Table.

   @param[in] Smbios     Smbios handle
   @param[out] MemArraySmbiosHandle     MemArray Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
SmbiosAddType16Table (
  IN EFI_SMBIOS_PROTOCOL   *Smbios,
  OUT EFI_SMBIOS_HANDLE    *MemArraySmbiosHandle
  )
{
  EFI_STATUS                      Status;
  UINT64                          MemoryCapacity;
  SMBIOS_TABLE_TYPE16             *Type16Record;

  MemoryCapacity = SmbiosGetMemoryCapacity ();
  DEBUG((DEBUG_INFO,"Memorysize=%d\n", MemoryCapacity));

  MemoryCapacity = MemoryCapacity * 1024;// MB to KB.*/

  //
  // Type 16 SMBIOS Record
  //
  Type16Record = AllocateZeroPool (sizeof (SMBIOS_TABLE_TYPE16) + 1 + 1);
  if (NULL == Type16Record) {
    return EFI_OUT_OF_RESOURCES;
  }

  Type16Record->Hdr.Type                          = EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY;
  Type16Record->Hdr.Length                        = sizeof (SMBIOS_TABLE_TYPE16);
  Type16Record->Hdr.Handle                        = 0x0;
  Type16Record->Location                          = MemoryArrayLocationSystemBoard;
  Type16Record->Use                               = MemoryArrayUseSystemMemory;
  Type16Record->MemoryErrorInformationHandle      = 0xFFFE;
  Type16Record->NumberOfMemoryDevices             = mMemoryDevInfo->DevicesNumber;//PCD
  Type16Record->MemoryErrorCorrection             = MemoryErrorCorrectionNone;

  if (MemoryCapacity >= 0x80000000) {
    Type16Record->MaximumCapacity = 0x80000000;   // in KB;
    Type16Record->ExtendedMaximumCapacity = MemoryCapacity << 10;  // Extended Max capacity should be stored in bytes.
  } else {
    Type16Record->MaximumCapacity = (UINT32)MemoryCapacity;  // Max capacity should be stored in kilo bytes.
    Type16Record->ExtendedMaximumCapacity = 0;
  }

  *MemArraySmbiosHandle = SMBIOS_HANDLE_MEMORY;
  Status = Smbios->Add (Smbios, NULL, MemArraySmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *)Type16Record);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "[%a]:[%dL] Smbios Type16 Table Log Failed! %r \n", __FUNCTION__, __LINE__, Status));
  }

  FreePool (Type16Record);
  return Status;
}

/**
   Install Type19 Table.

   @param[in]   Smbios     Smbios handle
   @param[out]  Type19SmbiosHandle     type19 Smbios handle
   @param[in]   MemArraySmbiosHandle     MemArray Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
SmbiosAddType19Table (
  IN EFI_SMBIOS_PROTOCOL  *Smbios,
  OUT EFI_SMBIOS_HANDLE   *Type19SmbiosHandle,
  IN EFI_SMBIOS_HANDLE     MemArraySmbiosHandle
  )
{
  EFI_STATUS                      Status;
  UINT64                          TotalMemorySize;
  SMBIOS_TABLE_TYPE19             *Type19Record;
  MEMORY_BLOCK                    *MemBlock;
  MEM_BLOCK                       MemBlcok[2];
  UINTN                           Index1;
  Index1 = 0;

  TotalMemorySize = SmbiosGetMemoryCapacity ();
  TotalMemorySize = TotalMemorySize * 1024; //MB => KB
  if (TotalMemorySize == 0) {
   return EFI_NOT_FOUND;
  }

    //
  // Type 19 SMBIOS Record
  //
  Type19Record = AllocateZeroPool (sizeof (SMBIOS_TABLE_TYPE19) + 1 + 1);
  if (NULL == Type19Record) {
  return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((DEBUG_INFO,"Memory Block Count:%d\n", mMemInfor->MemBlockCount));

  MemBlock = mMemInfor->MemBlock;
  DEBUG ((DEBUG_INFO,"MemBlock:%x\n",MemBlock));
  for (Index1 = 0; Index1 < mMemInfor->MemBlockCount; Index1 ++) {
    DEBUG ((DEBUG_INFO,"Memory %d MemStart:0x%lx,MemSize:0x%lx,MemNodeId:%lx\n",Index1, MemBlock->MemStart, MemBlock->MemSize,MemBlock->MemNodeId));
    MemBlcok[Index1].Base = MemBlock->MemStart;
    MemBlcok[Index1].Top  = MemBlock->MemStart + MemBlock->MemSize;
    MemBlcok[Index1].Size = MemBlock->MemSize;
    DEBUG((DEBUG_INFO,"MemBlcok[%d].Base  is 0x%lx\n",Index1,MemBlcok[Index1].Base));
    DEBUG((DEBUG_INFO,"MemBlcok[%d].Top   is 0x%lx\n",Index1,MemBlcok[Index1].Top));
    DEBUG((DEBUG_INFO,"MemBlcok[%d].Size  is 0x%lx\n",Index1,MemBlcok[Index1].Size));

    Type19Record->Hdr.Type              = EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS;
    Type19Record->Hdr.Length            = sizeof (SMBIOS_TABLE_TYPE19);
    Type19Record->Hdr.Handle            = SMBIOS_HANDLE_PI_RESERVED;
    Type19Record->MemoryArrayHandle     = MemArraySmbiosHandle;
    Type19Record->PartitionWidth        = (UINT8)mMemoryDevInfo->DevicesNumber;
    if (MemBlcok[Index1].Base / 1024 > 0xFFFFFFFF) {
      Type19Record->StartingAddress         = 0xFFFFFFFF;
      Type19Record->EndingAddress           = 0xFFFFFFFF;
      Type19Record->ExtendedStartingAddress = MemBlcok[Index1].Base;// in B;
      Type19Record->ExtendedEndingAddress   = MemBlcok[Index1].Top - 1;
    } else {
      Type19Record->StartingAddress         = MemBlcok[Index1].Base / 1024;
      Type19Record->EndingAddress           = (MemBlcok[Index1].Top - 1) / 1024; // in KB, -1 aviod OS fill;
      Type19Record->ExtendedStartingAddress = 0;
      Type19Record->ExtendedEndingAddress   = 0x0;
    }
    *Type19SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    Status = Smbios->Add (Smbios, NULL, Type19SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *)Type19Record);
    if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "[%a]:[%dL] Smbios Type19 Table Log Failed! %r \n", __FUNCTION__, __LINE__, Status));
    }
    MemBlock++;
  }
  FreePool (Type19Record);
  return Status;
}

/**
   Change DimmFreq.

   @param[in] DimmFreq     Dimm Frequency

   @retval Fixed Dimm Frequency

**/
UINT16
FixDimmFreq(
  IN UINT32 DimmFreq
  )
{
  switch (DimmFreq) {
    case 1868:
      return 1866;

    case 2132:
      return 2133;

    case 2668:
      return 2666;

    case 2932:
      return 2933;

    default:
      return DimmFreq;
  }
}

/**
   Install Type17 Table.

   @param[in] Smbios     Smbios handle
   @param[in] Type16SmbiosHandle     type16 Smbios handle
   @param[in] Type19SmbiosHandle     type19 Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
SmbiosAddType17Table (
  IN EFI_SMBIOS_PROTOCOL *Smbios,
  IN EFI_SMBIOS_HANDLE  Type16SmbiosHandle,
  IN EFI_SMBIOS_HANDLE  Type19SmbiosHandle
  )
{
  EFI_STATUS                      Status;
  SMBIOS_TABLE_TYPE17             *Type17Record;
  SMBIOS_TABLE_TYPE20             *Type20Record;
  EFI_SMBIOS_HANDLE               MemDevSmbiosHandle;
  UINTN                           StringBufferSize;
  UINTN                           MemoryDeviceSize;
  UINT16                          ConfigMemorySpeed;
  CHAR8                           *OptionalStrStart;
  CHAR8                           *DeviceLocatorStr;
  CHAR8                           *BankLocatorStr;
  CHAR8                           *ManufactureStr;
  CHAR8                           *SerialNumberStr;
  CHAR8                           *AssertTagStr;
  CHAR8                           *PartNumberStr;
  UINT8                           Index;
  CHAR8                           Serial[9];
  UINT8                           SerialNumberIndex;
  UINT8                           SerialIndex;
  UINT8                           Head;
  UINT8                           Num;

  Type17Record        = NULL;
  Type20Record        = NULL;
  DeviceLocatorStr    = NULL;
  BankLocatorStr      = NULL;
  ManufactureStr      = NULL;
  SerialNumberStr     = NULL;
  AssertTagStr        = NULL;
  PartNumberStr       = NULL;



  ConfigMemorySpeed         = FixDimmFreq (mMcuDimms->MemFreq);
  MemoryDeviceSize          = 0;
  Status                    = 0;
  StringBufferSize          = (sizeof (CHAR8)) * SMBIOS_STRING_MAX_LENGTH;

  //
  // Manufacture
  //
  ManufactureStr = AllocateZeroPool (StringBufferSize);
  if (NULL == ManufactureStr) {
    Status = EFI_OUT_OF_RESOURCES;
    return Status;
  }

  //
  // SerialNumber
  //
  SerialNumberStr = AllocateZeroPool (StringBufferSize);
  if (NULL == SerialNumberStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_STR_MAN;
  }

  //
  // AssetTag
  //
  AssertTagStr = AllocateZeroPool (StringBufferSize);
  if (NULL == AssertTagStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_STR_SN;
  }

  //
  // PartNumber
  //
  PartNumberStr = AllocateZeroPool (StringBufferSize);
  if (NULL == PartNumberStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_STR_AST;
  }

  //
  // DeviceLocator
  //
  DeviceLocatorStr = AllocateZeroPool (StringBufferSize);
  if (NULL == DeviceLocatorStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_STR_PN;
  }

  //
  // BankLocator
  //
  BankLocatorStr = AllocateZeroPool (StringBufferSize);
  if (NULL == BankLocatorStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_STR_DEV;
  }

  //
  // Report Type 17 SMBIOS Record
  //
  StringBufferSize = (sizeof (SMBIOS_TABLE_TYPE17) + StringBufferSize * 6 + 1 + 1);

  Type17Record = AllocateZeroPool (StringBufferSize);
  if (Type17Record == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_BL;
  }
  Type20Record = AllocateZeroPool (sizeof (SMBIOS_TABLE_TYPE20) + 2);
  if (Type20Record == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }

  for (Index = 0; Index < mMemoryDevInfo->DimmCount; Index++) {
    SerialIndex = 0;
    Head = 7;

    MemoryDeviceSize = mMemoryDevInfo->SpdInfo[Index].Size;
    DEBUG((DEBUG_INFO, "DimmCount = %d, MemoryDeviceSize = %llx\n", mMemoryDevInfo->DimmCount, MemoryDeviceSize));
    if (MemoryDeviceSize <= 0) {
      continue;
    }
    DEBUG((DEBUG_INFO, "the SerialNumber is %x\n",
           mMemoryDevInfo->SpdInfo[Index].SerialNumber));
    /* add SMBIOS_TABLE_STRING */
    AsciiSPrint (
        DeviceLocatorStr, SMBIOS_STRING_MAX_LENGTH,
        "SOCKET %x CHANNEL %x DIMM %x", 0,
        mMemoryDevInfo->SpdInfo[Index].ChannelNumer,
        mMemoryDevInfo->SpdInfo[Index].DimmNumer); // ChannelNumber DimmNumber.
    AsciiSPrint (BankLocatorStr, SMBIOS_STRING_MAX_LENGTH, "Bank0");
    AsciiSPrint (AssertTagStr, SMBIOS_STRING_MAX_LENGTH, "Not Set");
    SmbiosGetManufacturer ((UINT8)mMemoryDevInfo->SpdInfo[Index].Manufacturer,
                          ManufactureStr);
    DEBUG((DEBUG_INFO, "the SerialNumber is %a,ManufactureStr %a\n", mMemoryDevInfo->SpdInfo[Index].SerialNumber, ManufactureStr));
    for (SerialNumberIndex = 0; SerialNumberIndex < 8; SerialNumberIndex++) {
      Num = (UINT8)((mMemoryDevInfo->SpdInfo[Index].SerialNumber >> (4 * Head)) & 0xF);
      if (Num < 0x0A) {
        Serial[SerialIndex] = Num + 0x30;
      } else {
        Serial[SerialIndex] = Num + 0x37;
      }
      DEBUG((DEBUG_INFO, "the Serial[%d] is 0x%x\n", SerialIndex, Serial[SerialIndex]));
      Head--;
      SerialIndex++;
    }
    Serial[8] = '\0';
    DEBUG((DEBUG_INFO, "the serialnumber is %a", Serial));

    if (mForceSpd == 1) {
      AsciiStrCpyS (PartNumberStr, StringBufferSize, "Not Set");
      AsciiStrCpyS (Serial, sizeof (Serial), "Not Set");
    } else {
      AsciiStrCpyS (PartNumberStr, StringBufferSize, mMemoryDevInfo->SpdInfo[Index].PartNumber);
    }

    AsciiStrCpyS (SerialNumberStr, StringBufferSize, Serial);

    /* fill the Type17*/
    Type17Record->Hdr.Type   = EFI_SMBIOS_TYPE_MEMORY_DEVICE;
    Type17Record->Hdr.Length = sizeof(SMBIOS_TABLE_TYPE17);
    Type17Record->Hdr.Handle = SMBIOS_HANDLE_DIMM;
    Type17Record->MemoryArrayHandle = SMBIOS_HANDLE_MEMORY;
    Type17Record->MemoryErrorInformationHandle = 0xFFFE;
    if ((mMcuDimms->McuDimm[Index].MemFeatures & 1) == 1) {
      Type17Record->TotalWidth = 72;
    } else {
      Type17Record->TotalWidth = 64;
    }

    Type17Record->DataWidth = 64;
    Type17Record->FormFactor = MemoryFormFactorDimm;
    Type17Record->DeviceLocator = 1;
    Type17Record->BankLocator = 2;
    Type17Record->MemoryType = mMemoryDevInfo->SpdInfo[Index].MemoryType; // MemoryTypeDdr3 = 0x18,
    Type17Record->TypeDetail.Synchronous = 1;
    Type17Record->Speed = ConfigMemorySpeed; // in MHZ
    Type17Record->Manufacturer = 3;
    Type17Record->SerialNumber = 4;
    Type17Record->AssetTag = 5;
    Type17Record->PartNumber = 6;
    if (MemoryDeviceSize > 32767) {
      Type17Record->Size = 0x7FFF; // in MB
      Type17Record->ExtendedSize = MemoryDeviceSize;
    } else {
      Type17Record->Size = (UINT16)MemoryDeviceSize; // in MB
      Type17Record->ExtendedSize = 0;
    }
    if (MemoryDeviceSize > 0) {
      Type17Record->ConfiguredMemoryClockSpeed = ConfigMemorySpeed;
    }
    //
    // Add for smbios 3.6
    //

    Type17Record->MinimumVoltage                = mMemoryDevInfo->SpdInfo[Index].MinVoltage;
    Type17Record->MaximumVoltage                = mMemoryDevInfo->SpdInfo[Index].MaxVoltage;
    Type17Record->ConfiguredVoltage             = mMemoryDevInfo->SpdInfo[Index].ConfiguredVoltage;

    OptionalStrStart = (CHAR8 *)(Type17Record + 1);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (DeviceLocatorStr) + 1, DeviceLocatorStr);

    OptionalStrStart += AsciiStrSize (DeviceLocatorStr);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (BankLocatorStr) + 1, BankLocatorStr);

    OptionalStrStart += AsciiStrSize (BankLocatorStr);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (ManufactureStr) + 1, ManufactureStr);

    OptionalStrStart += AsciiStrSize (ManufactureStr);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (SerialNumberStr) + 1, SerialNumberStr);

    OptionalStrStart += AsciiStrSize (SerialNumberStr);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (AssertTagStr) + 1, AssertTagStr);

    OptionalStrStart += AsciiStrSize (AssertTagStr);
    AsciiStrCpyS (OptionalStrStart, AsciiStrLen (PartNumberStr) + 1, PartNumberStr);
    //
    // Add for smbios 3.6
    //
    Type17Record->MemoryTechnology = mMemoryDevInfo->SpdInfo[Index].MemoryTechnology;
    Type17Record->MemoryOperatingModeCapability.Uint16 = 0x38;
    Type17Record->NonVolatileSize = 0;
    Type17Record->VolatileSize = MemoryDeviceSize;
    Type17Record->CacheSize = 0;
    Type17Record->LogicalSize = 0;
    Type17Record->ModuleManufacturerID = mMemoryDevInfo->SpdInfo[Index].ModuleManufacturerID;
    Type17Record->ModuleProductID = mMemoryDevInfo->SpdInfo[Index].ModuleProductID;
    Type17Record->MemorySubsystemControllerManufacturerID = mMemoryDevInfo->SpdInfo[Index].MemorSubsystemControllerManufacturerID;
    Type17Record->MemorySubsystemControllerProductID = mMemoryDevInfo->SpdInfo[Index].MemorSubsystemControllerProductID;

    MemDevSmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    Status = Smbios->Add (Smbios, NULL, &MemDevSmbiosHandle,
                         (EFI_SMBIOS_TABLE_HEADER *)Type17Record);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "[%a]:[%dL] Smbios Type17 Table Log Failed! %r \n",
             __FUNCTION__, __LINE__, Status));
    }
  }

  FreePool (Type20Record);

ProcExit:
  FreePool (Type17Record);

FREE_BL:
  FreePool (BankLocatorStr);

FREE_STR_DEV:
  FreePool (DeviceLocatorStr);

FREE_STR_PN:
  FreePool (PartNumberStr);

FREE_STR_AST:
  FreePool (AssertTagStr);

FREE_STR_SN:
  FreePool (SerialNumberStr);

FREE_STR_MAN:
  FreePool (ManufactureStr);

  return Status;
}

/**
   Install default structures by SmbiosData.c.

   @param[in] Smbios     Smbios handle
   @param[in] DefaultTables     DefaultTables defined at top of the file


   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
InstallStructures (
  IN EFI_SMBIOS_PROTOCOL       *Smbios,
  IN VOID *DefaultTables[]
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  UINTN                     TableEntry;

  Status = EFI_SUCCESS;

  for (TableEntry = 0; DefaultTables[TableEntry] != NULL; TableEntry++) {
    SmbiosHandle = ((EFI_SMBIOS_TABLE_HEADER *)DefaultTables[TableEntry])->Handle;
    Status = Smbios->Add (Smbios, NULL, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *)DefaultTables[TableEntry]);
    if (EFI_ERROR(Status)) {
      break;
    }
  }
  return Status;
}

/**
   Return Board Power Control Source.

   @retval PowerManger Value.

**/
UINT32
GetPowerContrlSource(
  VOID
  )
{
  EFI_STATUS      Status;
  UINT8           Buffer[0x100];
  BOARD_CONFIG    *BoardConfig;

  Status = GetParameterInfo(PM_BOARD, Buffer, sizeof(Buffer));
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Get Board Parameter Fail.\n"));
    while(1);
  }

  BoardConfig = (BOARD_CONFIG *)Buffer;
  if (PM_GPIO == BoardConfig->PowerManger) {
    DEBUG((DEBUG_INFO, "PowerManger is GPIO\n"));
  } else if (PM_EC == BoardConfig->PowerManger) {
    DEBUG((DEBUG_INFO, "PowerManger is EC\n"));
  }
  return BoardConfig->PowerManger;
}

/**
  Change type3 table.
**/
VOID
ChangeType03(
  VOID
  )
{
  UINT8               Type;
  UINT8               Height;

  Type = PcdGet8 (PcdType);
  Height = PcdGet8 (PcdHeight);

  SystemEnclosureType3.Base.Type = Type;
  SystemEnclosureType3.Base.Height = Height;
}

/**
  Change type0 table.
**/
VOID
ChangeType00(
  VOID
  )
{
  UINT8                BiosSize;
  UINT8                EcMajorVer;
  UINT8                EcMinorVer;

  BiosSize    = PcdGet8 (PcdBiosSize);
//EC reserved

  EcMajorVer = 0xff;
  EcMinorVer = 0xff;
  BiosInfoType0.Base.BiosSize    = BiosSize;

  if (BiosSize == 0xff) {
    BiosInfoType0.Base.ExtendedBiosSize.Unit = 0;
    BiosInfoType0.Base.ExtendedBiosSize.Size = 0x10;
  }
  //EC acpi
  if (PM_EC == GetPowerContrlSource()) {
    EcReadMem(0x0,&EcMajorVer);
    EcReadMem(0x1,&EcMinorVer);
  }
  BiosInfoType0.Base.EmbeddedControllerFirmwareMajorRelease = EcMajorVer;
  BiosInfoType0.Base.EmbeddedControllerFirmwareMinorRelease = EcMinorVer;
}

/**
  Add processor Info Type4 Table.

  @param Smbios     Smbios handle

**/
VOID
AddprocessorInfoType4Table (
  IN EFI_SMBIOS_PROTOCOL       *Smbios
  )
{
  UINTN                 Freq;
  UINT8                 Voltage;
  UINT8                 Buffer[0x100];
  PLL_CONFIG            *PllConfigData;
  EFI_SMBIOS_HANDLE     SmbiosHandle;
  EFI_STATUS            Status;

  Freq = ArmReadCntFrq ();
  Voltage = BIT7 | mCpuVoltage;

  Status = GetParameterInfo (PM_PLL, Buffer, sizeof(Buffer));
  if(!EFI_ERROR(Status)) {
    PllConfigData = (PLL_CONFIG *)(UINTN)Buffer;
  } else {
    DEBUG((DEBUG_ERROR, "Get PLL Parameter Fail.\n"));
    while(1);
  }

  if (mCoreCountEfficicency > 0) {
    *(UINT8*)&ProcessorInfoType4Efficiency.Base.Voltage = Voltage;
    ProcessorInfoType4Efficiency.Base.ExternalClock = (UINT16)(Freq/1000/1000);
    ProcessorInfoType4Efficiency.Base.CurrentSpeed = (UINT16)PllConfigData->EfficiencyCoreFreq;
    *(UINT64*)&ProcessorInfoType4Efficiency.Base.ProcessorId = 0x700F3034;
    ProcessorInfoType4Efficiency.Base.CoreCount = mCoreCountEfficicency;
    ProcessorInfoType4Efficiency.Base.EnabledCoreCount = mCoreCountEfficicency;
    SmbiosHandle = ((EFI_SMBIOS_TABLE_HEADER *)&ProcessorInfoType4Efficiency)->Handle;
    Status = Smbios->Add (Smbios, NULL, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *)&ProcessorInfoType4Efficiency);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Add Type4 Table Fail.\n"));
      while(1);
    }
  }

  if (mCoreCountFunc > 0) {
    *(UINT8*)&ProcessorInfoType4Func.Base.Voltage = Voltage;
    ProcessorInfoType4Func.Base.ExternalClock = (UINT16)(Freq/1000/1000);
    ProcessorInfoType4Func.Base.CurrentSpeed = (UINT16)PllConfigData->PerformanceCoreFreq;
    *(UINT64*)&ProcessorInfoType4Func.Base.ProcessorId = 0x701F6643;
    ProcessorInfoType4Func.Base.CoreCount = mCoreCountFunc;
    ProcessorInfoType4Func.Base.EnabledCoreCount = mCoreCountFunc;
    SmbiosHandle = ((EFI_SMBIOS_TABLE_HEADER *)&ProcessorInfoType4Func)->Handle;
    Status = Smbios->Add (Smbios, NULL, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *)&ProcessorInfoType4Func);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Add Type4 Table Fail.\n"));
      while(1);
    }
  }


}

/**
   Install Memory Table16,19,17.

   @param[in] Smbios     Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
InstallMemorySubClass (
  IN EFI_SMBIOS_PROTOCOL       *Smbios
  )
{
  EFI_HOB_GUID_TYPE               *GuidHob;
  EFI_STATUS                      Status;
  EFI_SMBIOS_HANDLE               MemArraySmbiosHandle;
  EFI_SMBIOS_HANDLE               Type19SmbiosHandle;
  UINTN                           Size;

  GuidHob = GetFirstGuidHob (&gPlatformMemoryInforGuid);
  if (NULL == GuidHob) {
    DEBUG((DEBUG_ERROR, "Could not get MemoryMap Guid hob.\n"));
    return EFI_NOT_FOUND;
  }
  mMemInfor = (PHYTIUM_MEMORY_INFOR *) GET_GUID_HOB_DATA (GuidHob);
  Size = sizeof (mMemInfor->MemBlockCount) + sizeof (MEMORY_BLOCK) * mMemInfor->MemBlockCount;
  mMcuDimms = (MCU_DIMMS *)((UINTN)mMemInfor + Size);

  mMemoryDevInfo = (MEMORY_DEVICES_INFO *)AllocateZeroPool(sizeof (MEMORY_DEVICES_INFO));
  SmbiosGetMemoryInformation ();
//// add table
  Status = SmbiosAddType16Table (Smbios,&MemArraySmbiosHandle);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Smbios Add Type16 Table Failed.  %r\n", Status));
    return Status;
  }

  Status = SmbiosAddType19Table (Smbios,&Type19SmbiosHandle, MemArraySmbiosHandle);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Smbios Add Type19 Table Failed.  %r\n", Status));
    return Status;
  }

  Status = SmbiosAddType17Table (Smbios,MemArraySmbiosHandle, Type19SmbiosHandle);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Smbios Add Type17 Table Failed.  %r\n", Status));
  }
  return Status;
}

/**
   Config E2000 Pcie by data sheet.

   @param[in] PcieConf     PCIE_CONFIG Pointer

**/
VOID
E2000PciExplainPciE(
  IN PCIE_CONFIG               *PcieConf
)
{
  UINT8 PciIndex;
  PciIndex = 0;
  //PCIE0
  if ((PcieConf->IndependentTree & (0xf)) == 0) {
    mSystemSlots->SlotsNumber = 0;
    mSystemSlots->PcieInfo[0].Enable = 0;
    mSystemSlots->PcieInfo[1].Enable = 0;
  } else if ((PcieConf->IndependentTree & (0xf)) == 1) {
    mSystemSlots->SlotsNumber = 2;
    mSystemSlots->PcieInfo[0].Enable = 1;
    mSystemSlots->PcieInfo[0].Xnumber = 1;
    mSystemSlots->PcieInfo[1].Enable = 1;
    mSystemSlots->PcieInfo[1].Xnumber = 1;
    mSystemSlots->PcieInfo[0].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[1].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[0].GenNumber = PcieConf->Pcie0Controller0Conf & (0xf);
    mSystemSlots->PcieInfo[1].GenNumber = PcieConf->Pcie0Controller1Conf & (0xf);
  } else if ((PcieConf->IndependentTree & (0xf)) == 3) {
    mSystemSlots->SlotsNumber = 1;
    mSystemSlots->PcieInfo[0].Enable = 1;
    mSystemSlots->PcieInfo[0].Xnumber = 1;
    mSystemSlots->PcieInfo[0].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[0].GenNumber = PcieConf->Pcie0Controller0Conf & (0xf);
  } else if ((PcieConf->IndependentTree & (0xf)) == 5) {
    mSystemSlots->SlotsNumber = 1;
    mSystemSlots->PcieInfo[1].Enable = 1;
    mSystemSlots->PcieInfo[1].Xnumber = 1;
    mSystemSlots->PcieInfo[1].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[1].GenNumber = PcieConf->Pcie0Controller1Conf & (0xf);
  }
  //PCIE1
  if (((PcieConf->IndependentTree >> 16) & (0xf)) == 0) {
    mSystemSlots->SlotsNumber += 0;
    mSystemSlots->PcieInfo[2].Enable = 0;
    mSystemSlots->PcieInfo[3].Enable = 0;
    mSystemSlots->PcieInfo[4].Enable = 0;
  } else if (((PcieConf->IndependentTree >> 16) & (0xf)) == 1) {
    mSystemSlots->SlotsNumber += 1;
    mSystemSlots->PcieInfo[2].Enable = 1;
    mSystemSlots->PcieInfo[2].Xnumber = 4;
    mSystemSlots->PcieInfo[2].SlotDataBusWidth = SlotDataBusWidth4X;
    mSystemSlots->PcieInfo[2].GenNumber = PcieConf->Pcie1Controller0Conf & (0xf);
  } else if (((PcieConf->IndependentTree >> 16) & (0xf)) == 3) {
    mSystemSlots->SlotsNumber += 3;
    mSystemSlots->PcieInfo[2].Enable = 1;
    mSystemSlots->PcieInfo[2].Xnumber = 2;
    mSystemSlots->PcieInfo[3].Enable = 1;
    mSystemSlots->PcieInfo[3].Xnumber = 1;
    mSystemSlots->PcieInfo[4].Enable = 1;
    mSystemSlots->PcieInfo[4].Xnumber = 1;
    mSystemSlots->PcieInfo[2].SlotDataBusWidth = SlotDataBusWidth2X;
    mSystemSlots->PcieInfo[2].GenNumber = PcieConf->Pcie1Controller0Conf & (0xf);
    mSystemSlots->PcieInfo[3].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[3].GenNumber = PcieConf->Pcie1Controller1Conf & (0xf);
    mSystemSlots->PcieInfo[4].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[4].GenNumber = PcieConf->Pcie1Controller2Conf & (0xf);
  } else if ((((PcieConf->IndependentTree >> 16) & (0xf)) == 5)) {
    mSystemSlots->SlotsNumber += 4;
    mSystemSlots->PcieInfo[2].Enable = 1;
    mSystemSlots->PcieInfo[2].Xnumber = 1;
    mSystemSlots->PcieInfo[3].Enable = 1;
    mSystemSlots->PcieInfo[3].Xnumber = 1;
    mSystemSlots->PcieInfo[4].Enable = 1;
    mSystemSlots->PcieInfo[4].Xnumber = 1;
    mSystemSlots->PcieInfo[5].Enable = 1;
    mSystemSlots->PcieInfo[5].Xnumber = 1;
    mSystemSlots->PcieInfo[2].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[2].GenNumber = PcieConf->Pcie1Controller0Conf & (0xf);
    mSystemSlots->PcieInfo[3].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[3].GenNumber = PcieConf->Pcie1Controller1Conf & (0xf);
    mSystemSlots->PcieInfo[4].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[4].GenNumber = PcieConf->Pcie1Controller2Conf & (0xf);
    mSystemSlots->PcieInfo[5].SlotDataBusWidth = SlotDataBusWidth1X;
    mSystemSlots->PcieInfo[5].GenNumber = PcieConf->Pcie1Controller2Conf & (0xf);
  }

  for (PciIndex = 0; PciIndex < 6; PciIndex++) {
    if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 1 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 1) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressX1;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 1 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 2) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressX2;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 1 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 4) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressX4;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 2 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 1) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen2X1;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber ==2 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 2) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen2X2;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 2 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 4) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen2X4;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 3 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 1) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen3X1;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 3 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 2) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen3X2;
    }else if (mSystemSlots->PcieInfo[PciIndex].GenNumber == 3 && mSystemSlots->PcieInfo[PciIndex].Xnumber == 4) {
      mSystemSlots->PcieInfo[PciIndex].SlotType = SlotTypePciExpressGen3X4;
    }
  }
}

/**
   Install Type9 Table.

   @param[in] Smbios     Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
InstallPCIeTable (
  IN EFI_SMBIOS_PROTOCOL       *Smbios
  )
{
  EFI_STATUS                Status;
  SMBIOS_TABLE_TYPE9        *Type9Record;
  EFI_SMBIOS_HANDLE         PCIeDevSmbiosHandle;
  CHAR8                     *SlotDesignationStr;
  UINTN                     StringBufferSize;
  UINT8                     Index;
  UINT8                     *Buffer;
  PCIE_CONFIG               *PcieConf;

  SlotDesignationStr       = NULL;
  Type9Record              = NULL;
  Status = 0;
  StringBufferSize = (sizeof (CHAR8)) * SMBIOS_STRING_MAX_LENGTH;

  Buffer = AllocateZeroPool (0x100);
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    return Status;
  }

  mSystemSlots = AllocateZeroPool (sizeof (SYSTEM_SLOTS));
  if (NULL == mSystemSlots) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_BUF;
  }

    //
  // SlotDesignationStr
  //
  SlotDesignationStr = AllocateZeroPool (StringBufferSize);
  if (NULL == SlotDesignationStr) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_SLOT;
  }
  //
  // Report Type 17 SMBIOS Record
  //
  StringBufferSize = (sizeof (SMBIOS_TABLE_TYPE9) + StringBufferSize *1 + 1);

  Type9Record = AllocateZeroPool (StringBufferSize);
  if (Type9Record == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_DES;
  }

  Status = GetParameterInfo (PM_PCIE, Buffer, 0x100);
  if (!EFI_ERROR (Status)) {
    PcieConf = (PCIE_CONFIG *)Buffer;
    DEBUG((DEBUG_INFO, "PcieMagic = %lx \n", PcieConf->PcieMagic));
    DEBUG((DEBUG_INFO, "Version = %lx \n", PcieConf->Version));
    DEBUG((DEBUG_INFO, "Size = %lx \n", PcieConf->Size));
    DEBUG((DEBUG_INFO, "IndependentTree = %lx \n", PcieConf->IndependentTree));
    DEBUG((DEBUG_INFO, "Pcie0Controller0Conf = %lx \n", PcieConf->Pcie0Controller0Conf));
    DEBUG((DEBUG_INFO, "Pcie0Controller1Conf = %lx \n", PcieConf->Pcie0Controller1Conf));
    DEBUG((DEBUG_INFO, "Pcie0Controller0Equalization = %lx \n", PcieConf->Pcie0Controller0Equalization));
    DEBUG((DEBUG_INFO, "Pcie0Controller1Equalization = %lx \n", PcieConf->Pcie0Controller1Equalization));
    DEBUG((DEBUG_INFO, "Pcie1Controller0Conf = %lx \n", PcieConf->Pcie1Controller0Conf));
    DEBUG((DEBUG_INFO, "Pcie1Controller1Conf = %lx \n", PcieConf->Pcie1Controller1Conf));
    DEBUG((DEBUG_INFO, "Pcie1Controller2Conf = %lx \n", PcieConf->Pcie1Controller2Conf));
    DEBUG((DEBUG_INFO, "Pcie1Controller3Conf = %lx \n", PcieConf->Pcie1Controller3Conf));
    DEBUG((DEBUG_INFO, "Pcie1Controller0Equalization = %lx \n", PcieConf->Pcie1Controller0Equalization));
    DEBUG((DEBUG_INFO, "Pcie1Controller1Equalization = %lx \n", PcieConf->Pcie1Controller1Equalization));
    DEBUG((DEBUG_INFO, "Pcie1Controller2Equalization = %lx \n", PcieConf->Pcie1Controller2Equalization));
    DEBUG((DEBUG_INFO, "Pcie1Controller3Equalization = %lx \n", PcieConf->Pcie1Controller3Equalization));
  } else {
    DEBUG ((DEBUG_ERROR, "Get PCIE Parameter Fail.\n"));
    while (1);
  };

  E2000PciExplainPciE (PcieConf);

  for(Index = 0; Index < 6; Index ++) {
    /* fill the Type9*/
    if (mSystemSlots->PcieInfo[Index].Enable == 0) {
      continue;
    }
    AsciiSPrint (SlotDesignationStr, SMBIOS_STRING_MAX_LENGTH, "PCI Express GEN %d X%x", mSystemSlots->PcieInfo[Index].GenNumber, mSystemSlots->PcieInfo[Index].Xnumber);
    DEBUG((DEBUG_INFO, "%d PCI Express GEN %d X%x \n", Index, mSystemSlots->PcieInfo[Index].GenNumber, mSystemSlots->PcieInfo[Index].Xnumber));
    Type9Record->Hdr.Type             = EFI_SMBIOS_TYPE_SYSTEM_SLOTS;
    Type9Record->Hdr.Length           = sizeof (SMBIOS_TABLE_TYPE9);
    Type9Record->Hdr.Handle           = 0;
    Type9Record->SlotDesignation      = 1;
    Type9Record->SlotType             = mSystemSlots->PcieInfo[Index].SlotType;
    Type9Record->SlotDataBusWidth     = mSystemSlots->PcieInfo[Index].SlotDataBusWidth;

    PCIeDevSmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    Status = Smbios->Add (Smbios, NULL, &PCIeDevSmbiosHandle, (EFI_SMBIOS_TABLE_HEADER*) Type9Record);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "[%a]:[%dL] Smbios Type9 Table Log Failed! %r \n", __FUNCTION__, __LINE__, Status));
    }
  }
  FreePool (Type9Record);
FREE_DES:
  FreePool (SlotDesignationStr);
FREE_SLOT:
  FreePool (mSystemSlots);
FREE_BUF:
  FreePool (Buffer);
  return Status;
}

/**
   Install All Structures for ARM platforms.

   @param Smbios     Smbios handle

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
InstallAllStructures (
  IN EFI_SMBIOS_PROTOCOL       *Smbios
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  ChangeType00 ();
  ChangeType03 ();
  AddprocessorInfoType4Table (Smbios);
  Status = InstallStructures (Smbios,DefaultCommonTables);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/**
   Installs SMBIOS information for ARM platforms.

   @param ImageHandle     Module's image handle
   @param SystemTable     Pointer of EFI_SYSTEM_TABLE

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
EFIAPI
SmbiosTablePublishEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{

  EFI_STATUS               Status;
  EFI_SMBIOS_PROTOCOL      *Smbios;
  CHAR8                    BiosVersion[100];
  CHAR16                   *ReleaseString;
  UINTN                    Index;
  EFI_SMBIOS_HANDLE        Handle;


  SmbiosGetCpuInfo ();
  ReleaseString = (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVersionString);
  UnicodeStrToAsciiStrS (ReleaseString, BiosVersion, StrLen (ReleaseString) + 1);
  //
  // Find the SMBIOS protocol
  //

  Status = gBS->LocateProtocol (
    &gEfiSmbiosProtocolGuid,
    NULL,
    (VOID**)&Smbios
    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = InstallAllStructures (Smbios);

  InstallMemorySubClass (Smbios);
  InstallPCIeTable (Smbios);

  //update type0
  Handle  = SMBIOS_HANDLE_BIOSINFOR;
  Index = 2;
  Smbios->UpdateString (Smbios, &Handle, &Index, BiosVersion);

  return Status;
}
