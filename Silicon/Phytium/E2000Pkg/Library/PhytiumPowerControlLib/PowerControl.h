#ifndef __POWERCONTROL__
#define __POWERCONTROL__
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/PhytiumPowerControlLib.h>

#define TICKS_PER_MS   48000
#define PM_GPIO  0
#define PM_EC    1
#define PM_SE    2

//
//PAD
//
#define PAD_BASE          0x32B30000

//
//GPIO
//
#define GPIO0_BASE        0x28034000
#define GPIO1_BASE        0x28035000
#define GPIO2_BASE        0x28036000
#define GPIO3_BASE        0x28037000
#define GPIO4_BASE        0x28038000
#define GPIO5_BASE        0x28039000
#define GPIO_SWPORTA_DR   0x00
#define GPIO_SWPORTA_DDR  0x04
#define GPIO_EXT_PORTA    0x08

#define DS_AG55           0x005c
#define DS_AG53           0x0060
#define DS_AE55           0x0064
/*
*  CPLD interface
*/
#if 0
#define PWR_CTR0_REG         0x28180480
#define PWR_CTR1_REG         0x28180484
#endif

#define S3_CLEAN_CPLD        1
#define REBOOT_CPLD          4
#define VTT_DISABLE_CPLD     5
#define VTT_ENABLE_CPLD      6
#define S3_SETUP_CPLD        8
#define SHUTDOWN_CPLD        12

#define LPC_BASE                 0x20000000
#define LPC_INT_APB_SPCE_CONF    (LPC_BASE + 0x7FFFFCC)
#define LPC_REG_LONG_TIMEOUT     (LPC_BASE + 0x7FFFFC8)
#define LPC_INT_STATE            (LPC_BASE + 0x7FFFFC4)
#define LPC_CLR_INT              (LPC_BASE + 0x7FFFFC0)
#define LPC_FIRMWARE_LEN_CONFG   (LPC_BASE + 0x7FFFFBC)
#define LPC_NU_SERIRQ_CONFIG     (LPC_BASE + 0x7FFFFB8)
#define LPC_ERROR_STT            (LPC_BASE + 0x7FFFFB4)
#define LPC_FIRMWR_ID_CONF_STRTB (LPC_BASE + 0x7FFFFB0)
#define LPC_DMA_CHNNLNU_CONF     (LPC_BASE + 0x7FFFFAC)
#define LPC_INT_MASK             (LPC_BASE + 0x7FFFFA8)
#define LPC_START_CYCLE_REG      (LPC_BASE + 0x7FFFFA4)
#define LPC_MEM_HIGHBIT_ADDR     (LPC_BASE + 0x7FFFFA0)
#define LPC_CLK_LPC_RSTN_O       (LPC_BASE + 0x7FFFF9C)
#define PM2_CMD                  0x66
#define PM2_DATA                 0x62
#define PM2_CMD_R                0x80
#define PM2_CMD_W                0x81
#define EC_S3_FLAG_MAGIC         0x55
#define EC_S3S4_FLAG_MAGIC       0xAA

typedef struct PowerContrlFunction{
  VOID   (*Poweroff)(VOID);
  VOID   (*Reboot)(VOID);
  UINT32 (*GetS3Flag)(VOID);
  VOID   (*SetS3Flag)(VOID);
  VOID   (*CleanS3Flag)(VOID);
  VOID   (*EnableVtt)(VOID);
  VOID   (*DisableVtt)(VOID);
  //todo
  //VOID   (*AutoWakeUp)(AUTO_WAKE_UP_CONFIG_DATA *AutoWakeUpConfigData);
}PowerContrlFunctionTable;

/*GPIO*/
typedef struct PadInformation {
  UINT32 PadSelRegister;
  UINT32 BitOffSet;
  UINT32 GPIOSwportDDR;
  UINT32 GPIOExtPort;
  UINT32 Port;
} PadInformationTable;

#if 0
#ifdef CONFIG_CHANGCHENG_XINAN
PadInformationTable S3FlagGPIO = {
  .PadSelRegister = 0x2818020c,
  .BitOffSet      = 24,
  .GPIOSwportDDR  = GPIO1_BASE + SWPORTB_DDR_OFFSET,
  .GPIOExtPort    = GPIO1_BASE + EXT_PORTB_OFFSET,
  .Port           = 3,
};
#else
PadInformationTable S3FlagGPIO = {
  .PadSelRegister = 0x28180200,
  .BitOffSet      = 20,
  .GPIOSwportDDR  = GPIO0_BASE + SWPORTA_DDR_OFFSET,
  .GPIOExtPort    = GPIO0_BASE + EXT_PORTA_OFFSET,
  .Port           = 1,
};
#endif
#endif

VOID
CpldPowerOff(
  VOID
  );

VOID
CpldReboot(
  void
  );

VOID
EcPowerOff(
  VOID
  );

VOID
EcReboot(
  VOID
  );

VOID
SePowerOff(
  VOID
  );

VOID
SeReboot(
  VOID
  );


UINT32
CPLDGetS3Flag(
  VOID
  );

VOID
CPLDSetS3Flag(
  VOID
  );

VOID
CPLDCleanS3Flag(
  VOID
  );

VOID
CPLDEnableVtt(
  VOID
  );

VOID
CPLDDisableVtt(
  VOID
  );

#if 0
VOID
CPLDAutoWakeUp(
  AUTO_WAKE_UP_CONFIG_DATA   *AutoWakeUpConfigData
  );
#endif

UINT32
EcGetS3Flag(
  VOID
  );

VOID
EcSetS3Flag(
  VOID
  );

VOID
EcCleanS3Flag(
  VOID
  );

VOID
EcEnableVtt(
  VOID
  );

VOID
EcDisableVtt(
  VOID
  );

#if 0
VOID
EcAutoWakeUp(
  AUTO_WAKE_UP_CONFIG_DATA   *AutoWakeUpConfigData
  );
#endif

STATIC PowerContrlFunctionTable PowerContrlCPLDFunctionTable = {
  .Poweroff    = CpldPowerOff,
  .Reboot      = CpldReboot,
  .GetS3Flag   = CPLDGetS3Flag,
  .SetS3Flag   = CPLDSetS3Flag,
  .CleanS3Flag = CPLDCleanS3Flag,
  .EnableVtt   = CPLDEnableVtt,
  .DisableVtt  = CPLDDisableVtt,
  //
  //todo
  //
  //.AutoWakeUp  = CPLDAutoWakeUp,
};

//
//maybe todo
//
STATIC PowerContrlFunctionTable PowerContrlEcFunctionTable = {
  .Poweroff    = EcPowerOff,
  .Reboot      = EcReboot,
  .GetS3Flag   = EcGetS3Flag,
  .SetS3Flag   = EcSetS3Flag,
  .CleanS3Flag = EcCleanS3Flag,
  .EnableVtt   = EcEnableVtt,
  .DisableVtt  = EcDisableVtt,
  //.AutoWakeUp  = EcAutoWakeUp,
};

#if 0
STATIC PowerContrlFunctionTable PowerContrlSeFunctionTable = {
  .Poweroff    = SePowerOff,
  .Reboot      = SeReboot,
  .GetS3Flag   = SeGetS3Flag,
  .SetS3Flag   = SeSetS3Flag,
  .CleanS3Flag = SeCleanS3Flag,
  .EnableVtt   = SeEnableVtt,
  .DisableVtt  = SeDisableVtt,
  .AutoWakeUp  = SeAutoWakeUp,
};
#endif

#endif
