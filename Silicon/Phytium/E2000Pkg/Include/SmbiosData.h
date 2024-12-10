/** @file
  Phytium E2000 SMBIOS Data Header File.

  Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef SMBIOSDATA_H_
#define SMBIOSDATA_H_

#include <Guid/DebugMask.h>

#include <IndustryStandard/SmBios.h>

#include <Library/ArmSmcLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <OEMSvc.h>
#include <PiDxe.h>
#include <Uefi.h>

#include <Protocol/Smbios.h>


extern UINT8 MemorySubClassStrings[];

#define CPU_MAX_SPEED 2200 // 2.2G
#define MAX_DIMM_SIZE 32 // In GB
#define OEM_SINGLE_SOCKET 1
#define OEM_DUAL_SOCKET 2
struct SPD_JEDEC_MANUFACTURER {
  UINT8 MfgIdLSB;
  UINT8 MfgIdMSB;
  CHAR8 *Name;
};

enum SMBIOS_REFRENCE_HANDLES {
  SMBIOS_HANDLE_BIOSINFOR = 0x1000,
  SMBIOS_HANDLE_SYSEMINFO,
  SMBIOS_HANDLE_L1I,
  SMBIOS_HANDLE_L1D,
  SMBIOS_HANDLE_L1I_FUNC,
  SMBIOS_HANDLE_L1D_FUNC,
  SMBIOS_HANDLE_L2,
  SMBIOS_HANDLE_L2_FUNC,
  SMBIOS_HANDLE_L3,
  SMBIOS_HANDLE_MOTHERBOARD,
  SMBIOS_HANDLE_CHASSIS,
  SMBIOS_HANDLE_CLUSTER,
  SMBIOS_HANDLE_CLUSTER_EX,
  SMBIOS_HANDLE_CLUSTER_FUNC,
  SMBIOS_HANDLE_CLUSTER_FUNC_EX,
  SMBIOS_HANDLE_MEMORY,
  SMBIOS_HANDLE_DIMM
};

#define TYPE0_STRINGS                                                          \
  "PHYTIUM LTD\0"   /* Vendor */                                               \
  "V2.0\0"          /* BiosVersion */                                          \
      __DATE__ "\0" /* BiosReleaseDate */

#define TYPE1_STRINGS                                                          \
  "PHYTIUM LTD\0" /* Manufacturer */                                           \
  "E2000\0"       /* Product Name */                                           \
  "Not Set\0"     /* Version */                                                \
  "Not Set\0"     /* SerialNumber */                                           \
  "Not Set\0"     /* SKUNumber */                                              \
  "ARMv8\0"       /* Family */

#define TYPE2_STRINGS                                                          \
  "PHYTIUM LTD\0" /* Manufacturer */                                           \
  "E2000\0"       /* Product Name */                                           \
  "Not Set\0"     /* Version */                                                \
  "Not Set\0"     /* Serial */                                                 \
  "Not Set\0"     /* BaseBoardAssetTag */                                      \
  "Not Set\0"     /* BaseBoardChassisLocation */

#define TYPE3_STRINGS                                                          \
  "PHYTIUM LTD\0" /* Manufacturer */                                           \
  "Not Set\0"     /* Version */                                                \
  "Not Set\0"     /* Serial  */                                                \
  "Not Set\0"     /* AssetTag  */                                              \
  "Not Set\0"     /* SKU Number  */

#define TYPE4_STRINGS_EFFICIENCY                                               \
  "Phytium E2000\0"         /* socket type */                                  \
  "PHYTIUM LTD\0"           /* manufactuer */                                  \
  "E2000 Efficiency Core\0" /* processor version */                            \
  "Not Set\0"               /* SerialNumber */                                 \
  "Not Set\0"               /* processor 2 description */                      \
  "Not Set\0"               /* AssetTag */

#define TYPE4_STRINGS_FUNCTION                                                 \
  "Phytium E2000\0"       /* socket type */                                    \
  "PHYTIUM LTD\0"         /* manufactuer */                                    \
  "E2000 Function Core\0" /* processor version */                              \
  "Not Set\0"             /* SerialNumber */                                   \
  "Not Set\0"             /* processor 2 description */                        \
  "Not Set\0"             /* AssetTag */

#define TYPE7_STRINGS                                                          \
  "L1 Instruction\0" /* L1I  */                                                \
  "L1 Data\0"        /* L1D  */                                                \
  "L2\0"             /* L2   */

#define TYPE7_L1DATA_STRINGS_EFFICIENCY                                        \
  "Efficiency Core L1 Data Cache\0" /* L1 data   */

#define TYPE7_L1INS_STRINGS_EFFICIENCY                                         \
  "Efficiency Core L1 Instruction Cache\0" /* L1 ins   */

#define TYPE7_L1DATA_STRINGS_FUNCTION                                          \
  "Function Core L1 Data Cache\0" /* L1 data   */

#define TYPE7_L1INS_STRINGS_FUNCTION                                           \
  "Function Core L1 Instruction Cache\0" /* L1 ins   */

#define TYPE7_L2_STRINGS_EFFICIENCY "Efficiency Core L2 Cache\0" /* L2   */

#define TYPE7_L2_STRINGS_FUNCTION "Function Core L2 Cache\0" /* L2   */

#define TYPE7_L3_STRINGS "L3 Cache\0" /* L3   */

#define TYPE9_STRINGS                                                          \
  "PCIE_SLOT0\0" /* Slot0 */                                                   \
  "PCIE_SLOT1\0" /* Slot1 */                                                   \
  "PCIE_SLOT2\0" /* Slot2 */                                                   \
  "PCIE_SLOT3\0" /* Slot3 */

#define TYPE9_STRINGS_PCIE0X16 "PCIE0_X16\0"

#define TYPE9_STRINGS_PCIE0X1 "PCIE0_X1\0"

#define TYPE9_STRINGS_PCIE1X16 "PCIE1_X16\0"

#define TYPE9_STRINGS_PCIE1X1 "PCIE1_X1\0"

#define TYPE13_STRINGS                                                         \
  "en|US|iso8859-1\0"                                                          \
  "zh|CN|unicode\0"

#define TYPE16_STRINGS "\0" /* nothing */

#define TYPE17_STRINGS                                                         \
  "DIMM1\0"                     /* location */                                 \
  "SOCKET 0 CHANNEL 1 DIMM 1\0" /* bank description */                         \
  "Kingston\0"                                                                 \
  "A6C900CF08\0"                                                               \
  "2018Mem\0"                                                                  \
  "6478545886\0"

#define TYPE19_STRINGS "\0" /* nothing */

#define TYPE32_STRINGS "\0" /* nothing */

#define TYPE39_STRINGS                                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/                                         \
  "Not specified\0" /* not specified*/

#define TYPE38_STRINGS "\0"

//
// Type definition and contents of the default SMBIOS table.
// This table covers only the minimum structures required by
// the SMBIOS specification (section 6.2, version 3.0)
//
#pragma pack(1)
typedef struct {
  SMBIOS_TABLE_TYPE0 Base;
  UINT8 Strings[sizeof(TYPE0_STRINGS)];
} ARM_TYPE0;

typedef struct {
  SMBIOS_TABLE_TYPE1 Base;
  UINT8 Strings[sizeof(TYPE1_STRINGS)];
} ARM_TYPE1;

typedef struct {
  SMBIOS_TABLE_TYPE2 Base;
  UINT8 Strings[sizeof(TYPE2_STRINGS)];
} ARM_TYPE2;

typedef struct {
  SMBIOS_TABLE_TYPE3 Base;
  UINT8 Strings[sizeof(TYPE3_STRINGS)];
} ARM_TYPE3;

typedef struct {
  SMBIOS_TABLE_TYPE4 Base;
  UINT8 Strings[sizeof(TYPE4_STRINGS_EFFICIENCY)];
} ARM_TYPE4_EFFICIENCY;

typedef struct {
  SMBIOS_TABLE_TYPE4 Base;
  UINT8 Strings[sizeof(TYPE4_STRINGS_FUNCTION)];
} ARM_TYPE4_FUNCTION;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L1DATA_STRINGS_EFFICIENCY)];
} ARM_TYPE7_L1DATA_EFFICIENCY;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L1INS_STRINGS_EFFICIENCY)];
} ARM_TYPE7_L1INS_EFFICIENCY;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L1DATA_STRINGS_FUNCTION)];
} ARM_TYPE7_L1DATA_FUNCTION;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L1INS_STRINGS_FUNCTION)];
} ARM_TYPE7_L1INS_FUNCTION;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L2_STRINGS_EFFICIENCY)];
} ARM_TYPE7_L2_EFFICIENCY;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L2_STRINGS_FUNCTION)];
} ARM_TYPE7_L2_FUNCTION;

typedef struct {
  SMBIOS_TABLE_TYPE7 Base;
  UINT8 Strings[sizeof(TYPE7_L3_STRINGS)];
} ARM_TYPE7_L3;

typedef struct {
  SMBIOS_TABLE_TYPE9 Base;
  UINT8 Strings[sizeof(TYPE9_STRINGS)];
} ARM_TYPE9;

typedef struct {
  SMBIOS_TABLE_TYPE9 Base;
  UINT8 Strings[sizeof(TYPE9_STRINGS_PCIE0X16)];
} ARM_TYPE9_PCIE0X16;

typedef struct {
  SMBIOS_TABLE_TYPE9 Base;
  UINT8 Strings[sizeof(TYPE9_STRINGS_PCIE0X1)];
} ARM_TYPE9_PCIE0X1;

typedef struct {
  SMBIOS_TABLE_TYPE9 Base;
  UINT8 Strings[sizeof(TYPE9_STRINGS_PCIE1X16)];
} ARM_TYPE9_PCIE1X16;

typedef struct {
  SMBIOS_TABLE_TYPE9 Base;
  UINT8 Strings[sizeof(TYPE9_STRINGS_PCIE1X1)];
} ARM_TYPE9_PCIE1X1;

typedef struct {
  SMBIOS_TABLE_TYPE13 Base;
  UINT8 Strings[sizeof(TYPE13_STRINGS)];
} ARM_TYPE13;

typedef struct {
  SMBIOS_TABLE_TYPE16 Base;
  UINT8 Strings[sizeof(TYPE16_STRINGS)];
} ARM_TYPE16;

typedef struct {
  SMBIOS_TABLE_TYPE17 Base;
  UINT8 Strings[sizeof(TYPE17_STRINGS)];
} ARM_TYPE17;

typedef struct {
  SMBIOS_TABLE_TYPE19 Base;
  UINT8 Strings[sizeof(TYPE19_STRINGS)];
} ARM_TYPE19;

typedef struct {
  SMBIOS_TABLE_TYPE32 Base;
  UINT8 Strings[sizeof(TYPE32_STRINGS)];
} ARM_TYPE32;

typedef struct {
  SMBIOS_TABLE_TYPE38 Base;
  UINT8 Strings[sizeof(TYPE38_STRINGS)];
} ARM_TYPE38;

typedef struct {
  SMBIOS_TABLE_TYPE39 Base;
  UINT8 Strings[sizeof(TYPE39_STRINGS)];
} ARM_TYPE39;

#endif
