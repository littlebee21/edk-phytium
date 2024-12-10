/** @file
Phytium usb2 dma type and struct description.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PHYTIUM_USB2_DMA_H_
#define  PHYTIUM_USB2_DMA_H_

#include "Reg.h"

typedef struct _DMA_TRB               DMA_TRB;
typedef enum   _DMA_STATUS            DMA_STATUS;
typedef struct _DMA_CFG               DMA_CFG;
typedef enum   _DMA_MODE              DMA_MODE;
typedef struct _DMA_CONTROLLER        DMA_CONTROLLER;
typedef struct _PHYTIUM_USB2_DMA_OBJ  PHYTIUM_USB2_DMA_OBJ;

struct _DMA_TRB {
  UINT32 DmaAddr;
  UINT32 DmaSize;
  UINT32 Ctrl;
};

enum _DMA_STATUS {
  DMA_STATUS_UNKNOW,
  DMA_STATUS_FREE,
  DMA_STATUS_ABORT,
  DMA_STATUS_BUSY,
  DMA_STATUS_ARMED
};

struct _DMA_CFG {
  UINT64  RegBase;
  UINT16  DmaModeTx;
  UINT16  DmaModeRx;
  VOID    *TrbAddr;
  UINT64  TrbDmaAddr;
};

enum _DMA_MODE {
  DMA_MODE_GLOBAL_DMULT,
  DMA_MODE_GLOBAL_DSING,
  DMA_MODE_CHANNEL_INDIVIDUAL,
};

struct _DMA_CONTROLLER {
  DMA_REG               *Regs;
  PHYTIUM_USB2_DMA_OBJ  *DmaDrv;
  DMA_CFG               DmaCfg;
  DMA_MODE              DmaMode;
  UINT8                 IsHostCtrlMode;
};

/**
  Configure the DMA channel of the current EpNum and direction, and trigger
  the current trb transmission.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.
  @param[in]  TrbAddr     Trb address.

  @retval     EFI_SUCCESS Successfully.
**/
EFI_STATUS
PhytiumDmaChannelProgram (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn,
  IN UINT32         TrbAddr
  );

/**
  Release the DMA channel of the current EpNum and direction.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

  @retval     EFI_SUCCESS Successfully.
**/
EFI_STATUS
PhytiumDmaChannelRelease (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Check whether the channel DMA transmission of the current EpNum and direction
  is completed.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

  @retval     TRUE        Transfer is completed.
  @retval     FALSE       Transfer is not completed.
**/
BOOLEAN
PhytiumDmaCheckInterrupt (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Clear Interrupt in the specified channel and direction.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

**/
VOID
PhytiumDmaClearInterrupt (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Reset all DMA confituration.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.

**/
VOID
PhytiumDmaControllerReset (
  IN DMA_CONTROLLER  *Ctrl
  );

/**
  Get the DMA Protocol function entry.

  @retval  A pointer to PHYTIUM_USB2_DMA_OBJ.
**/
PHYTIUM_USB2_DMA_OBJ *
DMAGetInstance (
  VOID
  );

/**
  Configure the DMA channel of the current EpNum and direction, and trigger
  the current trb transmission.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.
  @param[in]  TrbAddr     Trb address.

  @retval     EFI_SUCCESS Successfully.
**/
typedef
EFI_STATUS
(*PHYTIUM_USB2_DMA_CHANNEL_PROGRAM) (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn,
  IN UINT32         TrbAddr
  );

/**
  Release the DMA channel of the current EpNum and direction.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

  @retval     EFI_SUCCESS Successfully.
**/
typedef
EFI_STATUS
(*PHYTIUM_USB2_DMA_CHANNEL_RELEASE) (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Check whether the channel DMA transmission of the current EpNum and direction
  is completed.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

  @retval     TRUE        Transfer is completed.
  @retval     FALSE       Transfer is not completed.
**/
typedef
BOOLEAN
(*PHYTIUM_USB2_DMA_CHECK_INTERRUPT) (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Clear Interrupt in the specified channel and direction.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.
  @param[in]  EpNum       Epdpoint number, 0 - 15.
  @param[in]  IsIn        Transmission direction, 1 - in, 0 - out.

**/
typedef
VOID
(*PHYTIUM_USB2_DMA_CLEAR_INTERRUPT) (
  IN DMA_CONTROLLER *Ctrl,
  IN UINT8          EpNum,
  IN UINT8          IsIn
  );

/**
  Reset all DMA confituration.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.

**/
typedef
VOID
(*PHYTIUM_USB2_DMA_CONTROLLER_RESET) (
  IN DMA_CONTROLLER  *Ctrl
  );

struct _PHYTIUM_USB2_DMA_OBJ {
  PHYTIUM_USB2_DMA_CHANNEL_PROGRAM    DmaChannelProgram;
  PHYTIUM_USB2_DMA_CHANNEL_RELEASE    DmaChannelRelease;
  PHYTIUM_USB2_DMA_CHECK_INTERRUPT    DmaCheckInterrupt;
  PHYTIUM_USB2_DMA_CONTROLLER_RESET   DmaControllerReset;
  PHYTIUM_USB2_DMA_CLEAR_INTERRUPT    DmaClearInterrupt;
};

#endif
