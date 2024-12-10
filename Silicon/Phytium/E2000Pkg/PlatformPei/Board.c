#include "Board.h"

EFI_STATUS pcie_cfg_sel(UINT8 *buff, UINT32 Type)
{
  peu_config_t *pcie_cfg = (peu_config_t *)buff;

  switch (Type) {
  case 0x0://Q:A
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X4 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x1://Q:B
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X2X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x2://Q:C1
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x3://Q:C2
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x4://D/S:A
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x5://D/S:B
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x6://D/S:C1
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  case 0x7://D/S:C2
    pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X1X1X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    case 0x8:
      pcie_cfg->base_cfg = ((CONFIG_PCI_PEU1 | (X2X1X1 << 1)) << PEU1_OFFSET | (CONFIG_PCI_PEU0 | (X1X1 << 1)));
    break;
  }
  return EFI_SUCCESS;
}

EFI_STATUS ddr_cfg_sel(UINT8 *buff, UINT32 Type)
{
  mcu_config_t *mcu_info = (mcu_config_t *)buff;

  switch (Type) {//A/B:ddr4,C:lpddr4
  case 0x0://Q:A
    mcu_info->misc_enable = 0x1c;//no spd force，其他参数读取spd
    break;
  case 0x1://Q:B
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x1d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x0;
    mcu_info->slice0_dq_swizzle = 0x0;
    mcu_info->slice1_dq_swizzle = 0x0;
    mcu_info->slice2_dq_swizzle = 0x0;
    mcu_info->slice3_dq_swizzle = 0x0;
    mcu_info->slice4_dq_swizzle = 0x0;
    mcu_info->slice5_dq_swizzle = 0x0;
    mcu_info->slice6_dq_swizzle = 0x0;
    mcu_info->slice7_dq_swizzle = 0x0;

    mcu_info->ddr_spd_info.dimm_type = 0x2;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0xc;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x11;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x2;
    mcu_info->ddr_spd_info.bank_num = 0x4;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x35b6;
    mcu_info->ddr_spd_info.tRCDmin = 0x35b6;

    mcu_info->ddr_spd_info.tRPmin = 0x35b6;
    mcu_info->ddr_spd_info.tRASmin = 0x7d00;
    mcu_info->ddr_spd_info.tRCmin = 0xb2b6;
    mcu_info->ddr_spd_info.tFAWmin = 0x7530;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x14b4;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x1900;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x1388;
    mcu_info->ddr_spd_info.tWRmin = 0x3a98;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x9c4;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x1d4c;
    mcu_info->ddr_spd_info.tRFC1min = 0x27100;
    mcu_info->ddr_spd_info.tRFC2min = 0x1adb0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0x15f90;
    break;
  case 0x2://Q:C1
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x10d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x76542301;
    mcu_info->slice0_dq_swizzle = 0x10543672;
    mcu_info->slice1_dq_swizzle = 0x61273054;
    mcu_info->slice2_dq_swizzle = 0x32761504;
    mcu_info->slice3_dq_swizzle = 0x35417026;
    mcu_info->slice4_dq_swizzle = 0x76215403;
    mcu_info->slice5_dq_swizzle = 0x10547632;
    mcu_info->slice6_dq_swizzle = 0x14532760;
    mcu_info->slice7_dq_swizzle = 0x23671450;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x2;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  case 0x3://Q:C2
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x10d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x76542301;
    mcu_info->slice0_dq_swizzle = 0x10543672;
    mcu_info->slice1_dq_swizzle = 0x61273054;
    mcu_info->slice2_dq_swizzle = 0x32761504;
    mcu_info->slice3_dq_swizzle = 0x35417026;
    mcu_info->slice4_dq_swizzle = 0x76215403;
    mcu_info->slice5_dq_swizzle = 0x10547632;
    mcu_info->slice6_dq_swizzle = 0x14532760;
    mcu_info->slice7_dq_swizzle = 0x23671450;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x2;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  case 0x4://D/S:A
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x201d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x0;
    mcu_info->slice0_dq_swizzle = 0x0;
    mcu_info->slice1_dq_swizzle = 0x0;
    mcu_info->slice2_dq_swizzle = 0x0;
    mcu_info->slice3_dq_swizzle = 0x0;
    mcu_info->slice4_dq_swizzle = 0x0;
    mcu_info->slice5_dq_swizzle = 0x0;
    mcu_info->slice6_dq_swizzle = 0x0;
    mcu_info->slice7_dq_swizzle = 0x0;

    mcu_info->ddr_spd_info.dimm_type = 0x2;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0xc;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x11;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x2;
    mcu_info->ddr_spd_info.bank_num = 0x4;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x35b6;
    mcu_info->ddr_spd_info.tRCDmin = 0x35b6;

    mcu_info->ddr_spd_info.tRPmin = 0x35b6;
    mcu_info->ddr_spd_info.tRASmin = 0x7d00;
    mcu_info->ddr_spd_info.tRCmin = 0xb2b6;
    mcu_info->ddr_spd_info.tFAWmin = 0x7530;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x14b4;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x1900;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x1388;
    mcu_info->ddr_spd_info.tWRmin = 0x3a98;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x9c4;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x1d4c;
    mcu_info->ddr_spd_info.tRFC1min = 0x27100;
    mcu_info->ddr_spd_info.tRFC2min = 0x1adb0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0x15f90;
    break;
  case 0x5://D/S:B
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x201d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x0;
    mcu_info->slice0_dq_swizzle = 0x0;
    mcu_info->slice1_dq_swizzle = 0x0;
    mcu_info->slice2_dq_swizzle = 0x0;
    mcu_info->slice3_dq_swizzle = 0x0;
    mcu_info->slice4_dq_swizzle = 0x0;
    mcu_info->slice5_dq_swizzle = 0x0;
    mcu_info->slice6_dq_swizzle = 0x0;
    mcu_info->slice7_dq_swizzle = 0x0;

    mcu_info->ddr_spd_info.dimm_type = 0x2;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0xc;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x11;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x2;
    mcu_info->ddr_spd_info.bank_num = 0x4;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x35b6;
    mcu_info->ddr_spd_info.tRCDmin = 0x35b6;

    mcu_info->ddr_spd_info.tRPmin = 0x35b6;
    mcu_info->ddr_spd_info.tRASmin = 0x7d00;
    mcu_info->ddr_spd_info.tRCmin = 0xb2b6;
    mcu_info->ddr_spd_info.tFAWmin = 0x7530;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x14b4;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x1900;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x1388;
    mcu_info->ddr_spd_info.tWRmin = 0x3a98;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x9c4;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x1d4c;
    mcu_info->ddr_spd_info.tRFC1min = 0x27100;
    mcu_info->ddr_spd_info.tRFC2min = 0x1adb0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0x15f90;
    break;
  case 0x6://D/S:C1
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x200d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x67453210;
    mcu_info->slice0_dq_swizzle = 0x23074516;
    mcu_info->slice1_dq_swizzle = 0x54176230;
    mcu_info->slice2_dq_swizzle = 0x67435120;
    mcu_info->slice3_dq_swizzle = 0x21630547;
    mcu_info->slice4_dq_swizzle = 0x16057423;
    mcu_info->slice5_dq_swizzle = 0x16057423;
    mcu_info->slice6_dq_swizzle = 0x16057423;
    mcu_info->slice7_dq_swizzle = 0x16057423;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  case 0x7://D/S:C2
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x200d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x67453210;
    mcu_info->slice0_dq_swizzle = 0x23074516;
    mcu_info->slice1_dq_swizzle = 0x54176230;
    mcu_info->slice2_dq_swizzle = 0x67435120;
    mcu_info->slice3_dq_swizzle = 0x21630547;
    mcu_info->slice4_dq_swizzle = 0x16057423;
    mcu_info->slice5_dq_swizzle = 0x16057423;
    mcu_info->slice6_dq_swizzle = 0x16057423;
    mcu_info->slice7_dq_swizzle = 0x16057423;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  case 0x8:    //GreatWall-F080-E
    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x10d;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x67453210;
    mcu_info->slice0_dq_swizzle = 0x23760541;
    mcu_info->slice1_dq_swizzle = 0x06715423;
    mcu_info->slice2_dq_swizzle = 0x75134062;
    mcu_info->slice3_dq_swizzle = 0x61342750;
    mcu_info->slice4_dq_swizzle = 0x45106723;
    mcu_info->slice5_dq_swizzle = 0x12764530;
    mcu_info->slice6_dq_swizzle = 0x73264015;
    mcu_info->slice7_dq_swizzle = 0x54016237;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x2;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  case 0x9:
    //mcu_info->magic = 0x54460014;
    //mcu_info->version =  0x10000;
    //mcu_info->size = 0x100;

    //mcu_info->ch_enable = 0x1;
    //mcu_info->misc_enable = 0x200d;
    //mcu_info->train_debug = 0x14;
    //mcu_info->train_recover = 0x0;

    //mcu_info->train_param_type = 0x0;
    //mcu_info->train_param_1 = 0x0;
    //mcu_info->train_param_2 = 0x0;
    //mcu_info->train_param_3 = 0x0;
    //mcu_info->train_param_4 = 0x0;
    //mcu_info->train_param_5 = 0x0;
    //mcu_info->train_param_6 = 0x0;

    //mcu_info->data_byte_swap = 0x76543210;
    //mcu_info->slice0_dq_swizzle = 0x76543210;
    //mcu_info->slice1_dq_swizzle = 0x76543210;
    //mcu_info->slice2_dq_swizzle = 0x76543210;
    //mcu_info->slice3_dq_swizzle = 0x76543210;
    //mcu_info->slice4_dq_swizzle = 0x76543210;
    //mcu_info->slice5_dq_swizzle = 0x76543210;
    //mcu_info->slice6_dq_swizzle = 0x76543210;
    //mcu_info->slice7_dq_swizzle = 0x76543210;

    //mcu_info->ddr_spd_info.dimm_type = 0x1;
    //mcu_info->ddr_spd_info.data_width = 0x2;
    //mcu_info->ddr_spd_info.mirror_type = 0x0;
    //mcu_info->ddr_spd_info.ecc_type = 0x0;
    //mcu_info->ddr_spd_info.dram_type = 0x10;
    //mcu_info->ddr_spd_info.rank_num = 0x1;
    //mcu_info->ddr_spd_info.row_num = 0x10;
    //mcu_info->ddr_spd_info.col_num = 0xa;

    //mcu_info->ddr_spd_info.bg_num = 0x0;
    //mcu_info->ddr_spd_info.bank_num = 0x8;
    //mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    //mcu_info->ddr_spd_info.tAAmin = 0x0;
    //mcu_info->ddr_spd_info.tRCDmin = 0x0;

    //mcu_info->ddr_spd_info.tRPmin = 0x0;
    //mcu_info->ddr_spd_info.tRASmin = 0x0;
    //mcu_info->ddr_spd_info.tRCmin = 0x0;
    //mcu_info->ddr_spd_info.tFAWmin = 0x0;

    //mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    //mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    //mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    //mcu_info->ddr_spd_info.tWRmin = 0x0;

    //mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    //mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    //mcu_info->ddr_spd_info.tRFC1min = 0x0;
    //mcu_info->ddr_spd_info.tRFC2min = 0x0;
    //mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;

    mcu_info->magic = 0x54460014;
    mcu_info->version =  0x10000;
    mcu_info->size = 0x100;

    mcu_info->ch_enable = 0x1;
    mcu_info->misc_enable = 0x2001;
    mcu_info->train_debug = 0x14;
    mcu_info->train_recover = 0x0;

    mcu_info->train_param_type = 0x0;
    mcu_info->train_param_1 = 0x0;
    mcu_info->train_param_2 = 0x0;
    mcu_info->train_param_3 = 0x0;
    mcu_info->train_param_4 = 0x0;
    mcu_info->train_param_5 = 0x0;
    mcu_info->train_param_6 = 0x0;

    mcu_info->data_byte_swap = 0x76543210;
    mcu_info->slice0_dq_swizzle = 0x3145726;
    mcu_info->slice1_dq_swizzle = 0x54176230;
    mcu_info->slice2_dq_swizzle = 0x57604132;
    mcu_info->slice3_dq_swizzle = 0x20631547;
    mcu_info->slice4_dq_swizzle = 0x16057423;
    mcu_info->slice5_dq_swizzle = 0x16057423;
    mcu_info->slice6_dq_swizzle = 0x16057423;
    mcu_info->slice7_dq_swizzle = 0x16057423;

    mcu_info->ddr_spd_info.dimm_type = 0x1;
    mcu_info->ddr_spd_info.data_width = 0x2;
    mcu_info->ddr_spd_info.mirror_type = 0x0;
    mcu_info->ddr_spd_info.ecc_type = 0x0;
    mcu_info->ddr_spd_info.dram_type = 0x10;
    mcu_info->ddr_spd_info.rank_num = 0x1;
    mcu_info->ddr_spd_info.row_num = 0x10;
    mcu_info->ddr_spd_info.col_num = 0xa;

    mcu_info->ddr_spd_info.bg_num = 0x0;
    mcu_info->ddr_spd_info.bank_num = 0x8;
    mcu_info->ddr_spd_info.module_manufacturer_id = 0x0;
    mcu_info->ddr_spd_info.tAAmin = 0x0;
    mcu_info->ddr_spd_info.tRCDmin = 0x0;

    mcu_info->ddr_spd_info.tRPmin = 0x0;
    mcu_info->ddr_spd_info.tRASmin = 0x0;
    mcu_info->ddr_spd_info.tRCmin = 0x0;
    mcu_info->ddr_spd_info.tFAWmin = 0x0;

    mcu_info->ddr_spd_info.tRRD_Smin = 0x0;
    mcu_info->ddr_spd_info.tRRD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tCCD_Lmin = 0x0;
    mcu_info->ddr_spd_info.tWRmin = 0x0;

    mcu_info->ddr_spd_info.tWTR_Smin = 0x0;
    mcu_info->ddr_spd_info.tWTR_Lmin = 0x0;
    mcu_info->ddr_spd_info.tRFC1min = 0x0;
    mcu_info->ddr_spd_info.tRFC2min = 0x0;
    mcu_info->ddr_spd_info.tRFC4_RFCsbmin = 0;
    break;
  }

    return EFI_SUCCESS;
}
