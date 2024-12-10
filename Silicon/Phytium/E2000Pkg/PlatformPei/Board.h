#ifndef  _PLAT_BOARD_H_
#define  _PLAT_BOARD_H_

#include <Library/DebugLib.h>
#include <Library/ParameterTable.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PhytiumPowerControlLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/I2CLib.h>

//#define  NO_PARTABLE
/***********************defult PEU set*****************************************/
#define CONFIG_PCI_PEU0     0x1
#define CONFIG_PCI_PEU1         0x1  /*0x0 : disable peu1, 0x01 : enable peu1 configuartions */
#define CONFIG_INDEPENDENT_TREE 0x0
/*peu*/
#define X4            0x0
#define X2X1X1          0x1
#define X1X1X1X1        0x2
/*peu_psu*/
#define X1X1          0x0
#define X0X1          0x1
#define X1X0          0x2
#define X0X0          0x3

#define CONFIG_PEU0_SPLIT_MODE  X4
#define CONFIG_PEU1_SPLIT_MODE  X1X1

/* peu device mode: 0 ep, 1 rc default */
#define EP_MODE                 0x0
#define RC_MODE                 0x1
#define PEU1_OFFSET       16
#define PEU_C_OFFSET_MODE   16
#define PEU_C_OFFSET_SPEED    0

#define GEN3          3
#define GEN2          2
#define GEN1          1

/*-------------------PEU------------------*/
typedef struct pcu_ctr{
  UINT32 base_config[4];
  UINT32 equalization[4];
  UINT8  rev[72];
}__attribute__((aligned(sizeof(unsigned long)))) peu_ctr_t;

typedef struct pcu_config {
  UINT32 magic;
  UINT32 version;
  UINT32 size;
  UINT8 rev1[4];
  UINT32 independent_tree;
  UINT32 base_cfg;
  UINT8 rev2[16];
  peu_ctr_t ctr_cfg[2];
}__attribute__((aligned(sizeof(unsigned long)))) peu_config_t;

/*-------------------MCU------------------*/
typedef struct ddr_spd {
    /******************* read from spd *****************/
    UINT8  dimm_type;     /* 1: RDIMM;    2: UDIMM;  3: SODIMM;    4: LRDIMM */
    UINT8  data_width;    /* 0: x4;       1: x8;     2: x16        3: X32*/
    UINT8  mirror_type;   /* 0: stardard; 1: mirror  DDR5 无*/
    UINT8  ecc_type;      /* 0: no-ecc;   1:ecc */
    UINT8  dram_type;     /* 0xB: DDR3;   0xC: DDR4  0x10: LPDDR4, 0x12: DDR5*/
    UINT8  rank_num;
    UINT8  row_num;
    UINT8  col_num;

    UINT8  bg_num;      /*DDR4 / DDR5*/
    UINT8  bank_num;
    UINT16 module_manufacturer_id;
    UINT16 tAAmin;
    UINT16 tRCDmin;

    UINT16 tRPmin;
    UINT16 tRASmin;
    UINT16 tRCmin;
    UINT16 tFAWmin;   /*only DDR3/DDR4*/

    UINT16 tRRD_Smin;   /*only DDR4*/
    UINT16 tRRD_Lmin;   /*only DDR4*/
    UINT16 tCCD_Lmin;   /*only DDR4*/
    UINT16 tWRmin;

    UINT16 tWTR_Smin;   /*only DDR4*/
    UINT16 tWTR_Lmin;   /*only DDR4*/
    UINT32 tRFC1min;

    UINT32 tRFC2min;
  UINT32 tRFC4_RFCsbmin;    /*DDR4: tRFC4min;  DDR5: tRFCsbmin*/

  UINT8  resv[8];

    /******************* RCD control words *****************/
    UINT8  F0RC03;  /*bit[3:2]:CS         bit[1:0]:CA  */
    UINT8  F0RC04;  /*bit[3:2]:ODT        bit[1:0]:CKE */
    UINT8  F0RC05;  /*bit[3:2]:CLK-A side bit[1:0]:CLK-B side */
    UINT8  rcd_num;     /* Registers used on RDIMM */

  UINT8  lrdimm_resv[4];
  UINT8  lrdimm_resv1[8];
  UINT8  lrdimm_resv2[8];

}__attribute__((aligned(sizeof(unsigned long)))) ddr_spd_info_t;

typedef struct mcu_config {
    UINT32 magic;
  UINT32 version;
  UINT32 size;
  UINT8 rev1[4];

  UINT8  ch_enable;
  UINT8  resv1[7];

  UINT64 misc_enable;

  UINT8  train_debug;
  UINT8  train_recover;   /*为1时跳过training*/
  UINT8  train_param_type;
  UINT8  train_param_1;   /* DDR4: cpu_odt     */
  UINT8  train_param_2;   /* DDR4: cpu_drv     */
  UINT8  train_param_3;   /* DDR4: mr_drv      */
  UINT8  train_param_4;   /* DDR4: rtt_nom     */
  UINT8  train_param_5;   /* DDR4: rtt_park    */

  UINT8  train_param_6;   /* DDR4: rtt_wr      */
  UINT8  resv2[7];

/*******for LPDDR4 dq swap************/
  UINT32 data_byte_swap;
  UINT32 slice0_dq_swizzle;

  UINT32 slice1_dq_swizzle;
  UINT32 slice2_dq_swizzle;

  UINT32 slice3_dq_swizzle;
  UINT32 slice4_dq_swizzle;

  UINT32 slice5_dq_swizzle;
  UINT32 slice6_dq_swizzle;

  UINT32 slice7_dq_swizzle;
  UINT8  resv3[4];

  UINT8  resv4[8];

    ddr_spd_info_t ddr_spd_info;
}__attribute__((aligned(sizeof(unsigned long)))) mcu_config_t;


extern EFI_STATUS pcie_cfg_sel(UINT8 *buff, UINT32 Type);
extern EFI_STATUS ddr_cfg_sel(UINT8 *buff, UINT32 Type);
#endif
