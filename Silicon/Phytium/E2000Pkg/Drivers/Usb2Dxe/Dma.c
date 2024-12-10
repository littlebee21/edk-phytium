/** @file
  Phytium usb2 dma function description.

  Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Usb2.h"

PHYTIUM_USB2_DMA_OBJ mDmaDrv = {
  PhytiumDmaChannelProgram,
  PhytiumDmaChannelRelease,
  PhytiumDmaCheckInterrupt,
  PhytiumDmaControllerReset,
  PhytiumDmaClearInterrupt,
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
  )
{
  UINT32  Temp;

  //Single Mode
  PhytiumUsbWrite32 (&Ctrl->Regs->Conf, BIT8);
  //Enable TRB Error Interrupt
  PhytiumUsbWrite32 (&Ctrl->Regs->EpStsEn, BIT7);
  if (IsIn == TRANSFER_IN) {
    //EpNum and In
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, EpNum);
    //Enable Interrupt
    Temp = PhytiumUsbRead32 (&Ctrl->Regs->EpIen);
    Temp |= BIT((EpNum + 16));
    PhytiumUsbWrite32 (&Ctrl->Regs->EpIen, Temp);
  } else {
    //EpNum and Out
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, BIT7 | EpNum);
    //Enable Interrupt
    Temp = PhytiumUsbRead32 (&Ctrl->Regs->EpIen);
    Temp |= BIT(EpNum);
    PhytiumUsbWrite32 (&Ctrl->Regs->EpIen, Temp);
  }
  //Trb Address
  PhytiumUsbWrite32 (&Ctrl->Regs->Traddr, TrbAddr);
  PhytiumUsbWrite32 (&Ctrl->Regs->EpCmd, BIT6);
  PhytiumUsbWrite32 (&Ctrl->Regs->EpCfg, BIT0);

  return EFI_SUCCESS;
}

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
  )
{
  if (IsIn == TRANSFER_IN) {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, EpNum);
  } else {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, BIT7 | EpNum);
  }
  PhytiumUsbWrite32 (&Ctrl->Regs->EpCmd, BIT0);

  return EFI_SUCCESS;
}

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
  )
{
  if (IsIn == TRANSFER_IN) {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, EpNum);
  } else {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, BIT7 | EpNum);
  }

  if (PhytiumUsbRead32 (&Ctrl->Regs->EpSts) &  BIT2) {
    //PhytiumUsbWrite32 (&Ctrl->Regs->EpSts, BIT2);
    return TRUE;
  } else {
    return FALSE;
  }
}

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
  )
{
  if (IsIn == TRANSFER_IN) {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, EpNum);
  } else {
    PhytiumUsbWrite32 (&Ctrl->Regs->EpSel, BIT7 | EpNum);
  }
  PhytiumUsbWrite32 (&Ctrl->Regs->EpSts, BIT2);
}

/**
  Reset all DMA confituration.

  @param[in]  Ctrl        A pointer to DMA_CONTROLLER.

**/
VOID
PhytiumDmaControllerReset (
  IN DMA_CONTROLLER  *Ctrl
  )
{
  UINT32  Conf;

  if (Ctrl == NULL) {
    return;
  }

  Conf = PhytiumUsbRead32 (&Ctrl->Regs->Conf);
  Conf |= BIT0;
  PhytiumUsbWrite32 (&Ctrl->Regs->Conf, Conf);
}

/**
  Get the DMA Protocol function entry.

  @retval  A pointer to PHYTIUM_USB2_DMA_OBJ.
**/
PHYTIUM_USB2_DMA_OBJ *
DMAGetInstance (
  VOID
  )
{
  return &mDmaDrv;
}

