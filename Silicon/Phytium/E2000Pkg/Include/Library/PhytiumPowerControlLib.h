#ifndef __POWERCONTROL__
#define __POWERCONTROL__
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

VOID
SystemOff(
  VOID
  );

VOID
SystemReboot(
  VOID
  );

UINT32
GetS3Flag(
  VOID
  );

VOID
SetS3Flag(
  VOID
  );

VOID
CleanS3Flag(
  VOID
  );

VOID
EnableVtt(
  VOID
  );

VOID
DisableVtt(
  VOID
  );

VOID
MillisecondDelay(
  UINT32 Time
  );

UINT32
GetPowerContrlSource(
  VOID
  );

#endif
