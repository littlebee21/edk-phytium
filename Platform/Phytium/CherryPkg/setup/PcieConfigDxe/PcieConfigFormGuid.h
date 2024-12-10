/** @file
  The guid define header file of HII Config Access protocol implementation of pcie configuration module.
**/
#ifndef  _PCIECONFIG_FORM_GUID_H_
#define  _PCIECONFIG_FORM_GUID_H_

#define PCIE_SPLIT_MODE_X16    1
#define PCIE_SPLIT_MODE_X8X8   3
#define PCIE_SPEED_GEN1        1
#define PCIE_SPEED_GEN2        2
#define PCIE_SPEED_GEN3        3

#define PCIECONFIG_FORMSET_GUID\
  {0x3dc58baa, 0x21c6, 0x11ec, {0xad, 0x54, 0x93, 0x37, 0x2d, 0x38, 0xb3, 0x74}}

#define  PCIE_CONFIG_VARIABLE          L"PcieConfigSetup"

#define  FORM_PCIECONFIG_ID            0x7001
#define  VARSTORE_ID_PCIE_CONFIG       0x7100

#define  LABEL_FORM_PCIECONFIG_START   0xff0c
#define  LABEL_FORM_PCIECONFIG_END     0xff0d
#define  LABEL_END                     0xffff

#endif
