#ifndef  _S3_RESUME_H_
#define  _S3_RESUME_H_

#include <Uefi.h>

#define PWR_CTR0_REG            0x28180480
#define PWR_CTR1_REG            0x28180484

#define S3_CLEAN_CPLD           1
#define REBOOT_CPLD             4
#define S3_SETUP_CPLD           8
#define SHUTDOWN_CPLD           12

#define TICKS_PER_MS            48000

#define PFDI_REGISTER           0xc2000012
#define PFDI_DONE               0xc2000013
//Phy
#define PHY_INVALID_ID                        0xFFFF
#define GMACBASEADDRESS0                      0x2820c000
#define GMACBASEADDRESS1                      0x28210000

#define PHY_ID1                               2
#define PHY_ID2                               3

#define MIIADDRSHIFT                          (11)
#define MIIREGSHIFT                           (6)
#define MII_REGMSK                            (0x1F << 6)
#define MII_ADDRMSK                           (0x1F << 11)

#define DW_EMAC_GMACGRP_GMII_ADDRESS_OFST                            0x010
#define DW_EMAC_GMACGRP_GMII_DATA_OFST                               0x014
#define DW_EMAC_GMACGRP_GMII_ADDRESS_GB_GET(value)                  (((value) & 0x00000001) >> 0)
#define DW_EMAC_GMACGRP_GMII_DATA_GD_GET(value)                     (((value) & 0x0000ffff) >> 0)

// HPS MII
#define MII_BUSY                              (1 << 0)
#define MII_WRITE                             (1 << 1)
#define MII_CLKRANGE_60_100M                  (0x0)
#define MII_CLKRANGE_100_150M                 (0x4)
#define MII_CLKRANGE_20_35M                   (0x8)
#define MII_CLKRANGE_35_60M                   (0xC)
#define MII_CLKRANGE_150_250M                 (0x10)
#define MII_CLKRANGE_250_300M                 (0x14)

typedef struct  {
  UINTN SystemOffEntry;
  UINTN SystemResetEntry;
  UINTN SuspendStartEntry;
  UINTN SuspendEndEntry;
  UINTN SuspendFinishEntry;
} PfdiVectors;

EFI_STATUS
S3FuncRegister (
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
S3SystemOffEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SystemResetEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendStartEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendFinishEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

VOID
S3SuspendEndEntry (
  UINTN PfdiNum,
  UINTN GdBase
  );

#endif
