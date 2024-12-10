/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiPei.h>

//
// The protocols, PPI and GUID definitions for this module
//
#include <Ppi/MasterBootMode.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/GuidedSectionExtraction.h>
//
// The Library classes this module consumes
//
#include <Library/ArmPlatformLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include <Library/PcdLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/PadLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <OemConfigData.h>
#include <Library/ParameterTable.h>
#include "McuInfo.h"
#include <Library/PhytiumPowerControlLib.h>
#include <Library/EcLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <PcieConfigData.h>
#include "Board.h"
#include <Library/ScmiLib.h>
#include <Library/PhytiumGpioLib.h>

#define CPU_GET_PARAMETER_VERSION     0xC2000F00
#define CPU_GET_RST_SOURCE            0xC2000F01
#define CPU_INIT_PLL                  0xC2000F02
#define CPU_INIT_PCIE                 0xC2000F03
#define CPU_INIT_DDR                  0xC2000F04
#define CPU_RELOCATE                  0xC2000F05
#define CPU_SECURITY_CFG              0xC2000F07
#define CPU_SECURITY_CFG              0xC2000F07
#define CPU_GET_MEM_INFO              0xC2000F14

#define DDR_INIT                      0x0
#define DDR_SUSPEND_ENTRY             0x1
#define DDR_SUSPEND_LOCK              0x2
#define DDR_SUSPEND_UNLOCK            0x3
#define TAZ_AES_INIT                  0x6
#define DDR_FAST_TRAINNING            0xA
#define DDR_BIST                      0xB

#define POWER_ON_RESET   0x1
#define SOFT_WARM_RESET  0x2

#define LAN_STEP      0x1000

//
//MHU address space
//
#define MHU_BASE                   (0x32a00000)
#define MHU_SCP_UBOOT_BASE         (MHU_BASE + 0x20)
#define MHU_UBOOT_SCP_BASE         (MHU_BASE + 0x120)
#define MHU_CONFIG_BASE            (MHU_BASE + 0x500)
//
//MHU os chanle address
//
#define MHU_SCP_OS_BASE            (MHU_BASE + 0x0)
#define MHU_OS_SCP_BASE            (MHU_BASE + 0x100)
#define SHARE_MEM_BASE             (0x32a10000)
#define SCP_TO_AP_OS_MEM_BASE      (0x32a10000 + 0x1000)
#define AP_TO_SCP_OS_MEM_BASE      (0x32a10000 + 0x1400)

//
//shared memroy base
//
#define SCP_TO_AP_SHARED_MEM_BASE  (0x32a10000 + 0x800)
#define AP_TO_SCP_SHARED_MEM_BASE  (0x32a10000 + 0xC00)

#define SCMI_MSG_CREATE_E(Protocol,  MsgId, Token)   \
    ((((Protocol) & 0xff) << 10) |  \
    (((MsgId) & 0xff) << 0) |      \
    (((Token) & 0x3ff) << 18))

typedef struct  _MONTH_DESCRIPTION {
  CONST CHAR8* MonthStr;
  UINT32       MonthInt;
} MONTH_DESCRIPTION;

MONTH_DESCRIPTION gMonthDescription[] = {
  { "Jan", 1 },
  { "Feb", 2 },
  { "Mar", 3 },
  { "Apr", 4 },
  { "May", 5 },
  { "Jun", 6 },
  { "Jul", 7 },
  { "Aug", 8 },
  { "Sep", 9 },
  { "Oct", 10 },
  { "Nov", 11 },
  { "Dec", 12 },
  { "???", 1 },  // Use 1 as default month
};

typedef struct {
  UINT32 BaseConfig[4];
  UINT32 Equalization[4];
  UINT8  rev[72];
} PeuCtrT;

typedef struct {
  UINT32  Magic;
  UINT32  Version;
  UINT32  Size;
  UINT8   Rev1[4];
  UINT32  IndependentTree;
  UINT32  BaseCfg;
  UINT8   Rev2[16];
  PeuCtrT CtrCfg[2];
} PeuConfigT;

typedef struct MAILBOX_MEM_E {
  UINT32 ResA;
  volatile UINT32 Status;
  UINT64 ResB;
  UINT32 Flags;
  volatile UINT32 Len;
  UINT32 MsgHeader;
  UINT32 Payload[];
} MAILBOX_MEM_E_T;
VOID Pulldown(VOID);
VOID Alarm(UINTN   ErrorType);

EFI_STATUS
EFIAPI
InitializePlatformPeim (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  );

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  );

//
// Module globals
//
CONST EFI_PEI_PPI_DESCRIPTOR  mPpiListBootMode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiMasterBootModePpiGuid,
  NULL
};

CONST EFI_PEI_PPI_DESCRIPTOR  mPpiListRecoveryBootMode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiBootInRecoveryModePpiGuid,
  NULL
};

#if 0
UINT16
GetMaxFreq (
  VOID
  )
{
  UINT64              I2CBaseAddress;
  UINT32              I2CSpeed;
  UINT8               Buffer[SPD_NUM] = {0};
  UINT16              Temp;
  UINT32              SlaveAddress[2]= {0x50, 0x51};
  UINT8               i;
  UINT16              Temp2;
  Temp2 = 0xffff;
  I2CBaseAddress = PcdGet64 (PcdSpdI2cControllerBaseAddress);
  I2CSpeed = PcdGet32 (PcdSpdI2cControllerSpeed);
  //
  // Initialize I2C Bus which contain SPD
  //
  for (i = 0; i < 1; i++){
    i2c_init(I2CBaseAddress, I2CSpeed, SlaveAddress[i]);
    Temp = i2c_read(SlaveAddress[i], 0, 1, Buffer, 256);
    if (Temp == 0) {
      //Temp = (Buffer[18] *125 +Buffer[125] *1);
      //DEBUG((DEBUG_INFO,"(Buffer[18] *125 +Buffer[125] *1) is %d.\n",Temp));
      Temp = (1000000/(Buffer[18] *125 +(INT8)Buffer[125] *1))/2;
      DEBUG((DEBUG_INFO,"Temp is %d.\n",Temp));
      if (Temp == 666){
           Temp = 667;
      }else if(Temp == 466){
         Temp = 467;
      }

      DEBUG((DEBUG_INFO,"Temp is %d----.\n",Temp));
      if (Temp < Temp2){
        Temp2 = Temp;
      }
    }else{
       DEBUG((DEBUG_ERROR,"Get the DDR Max Freq fail.\n"));
    }
  }
  return Temp2;
}
#endif

/**

**/
EFI_STATUS
CreatDdrTrainInfoHob (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT64        DdrInfoSrc;
  UINT64        DdrInfoSize;

  //
  //Get Ddr Training Info from pbf
  //
  ZeroMem (&ArmSmcArgs, sizeof(ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = CPU_GET_MEM_INFO;
  ArmCallSmc (&ArmSmcArgs);
  DEBUG ((DEBUG_INFO, "ArmSmcArgs.Arg0 : %llx\n", ArmSmcArgs.Arg0));
  if ((0 == ArmSmcArgs.Arg0) || (-1 == (INT64) ArmSmcArgs.Arg0)) {
    DEBUG ((DEBUG_ERROR, "Get ddr training info from pbf failed!\n"));
    return EFI_NOT_FOUND;
  }
  else {
    DdrInfoSrc = ArmSmcArgs.Arg0;
    DdrInfoSize = MmioRead64 (ArmSmcArgs.Arg0);
    DEBUG ((DEBUG_INFO, "Ddr Info Size : %d, Addr : %x\n", DdrInfoSize, DdrInfoSrc));
    BuildGuidDataHob (&gDdrTrainInfoAddrHobGuid, &DdrInfoSrc, sizeof (UINT64));
    return EFI_SUCCESS;
  }
  return EFI_SUCCESS;
}

/**


**/
BOOLEAN
CheckTrainInfo (
  UINT64  Addr
  )
{
  UINT64  Temp;
  Temp = MmioRead32 (Addr);
  DEBUG ((DEBUG_INFO, "Ddr Info check : %x\n", Temp));
  if (Temp == DDR_TRAIN_INFO_CHECK) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

UINT32
GetResetSource (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  SetMem ((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
  ArmSmcArgs.Arg0 = CPU_GET_RST_SOURCE;
  ArmCallSmc (&ArmSmcArgs);

  return (UINT32)(ArmSmcArgs.Arg0);
}

VOID
PllInit (
  IN UINT16   MaxFrequency
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  EFI_STATUS    Status;
  UINT8         Buffer[0x100];
  PLL_CONFIG    *PllConfigData;
  UINT8         Buffer1[0x100];
  //DDR_CONFIG    *DdrConfigData;

  GetParameterInfo(PM_DDR, Buffer1, sizeof(Buffer1));
  //DdrConfigData = (DDR_CONFIG *)Buffer1;

  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
  Status = GetParameterInfo(PM_PLL, Buffer, sizeof(Buffer));
  if(!EFI_ERROR(Status)) {

  } else {
    DEBUG((DEBUG_ERROR, "Get PLL Parameter Fail.\n"));
    while(1);
  }

  PllConfigData = (PLL_CONFIG *)(UINTN)Buffer;
  DEBUG ((DEBUG_INFO,
          "Performance Core Frequency is %d\n",
          PllConfigData->PerformanceCoreFreq));
  DEBUG ((DEBUG_INFO,
          "Efficiency Core Frequency is %d\n",
          PllConfigData->EfficiencyCoreFreq));
  DEBUG ((DEBUG_INFO,
          "LMU Frequency is %d\n",
          PllConfigData->LmuFreq));
  //
  //todo:
  //  1.max ddr freq from spd.
  //  2.pll config from setup.
  //
  //if (!(DdrConfigData->ForceSpdEnable)){
  //  DEBUG((DEBUG_INFO, "DdrConfigData->ForceSpdEnable is 0x%x \n",DdrConfigData->ForceSpdEnable));
  //  if (PllConfigData->DdrFrequency >= MaxFrequency){
  //    PllConfigData->DdrFrequency = MaxFrequency;
  //  }
  //}
  ArmSmcArgs.Arg0 = CPU_INIT_PLL;
  ArmSmcArgs.Arg1 = 0;
  ArmSmcArgs.Arg2 = (UINTN)PllConfigData;
  DEBUG ((DEBUG_INFO, "arg0 : %x\narg1 : %x\narg2 : %x\n", ArmSmcArgs.Arg0, ArmSmcArgs.Arg1, ArmSmcArgs.Arg2));
  ArmCallSmc (&ArmSmcArgs);
}

#if 0
peu_config_t const peu_base_info  = {
  .magic = PARAMETER_PCIE_MAGIC,
  .version = 0x2,
  .size = 0x100,
  .independent_tree = CONFIG_INDEPENDENT_TREE,
  .base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1<< 1)) << PEU1_OFFSET | \
          (CONFIG_PCI_PEU0 | (X1X1 << 1))),
    .ctr_cfg[0].base_config[0] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
  .ctr_cfg[0].base_config[1] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
    .ctr_cfg[1].base_config[0] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
  .ctr_cfg[1].base_config[1] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
  .ctr_cfg[1].base_config[2] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
  .ctr_cfg[1].base_config[3] = (RC_MODE << PEU_C_OFFSET_MODE) | (GEN3 << PEU_C_OFFSET_SPEED),
  .ctr_cfg[0].equalization[0] = 0x7,
  .ctr_cfg[0].equalization[1] = 0x7,
  .ctr_cfg[1].equalization[0] = 0x7,
  .ctr_cfg[1].equalization[1] = 0x7,
  .ctr_cfg[1].equalization[2] = 0x7,
  .ctr_cfg[1].equalization[3] = 0x7,
};
#endif

EFI_STATUS
EFIAPI
SearchAnyVariable(
  IN CONST CHAR16 *Name,
  IN CONST EFI_GUID *Guid,
  IN OUT VOID *Variable,
  IN UINTN *VariableSize)
{
  EFI_STATUS Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariablePpi;
  /*UINTN VariableSize = sizeof(OEM_CONFIG_DATA);*/

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&VariablePpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = VariablePpi->GetVariable(
             VariablePpi,
             Name,
             Guid,
             NULL,
             VariableSize,
             Variable
          );

  /*if(EFI_ERROR(Status)) {*/
      /*if(EFI_NOT_FOUND == Status) {*/
          /*return Status;*/
      /*} else {*/
        /*ASSERT_EFI_ERROR(Status);*/
      /*}*/
  /*}*/

  return Status;
}

void setup_config(UINT8 * Buffer){

    PARAMETER_PEU_CONFIG *ParameterPcieConfig;
    PCIE_CONFIG_DATA  PcieConfigData;
    BOOLEAN PcieConfigExist = FALSE;
   // ARM_SMC_ARGS  ArmSmcArgs;
    EFI_STATUS    Status;
    UINTN VarSize;

    UINT8 Peu0SplitMode, Peu1SplitMode;
    UINT8 Peu0C0Enable;
    UINT8 Peu0C1Enable;

    UINT8 Peu1C0Enable;
    UINT8 Peu1C1Enable;
    UINT8 Peu1C2Enable;
    UINT8 Peu1C3Enable;
      
    UINT8 Peu0C0LinkSpeed;
    UINT8 Peu0C1LinkSpeed;

    UINT8 Peu1C0LinkSpeed;
    UINT8 Peu1C1LinkSpeed;
    UINT8 Peu1C2LinkSpeed;
    UINT8 Peu1C3LinkSpeed;
    
    UINT8 Peu0C0EqualValue;
    UINT8 Peu0C1EqualValue;

    UINT8 Peu1C0EqualValue;
    UINT8 Peu1C1EqualValue;
    UINT8 Peu1C2EqualValue;
    UINT8 Peu1C3EqualValue;

    ParameterPcieConfig = (PARAMETER_PEU_CONFIG *)Buffer;

    VarSize = sizeof(PCIE_CONFIG_DATA);
    Status = SearchAnyVariable(VAR_PCIE_CONFIG_NAME, &gPcieConfigVarGuid,
    				  &PcieConfigData, &VarSize);
    if (!EFI_ERROR(Status))
         PcieConfigExist = TRUE;

    //get verabile from setup
    if(PcieConfigExist){
        
        Peu0SplitMode = PcieConfigData.PcieConfig[0].SplitMode;
        Peu1SplitMode = PcieConfigData.PcieConfig[1].SplitMode;

        Peu0C0Enable = PcieConfigData.PcieConfig[0].PcieControl[0].DisEnable;
        Peu0C1Enable = PcieConfigData.PcieConfig[0].PcieControl[1].DisEnable;

        Peu1C0Enable = PcieConfigData.PcieConfig[1].PcieControl[0].DisEnable;
        Peu1C1Enable = PcieConfigData.PcieConfig[1].PcieControl[1].DisEnable;
        Peu1C2Enable = PcieConfigData.PcieConfig[1].PcieControl[2].DisEnable;
        Peu1C3Enable = PcieConfigData.PcieConfig[1].PcieControl[3].DisEnable;

        Peu0C0LinkSpeed = PcieConfigData.PcieConfig[0].PcieControl[0].Speed;
        Peu0C1LinkSpeed = PcieConfigData.PcieConfig[0].PcieControl[1].Speed;

        Peu1C0LinkSpeed = PcieConfigData.PcieConfig[1].PcieControl[0].Speed;
        Peu1C1LinkSpeed = PcieConfigData.PcieConfig[1].PcieControl[1].Speed;
        Peu1C2LinkSpeed = PcieConfigData.PcieConfig[1].PcieControl[2].Speed;
        Peu1C3LinkSpeed = PcieConfigData.PcieConfig[1].PcieControl[3].Speed;

        Peu0C0EqualValue = PcieConfigData.PcieConfig[0].PcieControl[0].EqualValue;
        Peu0C1EqualValue = PcieConfigData.PcieConfig[0].PcieControl[1].EqualValue;

        Peu1C0EqualValue = PcieConfigData.PcieConfig[1].PcieControl[0].EqualValue;
        Peu1C1EqualValue = PcieConfigData.PcieConfig[1].PcieControl[1].EqualValue;
        Peu1C2EqualValue = PcieConfigData.PcieConfig[1].PcieControl[2].EqualValue;
        Peu1C3EqualValue = PcieConfigData.PcieConfig[1].PcieControl[3].EqualValue;
        
        DEBUG((EFI_D_INFO, "PcieConfigExist:Peu1C2Enable:0x%x,0x%x\n", Peu1C2Enable,PcieConfigData.PcieConfig[1].PcieControl[2].DisEnable));
        DEBUG((EFI_D_INFO, "PcieConfigExist:Peu1C3Enable:0x%x,0x%x\n", Peu1C3Enable,PcieConfigData.PcieConfig[1].PcieControl[3].DisEnable));

        /********************PEU0************************/
        //clear splitmode field
        ParameterPcieConfig->FunctionConfig &= (~(0xf << PEU1_SPLITMODE_OFFSET));
        ParameterPcieConfig->FunctionConfig &= (~(0xf << PEU0_SPLITMODE_OFFSET));
        //set splitmode field
        ParameterPcieConfig->FunctionConfig |= (Peu0SplitMode << PEU0_SPLITMODE_OFFSET);
        ParameterPcieConfig->FunctionConfig |= (Peu1SplitMode << PEU1_SPLITMODE_OFFSET);
        DEBUG((EFI_D_INFO, "PcieConfigExist:ParameterPcieConfig:0x%x\n", ParameterPcieConfig->FunctionConfig));
        DEBUG((EFI_D_INFO, "PcieConfigExist:ParameterPcieConfig Buffer:0x%x,0x%x\n", Buffer[20],Buffer[22]));

        //clear enable bit
        ParameterPcieConfig->Peu0ControllerConfig[0] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu0ControllerConfig[0] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bit
        ParameterPcieConfig->Peu0ControllerConfig[0] |= ((Peu0C0Enable << ENABLE_OFFSET) | Peu0C0LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu0ControllerEqualization[0] = Peu0C0EqualValue;
        //clear enable bit
        ParameterPcieConfig->Peu0ControllerConfig[1] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu0ControllerConfig[1] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bit
        ParameterPcieConfig->Peu0ControllerConfig[1] |= ((Peu0C1Enable << ENABLE_OFFSET) | Peu0C1LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu0ControllerEqualization[1] = Peu0C1EqualValue;
        
        DEBUG((EFI_D_INFO, "PcieConfigExist:Peu0ControllerConfig0:0x%x, Peu0ControllerConfig1:0x%x,\n",
                    ParameterPcieConfig->Peu0ControllerConfig[0], ParameterPcieConfig->Peu0ControllerConfig[1]));


        /********************PEU1************************/
        //clear enable bit
        ParameterPcieConfig->Peu1ControllerConfig[0] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu1ControllerConfig[0] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bi1
        ParameterPcieConfig->Peu1ControllerConfig[0] |= ((Peu1C0Enable << ENABLE_OFFSET) | Peu1C0LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu1ControllerEqualization[0] = Peu1C0EqualValue;

        //clear enable bit
        ParameterPcieConfig->Peu1ControllerConfig[1] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu1ControllerConfig[1] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bi1
        ParameterPcieConfig->Peu1ControllerConfig[1] |= ((Peu1C1Enable << ENABLE_OFFSET) | Peu1C1LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu1ControllerEqualization[1] = Peu1C1EqualValue;

        //clear enable bit
        ParameterPcieConfig->Peu1ControllerConfig[2] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu1ControllerConfig[2] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bi1
        ParameterPcieConfig->Peu1ControllerConfig[2] |= ((Peu1C2Enable << ENABLE_OFFSET) | Peu1C2LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu1ControllerEqualization[2] = Peu1C2EqualValue;

        //clear enable bit
        ParameterPcieConfig->Peu1ControllerConfig[3] &= (~(0x1 << ENABLE_OFFSET));
        //clear speed fieled
        ParameterPcieConfig->Peu1ControllerConfig[3] &= (~(0xff << SPEED_OFFSET));
        //set enable && speed bi1
        ParameterPcieConfig->Peu1ControllerConfig[3] |= ((Peu1C3Enable << ENABLE_OFFSET) | Peu1C3LinkSpeed);
        //set eq
        ParameterPcieConfig->Peu1ControllerEqualization[3] = Peu1C3EqualValue;
        DEBUG((EFI_D_INFO, "PcieConfigExist:Peu1ControllerConfig0:0x%x, Peu1ControllerConfig1:0x%x, Peu1ControllerConfig2:0x%x?¨º?Peu1ControllerConfig3:0x%x\n",
                ParameterPcieConfig->Peu1ControllerConfig[0], ParameterPcieConfig->Peu1ControllerConfig[1], ParameterPcieConfig->Peu1ControllerConfig[2],ParameterPcieConfig->Peu1ControllerConfig[3]));


    }
    
}

VOID
PcieInit(
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  EFI_STATUS    Status;
  UINT8         Buffer[0x100];
  PeuConfigT    *Ptr;
  UINT8         GpioIndex;
  UINT8         GpioPort;
  UINT8         Detect;

  //
  //todo:
  //  1.Get pcie config from setup variable.
  //
  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
//#ifdef  NO_PARTABLE
#if 0
  UINT32        BoardType;
  BoardType = FixedPcdGet32 (PcdPhytiumBoardType);
  DEBUG ((DEBUG_INFO, "Phytium Board Type : %d\n", BoardType));
  CopyMem (Buffer, (VOID*)&peu_base_info, sizeof (peu_config_t));
  Status = pcie_cfg_sel (Buffer, BoardType);
#else
  Status = GetParameterInfo(PM_PCIE, Buffer, sizeof(Buffer));
  if(!EFI_ERROR(Status)) {
  } else {
    DEBUG((DEBUG_ERROR, "Get PCIE Parameter Fail.\n"));
    while(1);
  }
#endif
  //
  //Sata0 and PCIe auto detect
  //
  Ptr = (PeuConfigT *)Buffer;
  if (PcdGetBool (PcdSata0PcieAutoDetectEnableEnable)) {
    GpioIndex = (PcdGet8 (PcdSata0PcieAutoDetectPort) >> 4) & 0xF;
    GpioPort = (PcdGet8 (PcdSata0PcieAutoDetectPort) >> 0) & 0xF;
    Detect = 0;
    Status = DirectGetGpioInputLevel (GpioIndex, GpioPort, &Detect);
    DEBUG ((DEBUG_INFO, "Sata0 and PCIe audo detect:\n"));
    DEBUG ((DEBUG_INFO,
            "Inde : %d, Port : %d, Detect : %d, Status : %r",
            GpioIndex, GpioPort, Detect, Status
            ));
    DEBUG ((DEBUG_INFO, "Peu0 Config before : %x\n", Ptr->BaseCfg));
    if (!EFI_ERROR (Status)) {
      if (Detect == AUTO_DETECT_SATA) {
        Ptr->BaseCfg |= BIT1;
      } else if (Detect == AUTO_DETECT_PCIE) {
        Ptr->BaseCfg &= ~(BIT1);
      }
      DEBUG ((DEBUG_INFO, "Peu0 Config after : %x\n", Ptr->BaseCfg));
    }
  }

  setup_config(Buffer);
  ArmSmcArgs.Arg0 = CPU_INIT_PCIE;
  ArmSmcArgs.Arg1 = 0;
  ArmSmcArgs.Arg2 = (UINTN)Buffer;
  ArmCallSmc (&ArmSmcArgs);

  if(ArmSmcArgs.Arg0 !=0) {
    DEBUG((DEBUG_INFO, "PCIe Init fail.\n"));
    ASSERT(FALSE);
  }
}

VOID
DdrInit(
  IN UINT32  S3Flag
  )
{
  UINT8         Buffer[0x100];
  UINT8         Index;
  ARM_SMC_ARGS  ArmSmcArgs;
  EFI_STATUS    Status;
  UINT64        DdrInfoAddr;

  DdrInfoAddr =  PcdGet64 (PcdDdrTrainInfoSaveBaseAddress);
  DEBUG ((DEBUG_INFO, "Pcd DdrInfoAddr : %x\n", DdrInfoAddr));
  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
#ifdef  NO_PARTABLE
  UINT32        BoardType;
  BoardType = FixedPcdGet32 (PcdPhytiumBoardType);
  ZeroMem (Buffer, sizeof (Buffer));
  DEBUG ((DEBUG_INFO, "Mcu config in Code\n"));
  Status = ddr_cfg_sel (Buffer, BoardType);
#else
  DDR_CONFIG    *DdrConfigData;
  Status = GetParameterInfo (PM_DDR, Buffer, sizeof(Buffer));
  if(!EFI_ERROR(Status)) {
    DdrConfigData = (DDR_CONFIG *)Buffer;
    GetDdrConfigParameter(DdrConfigData, S3Flag);
  } else {
    DEBUG((DEBUG_ERROR, "Get DDR Parameter Fail.\n"));
    while(1);
  }
#endif
  DEBUG((DEBUG_INFO, "%a() Line=%d\n", __FUNCTION__, __LINE__));
  ArmSmcArgs.Arg0 = CPU_INIT_DDR;
  ArmSmcArgs.Arg1 = DDR_INIT;
  ArmSmcArgs.Arg2 = (UINTN)Buffer;
  if (S3Flag) {
    if (!CheckTrainInfo (DdrInfoAddr)) {
      DEBUG ((DEBUG_ERROR, "Check Train Info Failed!\n"));
    }
    ArmSmcArgs.Arg3 = (UINTN)DdrInfoAddr + 4;
  }
  ArmCallSmc (&ArmSmcArgs);

  if (0 != ArmSmcArgs.Arg0) {
    DEBUG ((DEBUG_ERROR, "Error X0:0x%x, X1:0x%x\n", ArmSmcArgs.Arg0, ArmSmcArgs.Arg1));
    for (Index = 0; Index < 8; Index++) {
      if (((ArmSmcArgs.Arg1 >> 24) & 0xff) & (1 << Index)) {
        DEBUG((DEBUG_ERROR, "Chanle %d ", Index));
        break;
      }
    }
    switch (ArmSmcArgs.Arg1 & 0xffffff) {
      case 0x1:
        DEBUG((DEBUG_ERROR, "Traning Fail!\n"));
        //Alarm(ERROR_TYPE1);
        break;
      default:
        DEBUG((DEBUG_ERROR, "Error Code: 0x%x\n", ArmSmcArgs.Arg1 & 0xffffff));
        break;
    }
    while (1);
  }
  //
  //Get Ddr training information and creat hob.
  //
  CreatDdrTrainInfoHob ();

  if(S3Flag) {
    MillisecondDelay (10);
    EnableVtt();
    MillisecondDelay (40);
    SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
    ArmSmcArgs.Arg0 = CPU_INIT_DDR;
    ArmSmcArgs.Arg1 = DDR_SUSPEND_UNLOCK;
    ArmCallSmc (&ArmSmcArgs);
  }
}

VOID
TzcAesInit (
  VOID
  )
{
  //tzc aes init
  UINT8                Buff[0x100];
  ARM_SMC_ARGS         ArmSmcArgs;
  UINT64               SecureAddr;

  DEBUG((DEBUG_INFO,"Tzc aes init begin\n"));
  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
  ArmSmcArgs.Arg0 = CPU_INIT_DDR;
  ArmSmcArgs.Arg1 = TAZ_AES_INIT;
  ArmSmcArgs.Arg2 = (UINTN)Buff;
  SecureAddr = O_PARAMETER_BASE + PM_SECURE_OFFSET;

  CopyMem(Buff, (VOID *)SecureAddr, sizeof(SECURE_BASE_INFO));
  ArmCallSmc (&ArmSmcArgs);
  if (0 != ArmSmcArgs.Arg0){
    DEBUG((DEBUG_ERROR,"read TZC AES config error in Parameter\n"));
  }
}

/*
* S3Flag = 0, power on
* S3Flag = 1, Recover from S3
*/
VOID
CpuRelocate(
  IN UINT8 S3Flag
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT8         Buffer[0x100];
  EFI_STATUS    Status;

  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);
  Status = GetParameterInfo(PM_COMMON, Buffer, sizeof(Buffer));
  if(!EFI_ERROR(Status)) {

  } else {
    DEBUG((DEBUG_ERROR, "Get Common Parameter Fail.\n"));
    while (1);
  }

  ArmSmcArgs.Arg0 = CPU_RELOCATE;
  ArmSmcArgs.Arg1 = S3Flag;
  ArmSmcArgs.Arg2 = (UINTN)Buffer;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 !=0) {
    DEBUG((DEBUG_INFO, "Cpu Relocate fail.\n"));
    ASSERT(FALSE);
  }
}

VOID
GetReleaseTime (
  EFI_TIME *Time
  )
{
  CONST CHAR8  *ReleaseDate = __DATE__;
  CONST CHAR8  *ReleaseTime = __TIME__;
  UINTN        Index;

  for (Index = 0; Index < 12; Index++) {
    if (0 == AsciiStrnCmp (ReleaseDate, gMonthDescription[Index].MonthStr, 3)) {
      break;
    }
  }
  Time->Month = gMonthDescription[Index].MonthInt;
  Time->Day = AsciiStrDecimalToUintn (ReleaseDate + 4);
  Time->Year = AsciiStrDecimalToUintn (ReleaseDate + 7);
  Time->Hour = AsciiStrDecimalToUintn (ReleaseTime);
  Time->Minute = AsciiStrDecimalToUintn (ReleaseTime + 3);
  Time->Second = AsciiStrDecimalToUintn (ReleaseTime + 6);

  return;
}
/*++

Routine Description:
Arguments:

  FileHandle  - Handle of the file being invoked.
  PeiServices - Describes the list of possible PEI Services.

Returns:

  Status -  EFI_SUCCESS if the boot mode could be set

--*/
EFI_STATUS
EFIAPI
InitializePlatformPeim (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                    Status;
  EFI_BOOT_MODE                 BootMode;
  UINT32                        S3Flag;
  UINT16                        MaxDdrFrequency;
  CHAR8                         Buffer[100];
  UINTN                         CharCount;
  EFI_TIME                      Time = {0};
  INT32                         ScmiReturn;
  SCMI_ADDRESS_BASE             Base;

  Base.MhuConfigBase = PcdGet64 (PcdMhuConfigBaseAddress);
  Base.MhuBase = PcdGet64 (PcdMhuBaseAddress);
  Base.ShareMemoryBase = PcdGet64 (PcdMhuShareMemoryBase);
  //
  //version and build time
  //
  CONST CHAR16 *ReleaseString = (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVersionString);
  GetReleaseTime (&Time);
  CharCount = AsciiSPrint (
    Buffer,
    sizeof (Buffer),
    "UEFI Version %s %t\n",
    ReleaseString,
    &Time
    );
  SerialPortWrite ((UINT8 *) Buffer, CharCount);

  MaxDdrFrequency =0;

  DEBUG ((DEBUG_LOAD | DEBUG_INFO, "Platform PEIM Loaded\n"));

  Status = PeiServicesSetBootMode (ArmPlatformGetBootMode ());
  ASSERT_EFI_ERROR (Status);

  PlatformPeim ();

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesInstallPpi (&mPpiListBootMode);
  ASSERT_EFI_ERROR (Status);

  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    Status = PeiServicesInstallPpi (&mPpiListRecoveryBootMode);
    ASSERT_EFI_ERROR (Status);
  }

  //
  //  read spd to get MaxFreq
  //
  //MaxDdrFrequency = GetMaxFreq();
  //
  //Cppc switch
  //
  if (PcdGetBool (PcdCppcFunctionEnable) == TRUE) {
    DEBUG ((DEBUG_INFO, "Enable Cppc!\n"));
    Status = ScmiEnableCppc (&Base, 1, &ScmiReturn);
    if ((ScmiReturn != SCMI_SUCCESS) || (EFI_ERROR (Status))) {
      DEBUG ((DEBUG_ERROR, "Enable Cppc Function Failed!\n"));
    }
  } else {
    DEBUG ((DEBUG_INFO, "Disable Cppc!\n"));
    Status = ScmiEnableCppc (&Base, 0, &ScmiReturn);
    if ((ScmiReturn != SCMI_SUCCESS) || (EFI_ERROR (Status))) {
      DEBUG ((DEBUG_ERROR, "Disable Cppc Function Failed!\n"));
    }
  }
  LsdDmaChannelConfig ();
   if (PcdGetBool (PcdPhytiumQosConfig)) {
       QosConfig(0x1000000,0x0c070000);//set LSD qos
   }
  //
  //Pad Config
  //
  if (PcdGetBool (PcdPhytiumPadTableEnable)) {
    ConfigPad ();
  } else {
    ConfigPadWithBoardType (FixedPcdGet32 (PcdPhytiumBoardType));
  }
  MioConfigWithParTable ();
  PllInit(MaxDdrFrequency);
#if 1
  S3Flag = GetS3Flag();
  DEBUG ((DEBUG_INFO, "S3Flag : %d\n", S3Flag));
  CleanS3Flag();
#else
  S3Flag = 0;
#endif
  PcieInit();
  DdrInit(S3Flag);
  PhyConfigWithParTable ();
  TzcAesInit();
  CpuRelocate(S3Flag);
  return Status;
}
