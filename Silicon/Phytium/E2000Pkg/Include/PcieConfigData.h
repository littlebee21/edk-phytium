#ifndef  _PCIE_CONFIG_DATA_H_
#define  _PCIE_CONFIG_DATA_H

#define VAR_PCIE_CONFIG_NAME                     L"PcieConfigVar"
#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

typedef struct _PCIE_CONTROLLER {
  UINT8  DisEnable;
  UINT8  Speed;
  UINT8  EqualValue;
} PCIE_CONTROLLER;

typedef struct _PEU_CONFIG {
  UINT8            SplitMode;
  PCIE_CONTROLLER  PcieControl[4];
} PEU_CONFIG_UI;


typedef struct _PCIE_CONFIG_DATA {
  PEU_CONFIG_UI  PcieConfig[2];
} PCIE_CONFIG_DATA;

#endif
