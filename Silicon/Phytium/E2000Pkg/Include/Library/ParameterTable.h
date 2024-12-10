#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Uefi/UefiBaseType.h>
#include <Library/PcdLib.h>
//#define USE_PARAMETER             //采用读取参数表

#define O_PARAMETER_BASE            (FixedPcdGet64 (PcdParameterTableBase)) /*640k*/

#define PM_PLL_OFFSET               (0x0)
#define PM_PCIE_OFFSET              (0x100)
#define PM_COMMON_OFFSET            (0x200)
#define PM_DDR_OFFSET               (0x300)
#define PM_BOARD_OFFSET             (0x400)
#define PM_SECURE_OFFSET            (0x500)

#define PARAMETER_PLL_MAGIC                 0x54460020
#define PARAMETER_PCIE_MAGIC                0x54460021
#define PARAMETER_COMMON_MAGIC              0x54460013
#define PARAMETER_DDR_MAGIC                 0x54460024
#define PARAMETER_BOARD_MAGIC               0x54460015

#define DDR_TRAIN_INFO_CHECK                0x52441234
#define PEU1_SPLITMODE_OFFSET 16
#define PEU0_SPLITMODE_OFFSET 0

#define SPEED_OFFSET 0
#define ENABLE_OFFSET 9

#define PM_PLL              0
#define PM_PCIE             1
#define PM_COMMON           2
#define PM_DDR              3
#define PM_BOARD            4
#define PM_SECURE           5

typedef struct {
  UINT32 Magic;
  UINT32 Version;
  UINT32 Size;
  UINT8  Rev1[4];
}__attribute__((aligned(sizeof(UINT64)))) PARAMETER_HEADER;



typedef struct secure_config {
  UINT64  MiscEnable;       //0: tzc en; 1: aes en

  UINT64  TzcRegionLow1;  //tzc enable时,开放4个安全域供用户配置
  UINT64  TzcRegionTop1;
  UINT64  TzcRegionLow2;
  UINT64  TzcRegionTop2;
  UINT64  TzcRegionLow3;
  UINT64  TzcRegionTop3;
  UINT64  TzcRegionLow4;
  UINT64  TzcRegionTop4;
} __attribute__((aligned(sizeof(UINT64)))) SECURE_CONFIG_T;

static SECURE_CONFIG_T const SECURE_BASE_INFO = {
  .MiscEnable   = 0x0,
};

/*-------------------PLL------------------*/
typedef struct {
  PARAMETER_HEADER Head;
  UINT32           PerformanceCoreFreq;
  UINT32           Res1;
  UINT32           EfficiencyCoreFreq;
  UINT32           Res2;
  UINT32           LmuFreq;
  UINT32           Res3;
  UINT32           Res4;
} __attribute__((aligned(sizeof(UINT64)))) PLL_CONFIG;

/*-------------------PEU------------------*/
typedef struct {
  UINT32 BaseConfig[3];
  UINT32 Equalization[3];
  UINT8  Rev[80];
} __attribute__((aligned(sizeof(UINT64)))) PEU_FEATURES_CONFIG;

typedef struct {
  PARAMETER_HEADER Head;
  UINT32           IndependentTree;
  UINT32           BaseCfg;
  UINT8            Rev2[16];
  PEU_FEATURES_CONFIG FeaturesConfig[2];
} __attribute__((aligned(sizeof(UINT64)))) PEU_CONFIG;

typedef struct {
  PARAMETER_HEADER Head;
  UINT32 IndependentTree;
  UINT32 FunctionConfig;
  UINT8  Rev0[16];
  UINT32 Peu0ControllerConfig[2];
  UINT8  Rev1[8];
  UINT32 Peu0ControllerEqualization[2];
  UINT8  Rev2[80];
  UINT32 Peu1ControllerConfig[4];
  UINT32 Peu1ControllerEqualization[4];
}__attribute__((aligned(sizeof(UINT64)))) PARAMETER_PEU_CONFIG;

/*-------------------COMMON------------------*/
typedef struct {
  PARAMETER_HEADER Head;
  UINTN            CoreBitMap;
} __attribute__((aligned(sizeof(UINT64)))) COMMON_CONFIG;

/*-------------------MCU------------------*/
typedef struct {
  /******************* read from spd *****************/
  UINT8  DimmType;     /* 1: RDIMM;2: UDIMM;3: SODIMM;4: LRDIMM */
  UINT8  DataWidth;    /* 0: x4; 1: x8; 2: x16 */
  UINT8  MirrorType;   /* 0: stardard; 1: mirror */
  UINT8  EccType;      /* 0: no-ecc; 1:ecc */
  UINT8  DramType;     /* 0xB: DDR3; 0xC: DDR4 */
  UINT8  RankNum;
  UINT8  RowNum;
  UINT8  ColNum;

  UINT8  BgNum;        /*only DDR4*/
  UINT8  BankNum;
  UINT16 ModuleManufacturerId;
  UINT16 TAAmin;
  UINT16 TRCDmin;
  UINT16 TRPmin;
  UINT16 TRASmin;
  UINT16 TRCmin;
  UINT16 TFAWmin;

  UINT16 TRRDSmin;      /*only DDR4*/
  UINT16 TRRDLmin;      /*only DDR4*/
  UINT16 TCCDLmin;      /*only DDR4*/
  UINT16 TWRmin;

  UINT16 TWTRSmin;      /*only DDR4*/
  UINT16 TWTRLmin;      /*only DDR4*/
  UINT32 TRFC1min;
  UINT32 TRFC2min;
  UINT32 TRFC4RFCsbmin;  /*DDR4: tRFC4min;  DDR5: tRFCsbmin*/

  UINT8  Resv[8];
  /******************* RCD control words *****************/
  UINT8  F0RC03;    /*bit[3:2]:CS         bit[1:0]:CA  */
  UINT8  F0RC04;    /*bit[3:2]:ODT        bit[1:0]:CKE */
  UINT8  F0RC05;    /*bit[3:2]:CLK-A side bit[1:0]:CLK-B side */
  UINT8  RcdNum;   /* Registers used on RDIMM */

  UINT8  LrdimmResv[4];
  UINT8  LrdimmResv1[8];
  UINT8  LrdimmResv2[8];
}__attribute__((aligned(sizeof(UINT64)))) DDR_SPD_INFO;

typedef struct {
  PARAMETER_HEADER Head;
  UINT8  ChEnable;
  UINT8  Resv1[7];

  UINT64 MiscEnable;

  UINT8  TrainDebug;
  UINT8  TrainRecover;     /*为1时跳过training*/
  UINT8  TrainParam_type;
  UINT8  TrainParam_1;     /* DDR4: cpu_odt     */
  UINT8  TrainParam_2;     /* DDR4: cpu_drv     */
  UINT8  TrainParam_3;     /* DDR4: mr_drv      */
  UINT8  TrainParam_4;     /* DDR4: rtt_nom     */
  UINT8  TrainParam_5;     /* DDR4: rtt_park    */
  UINT8  TrainParam_6;     /* DDR4: rtt_wr      */
  UINT8  Resv2[7];

  /****for LPDDR4 dq swap************/
  UINT32 DataByteSwap;
  UINT32 Slice0DqSwizzle;
  UINT32 Slice1DqSwizzle;
  UINT32 Slice2DqSwizzle;
  UINT32 Slice3DqSwizzle;
  UINT32 Slice4DqSwizzle;
  UINT32 Slice5DqSwizzle;
  UINT32 Slice6DqSwizzle;
  UINT32 Slice7DqSwizzle;
  UINT8  Resv3[4];
  UINT8  Resv4[8];
  DDR_SPD_INFO     DdrSpdInfo;
} __attribute__((aligned(sizeof(UINT64)))) DDR_CONFIG;

typedef struct {
  PARAMETER_HEADER Head;
  UINT8            Misc;
  UINT8            PowerManger;
  UINT8            QspiFrequence;
  UINT8            Resev;
  UINT32           PhySelMode;
  UINT32           GmacSpeedMode;
  UINT32           MioSel;
  UINT32           ParamAutoSel;
} __attribute__((aligned(sizeof(unsigned long)))) BOARD_CONFIG;


EFI_STATUS
GetParameterInfo (
  UINT32 Id,
  UINT8 *Data,
  UINT32 DataSize
  );

#endif
