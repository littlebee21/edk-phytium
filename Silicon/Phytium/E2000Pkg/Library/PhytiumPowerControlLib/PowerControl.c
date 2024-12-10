#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/ParameterTable.h>
#include <Library/TimerLib.h>
#include <Library/EcLib.h>
#include <OEMSvc.h>

#include "PowerControl.h"

UINTN
CountTicks(
  VOID
  )
{
  UINTN Count;

  asm volatile("mrs %0, cntpct_el0" : "=r" (Count));

  return Count;
}

VOID
MillisecondDelay(
  UINT32 Time
  )
{
  UINTN Start;
  UINTN End;

  Start = CountTicks();
  End = Start + Time * TICKS_PER_MS;
  while (CountTicks() <= End) {
  }

}

VOID
SendCpldCommand(
  UINT32 Command
  )
{
  DEBUG ((DEBUG_INFO, "Send cmd to cpld : %d \n", Command));
  UINT32 Ret;
  UINT32 Cfg;
  UINT32 PortLevel;
  UINT32 Port;
  UINT32 High;
  UINT32 Low;
  UINT32 Index;

  PortLevel = 0;
  Port = 0;
  //
  //gpio pad
  //
  Ret = MmioRead32 (PAD_BASE + DS_AG55);//gpio1_0:func5
  Ret |= (1 << 0);
  Ret &= ~(1 << 1);
  Ret |= (1 << 2);
  MmioWrite32 (PAD_BASE + DS_AG55, Ret);

  Ret = MmioRead32 (PAD_BASE + DS_AG53);//gpio1_1:func5
  Ret |= (1 << 0);
  Ret &= ~(1 << 1);
  Ret |= (1 << 2);
  MmioWrite32 (PAD_BASE + DS_AG53, Ret);

  //
  //send cpld
  //读取之前配置方向寄存器
  //
  Cfg = MmioRead32 (GPIO1_BASE + GPIO_SWPORTA_DDR);
  Cfg |= (1 << Port);
  Port = 1;
  Cfg |= (1 << Port);
  //配置为输出模式, 0:输入，1:输出
  MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DDR, Cfg);

  //发送电平值
  PortLevel = MmioRead32 (GPIO1_BASE + GPIO_SWPORTA_DR);
  Port = 0;
  PortLevel |= (1 << Port);
  MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DR, PortLevel);//start
  MillisecondDelay (2);

  Port = 1;
  PortLevel |= (1 << Port);
  High = PortLevel;
  PortLevel &= ~(1 << Port);
  Low  = PortLevel;

  for (Index = 0; Index < Command; Index++){
    MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DR, High);
    MillisecondDelay (1);
    MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DR, Low);
    MillisecondDelay (1);
  }
  MillisecondDelay (2);
  Port = 0;
  PortLevel &= ~(1 << Port);
  MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DR, PortLevel);//end
}

VOID
SendEcCommand(
  UINT32 Command
  )
{
  DEBUG((DEBUG_INFO,"send cmd to ec : %d \n", Command));
  MmioWrite32 (LPC_CLK_LPC_RSTN_O, 1);
  MmioWrite32 (LPC_NU_SERIRQ_CONFIG, 0x80000000);
  MmioWrite32 (LPC_INT_MASK, 0);
  MmioWrite32 (LPC_START_CYCLE_REG, 0);
  MillisecondDelay (50);
  if (Command == 4)
    EcWriteMem (0x0A,0x08);
  else if (Command == 8)
    EcWriteMem (0x0A,0x01);
  else if (Command == 12)
    EcWriteMem (0x0A,0x04);
  MillisecondDelay (50);
}

VOID
CpldPowerOff(
  VOID
  )
{
  SendCpldCommand(SHUTDOWN_CPLD);
}

VOID
CpldReboot(
  VOID
  )
{
  SendCpldCommand (REBOOT_CPLD);
}

VOID
EcPowerOff(
  VOID
  )
{
  SendEcCommand (SHUTDOWN_CPLD);
}

VOID
EcReboot(
  VOID
  )
{
  SendEcCommand (REBOOT_CPLD);
}

UINT32
CPLDGetS3Flag(
  VOID
  )
{
  UINT32 Ret;
  UINT32 Cfg;
  UINT32 Port;
  UINT32 PadRecv0;
  UINT32 CfgRecv;

  Port = 0;
  DEBUG ((DEBUG_INFO, "S3 get flash by gpio\n"));
  //gpio pad
  Ret = MmioRead32 (PAD_BASE + DS_AE55);//gpio1_2:func5
  PadRecv0 = Ret;
  Ret |= (1 << 0);
  Ret &= ~(1 << 1);
  Ret |= (1 << 2);
  MmioWrite32 (PAD_BASE + DS_AE55, Ret);
  MillisecondDelay (10);

  //读取之前配置方向寄存器
  Cfg = MmioRead32 (GPIO1_BASE + GPIO_SWPORTA_DDR);
  CfgRecv = Ret;
  //配置为输出模式, 0:输入，1:输出
  Port = 2;
  Cfg &= ~(1 << Port);
  MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DDR, Cfg);

  //读取当前控io寄存器的值
  Ret = MmioRead32 (GPIO1_BASE + GPIO_EXT_PORTA);

  //恢复(专属，不需要恢复)
  MmioWrite32 (GPIO1_BASE + GPIO_SWPORTA_DDR, CfgRecv);
  MmioWrite32 (PAD_BASE + DS_AE55, PadRecv0);

  //判断对应io引脚是否为高电平
  if((1 << Port) == Ret)
    return  1;
  else
    return  0;
}

VOID
CPLDSetS3Flag(
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 set flag by gpio\n"));
  SendCpldCommand(S3_SETUP_CPLD);
}

VOID
CPLDCleanS3Flag(
  VOID
  )
{
  DEBUG((DEBUG_INFO,"s3 clean flag by gpio\n"));
  SendCpldCommand(S3_CLEAN_CPLD);
}

VOID
CPLDEnableVtt(
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 enable vtt by gpio"));
  SendCpldCommand(VTT_ENABLE_CPLD);
}

VOID
CPLDDisableVtt(
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 disable vtt by gpio"));
  SendCpldCommand(VTT_DISABLE_CPLD);
}

//
//todo!
//
#if 0
VOID
CPLDAutoWakeUp(
  AUTO_WAKE_UP_CONFIG_DATA   *AutoWakeUpConfigData
)
{

}
#endif

UINT32
EcGetS3Flag (
  VOID
  )
{
  UINT8 EcFlag;

  EcFlag = 0;
  DEBUG((DEBUG_INFO,"s3 flag form ec\n"));
  MmioWrite32 (LPC_CLK_LPC_RSTN_O, 1);
  DEBUG ((DEBUG_INFO, "LPC_CLK_LPC_RSTN_O %x\n", LPC_CLK_LPC_RSTN_O));
  MmioWrite32 (LPC_NU_SERIRQ_CONFIG, 0x80000000);
  MmioWrite32 (LPC_INT_MASK, 0);
  MmioWrite32 (LPC_START_CYCLE_REG, 0);
  MillisecondDelay (500);
  EcReadMem (0x0B,&EcFlag);
  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  if (EC_S3_FLAG_MAGIC == EcFlag || EC_S3S4_FLAG_MAGIC == EcFlag) {
    return 1;
  } else {
    return 0;
  }
}

VOID
EcSetS3Flag (
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 set flag by ec\n"));
  MmioWrite32 (LPC_CLK_LPC_RSTN_O, 1);
  MmioWrite32 (LPC_NU_SERIRQ_CONFIG, 0x80000000);
  MmioWrite32 (LPC_INT_MASK, 0);
  MmioWrite32 (LPC_START_CYCLE_REG, 0);
  MillisecondDelay (500);
  EcWriteMem (0x0B, 0x55);
  MillisecondDelay (500);
  SendEcCommand (S3_SETUP_CPLD);
  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
//#if ((defined NOTEBOOK_V1) || (defined NOTEBOOK_V2))
//    SendSeCommand (S3_SETUP_CPLD);
//#endif
}

VOID
EcCleanS3Flag(
  VOID
  )
{

}

VOID
EcEnableVtt(
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 enable vtt by ec"));
  MmioWrite32 (LPC_CLK_LPC_RSTN_O, 1);
  MmioWrite32 (LPC_NU_SERIRQ_CONFIG, 0x80000000);
  MmioWrite32 (LPC_INT_MASK, 0);
  MmioWrite32 (LPC_START_CYCLE_REG, 0);
  MillisecondDelay (500);
  EcWriteMem (0x0A, 0x10);
}

VOID
EcDisableVtt(
  VOID
  )
{
  DEBUG((DEBUG_INFO, "s3 disable vtt by ec"));
  MmioWrite32 (LPC_CLK_LPC_RSTN_O, 1);
  MmioWrite32 (LPC_NU_SERIRQ_CONFIG, 0x80000000);
  MmioWrite32 (LPC_INT_MASK, 0);
  MmioWrite32 (LPC_START_CYCLE_REG, 0);
  MillisecondDelay (500);
  EcWriteMem (0x0A, 0x20);
}

#if 0
VOID
EcAutoWakeUp(
  AUTO_WAKE_UP_CONFIG_DATA   *AutoWakeUpConfigData
)
{

}
#endif

UINT32
GetPowerContrlSource(
  VOID
  )
{
  EFI_STATUS  Status;
  UINT8 Buffer[0x100];
  BOARD_CONFIG *BoardConfig;

  Status = GetParameterInfo(PM_BOARD, Buffer, sizeof(Buffer));
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Get Board Parameter Fail.\n"));
    while(1);
  }

  BoardConfig = (BOARD_CONFIG *)Buffer;
  if (PM_GPIO == BoardConfig->PowerManger) {
    DEBUG((DEBUG_INFO, "PowerManger is GPIO\n"));
  } else if (PM_EC == BoardConfig->PowerManger){
    DEBUG((DEBUG_INFO, "PowerManger is EC\n"));
  }
  return BoardConfig->PowerManger;
}

VOID *
GetFlagFunction (
  VOID
  )
{
  switch(GetPowerContrlSource()){
    case 0:
      return &PowerContrlCPLDFunctionTable;
    case 1:
      return &PowerContrlEcFunctionTable;
    default:
      return &PowerContrlCPLDFunctionTable;
  }

}

VOID
SystemOff(
  VOID
  )
{

  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->Poweroff){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->Poweroff();
}

VOID
SystemReboot(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();


  if (NULL == PowerContrl || NULL == PowerContrl->Reboot){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->Reboot();
}

UINT32
GetS3Flag(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->GetS3Flag){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }
  return (PowerContrl->GetS3Flag());
}

VOID
SetS3Flag(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->SetS3Flag){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->SetS3Flag();
}

VOID
CleanS3Flag(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->CleanS3Flag){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->CleanS3Flag();
}

VOID
EnableVtt(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->EnableVtt){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->EnableVtt();
}

VOID
DisableVtt(
  VOID
  )
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->DisableVtt){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->DisableVtt();
}

//
//todo!
//
#if 0
VOID
AutoWakeUp(
  AUTO_WAKE_UP_CONFIG_DATA   *AutoWakeUpConfigData
)
{
  PowerContrlFunctionTable *PowerContrl;
  PowerContrl = GetFlagFunction();

  if (NULL == PowerContrl || NULL == PowerContrl->AutoWakeUp){
    DEBUG((DEBUG_ERROR, "%a(),Line=%d\n", __FUNCTION__, __LINE__));
    while(1);
  }

  PowerContrl->AutoWakeUp(AutoWakeUpConfigData);
}
#endif
