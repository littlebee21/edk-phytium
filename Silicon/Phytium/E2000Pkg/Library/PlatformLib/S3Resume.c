#include <OEMSvc.h>
#include "S3Resume.h"

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/PhytiumPowerControlLib.h>
#include <Library/ScmiLib.h>

VOID
S3SystemOffEntry (
  UINTN PfdiNum,
  UINTN GdBase
  )
{
  SystemOff();
  while(1);
}

VOID
S3SystemResetEntry (
  UINTN PfdiNum,
  UINTN GdBase
  )
{
  SystemReboot();
  while(1);
}

VOID
S3SuspendStartEntry(
  UINTN PfdiNum,
  UINTN GdBase
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  ArmSmcArgs.Arg0 = PFDI_DONE;

  DEBUG ((DEBUG_INFO, "SuspendStartEntry pfdi_num : %lld , gd_base : %llx \n", PfdiNum, GdBase));
  ArmCallSmc (&ArmSmcArgs);
  while(1);
}

VOID
S3SuspendFinishEntry (
  UINTN PfdiNum,
  UINTN GdBase
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  ArmSmcArgs.Arg0 = PFDI_DONE;

  DEBUG((DEBUG_INFO, "S3SuspendFinishEntry pfdi_num : %lld , gd_base : %llx \n", PfdiNum, GdBase));
  ArmCallSmc (&ArmSmcArgs);
  while(1);
}

VOID
S3SuspendEndEntry (
  UINTN PfdiNum,
  UINTN GdBase
  )
{
  ARM_SMC_ARGS      ArmSmcArgs;
  EFI_STATUS        Status;
  INT32             ScmiStatus;
  SCMI_ADDRESS_BASE Base;

  Base.MhuConfigBase = PcdGet64 (PcdMhuConfigBaseAddress);
  Base.MhuBase = PcdGet64 (PcdMhuBaseAddress);
  Base.ShareMemoryBase = PcdGet64 (PcdMhuShareMemoryBase);

  Status = EFI_SUCCESS;
  ScmiStatus = SCMI_SUCCESS;
  SetMem((VOID*)&ArmSmcArgs, sizeof(ARM_SMC_ARGS), 0);

  ArmSmcArgs.Arg0 = 0xC2000F04;
  ArmSmcArgs.Arg1 = 0x1;
  ArmCallSmc (&ArmSmcArgs);

  MillisecondDelay (10);
  MillisecondDelay (40);
  ArmSmcArgs.Arg0 = 0xC2000F04;
  ArmSmcArgs.Arg1 = 0x2;
  ArmCallSmc (&ArmSmcArgs);
  Status = ScmiPowerStateS3S4S5 (&Base, 1, 0x3, 0, &ScmiStatus);
  if ((ScmiStatus != SCMI_SUCCESS) || (EFI_ERROR (Status))) {
    DEBUG ((DEBUG_ERROR, "Config S3S4S5 power status Failed!\n"));
  }

  SetS3Flag();
  while(1);
}

EFI_STATUS
S3FuncRegister (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  PfdiVectors   PfdiV;

  PfdiV.SystemOffEntry = (UINT64) AsmS3SystemOffEntry;
  DEBUG ((DEBUG_INFO, "PfdiV.SystemOffEntry : %lx\n", PfdiV.SystemOffEntry));

  PfdiV.SystemResetEntry = (UINT64) AsmS3SystemResetEntry;
  DEBUG ((DEBUG_INFO, "PfdiV.SystemResetEntry : %lx\n", PfdiV.SystemResetEntry));

  PfdiV.SuspendStartEntry = (UINT64) AsmS3SuspendStartEntry;
  DEBUG ((DEBUG_INFO, "PfdiV.SuspendStartEntry : %lx\n", PfdiV.SuspendStartEntry));

  PfdiV.SuspendEndEntry = (UINT64) S3SuspendEndEntry;
  DEBUG ((DEBUG_INFO, "PfdiV.SuspendEndEntry : %lx\n", PfdiV.SuspendEndEntry));

  PfdiV.SuspendFinishEntry = (UINT64) AsmS3SuspendFinishEntry;
  DEBUG ((DEBUG_INFO, "PfdiV.SuspendFinishEntry : %lx\n", PfdiV.SuspendFinishEntry));

  ArmSmcArgs.Arg0 = PFDI_REGISTER;
  ArmSmcArgs.Arg1 = (UINTN) &PfdiV;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 != 0) {
      DEBUG((DEBUG_INFO, "register pfdi fail.\n"));
      while(1);
  }
  return EFI_SUCCESS;
}
