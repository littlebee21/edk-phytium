/** @file
*
*  Copyright (c) 2018, GreatWall. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <OEMSvc.h>
#include <S3Var.h>
#include "S3Resume.h"
#include <Library/ParameterTable.h>
#include <Library/SerialPortLib.h>
// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 32

// DDR attributes
#define DDR_ATTRIBUTES_CACHED              ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED            ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED


typedef struct {
  UINT64  TzcRegionLow;
  UINT64  TzcRegionTop;
} TZC_REGION;

typedef struct {
  UINT64     Base;
  UINT64     Top;
  UINT64     Size;
} MEM_BLOCK;

VOID
Sort(
  UINT64     *Data,
  UINT32      N
  )
{
  UINT64  Temp;
  UINT64  Key;
  UINT64  IndexI;
  UINT64  IndexJ;

  for (IndexI = 0; IndexI < N; IndexI++){
    Key = IndexI;
    for (IndexJ = IndexI + 1; IndexJ < N; IndexJ++) {
      if (Data[IndexJ] < Data[Key]){
        Key = IndexJ;
      }
    }
    if (Key != IndexI){
      Temp      = Data[Key];
      Data[Key] = Data[IndexI];
      Data[IndexI]   = Temp;
      //DEBUG((DEBUG_INFO,"Data[%d] is 0x%lx\n",i,Data[i]));
    }
  }
}

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]  VirtualMemoryMap  Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                 Virtual Memory mapping. This array must be ended by a zero-filled
                                 entry
**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  //ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  EFI_RESOURCE_ATTRIBUTE_TYPE   ResourceAttributes;
  UINTN                         Index;
  UINTN                         Index1;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  ARM_SMC_ARGS                  ArmSmcArgs;
  UINT8                         *MemBuffer;
  PHYTIUM_MEMORY_INFOR          *MemInfor;
  MEMORY_BLOCK                  *MemBlock = NULL;
  MCU_DIMMS                     *McuDimms;
  MCU_DIMM                      *McuDimm;
  UINT64                        CpuVersion;
  PHYTIUM_CPU_COURE_INFOR       *CpuCoreInfor;
  PHYTIUM_CPU_MAP_INFOR         *CpuMapInfor;
  PHYTIUM_PCI_HOST_BRIDGE       *PciHostBridge;
  PCI_HOST_BLOCK                *PciHostBlock;
  PHYTIUM_PCI_CONTROLLER        *PciController;
  PCI_BLOCK                     *PciBlock;
  UINT64                        PciIobase;
  UINT64                        PciIoSize;
  UINT64                        PciMem32base;
  UINT64                        PciMem32Size;
  UINT64                        PciMem64base;
  UINT64                        PciMem64Size;
  UINT64                        Size;

  //SECURE_CONFIG_T               *SecureConfigTable = NULL;
  //UINT64                         SecureAddr;
  //UINT64                         SecureArray[8];
  //UINT64                         TzcRegionLow[5];
  //UINT64                         TzcRegionTop[5];
  //UINT64                         MemoryMapStart;
  //UINT64                         MemoryMapSize;
  //TZC_REGION                     TzcRegionY[5];
  //TZC_REGION                     TzcRegionX[5];
  //UINT8                          NumberInBlockX;
  //UINT8                          NumberInBlockY;
  MEM_BLOCK                      MemBlcok[2];
  //UINT8                          MapNumber;

  Index = 0;
  PciIobase = 0;
  PciIoSize = 0;
  PciMem32base = 0;
  PciMem32Size = 0;
  PciMem64base = 0;
  PciMem64Size = 0;

  //SecureConfigTable = AllocatePool(sizeof(SECURE_BASE_INFO));
  //SecureAddr = O_PARAMETER_BASE + PM_SECURE_OFFSET;
  //CopyMem(SecureConfigTable, (VOID *)SecureAddr, sizeof(SECURE_BASE_INFO));

  //SecureArray[0] = (SecureConfigTable->TzcRegionLow1 & (~0XFFF));
  //SecureArray[1] = (SecureConfigTable->TzcRegionTop1 & (~0XFFF));
  //SecureArray[2] = (SecureConfigTable->TzcRegionLow2 & (~0XFFF));
  //SecureArray[3] = (SecureConfigTable->TzcRegionTop2 & (~0XFFF));
  //SecureArray[4] = (SecureConfigTable->TzcRegionLow3 & (~0XFFF));
  //SecureArray[5] = (SecureConfigTable->TzcRegionTop3 & (~0XFFF));
  //SecureArray[6] = (SecureConfigTable->TzcRegionLow4 & (~0XFFF));
  //SecureArray[7] = (SecureConfigTable->TzcRegionTop4 & (~0XFFF));
  //Sort (SecureArray,8);
  //for (Index1 = 0; Index1 < 8 ; Index1++){
  //  DEBUG ((DEBUG_INFO, "the tzc_region_base_%d is 0x%lx\n",i,SecureArray[Index1]));
  //}

  //TzcRegionLow[1] = SecureArray[0];
  //TzcRegionLow[2] = SecureArray[2];
  //TzcRegionLow[3] = SecureArray[4];
  //TzcRegionLow[4] = SecureArray[6];

  //TzcRegionTop[1] = SecureArray[1];
  //TzcRegionTop[2] = SecureArray[3];
  //TzcRegionTop[3] = SecureArray[5];
  //TzcRegionTop[4] = SecureArray[7];
  //ASSERT (VirtualMemoryMap != NULL);

#ifdef S3_FUNC_USE
  S3FuncRegister();
#endif

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }

  //if (FeaturePcdGet (PcdCacheEnable) == TRUE) {
  //  CacheAttributes = DDR_ATTRIBUTES_CACHED;
  //} else {
  //  CacheAttributes = DDR_ATTRIBUTES_UNCACHED;
  //}

  // CPU peripherals. TRM. Manual says not all of them are implemented.
  //
  VirtualMemoryTable[+Index].PhysicalBase  = PcdGet64 (PcdSystemIoBase);
  VirtualMemoryTable[Index].VirtualBase    = PcdGet64 (PcdSystemIoBase);
  VirtualMemoryTable[Index].Length         = PcdGet64 (PcdSystemIoSize);
  VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // PCI Config Space
  VirtualMemoryTable[++Index].PhysicalBase = PcdGet64 (PcdPciConfigBase);
  VirtualMemoryTable[Index].VirtualBase    = PcdGet64 (PcdPciConfigBase);
  VirtualMemoryTable[Index].Length         = PcdGet64 (PcdPciConfigSize);
  VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  MemBuffer = AllocatePages(1);
  if (MemBuffer != NULL) {
    DEBUG ((DEBUG_ERROR,"MemBlock:%x\n",MemBuffer));
    ZeroMem(MemBuffer, EFI_PAGE_SIZE);
//
//pcie
//
#if 1
    ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_HOST_BRIDGE;
    ArmSmcArgs.Arg1 = (UINTN)MemBuffer;
    ArmSmcArgs.Arg2 = EFI_PAGE_SIZE;
    ArmCallSmc (&ArmSmcArgs);
    DEBUG ((DEBUG_ERROR,"Get PCI host bridge information Arg0:%d,\n",ArmSmcArgs.Arg0));
    if (ArmSmcArgs.Arg0 == 0) {
      PciHostBridge = (PHYTIUM_PCI_HOST_BRIDGE *)(UINTN)MemBuffer;
      DEBUG ((DEBUG_ERROR,"PCI host bridge count:%x\n",PciHostBridge->PciHostCount));
      PciHostBlock = PciHostBridge->PciHostBlock;
      PciIobase    = PciHostBlock->IoBase;
      PciMem32base = PciHostBlock->Mem32Base;
      PciMem64base = PciHostBlock->Mem64Base;
      for (Index1 = 0; Index1 < PciHostBridge->PciHostCount; Index1 ++) {
        DEBUG ((DEBUG_ERROR,"PciConfigBase:%lx, IoBase:%lx, IoSize:%x, Mem32Base:%lx, Mem32Size:%x, Mem64Base:%lx, Mem64Size:%lx\n",
        PciHostBlock->PciConfigBase, PciHostBlock->IoBase,PciHostBlock->IoSize, PciHostBlock->Mem32Base,PciHostBlock->Mem32Size,PciHostBlock->Mem64Base, PciHostBlock->Mem64Size));
        if (PciIobase > PciHostBlock->IoBase) {
          PciIobase    = PciHostBlock->IoBase;
        }
        if (PciMem32base > PciHostBlock->Mem32Base) {
          PciMem32base    = PciHostBlock->Mem32Base;
        }
        if (PciMem64base > PciHostBlock->Mem64Base) {
          PciMem64base    = PciHostBlock->Mem64Base;
        }
        PciIoSize    += PciHostBlock->IoSize;
        PciMem32Size += PciHostBlock->Mem32Size;
        PciMem64Size += PciHostBlock->Mem64Size;
        PciHostBlock ++;
      }
      // VirtualMemoryTable[++Index].PhysicalBase = PciHostBlock->PciConfigBase;
      // VirtualMemoryTable[Index].VirtualBase    = PciHostBlock->PciConfigBase;

      // PCI IO Space
      VirtualMemoryTable[++Index].PhysicalBase = PciIobase;
      VirtualMemoryTable[Index].VirtualBase    = PciIobase;
      VirtualMemoryTable[Index].Length         = PciIoSize;
      VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

      // PCI MEM32 Space
      VirtualMemoryTable[++Index].PhysicalBase = PciMem32base;
      VirtualMemoryTable[Index].VirtualBase    = PciMem32base;
      VirtualMemoryTable[Index].Length         = PciMem32Size;
      VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

      // PCI MEM64 Space
      VirtualMemoryTable[++Index].PhysicalBase = PciMem64base;
      VirtualMemoryTable[Index].VirtualBase    = PciMem64base;
      VirtualMemoryTable[Index].Length         = PciMem64Size;
      VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

      Size = sizeof (PciHostBridge->PciHostCount) + PciHostBridge->PciHostCount * sizeof (PCI_HOST_BLOCK);
      PciController = (PHYTIUM_PCI_CONTROLLER *)(MemBuffer + Size);
      ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_PCI_CONTROLLER;
      ArmSmcArgs.Arg1 = (UINTN)PciController;
      ArmSmcArgs.Arg2 = EFI_PAGE_SIZE - Size;
      ArmCallSmc (&ArmSmcArgs);
      DEBUG ((DEBUG_ERROR,"Get PCI Arg0:%x\n",ArmSmcArgs.Arg0));
      if (ArmSmcArgs.Arg0 == 0) {
        DEBUG ((DEBUG_ERROR,"PciController:%lx, PciCount:%x\n", PciController,PciController->PciCount));
        PciBlock = PciController->PciBlock;
        for (Index1 = 0; Index1 < PciController->PciCount; Index1 ++) {
          DEBUG ((DEBUG_ERROR,"Index:%x, PciBlock:%lx, PciLane:%x, PciSpeed:%x\n", Index1, PciBlock, PciBlock->PciLane, PciBlock->PciSpeed));
          PciBlock ++;
        }
        Size += sizeof (PciController->PciCount) + PciController->PciCount * sizeof (PCI_BLOCK);
      }

      BuildGuidDataHob (&gPlatformPciHostInforGuid, MemBuffer, Size);
    }
#endif
    // Get Memory information
    ZeroMem(MemBuffer, EFI_PAGE_SIZE);
    ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_MEM_REGIONS;
    ArmSmcArgs.Arg1 = (UINTN)MemBuffer;
    ArmSmcArgs.Arg2 = EFI_PAGE_SIZE;
    ArmCallSmc (&ArmSmcArgs);
    DEBUG ((DEBUG_INFO,"Get memory information Arg0:%d,\n",ArmSmcArgs.Arg0));
    if (ArmSmcArgs.Arg0 == 0) {
      MemInfor = (PHYTIUM_MEMORY_INFOR *)(UINTN)MemBuffer;
      DEBUG ((DEBUG_INFO,"Memory Block Count:%d\n", MemInfor->MemBlockCount));
      ResourceAttributes =
        EFI_RESOURCE_ATTRIBUTE_PRESENT |
        EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
        EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
        EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
        EFI_RESOURCE_ATTRIBUTE_TESTED;
      MemBlock = MemInfor->MemBlock;
      DEBUG ((DEBUG_INFO,"MemBlock:%x,ResourceAttributes:%x\n",MemBlock,ResourceAttributes));
      for (Index1 = 0; Index1 < MemInfor->MemBlockCount; Index1 ++) {
        DEBUG ((DEBUG_INFO,"Memory %d MemStart:0x%lx,MemSize:0x%lx,MemNodeId:%lx\n",Index1, MemBlock->MemStart, MemBlock->MemSize,MemBlock->MemNodeId));
        MemBlcok[Index1].Base = MemBlock->MemStart;
        MemBlcok[Index1].Top  = MemBlock->MemStart + MemBlock->MemSize;
        MemBlcok[Index1].Size = MemBlock->MemSize;
        DEBUG((DEBUG_INFO,"MemBlcok[%d].Base  is 0x%lx\n",Index1,MemBlcok[Index1].Base));
        DEBUG((DEBUG_INFO,"MemBlcok[%d].Top   is 0x%lx\n",Index1,MemBlcok[Index1].Top));
        DEBUG((DEBUG_INFO,"MemBlcok[%d].Size  is 0x%lx\n",Index1,MemBlcok[Index1].Size));
        MemBlock++;
      }
      //for (Index1 = 0; Index1 < 1; Index1 ++) {
      //  DEBUG ((DEBUG_INFO,"Memory %d MemStart:0x%lx,MemSize:0x%lx,MemNodeId:%lx\n",Index1, MemBlock->MemStart, MemBlock->MemSize,MemBlock->MemNodeId));
      //  MemBlcok[Index1].Base = 0x80000000;
      //  MemBlcok[Index1].Top  = 0xFA000000;
      //  MemBlcok[Index1].Size = 0x7A000000;
      //  DEBUG((DEBUG_INFO,"MemBlcok[%d].Base  is 0x%lx\n",Index1,MemBlcok[Index1].Base));
      //  DEBUG((DEBUG_INFO,"MemBlcok[%d].Top   is 0x%lx\n",Index1,MemBlcok[Index1].Top));
      //  DEBUG((DEBUG_INFO,"MemBlcok[%d].Size  is 0x%lx\n",Index1,MemBlcok[Index1].Size));
      //  MemBlock++;
      //}
#if 0
      if ( (SecureConfigTable->MiscEnable == 0x1) || (SecureConfigTable->MiscEnable == 0x3)){
      //if ( (SecureAddr == 0x1) || (SecureAddr == 0x3)){
        DEBUG((DEBUG_INFO,"Create VirtualMemoryTable systerm memory (TZC)\n"));
        NumberInBlockX = 1;
        NumberInBlockY = 1;
        //
        //Classify the four regions and determine whether they are legal.
        //
        for (Index1 = 1; Index1 < 5 ; Index1++){
          if ((TzcRegionLow[Index1] >= MemBlcok[0].Base) && (TzcRegionTop[Index1] <= MemBlcok[0].Top)){
            //Char = 'X';
            //SerialPortWrite((UINT8 *) &Char, 1);
            //DEBUG((DEBUG_INFO,"TzcRegion in BlockX\n"));
            TzcRegionX[NumberInBlockX].TzcRegionLow = TzcRegionLow[Index1];
            TzcRegionX[NumberInBlockX].TzcRegionTop = TzcRegionTop[Index1];
            NumberInBlockX++;
          } else if((TzcRegionLow[Index1] >= MemBlcok[1].Base) && (TzcRegionTop[Index1] <= MemBlcok[1].Top)){
            //Char = 'Y';
            //SerialPortWrite((UINT8 *) &Char, 1);
            //DEBUG((DEBUG_INFO,"TzcRegion in BlockY\n"));
            TzcRegionY[NumberInBlockY].TzcRegionLow = TzcRegionLow[Index1];
            TzcRegionY[NumberInBlockY].TzcRegionTop = TzcRegionTop[Index1];
            NumberInBlockY++;
          } else if((TzcRegionLow[Index1] == 0) && (TzcRegionTop[Index1] == 0)){
            //Char = '0';
            //SerialPortWrite((UINT8 *) &Char, 1);
            //DEBUG((DEBUG_INFO,"TzcRegion set 0\n"));
          } else{
            //Char = 'E';
            //SerialPortWrite((UINT8 *) &Char, 1);
            DEBUG((DEBUG_ERROR,"TzcRegion set error,not in the legal memory range!\n"));
            while(1);
          }
        }
        DEBUG ((DEBUG_INFO,"NumberInBlockX is %d\n",(NumberInBlockX-1)));
        DEBUG ((DEBUG_INFO,"NumberInBlockY is %d\n",(NumberInBlockY-1)));
        //
        //Cull TZC regions from Memory Space and generated Virtual Memory tables.
        //
        TzcRegionX[0].TzcRegionTop              = MemBlcok[0].Base;
        TzcRegionY[0].TzcRegionTop              = MemBlcok[1].Base;
        TzcRegionX[NumberInBlockX].TzcRegionLow = MemBlcok[0].Top;
        TzcRegionY[NumberInBlockY].TzcRegionLow = MemBlcok[1].Top;
        if ( NumberInBlockX > 1 ){
          MapNumber = 1;
          for (Index1 = 1; Index1 <= NumberInBlockX ; Index1++){
            MemoryMapStart = TzcRegionX[Index1-1].TzcRegionTop;
            MemoryMapSize  = (TzcRegionX[Index1].TzcRegionLow - TzcRegionX[Index1-1].TzcRegionTop);
            if (MemoryMapSize != 0){
              DEBUG ((DEBUG_INFO,"MemoryMapX %d MemStart:0x%lx,MemSize:0x%lx\n",MapNumber, MemoryMapStart, MemoryMapSize));
              BuildResourceDescriptorHob (
                EFI_RESOURCE_SYSTEM_MEMORY,
                ResourceAttributes,
                MemoryMapStart,
                MemoryMapSize);
              VirtualMemoryTable[++Index].PhysicalBase = MemoryMapStart;
              VirtualMemoryTable[Index].VirtualBase    = MemoryMapStart;
              VirtualMemoryTable[Index].Length         = MemoryMapSize;
              VirtualMemoryTable[Index].Attributes     = CacheAttributes;
              MapNumber++;
            }
          }
        }else{
          BuildResourceDescriptorHob (
            EFI_RESOURCE_SYSTEM_MEMORY,
            ResourceAttributes,
            MemBlcok[0].Base,
            MemBlcok[0].Size);
          VirtualMemoryTable[++Index].PhysicalBase = MemBlcok[0].Base;
          VirtualMemoryTable[Index].VirtualBase    = MemBlcok[0].Base;
          VirtualMemoryTable[Index].Length         = MemBlcok[0].Size;
          VirtualMemoryTable[Index].Attributes     = CacheAttributes;
        }
        if ( NumberInBlockY > 1 ){
          MapNumber = 1;
          for (Index1 = 1; Index1 <= NumberInBlockY ; Index1++){
            MemoryMapStart = TzcRegionY[Index1-1].TzcRegionTop;
            MemoryMapSize  = (TzcRegionY[Index1].TzcRegionLow - TzcRegionY[Index1-1].TzcRegionTop);
            if (MemoryMapSize != 0){
              DEBUG ((DEBUG_INFO,"MemoryMapY %d MemStart:0x%lx,MemSize:0x%lx\n",MapNumber, MemoryMapStart, MemoryMapSize));
              BuildResourceDescriptorHob (
                EFI_RESOURCE_SYSTEM_MEMORY,
                ResourceAttributes,
                MemoryMapStart,
                MemoryMapSize);
              VirtualMemoryTable[++Index].PhysicalBase = MemoryMapStart;
              VirtualMemoryTable[Index].VirtualBase    = MemoryMapStart;
              VirtualMemoryTable[Index].Length         = MemoryMapSize;
              VirtualMemoryTable[Index].Attributes     = CacheAttributes;
              MapNumber++;
            }
          }
        }else{
          BuildResourceDescriptorHob (
            EFI_RESOURCE_SYSTEM_MEMORY,
            ResourceAttributes,
            MemBlcok[1].Base,
            MemBlcok[1].Size);
          VirtualMemoryTable[++Index].PhysicalBase = MemBlcok[1].Base;
          VirtualMemoryTable[Index].VirtualBase    = MemBlcok[1].Base;
          VirtualMemoryTable[Index].Length         = MemBlcok[1].Size;
          VirtualMemoryTable[Index].Attributes     = CacheAttributes;
        }
      }else{
        DEBUG((DEBUG_INFO, "Create VirtualMemoryTable systerm memory\n"));
        for (Index1 = 0; Index1 < 2; Index1 ++) {
          BuildResourceDescriptorHob (
            EFI_RESOURCE_SYSTEM_MEMORY,
            ResourceAttributes,
            MemBlcok[Index1].Base,
            MemBlcok[Index1].Size);
          VirtualMemoryTable[++Index].PhysicalBase = MemBlcok[Index1].Base;
          VirtualMemoryTable[Index].VirtualBase    = MemBlcok[Index1].Base;
          VirtualMemoryTable[Index].Length         = MemBlcok[Index1].Size;
          VirtualMemoryTable[Index].Attributes     = CacheAttributes;
          DEBUG ((DEBUG_INFO,"MemoryMap %d MemStart:0x%lx,MemSize:0x%lx\n",Index1+1, MemBlcok[Index1].Base, MemBlcok[Index1].Size));
        }
      }
#endif
      DEBUG((DEBUG_INFO, "Create VirtualMemoryTable systerm memory\n"));
      for (Index1 = 0; Index1 < MemInfor->MemBlockCount; Index1 ++) {
        BuildResourceDescriptorHob (
          EFI_RESOURCE_SYSTEM_MEMORY,
          ResourceAttributes,
          MemBlcok[Index1].Base,
          MemBlcok[Index1].Size);
        VirtualMemoryTable[++Index].PhysicalBase = MemBlcok[Index1].Base;
        VirtualMemoryTable[Index].VirtualBase    = MemBlcok[Index1].Base;
        VirtualMemoryTable[Index].Length         = MemBlcok[Index1].Size;
        VirtualMemoryTable[Index].Attributes     = 2;
        DEBUG ((DEBUG_INFO,"MemoryMap %d MemStart:0x%lx,MemSize:0x%lx\n",Index1+1, MemBlcok[Index1].Base, MemBlcok[Index1].Size));
      }
      Size = sizeof (MemInfor->MemBlockCount) + sizeof (MEMORY_BLOCK) * MemInfor->MemBlockCount;
      McuDimms = (MCU_DIMMS *)(UINTN)(MemBuffer + Size);
      ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_MCU_DIMMS;
      ArmSmcArgs.Arg1 = (UINTN)McuDimms;
      ArmSmcArgs.Arg2 = EFI_PAGE_SIZE - Size;
      ArmCallSmc (&ArmSmcArgs);
      DEBUG ((DEBUG_ERROR,"Get memory DIMMS Arg0:%d,\n",ArmSmcArgs.Arg0));
      if (ArmSmcArgs.Arg0 == 0) {
        DEBUG ((DEBUG_ERROR,"McuDimms:%x, Freq:%d,MemDimmCount:%x \n",McuDimms,McuDimms->MemFreq, McuDimms->MemDimmCount));
        Size += sizeof (McuDimms->MemFreq) + sizeof (McuDimms->MemDimmCount);
        McuDimm  = McuDimms->McuDimm;
        DEBUG ((DEBUG_ERROR,"McuDimm:%x\n",McuDimm));
        for (Index1 = 0; Index1 < McuDimms->MemDimmCount; Index1 ++) {
          DEBUG ((DEBUG_ERROR,"DIMM %d Size:%dMB, MemDramId:%x,MemModuleId:%x,MemSerial:%x,MemSlotNumber:%x,MemFeatures:%x\n",
               Index1, McuDimm->MemSize,McuDimm->MemDramId, McuDimm->MemModuleId, McuDimm->MemSerial, McuDimm->MemSlotNumber,McuDimm->MemFeatures));
          McuDimm ++;
          Size += sizeof (MCU_DIMM);
        }
      }
      BuildGuidDataHob (&gPlatformMemoryInforGuid, MemBuffer, Size);
    }

    // Get CPU informations
    ZeroMem(MemBuffer, EFI_PAGE_SIZE);
    ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_CPU_VERSION;
    ArmSmcArgs.Arg1 = 0;
    ArmCallSmc (&ArmSmcArgs);
    CpuVersion = ArmSmcArgs.Arg1;
    DEBUG((DEBUG_INFO, "CPU version :%d\n", CpuVersion));
    *(UINTN*)(UINTN)MemBuffer = CpuVersion;
    Size = sizeof (UINT64);
    CpuCoreInfor = (PHYTIUM_CPU_COURE_INFOR *)(UINTN)(MemBuffer + Size);
    ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_CPU_CONF;
    ArmSmcArgs.Arg1 = (UINTN)(CpuCoreInfor);
    ArmSmcArgs.Arg2 = EFI_PAGE_SIZE -Size;
    ArmCallSmc (&ArmSmcArgs);
    DEBUG((DEBUG_ERROR, "CPU core infor ArmSmcArgs.Arg0:%d\n",ArmSmcArgs.Arg0));
    if(ArmSmcArgs.Arg0 == 0) {
      DEBUG((DEBUG_ERROR, "CpuCoreInfor:%x,CpuFreq:%d,CpuL3CacheSize:%x,CpuL3CacheLineSize:%x\n",
      CpuCoreInfor, CpuCoreInfor->CpuFreq, CpuCoreInfor->CpuL3CacheSize, CpuCoreInfor->CpuL3CacheLineSize));
      Size += sizeof (PHYTIUM_CPU_COURE_INFOR);
    }
    CpuMapInfor = (PHYTIUM_CPU_MAP_INFOR *)(UINTN)(MemBuffer + Size);
    ArmSmcArgs.Arg0 = PHYTIUM_OEM_SVC_CPU_MAPS;
    ArmSmcArgs.Arg1 = (UINTN)(CpuMapInfor);
    ArmSmcArgs.Arg2 = EFI_PAGE_SIZE - Size;
    ArmCallSmc (&ArmSmcArgs);
    DEBUG((DEBUG_ERROR, "CPU map infor ArmSmcArgs.Arg0:%d\n",ArmSmcArgs.Arg0));
    if(ArmSmcArgs.Arg0 == 0) {
      DEBUG((DEBUG_ERROR, "CpuMapCount:%x\n", CpuMapInfor, CpuMapInfor->CpuMapCount));
      Size += sizeof (CpuMapInfor->CpuMapCount) + CpuMapInfor->CpuMapCount * sizeof (CpuMapInfor->CpuMap);
    }
    DEBUG((DEBUG_INFO,"CpuVersion:%lld\nCpuCoreInfor->CpuFreq:%lld,CpuCoreInfor->CpuL3CacheSize:%lld,CpuCoreInfor->CpuL3CacheLineSize:%lld\nCpuMapInfor->CpuMapCount:%lld,CpuMapInfor->CpuMap[0]:%llx\n",
    CpuVersion,CpuCoreInfor->CpuFreq,CpuCoreInfor->CpuL3CacheSize,CpuCoreInfor->CpuL3CacheLineSize,
    CpuMapInfor->CpuMapCount,CpuMapInfor->CpuMap[0]));
    //for(i=0;i<Size;i++){
    //DEBUG((DEBUG_INFO,"---index:%d,total:%d,%02x\n",i,Size,MemBuffer[i]));
    //}
    BuildGuidDataHob (&gPlatformCpuInforGuid, MemBuffer, Size);

    FreePages (MemBuffer, 1);
  }


  VirtualMemoryTable[++Index].PhysicalBase = 0x38000000;
  VirtualMemoryTable[Index].VirtualBase    = 0x38000000;
  VirtualMemoryTable[Index].Length         = 0x4000000;
  VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK_NONSHAREABLE;
  //
  //QSPI
  //
  VirtualMemoryTable[++Index].PhysicalBase = 0x0;
  VirtualMemoryTable[Index].VirtualBase    = 0x0;
  VirtualMemoryTable[Index].Length         = 0x4000000;
  VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK_NONSHAREABLE;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase    = 0;
  VirtualMemoryTable[Index].Length         = 0;
  VirtualMemoryTable[Index].Attributes     = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  for (Index1 = 0; Index1 < Index; Index1++) {
    DEBUG((DEBUG_ERROR, "PhysicalBase %12lx VirtualBase %12lx Length %12lx Attributes %12lx\n", VirtualMemoryTable[Index1].PhysicalBase,\
            VirtualMemoryTable[Index1].VirtualBase, VirtualMemoryTable[Index1].Length, VirtualMemoryTable[Index1].Attributes));
  }
  //FreePool (MemData);
  *VirtualMemoryMap = VirtualMemoryTable;
}

