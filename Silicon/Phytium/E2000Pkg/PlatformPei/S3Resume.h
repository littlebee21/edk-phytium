#ifndef  _S3_RESUME_H_
#define  _S3_RESUME_H_

#define PWR_CTR0_REG	0x28180480
#define PWR_CTR1_REG	0x28180484

#define S3_CLEAN_CPLD      1
#define REBOOT_CPLD        4
#define S3_SETUP_CPLD      8
#define SHUTDOWN_CPLD      12

#define TICKS_PER_MS       48000

#define PFDI_REGISTER      0xc2000012
#define PFDI_DONE          0xc2000013

typedef struct  {
  UINTN SystemOffEntry;
  UINTN SystemResetEntry;
  UINTN SuspendStartEntry;
  UINTN SuspendEndEntry;
  UINTN SuspendFinishEntry;
} PfdiVectors;

EFI_STATUS
S3FuncRegister(
  VOID
);

VOID
AsmS3SystemOffEntry (
  VOID
  );

VOID
AsmS3SystemResetEntry (
  VOID
  );

VOID
AsmS3SuspendStartEntry (
  VOID
  );

VOID
AsmS3SuspendFinishEntry (
  VOID
  );

VOID
AsmS3SuspendEndEntry (
  VOID
  );

VOID
S3SystemOffEntry(
  UINTN pfdi_mum,
  UINTN gd_base
  );

VOID
S3SystemResetEntry(
  UINTN pfdi_mum,
  UINTN gd_base
  );

VOID
S3SuspendStartEntry(
  UINTN pfdi_mum,
  UINTN gd_base
  );

VOID
S3SuspendFinishEntry(
  UINTN pfdi_mum,
  UINTN gd_base
  );

VOID
S3SuspendEndEntry(
  UINTN pfdi_mum,
  UINTN gd_base
  );

#endif
