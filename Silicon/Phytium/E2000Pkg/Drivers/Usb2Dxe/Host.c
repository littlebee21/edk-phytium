/** @file
Phytium usb2 host function description.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Usb2.h"

PHYTIUM_USB2_HOST_OBJ mHostDrv = {
  PhytiumHostInit,
  PhytiumHostDestroy,
  PhytiumHostStart,
  PhytiumHostStop,
  PhytiumHostOtgIrq,
  PhytiumHostEpDisable,
  PhytiumHubPortSuspend,
  PhytiumHubPortReset,
  PhytiumHostControlTransfer,
  PhytiumHostBulkTransfer,
  PhytiumHostInterruptTransfer,
  PhytiumHostAsyncInterruptTransfer,
  PhytiumHostAsyncCheckComplete,
};

/**
  Calculate the number of packets and the length of the last packet according
  to the length of data to be transmitted.

  @param[in]  Size    The length of data to be transmitted.
  @param[out] Num     The number of packets.
  @param[out] Len     The length of last packet.
**/
STATIC
VOID
CalDmaPacketNum (
  IN  UINT32  Size,
  OUT UINT32  *Num,
  OUT UINT32  *Len
  )
{
  UINT32  NumTemp;
  UINT32  LenTemp;

  LenTemp = Size % SIZE_PER_DMA_PACKET;
  NumTemp = Size / SIZE_PER_DMA_PACKET;
  if (LenTemp == 0) {
    *Num = NumTemp;
    *Len = SIZE_PER_DMA_PACKET;
  } else {
    *Num = NumTemp + 1;
    *Len = LenTemp;
  }
  //DEBUG ((DEBUG_INFO,
  //        "DMA Packet Num : %d, Last Packet Length : %d\n",
  //        *Num, *Len));
}

/**
  Convert USB Host error type to UEFI USB error type.

  @param[in]  Code    USB Host transmission error type.

  @retval     UEFI spec USB error type.
**/
STATIC
UINT8
ParseErrorCode (
  IN UINT8  Code
  )
{
  //DEBUG ((DEBUG_INFO, "%a(), Code : %x\n", __FUNCTION__, Code));
  switch (Code) {
  case ERR_NONE:
    return EFI_USB_NOERROR;
  case ERR_STALL:
    return EFI_USB_ERR_STALL;
  case ERR_CRC:
    return EFI_USB_ERR_CRC;
  case ERR_TIMEOUT:
    return EFI_USB_ERR_TIMEOUT;
  default:
    return EFI_USB_ERR_BABBLE;
  }
}

/**
  Function Switch On or Switch Off VBUS depending on isOn parameter.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  IsOn    1- On, 0 - Off.

 **/
STATIC
VOID
HostSetVbus (
  IN HOST_CTRL *Ctrl,
  IN UINT8     IsOn
  )
{
  UINT8 Otgctrl;

  Otgctrl= PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);

  if (IsOn == 1) {
    if (!(Otgctrl & OTGCTRL_BUSREQ) || (Otgctrl & OTGCTRL_ABUSDROP)) {
      Otgctrl &= ~OTGCTRL_ABUSDROP;
      Otgctrl |= OTGCTRL_BUSREQ;
      PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
    }
    Ctrl->OtgState = HOST_OTG_STATE_A_WAIT_BCON;
  } else {
    if ((Otgctrl & OTGCTRL_BUSREQ) || (Otgctrl & OTGCTRL_ABUSDROP)) {
      Otgctrl |= OTGCTRL_ABUSDROP;
      Otgctrl &= ~OTGCTRL_BUSREQ;
      PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
    }
    Ctrl->OtgState = HOST_OTG_STATE_A_IDLE;
  }
}

/**
  Disconnect devices from host.

  @param[in]  Ctrl    A pointer to HOST_CTRL.

**/
STATIC
VOID
DisconnectHostDetect (
  IN HOST_CTRL *Ctrl
  )
{
  UINT8  Otgctrl;
  UINT8  Otgstate;
  UINT32 GenCfg;

  if (Ctrl == NULL) {
    return;
  }

  Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);
  if ((Otgctrl & OTGCTRL_ASETBHNPEN) && Ctrl->OtgState == HOST_OTG_STATE_A_SUSPEND) {
    DEBUG ((DEBUG_INFO, "Device no Response\n"));
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_CONIRQ);
Retry:
  Otgstate = PhytiumUsbRead8 (&Ctrl->Regs->Otgstate);
  if ((Otgstate == HOST_OTG_STATE_A_HOST || Otgstate == HOST_OTG_STATE_B_HOST)) {
    DEBUG ((DEBUG_INFO, "IRQ OTG: DisconnIrq Babble\n"));
    goto Retry;
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0 | 0x04);
  PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, FIFOCTRL_FIFOAUTO | FIFOCTRL_IO_TX | 0 | 0x04);

  Ctrl->PortStatus = USB_PORT_STAT_POWER;
  Ctrl->PortStatus |= USB_PORT_STAT_C_CONNECTION << 16;

  if (Ctrl->OtgState == HOST_OTG_STATE_A_SUSPEND) {
    HostSetVbus (Ctrl, 1);
  }

  Ctrl->OtgState = HOST_OTG_STATE_A_IDLE;
  if (Ctrl->CustomRegs) {
    PhytiumUsbWrite32 (&Ctrl->CustomRegs->Wakeup, 1);
  } else {
    GenCfg = PhytiumUsbRead32 (&Ctrl->VhubRegs->GenCfg);
    GenCfg |= BIT1;
    PhytiumUsbWrite32 (&Ctrl->VhubRegs->GenCfg, GenCfg);
  }
}

/**
  Connect devices from host.

  @param[in]  Ctrl      A pointer to HOST_CTRL.
  @param[in]  OtgState  Otg state to set.

**/
STATIC
VOID
ConnectHostDetect (
  IN HOST_CTRL *Ctrl,
  IN UINT8     OtgState
  )
{
  UINT32  GenCfg;

  if (Ctrl == NULL) {
    return;
  }
  //DEBUG ((DEBUG_INFO, "otgState:0x%x pirv->otgState:0x%x\n", OtgState, Ctrl->OtgState));
  if (Ctrl->CustomRegs) {
    PhytiumUsbWrite32 (&Ctrl->CustomRegs->Wakeup, 0);
  } else {
    GenCfg = PhytiumUsbRead32 (&Ctrl->VhubRegs->GenCfg);
    GenCfg &= ~BIT1;
    PhytiumUsbWrite32 (&Ctrl->VhubRegs->GenCfg, GenCfg);
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_CONIRQ);

  if ((OtgState != HOST_OTG_STATE_A_HOST) && (OtgState != HOST_OTG_STATE_B_HOST)) {
    return;
  }

  if ((Ctrl->OtgState == HOST_OTG_STATE_A_PERIPHERAL)
      || (Ctrl->OtgState == HOST_OTG_STATE_B_PERIPHERAL)) {
    Ctrl->OtgState = OtgState;
  }

  Ctrl->Ep0State = HOST_EP0_STAGE_IDLE;

  Ctrl->PortStatus &= ~(USB_PORT_STAT_LOW_SPEED | USB_PORT_STAT_HIGH_SPEED |
      USB_PORT_STAT_ENABLE);

  Ctrl->PortStatus |= USB_PORT_STAT_C_CONNECTION | (USB_PORT_STAT_C_CONNECTION << 16);
  Ctrl->PortResetting = 1;
  HostSetVbus (Ctrl, 1);

  switch (PhytiumUsbRead8 (&Ctrl->Regs->Speedctrl)) {
  case SPEEDCTRL_HS:
    Ctrl->PortStatus |= USB_PORT_STAT_HIGH_SPEED;
    DEBUG ((DEBUG_INFO, "detect High speed device\n"));
    break;
  case SPEEDCTRL_FS:
    Ctrl->PortStatus &= ~(USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_LOW_SPEED);
    DEBUG ((DEBUG_INFO, "detect Full speed device\n"));
    break;
  case SPEEDCTRL_LS:
    Ctrl->PortStatus |= USB_PORT_STAT_LOW_SPEED;
    DEBUG ((DEBUG_INFO, "detect Low speed device\n"));
    break;
  }

  Ctrl->VBusErrCnt = 0;

  Ctrl->OtgState = OtgState;
}

/**
  A_IDLE State Handling.

  @param[in]  Ctrl      A pointer to HOST_CTRL.
  @param[in]  Otgstate  Otg state to set.

**/
STATIC
VOID
AIdleDetect (
  HOST_CTRL *Ctrl,
  UINT8     Otgstate
  )
{
  UINT8  Otgctrl;

  if (Ctrl == NULL) {
    return;
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDLEIRQ);

  if (Otgstate != HOST_OTG_STATE_A_IDLE) {
    DEBUG ((DEBUG_INFO, "IRQ OTG: A_IDLE Babble\n"));
    return;
  }

  Ctrl->PortStatus = 0;
  Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);
  Otgctrl &= ~OTGCTRL_ASETBHNPEN;
  PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);

  HostSetVbus (Ctrl, 1);

  Ctrl->OtgState = HOST_OTG_STATE_A_IDLE;
}

/**
  B_IDLE State Handling.

  @param[in]  Ctrl      A pointer to HOST_CTRL.
  @param[in]  Otgstate  Otg state to set.

**/
STATIC
VOID
BIdleDetect (
  IN HOST_CTRL *Ctrl,
  IN UINT8     Otgstate
  )
{
  UINT8 Otgctrl;
  UINT8 Usbcs;

  if (Ctrl == NULL) {
    return;
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDLEIRQ);

  if (Otgstate != HOST_OTG_STATE_B_IDLE) {
    DEBUG ((DEBUG_INFO, "IRQ OTG: B_IDLE Babble\n"));
    return;
  }

  Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);
  Otgctrl &= ~OTGCTRL_ASETBHNPEN;
  PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);

  HostSetVbus (Ctrl, 0);

  Ctrl->OtgState = HOST_OTG_STATE_B_IDLE;

  Usbcs = PhytiumUsbRead8 (&Ctrl->Regs->Usbcs);
  Usbcs &= ~USBCS_DISCON;
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbcs, Usbcs);
}

/**
  Perform a host configuration according to the request parameters.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  Req     Request parameter.

  @retval     EFI_INVALID_PARAMETER         Some parameters are invalid.
  @retval     EFI_SUCCESS                   Request parameters config successfully.
**/
STATIC
EFI_STATUS
HostRequestProgram (
  IN HOST_CTRL *Ctrl,
  IN HOST_REQ  *Req
  )
{
  UINT8  RegCon;
  UINT8  EpNum;
  UINT8  DeviceAddress;
  UINT16 MaximumPacketLength;

  if ((Ctrl == NULL) || (Req == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  MicroSecondDelay (10);
  MaximumPacketLength = (UINT16)Req->MaximumPacketLength;
  DeviceAddress = Req->DeviceAddress;
  EpNum = Req->EpNum;
  RegCon = 0;
  switch (Req->Type) {
  case UsbRequestControl:
    RegCon |= 0x0;
    break;
  case UsbRequestBulk:
    RegCon |= 0x8;
    break;
  case UsbRequestInt:
    RegCon |= 0xC;
    break;
  }

  if (Req->IsIn) {  /*In Direction*/
    if (Req->EpNum) {  /*Ep 1-15*/
      //Enable Rx Interrupt
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxien,
        PhytiumUsbRead16 (&Ctrl->Regs->Rxien) | BIT(Req->EpNum));
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrien,
        PhytiumUsbRead16 (&Ctrl->Regs->Rxerrien) | BIT(Req->EpNum));
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCon, RegCon);
      //Function Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Fnaddr, (UINT8) DeviceAddress);
      //set in
      PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, EpNum);
      //reset toggle and fifo
      if (Req->Toggle) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, BIT7 | BIT6 | EpNum);
      } else {
        PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, BIT6 | BIT5 | EpNum);
      }
      PhytiumUsbWrite8 (&Ctrl->Regs->Fifoctrl, BIT5 | EpNum);
      //coutn switch epnum
      PhytiumUsbWrite8 (&Ctrl->Regs->EpExt[EpNum - 1].RxCtrl, EpNum);
      //Max packet size
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxmaxpack[EpNum - 1], MaximumPacketLength);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCs, 1);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCs, 1);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCs, 1);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCs, 1);
      //Transfer mode. Enable Ep
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].RxCon, RegCon | BIT7);
      if (Req->Type == UsbRequestInt) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Irqmode[EpNum - 1].Inirqmode,
          PhytiumUsbRead8 (&Ctrl->Regs->Irqmode[EpNum - 1].Inirqmode) | BIT7);
      }
      //Int Transfer Interval
      if (Req->FrameInterval != 0) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Rxsofttimer[EpNum].Ctrl,
          PhytiumUsbRead8 (&Ctrl->Regs->Rxsofttimer[EpNum].Ctrl) | BIT1);
        PhytiumUsbWrite16 (&Ctrl->Regs->Rxsofttimer[EpNum].Timer, Req->FrameInterval);
        PhytiumUsbWrite8 (&Ctrl->Regs->Rxsofttimer[EpNum].Ctrl, 0x83);
      }
    } else {  /*Ep 0*/
      //Enable Rx Interrupt
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxien,
        PhytiumUsbRead16 (&Ctrl->Regs->Rxien) | BIT0);
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrien,
        PhytiumUsbRead16 (&Ctrl->Regs->Rxerrien) | BIT0);
      //Set Ep0 Max Packet Number
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0maxpack, (UINT8)Req->MaximumPacketLength);
      //Set Ep0 Device Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Fnaddr, (UINT8)Req->DeviceAddress);
      //Set Ep0 EP Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0ctrl, 0);
      //Reset Fifo
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, 0x0);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, BIT5 | BIT2);
      //CS Set
      if ((Req->Stage == HOST_EP0_STAGE_STATUSIN) || (Req->Stage == HOST_EP0_STAGE_IN)) {
        //Set toggle
        PhytiumUsbWrite8 (&Ctrl->Regs->Ep0cs, BIT6);
      }
    }
  } else {  /*Out direction*/
    if (Req->EpNum) { /*Ep 1-15*/
      //Enable interrupt
      PhytiumUsbWrite16 (&Ctrl->Regs->Txien,
        PhytiumUsbRead16 (&Ctrl->Regs->Txien) | BIT(Req->EpNum));
      //Transfer mode. Ep num
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].TxCon, RegCon);
      //Set Ep0 Device Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Fnaddr, (UINT8) DeviceAddress);
      //set out
      PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, BIT4 | EpNum);
      //reset toggle and fifo
      if (Req->Toggle) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, BIT6 | BIT7 | BIT4 | EpNum);
      } else {
        PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, BIT6 | BIT5 | BIT4 | EpNum);
      }
      //fifo control out
      PhytiumUsbWrite8 (&Ctrl->Regs->Fifoctrl, BIT5 | BIT4 | EpNum);
      //coutn switch epnum
      PhytiumUsbWrite8 (&Ctrl->Regs->EpExt[EpNum - 1].TxCtrl, EpNum);
      //Max packet size
      PhytiumUsbWrite16 (&Ctrl->Regs->Txmaxpack[EpNum - 1], MaximumPacketLength);
      //bulk mode. Ep num
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep[EpNum - 1].TxCon, RegCon | BIT7);
      if (Req->Type == UsbRequestInt) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Irqmode[EpNum - 1].Outirqmode,
          PhytiumUsbRead8 (&Ctrl->Regs->Irqmode[EpNum - 1].Outirqmode) | BIT7);
      }
      //Int Transfer Interval
      if (Req->FrameInterval != 0) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Txsofttimer[EpNum].Ctrl,
          PhytiumUsbRead8 (&Ctrl->Regs->Txsofttimer[EpNum].Ctrl) | BIT1);
        PhytiumUsbWrite16 (&Ctrl->Regs->Txsofttimer[EpNum].Timer, Req->FrameInterval);
        PhytiumUsbWrite8 (&Ctrl->Regs->Txsofttimer[EpNum].Ctrl, 0x83);
      }
    } else {          /*Ep 0*/
      //Enable Tx Interrupt
      PhytiumUsbWrite16 (&Ctrl->Regs->Txien,
        PhytiumUsbRead16 (&Ctrl->Regs->Txien) | BIT0);
      PhytiumUsbWrite16 (&Ctrl->Regs->Txerrien,
        PhytiumUsbRead16 (&Ctrl->Regs->Txerrien) | BIT0);
      //Set Ep0 Max Packet Number
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0maxpack, (UINT8)Req->MaximumPacketLength);
      //Set Ep0 Device Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Fnaddr, (UINT8)Req->DeviceAddress);
      //Set Ep0 EP Address
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0ctrl, 0);
      //Reset Fifo
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, BIT4);
      PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, BIT5 | BIT2 | BIT4);
      //HSet
      if (Req->Stage == HOST_EP0_STAGE_SETUP) {
        PhytiumUsbWrite8 (&Ctrl->Regs->Ep0cs,
          PhytiumUsbRead8 (&Ctrl->Regs->Ep0cs) | BIT4);
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Check whether the transmission is completed. The controller and DMA status
  must be completed at the same time. If the transfer is completed, clear the
  completion interrupt of the DMA and the corresponding channel of the
  controller.

  @param[in]  Ctrl            A pointer ot HOST_CTRL.
  @param[in]  EpNum           Epdpoint number, 0 - 15.
  @param[in]  IsIn            Transmission direction, 1 - in, 0 - out.
  @param[out] TransferResult  Uefi spec USB transfer result.

  @retval     TRUE            Transfer completed or an error occurred.
  @retval     FALSE           Transfer not completed and no errors occurred.
**/
STATIC
BOOLEAN
HostCheckTransferComplete (
  IN  HOST_CTRL  *Ctrl,
  IN  UINT8      EpNum,
  IN  UINT8      IsIn,
  OUT UINT32     *TransferResult
  )
{
  BOOLEAN  State;
  UINT8    Code;
  UINT8    Error;
  //
  //not completed default
  //
  State = FALSE;
  *TransferResult = EFI_USB_NOERROR;

  if (IsIn == TRANSFER_IN) {
    if ((PhytiumUsbRead16 (&Ctrl->Regs->Rxirq) & BIT(EpNum))
          && (Ctrl->DmaDrv->DmaCheckInterrupt (Ctrl->DmaController, EpNum, IsIn) == TRUE)) {
      //clear epn rx interrupt
      State = TRUE;
      Ctrl->DmaDrv->DmaClearInterrupt (Ctrl->DmaController, EpNum, IsIn);
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxirq, BIT(EpNum));
      goto ProcExit;
    }
    if (PhytiumUsbRead16 (&Ctrl->Regs->Rxerrirq) & BIT(EpNum)) {
      Code = (PhytiumUsbRead8 (&Ctrl->Regs->EpExt[EpNum - 1].RxErr) >> 2) & 0x7;
      State = TRUE;
      //Check ep tx error irq
      Error = ParseErrorCode (Code);
      //clear ep tx error irq
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrirq, BIT(EpNum));
      if (Error != EFI_USB_NOERROR) {
        *TransferResult = Error;
      }
      //reset DMA
      Ctrl->DmaDrv->DmaControllerReset (Ctrl->DmaController);
      goto ProcExit;
    }
  } else {
    if ((PhytiumUsbRead16 (&Ctrl->Regs->Txirq) & BIT(EpNum))
          && (Ctrl->DmaDrv->DmaCheckInterrupt (Ctrl->DmaController, EpNum, IsIn) == TRUE)) {
      //clear ep tx interrupt
      State = TRUE;
      Ctrl->DmaDrv->DmaClearInterrupt (Ctrl->DmaController, EpNum, IsIn);
      PhytiumUsbWrite16 (&Ctrl->Regs->Txirq, BIT(EpNum));
      goto ProcExit;
    }
    if (PhytiumUsbRead16 (&Ctrl->Regs->Txerrirq) & BIT(EpNum)) {
      Code = (PhytiumUsbRead8 (&Ctrl->Regs->EpExt[EpNum - 1].TxErr) >> 2) & 0x7;
      State = TRUE;
      //Check ep tx error irq
      Error = ParseErrorCode (Code);
      //clear ep tx error irq
      PhytiumUsbWrite16 (&Ctrl->Regs->Txerrirq, BIT(EpNum));
      if (Error != EFI_USB_NOERROR) {
        *TransferResult = Error;
      }
      //reset DMA
      Ctrl->DmaDrv->DmaControllerReset (Ctrl->DmaController);
      goto ProcExit;
    }
  }

ProcExit:
  return State;
}

/**
  Read toggle state form host controller.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  EpNum   Epdpoint number, 0 - 15.

  @retval     Toggle state, 0 or 1.
**/
STATIC
UINT8
HostGetToggle (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               EpNum
  )
{
  UINT8  Toggle;

  //PhytiumUsbWrite8 (&Ctrl->Regs->Endprst, EpNum);
  Toggle = (PhytiumUsbRead8 (&Ctrl->Regs->Endprst) & BIT7) >> 7;

  return Toggle;
}

/**
  Handle to detect OTG interrupt state. Handling of controller state changes
  caused by some equipment states.

  @param[in]  Ctrl    A pointer to HOST_CTRL.

**/
VOID
PhytiumHostOtgIrq (
  IN HOST_CTRL *Ctrl
  )
{
  UINT8 Otgirq;
  UINT8 Otgien;
  UINT8 Otgstatus;
  UINT8 Otgstate;
  UINT8 Otgctrl;

  if (Ctrl == NULL) {
    return;
  }

  Otgirq = PhytiumUsbRead8 (&Ctrl->Regs->Otgirq);
  Otgien = PhytiumUsbRead8 (&Ctrl->Regs->Otgien);
  Otgstatus = PhytiumUsbRead8 (&Ctrl->Regs->Otgstatus);
  Otgstate = PhytiumUsbRead8 (&Ctrl->Regs->Otgstate);
  Otgirq &= Otgien;

  if (!Otgirq) {
    return;
  }

  if (Otgirq & OTGIRQ_BSE0SRPIRQ) {
    Otgirq &= ~OTGIRQ_BSE0SRPIRQ;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_BSE0SRPIRQ);

    Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);
    Otgctrl &= ~OTGIRQ_BSE0SRPIRQ;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
  }

  if (Otgirq & OTGIRQ_SRPDETIRQ) {
    Otgirq &= ~OTGIRQ_SRPDETIRQ;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_SRPDETIRQ);

    Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);
    Otgctrl &= ~OTGIRQ_SRPDETIRQ;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
  }

  if (Otgirq & OTGIRQ_VBUSERRIRQ) {
    Otgirq &= ~OTGIRQ_VBUSERRIRQ;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_VBUSERRIRQ);

    if (Otgstate != HOST_OTG_STATE_A_VBUS_ERR) {
      DEBUG ((DEBUG_INFO, "IRQ OTG: VBUS ERROR Babble\n"));
      return;
    }

    HostSetVbus (Ctrl, 0);
    Ctrl->OtgState = HOST_OTG_STATE_A_VBUS_ERR;
    if (Ctrl->PortStatus & USB_PORT_STAT_CONNECTION) {
      Ctrl->PortStatus = USB_PORT_STAT_POWER | (USB_PORT_STAT_C_CONNECTION << 16);
      return;
    }

    if (Ctrl->VBusErrCnt >= 3) {
      Ctrl->VBusErrCnt = 0;
      DEBUG ((DEBUG_INFO, "%s %d VBUS OVER CURRENT\n", __FUNCTION__, __LINE__));
      Ctrl->PortStatus |= USB_PORT_STAT_OVERCURRENT |
        (USB_PORT_STAT_C_OVERCURRENT << 16);

      PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDLEIRQ);
    } else {
      Ctrl->VBusErrCnt++;
      HostSetVbus (Ctrl, 1);
      PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDLEIRQ);
    }
  }

  if (Otgirq & OTGIRQ_CONIRQ) {
    if (Ctrl->OtgState == HOST_OTG_STATE_A_HOST ||
        Ctrl->OtgState == HOST_OTG_STATE_B_HOST ||
        Ctrl->OtgState == HOST_OTG_STATE_A_SUSPEND) {
      if (Otgstate == HOST_OTG_STATE_A_WAIT_VFALL ||
          Otgstate == HOST_OTG_STATE_A_WAIT_BCON ||
          Otgstate == HOST_OTG_STATE_A_SUSPEND) {
        DisconnectHostDetect (Ctrl);
        }
    } else if (Ctrl->OtgState != HOST_OTG_STATE_A_HOST &&
        Ctrl->OtgState != HOST_OTG_STATE_B_HOST &&
        Ctrl->OtgState != HOST_OTG_STATE_A_SUSPEND) {
      ConnectHostDetect (Ctrl, Otgstate);
    }

    PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_CONIRQ);
  }

  if (Otgirq & OTGIRQ_IDLEIRQ) {
    if (!(Otgstatus & OTGSTATUS_ID)) {
      AIdleDetect (Ctrl, Otgstate);
    } else {
      BIdleDetect (Ctrl, Otgstate);
    }
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDCHANGEIRQ | OTGIRQ_SRPDETIRQ);
}

/**
  Handling of host controller to suspend or not suspend the port.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  On      1 - suspend, 0 - not suspend.

  @retval     EFI_SUCCESS    Success.
**/
EFI_STATUS
PhytiumHubPortSuspend (
  IN HOST_CTRL *Ctrl,
  IN UINT16    On
  )
{
  UINT8 Otgctrl;

  if (Ctrl == NULL) {
    return EFI_SUCCESS;
  }

  Otgctrl = PhytiumUsbRead8 (&Ctrl->Regs->Otgctrl);

  if (On != 0) {
    Otgctrl &= ~OTGCTRL_BUSREQ;
    Otgctrl &= ~OTGCTRL_BHNPEN;

    Ctrl->PortStatus |= USB_PORT_STAT_SUSPEND;

    switch (PhytiumUsbRead8 (&Ctrl->Regs->Otgstate)) {
    case HOST_OTG_STATE_A_HOST:
      Ctrl->OtgState = HOST_OTG_STATE_A_SUSPEND;
      Otgctrl |= OTGCTRL_ASETBHNPEN;
      break;
    case HOST_OTG_STATE_B_HOST:
      Ctrl->OtgState = HOST_OTG_STATE_B_HOST_2;
      break;
    default:
      break;
    }
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
  } else {
    Otgctrl |= OTGCTRL_BUSREQ;
    Otgctrl &= ~OTGCTRL_ASETBHNPEN;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, Otgctrl);
    Ctrl->PortStatus |= USB_PORT_STAT_RESUME;
  }

  return EFI_SUCCESS;
}

/**
  Handling of host controller to reset or not reset the port.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  On      1 - reset, 0 - not reset.

**/
VOID
PhytiumHubPortReset (
  IN HOST_CTRL *Ctrl,
  IN UINT8     On
  )
{
  UINT8 Speed;

  if (Ctrl == NULL) {
    return;
  }

  if (On != 0) {
    PhytiumUsbWrite16 (&Ctrl->Regs->Txerrirq, 0xFFFF);
    PhytiumUsbWrite16 (&Ctrl->Regs->Txirq, 0xFFFF);
    PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrirq, 0xFFFF);
    PhytiumUsbWrite16 (&Ctrl->Regs->Rxirq, 0xFFFF);

    PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, FIFOCTRL_FIFOAUTO | 0 | 0x04);
    PhytiumUsbWrite8 (&Ctrl->Regs->Ep0fifoctrl, FIFOCTRL_FIFOAUTO |
        FIFOCTRL_IO_TX | 0 | 0x04);

    Ctrl->PortStatus |= USB_PORT_STAT_RESET;
    Ctrl->PortStatus &= ~USB_PORT_STAT_ENABLE;
    Ctrl->PortResetting = 0;
  } else {
    Speed = PhytiumUsbRead8 (&Ctrl->Regs->Speedctrl);
    if (Speed == SPEEDCTRL_HS) {
      Ctrl->PortStatus |= USB_PORT_STAT_HIGH_SPEED;
    } else if (Speed == SPEEDCTRL_FS) {
      Ctrl->PortStatus &= ~(USB_PORT_STAT_HIGH_SPEED | USB_PORT_STAT_LOW_SPEED);
    } else {
      Ctrl->PortStatus |= USB_PORT_STAT_LOW_SPEED;
    }
    Ctrl->PortStatus &= ~USB_PORT_STAT_RESET;
    Ctrl->PortStatus |= USB_PORT_STAT_ENABLE | (USB_PORT_STAT_C_RESET << 16)
      | (USB_PORT_STAT_C_ENABLE << 16);
  }
}

/**
  Initialize USB controller, clear all interrupt state. Get host protocol,
  DMA protocol and register base address.

  @param[in]  Ctrl        A pointer to HOST_CTRL.
  @param[in]  Config      A pointer to HOST_CFG.
  @param[in]  Ptr         A pointer to DMA_CONTROLLER.
  @param[in]  IsVhubHost  Vhub host state, 1 - host, 0 - not host.

  @retval     EFI_INVALID_PARAMETER   Some parameters are invalid.
  @retval     EFI_SUCCESS             Success.
**/
EFI_STATUS
PhytiumHostInit (
  IN HOST_CTRL      *Ctrl,
  IN HOST_CFG       *Config,
  IN DMA_CONTROLLER *Ptr,
  IN BOOLEAN        IsVhubHost
  )
{
  if ((Config == NULL) || (Ctrl == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&Ctrl->HostCfg, Config, sizeof (HOST_CFG));
  Ctrl->Regs = (HW_REGS *)Config->RegBase;
  Ctrl->DmaRegs = (DMA_REG *)(Config->RegBase + 0x400);
  Ctrl->HostDrv = HOSTGetInstance();

  Ctrl->DmaDrv = DMAGetInstance();
  Ctrl->DmaCfg.DmaModeRx = 0xFFFF;
  Ctrl->DmaCfg.DmaModeTx = 0xFFFF;
  Ctrl->DmaCfg.RegBase = Config->RegBase + 0x400;

  Ptr->Regs = (DMA_REG *)Ctrl->DmaCfg.RegBase;
  Ctrl->DmaController = (VOID *)Ptr;

  PhytiumUsbWrite8 (&Ctrl->Regs->Cpuctrl, BIT1);
  PhytiumUsbWrite8 (&Ctrl->Regs->Otgctrl, OTGCTRL_ABUSDROP);
  PhytiumUsbWrite8 (&Ctrl->Regs->Ep0maxpack, 0x40);

  //
  //disable interrupts
  //
  PhytiumUsbWrite8 (&Ctrl->Regs->Otgien, 0x0);
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbien, 0x0);
  PhytiumUsbWrite16 (&Ctrl->Regs->Txerrien, 0x0);
  PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrien, 0x0);

  //
  //clear all interrupt except otg idle irq
  //
  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, 0xFE);
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbirq, 0xFF);
  PhytiumUsbWrite16 (&Ctrl->Regs->Txerrirq, 0xFF);
  PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrirq, 0xFF);

  PhytiumUsbWrite8 (&Ctrl->Regs->Tawaitbcon, 0x80);

  Ctrl->OtgState = HOST_OTG_STATE_B_IDLE;

  if (IsVhubHost) {
    Ctrl->VhubRegs = (VHUB_REG *)(Config->PhyRegBase);
  } else {
    Ctrl->CustomRegs = (CUSTOM_REG *)(Config->PhyRegBase);
  }

  return EFI_SUCCESS;
}

/**
  Destroy the usb controller configuration.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
VOID
PhytiumHostDestroy (
  IN HOST_CTRL *Ctrl
  )
{
}

/**
  This function can be called only if firmware configuration supports OTG (isOtg)
  or Embedded Host (isEmbeddedHost) feature. The controller can work normally
  only after it starts.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
VOID
PhytiumHostStart (
  HOST_CTRL *Ctrl
  )
{
  UINT8  Otgstate;
  UINT8  Usbien;

  if (Ctrl == NULL) {
    return;
  }

  Usbien = PhytiumUsbRead8 (&Ctrl->Regs->Usbien);
  Usbien = Usbien | USBIR_URES | USBIR_SUSP;
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbien, Usbien);
Retry:
  Otgstate = PhytiumUsbRead8 (&Ctrl->Regs->Otgstate);
  switch (Otgstate) {
  case HOST_OTG_STATE_A_IDLE:
    Ctrl->Ep0State = HOST_EP0_STAGE_IDLE;
    Ctrl->OtgState = HOST_OTG_STATE_A_IDLE;
    PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, OTGIRQ_IDLEIRQ);
    HostSetVbus (Ctrl, 1);
    break;
  case HOST_OTG_STATE_B_IDLE:
    HostSetVbus (Ctrl, 1);
    break;
  case HOST_OTG_STATE_A_WAIT_VFALL:
    goto Retry;
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgien, OTGIRQ_CONIRQ |
      OTGIRQ_VBUSERRIRQ | OTGIRQ_SRPDETIRQ);
}

/**
  Stop the host controller. Clear and disable all interrupts, power off Vbus.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
VOID
PhytiumHostStop (
  IN HOST_CTRL *Ctrl
  )
{
  if (Ctrl == NULL) {
    return;
  }

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgien, 0x0);
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbien, 0x0);
  PhytiumUsbWrite16 (&Ctrl->Regs->Txerrien, 0x0);
  PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrien, 0x0);

  PhytiumUsbWrite8 (&Ctrl->Regs->Otgirq, 0xFE);
  PhytiumUsbWrite8 (&Ctrl->Regs->Usbirq, 0xFF);
  PhytiumUsbWrite16 (&Ctrl->Regs->Txerrirq, 0xFF);
  PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrirq, 0xFF);

  HostSetVbus (Ctrl, 0);
}

/**
  Disable all endpoints of host controller.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

  @retval     EFI_SUCCESS Success.
**/
EFI_STATUS
PhytiumHostEpDisable (
  IN HOST_CTRL *Ctrl
  )
{
  return EFI_SUCCESS;
}

/**
  Submits control transfer to a target USB device, in direction.
  Three stage:
  1.Setup token.
  2.Data in.
  3.Ack out.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         The target device address.
  @param  DeviceSpeed           Target device speed.
  @param  MaximumPacketLength   Maximum packet size the default control transfer
                                endpoint is capable of sending or receiving.
  @param  Request               USB device request to send.
  @param  Data                  Data buffer to be transmitted or received from USB
                                device.
  @param  DataLength            The size (in bytes) of the data buffer.
  @param  TimeOut               Indicates the maximum timeout, in millisecond.
  @param  Translator            Transaction translator to be used by this device.
  @param  TransferResult        Return the result of this control transfer.

  @retval EFI_SUCCESS           Transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resources.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           Transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      Transfer failed due to host controller or device error.
**/
EFI_STATUS
PhytiumHostControlTransferDataIn (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  UINT32      Index;
  DMA_TRB     *TrbAddr;
  DMA_TRB     *Trb;
  VOID        *Setup;
  HOST_REQ    Req;
  VOID        *Buffer;

  Times = TimeOut;
  Status = EFI_SUCCESS;
  if (Request == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8)  && (MaximumPacketLength != 16) &&
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Data == NULL) || (*DataLength == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Setup = AllocatePool (8);
  if (Setup == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Trb = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB) * 3);
  if (Trb == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  ZeroMem (&Req, sizeof (HOST_REQ));

  //Stage 1 : Send token setup
  CopyMem (Setup, Request, 8);
  Req.Type = UsbRequestControl;
  Req.FrameInterval = 0;
  Req.DeviceSpeed = DeviceSpeed;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = 0;
  Req.Stage = HOST_EP0_STAGE_SETUP;
  Req.IsIn = TRANSFER_OUT;
  HostRequestProgram (Ctrl, &Req);
  //Dma config
  TrbAddr = (DMA_TRB *) Trb;
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Setup);
  TrbAddr->DmaSize = 8;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 tx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  //Stage 2 : Data In
  TrbAddr = (DMA_TRB *) (Trb + 1);
  TrbAddr->DmaAddr = (UINT32)(UINT64)Buffer;
  TrbAddr->DmaSize = *DataLength;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT2 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  //MicroSecondDelay (1 * 1000);
  Req.Stage = HOST_EP0_STAGE_IN;
  Req.IsIn = TRANSFER_IN;
  HostRequestProgram (Ctrl, &Req);
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 rx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "Data[%d] : 0x%02x\n", Index, (UINT8)*(UINT8 *)(Data + Index)));
  }
  //Stage 3 : send ACK
  TrbAddr = (DMA_TRB *) (Trb + 2);
  TrbAddr->DmaAddr = (UINT32)(UINT64)Setup;
  TrbAddr->DmaSize = 0;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  Req.Stage = HOST_EP0_STAGE_STATUSOUT;
  Req.IsIn = TRANSFER_OUT;
  HostRequestProgram (Ctrl, &Req);
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 rx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  *TransferResult = EFI_USB_NOERROR;
  CopyMem (Data, Buffer, *DataLength);

ProcError:
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (Trb != NULL) {
    FreePool (Trb);
  }
  if (Setup != NULL) {
    FreePool (Setup);
  }
  if (Buffer != NULL) {
    FreePool (Buffer);
  }
  return Status;
}

/**
  Submits control transfer to a target USB device, out direction.
  Three stage:
  1.Setup token.
  2.Data out.
  3.Ack in.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         The target device address.
  @param  DeviceSpeed           Target device speed.
  @param  MaximumPacketLength   Maximum packet size the default control transfer
                                endpoint is capable of sending or receiving.
  @param  Request               USB device request to send.
  @param  Data                  Data buffer to be transmitted or received from USB
                                device.
  @param  DataLength            The size (in bytes) of the data buffer.
  @param  TimeOut               Indicates the maximum timeout, in millisecond.
  @param  Translator            Transaction translator to be used by this device.
  @param  TransferResult        Return the result of this control transfer.

  @retval EFI_SUCCESS           Transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resources.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           Transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      Transfer failed due to host controller or device error.
**/
EFI_STATUS
PhytiumHostControlTransferDataOut (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  UINT32      Index;
  DMA_TRB     *TrbAddr;
  DMA_TRB     *Trb;
  VOID        *Setup;
  HOST_REQ    Req;
  VOID        *Buffer;

  Times = TimeOut;
  Status = EFI_SUCCESS;
  if (Request == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8)  && (MaximumPacketLength != 16) &&
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Data == NULL) || (*DataLength == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Setup = AllocatePool (8);
  if (Setup == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Trb = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB) * 3);
  if (Trb == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }

  //Stage 1 : Send token setup
  CopyMem (Setup, Request, 8);
  Req.Type = UsbRequestControl;
  Req.FrameInterval = 0;
  Req.DeviceSpeed = DeviceSpeed;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = 0;
  Req.Stage = HOST_EP0_STAGE_SETUP;
  Req.IsIn = TRANSFER_OUT;
  HostRequestProgram (Ctrl, &Req);
  //Dma config
  TrbAddr = (DMA_TRB *) Trb;
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Setup);
  TrbAddr->DmaSize = 8;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 tx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  //Stage 2 : Data Out
  //send in packet
  CopyMem (Buffer, Data, *DataLength);
  TrbAddr = (DMA_TRB *) (Trb + 1);
  TrbAddr->DmaAddr = (UINT32)(UINT64)Buffer;
  TrbAddr->DmaSize = *DataLength;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "Data[%d] : 0x%02x\n", Index, (UINT8)*(UINT8 *)(Data + Index)));
  }
  Req.Stage = HOST_EP0_STAGE_OUT;
  Req.IsIn = TRANSFER_OUT;
  HostRequestProgram (Ctrl, &Req);
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 tx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  //Stage 3 : Rx ACK
  TrbAddr = (DMA_TRB *) (Trb + 2);
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Setup);
  TrbAddr->DmaSize = 0;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Req.Stage = HOST_EP0_STAGE_STATUSIN;
  Req.IsIn = TRANSFER_IN;
  HostRequestProgram (Ctrl, &Req);
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 rx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_DEVICE_ERROR;
      goto ProcError;
    }
  }
  *TransferResult = EFI_USB_NOERROR;
ProcError:
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (Trb != NULL) {
    FreePool (Trb);
  }
  if (Setup != NULL) {
    FreePool (Setup);
  }
  if (Buffer != NULL) {
    FreePool (Buffer);
  }
  return Status;
}

/**
  Submits control transfer to a target USB device, no data.
  Three stage:
  1.Setup token.
  2.Ack in.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         The target device address.
  @param  DeviceSpeed           Target device speed.
  @param  MaximumPacketLength   Maximum packet size the default control transfer
                                endpoint is capable of sending or receiving.
  @param  Request               USB device request to send.
  @param  TimeOut               Indicates the maximum timeout, in millisecond.
  @param  Translator            Transaction translator to be used by this device.
  @param  TransferResult        Return the result of this control transfer.

  @retval EFI_SUCCESS           Transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resources.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           Transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      Transfer failed due to host controller or device error.
**/
EFI_STATUS
PhytiumHostControlTransferNoData (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  DMA_TRB     *TrbAddr;
  VOID        *Setup;
  HOST_REQ    Req;

  Times = TimeOut;
  Status = EFI_SUCCESS;
  if (Request == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Setup = AllocatePool (8);
  if (Setup == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  TrbAddr = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB));
  if (TrbAddr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }

  //Stage 1 : Send token setup
  CopyMem (Setup, Request, 8);
  Req.Type = UsbRequestControl;
  Req.FrameInterval = 0;
  Req.DeviceSpeed = DeviceSpeed;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = 0;
  Req.Stage = HOST_EP0_STAGE_SETUP;
  Req.IsIn = TRANSFER_OUT;
  HostRequestProgram (Ctrl, &Req);
  //Dma config
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Setup);
  TrbAddr->DmaSize = 8;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 tx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  //
  //Stage 2 : ACK
  //send in packet
  //
  TrbAddr->DmaSize = 0;
  Req.Stage = HOST_EP0_STAGE_IN;
  Req.IsIn = TRANSFER_IN;
  HostRequestProgram (Ctrl, &Req);
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //wait for ep0 rx irq
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  *TransferResult = EFI_USB_NOERROR;

ProcError:
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (TrbAddr != NULL) {
    FreePool (TrbAddr);
  }
  if (Setup != NULL) {
    FreePool (Setup);
  }
  return Status;
}

/**
  Submits control transfer to a target USB device. Three type constrol transfer,
  in, out or no data.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         The target device address.
  @param  DeviceSpeed           Target device speed.
  @param  MaximumPacketLength   Maximum packet size the default control transfer
                                endpoint is capable of sending or receiving.
  @param  TransferDirection     Specifies the data direction for the data stage
  @param  Request               USB device request to send.
  @param  Data                  Data buffer to be transmitted or received from USB
                                device.
  @param  DataLength            The size (in bytes) of the data buffer.
  @param  TimeOut               Indicates the maximum timeout, in millisecond.
  @param  Translator            Transaction translator to be used by this device.
  @param  TransferResult        Return the result of this control transfer.

  @retval EFI_SUCCESS           Transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resources.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           Transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      Transfer failed due to host controller or device error.
 **/
EFI_STATUS
PhytiumHostControlTransfer (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DATA_DIRECTION              TransferDirection,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;

  if ((Request == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbDataIn) &&
      (TransferDirection != EfiUsbDataOut) &&
      (TransferDirection != EfiUsbNoData)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection == EfiUsbNoData) &&
      ((Data != NULL) || (*DataLength != 0))) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbNoData) &&
      ((Data == NULL) || (*DataLength == 0))) {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8)  && (MaximumPacketLength != 16) &&
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DeviceSpeed == EFI_USB_SPEED_LOW) && (MaximumPacketLength != 8)) {
    return EFI_INVALID_PARAMETER;
  }

  switch (TransferDirection) {
    case EfiUsbNoData:
      Status = PhytiumHostControlTransferNoData (
                 Ctrl,
                 DeviceAddress,
                 DeviceSpeed,
                 MaximumPacketLength,
                 Request,
                 TimeOut,
                 Translator,
                 TransferResult
                 );
      break;
    case EfiUsbDataIn:
      Status = PhytiumHostControlTransferDataIn (
                 Ctrl,
                 DeviceAddress,
                 DeviceSpeed,
                 MaximumPacketLength,
                 Request,
                 Data,
                 DataLength,
                 TimeOut,
                 Translator,
                 TransferResult
                 );
      break;
    case EfiUsbDataOut:
      Status = PhytiumHostControlTransferDataOut (
                 Ctrl,
                 DeviceAddress,
                 DeviceSpeed,
                 MaximumPacketLength,
                 Request,
                 Data,
                 DataLength,
                 TimeOut,
                 Translator,
                 TransferResult
                 );
      break;
    default:
      break;
  }

  return Status;
}

/**
  Submits bulk transfer to a bulk endpoint of a USB device, out direction.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the endpoint is capable of
                                sending or receiving.
  @param  Data                  Data buffer to transmit.
  @param  DataLength            The lenght of the data buffer.
  @param  DataToggle            On input, the initial data toggle for the transfer;
                                On output, it is updated to to next data toggle to
                                use of the subsequent bulk transfer.
  @param  TimeOut               Indicates the maximum time, in millisecond, which
                                the transfer is allowed to complete.
  @param  Translator            A pointr to the transaction translator data.
  @param  TransferResult        A pointer to the detailed result information of the
                                bulk transfer.

  @retval EFI_SUCCESS           The transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           The transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      The transfer failed due to host controller error.
**/
EFI_STATUS
PhytiumHostBulkTransferDataOut (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  DMA_TRB     *Trb;
  DMA_TRB     *TrbAddr;
  UINT32      Index;
  UINT32      PNum;
  UINT32      LSize;
  HOST_REQ    Req;
  VOID        *Buffer;
  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  Times = TimeOut * 1000;
  CalDmaPacketNum (*DataLength, &PNum, &LSize);
  Trb = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB) * PNum);
  if (Trb == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  CopyMem (Buffer, Data, *DataLength);
  //Host Config
  Req.Type = UsbRequestBulk;
  Req.FrameInterval = 0;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = EpNum;
  Req.IsIn = TRANSFER_OUT;
  Req.Toggle = *DataToggle;
  HostRequestProgram (Ctrl, &Req);
  //DMA transfer
  for (Index = 0; Index < PNum; Index++) {
    TrbAddr = (DMA_TRB *) (Trb + Index);
    TrbAddr->DmaAddr = (UINT32)(UINT64) (Buffer) + Index * SIZE_PER_DMA_PACKET;
    if (Index < (PNum - 1)) {
      TrbAddr->DmaSize = SIZE_PER_DMA_PACKET;
    } else {
      TrbAddr->DmaSize = LSize;
    }
    TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
    //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
    //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
    //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
    Ctrl->DmaDrv->DmaChannelProgram (
                    Ctrl->DmaController,
                    Req.EpNum,
                    Req.IsIn,
                    (UINT32)(UINT64)TrbAddr
                    );
    //Wait Transfer Complete
    while (Times != 0) {
      MicroSecondDelay (1);
      if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
        if ((*TransferResult) != EFI_USB_NOERROR) {
          Status = EFI_DEVICE_ERROR;
          goto ProcError;
        } else {
          break;
        }
      }
      Times--;
      if (Times == 0) {
        *TransferResult = EFI_USB_ERR_TIMEOUT;
        Status = EFI_TIMEOUT;
        goto ProcError;
      }
    }
    Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  }
  *TransferResult = EFI_USB_NOERROR;

ProcError:
  //Update DataToggle
  *DataToggle = HostGetToggle (Ctrl, Req.EpNum);
  //Release DMA Channel
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (Trb != NULL) {
    FreePool (Trb);
  }
  if (Buffer != NULL) {
    FreePool (Buffer);
  }

  return Status;
}

/**
  Submits bulk transfer to a bulk endpoint of a USB device, in direction.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the endpoint is capable of
                                sending or receiving.
  @param  Data                  Data buffer to receive.
  @param  DataLength            The lenght of the data buffer.
  @param  DataToggle            On input, the initial data toggle for the transfer;
                                On output, it is updated to to next data toggle to
                                use of the subsequent bulk transfer.
  @param  TimeOut               Indicates the maximum time, in millisecond, which
                                the transfer is allowed to complete.
  @param  Translator            A pointr to the transaction translator data.
  @param  TransferResult        A pointer to the detailed result information of the
                                bulk transfer.

  @retval EFI_SUCCESS           The transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           The transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      The transfer failed due to host controller error.
**/
STATIC
EFI_STATUS
PhytiumHostBulkTransferDataIn (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  DMA_TRB     *Trb;
  DMA_TRB     *TrbAddr;
  UINT32      Index;
  UINT32      PNum;
  UINT32      LSize;
  HOST_REQ    Req;
  VOID        *Buffer;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  Times = TimeOut * 1000;
  CalDmaPacketNum (*DataLength, &PNum, &LSize);
  Trb = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB) * PNum);
  if (Trb == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  //Host Config
  Req.Type = UsbRequestBulk;
  Req.FrameInterval = 0;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = EpNum;
  Req.IsIn = TRANSFER_IN;
  Req.Toggle = *DataToggle;
  HostRequestProgram (Ctrl, &Req);
  //Dma Config
  for (Index = 0; Index < PNum; Index++) {
    TrbAddr = (DMA_TRB *) (Trb + Index);
    TrbAddr->DmaAddr = (UINT32)(UINT64) (Buffer) + Index * SIZE_PER_DMA_PACKET;
    if (Index < (PNum - 1)) {
      TrbAddr->DmaSize = SIZE_PER_DMA_PACKET;
    } else {
      TrbAddr->DmaSize = LSize;
    }
    TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
    //DEBUG ((DEBUG_INFO, "Trb Addr : 0x%x\n", (UINT64)TrbAddr));
    //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
    //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
    //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
    Ctrl->DmaDrv->DmaChannelProgram (
                    Ctrl->DmaController,
                    Req.EpNum,
                    Req.IsIn,
                    (UINT32)(UINT64)TrbAddr
                    );
    //Wait Transfer Complete
    while (Times != 0) {
      MicroSecondDelay (1);
      if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
        if ((*TransferResult) != EFI_USB_NOERROR) {
          Status = EFI_DEVICE_ERROR;
          goto ProcError;
        } else {
          break;
        }
      }
      Times--;
      if (Times == 0) {
        *TransferResult = EFI_USB_ERR_TIMEOUT;
        Status = EFI_TIMEOUT;
        goto ProcError;
      }
    }
    Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  }
  //Release DMA Channel
  *TransferResult = EFI_USB_NOERROR;
  CopyMem (Data, Buffer, *DataLength);

ProcError:
  //Update DataToggle
  *DataToggle = HostGetToggle (Ctrl, Req.EpNum);
  //Release DMA Channel
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (Trb != NULL) {
    FreePool (Trb);
  }
  if (Buffer != NULL) {
    FreePool (Buffer);
  }

  return Status;
}

/**
  Submits bulk transfer to a bulk endpoint of a USB device.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the endpoint is capable of
                                sending or receiving.
  @param  TransferDirection     Specifies the data direction for the data stage
  @param  Data                  Data buffer ot transmit or receive.
  @param  DataLength            The lenght of the data buffer.
  @param  DataToggle            On input, the initial data toggle for the transfer;
                                On output, it is updated to to next data toggle to
                                use of the subsequent bulk transfer.
  @param  TimeOut               Indicates the maximum time, in millisecond, which
                                the transfer is allowed to complete.
  @param  Translator            A pointr to the transaction translator data.
  @param  TransferResult        A pointer to the detailed result information of the
                                bulk transfer.

  @retval EFI_SUCCESS           The transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           The transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      The transfer failed due to host controller error.
**/
EFI_STATUS
PhytiumHostBulkTransfer (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DATA_DIRECTION              TransferDirection,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Index;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbDataIn) &&
      (TransferDirection != EfiUsbDataOut)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "[%d] : 0x%x\n", Index, *((UINT8*)Data + Index)));
  }
  switch (TransferDirection) {
  case EfiUsbDataOut:
    Status = PhytiumHostBulkTransferDataOut (
               Ctrl,
               DeviceAddress,
               EpNum,
               MaximumPacketLength,
               Data,
               DataLength,
               DataToggle,
               TimeOut,
               Translator,
               TransferResult
               );
    break;
  case EfiUsbDataIn:
    Status = PhytiumHostBulkTransferDataIn (
               Ctrl,
               DeviceAddress,
               EpNum,
               MaximumPacketLength,
               Data,
               DataLength,
               DataToggle,
               TimeOut,
               Translator,
               TransferResult
               );
    break;
  default:
    break;
  }

  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "[%d] : 0x%x\n", Index, *((UINT8*)Data + Index)));
  }

  return Status;
}

/**
  Submits synchronous interrupt transfer to an interrupt endpoint
  of a USB device, in direction, need to wait the completion.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the target endpoint is capable
                                of sending or receiving.
  @param  Data                  Buffer of data that will be received from USB device.
  @param  DataLength            On input, the size, in bytes, of the data buffer; On
                                output, the number of bytes transferred.
  @param  DataToggle            On input, the initial data toggle to use; on output,
                                it is updated to indicate the next data toggle.
  @param  TimeOut               Maximum time, in second, to complete.
  @param  Interval              Interval time.
  @param  Translator            Transaction translator to use.
  @param  TransferResult        Variable to receive the transfer result.

  @return EFI_SUCCESS           The transfer was completed successfully.
  @return EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @return EFI_INVALID_PARAMETER Some parameters are invalid.
  @return EFI_TIMEOUT           The transfer failed due to timeout.
  @return EFI_DEVICE_ERROR      The failed due to host controller or device error
**/
EFI_STATUS
PhytiumHostInterruptTransferIn (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  UINTN                               Interval,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  DMA_TRB     *TrbAddr;
  HOST_REQ    Req;
  VOID        *Buffer;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  Times = TimeOut;
  TrbAddr = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB));
  if (TrbAddr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }

  //Host Config
  Req.Type = UsbRequestInt;
  Req.FrameInterval = Interval;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = EpNum;
  Req.IsIn = TRANSFER_IN;
  Req.Toggle = *DataToggle;
  HostRequestProgram (Ctrl, &Req);
  //Dma Config
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Buffer);
  TrbAddr->DmaSize = *DataLength;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb Addr : 0x%x\n", (UINT64)TrbAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //Wait Transfer Complete
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  *TransferResult = EFI_USB_NOERROR;
  CopyMem (Data, Buffer, *DataLength);

ProcError:
  //Update DataToggle
  *DataToggle = HostGetToggle (Ctrl, Req.EpNum);
  //Release DMA Channel
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (TrbAddr != NULL) {
    FreePool (TrbAddr);
  }

  return Status;

}

/**
  Submits synchronous interrupt transfer to an interrupt endpoint
  of a USB device, out direction, need to wait the completion.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the target endpoint is capable
                                of sending or receiving.
  @param  Data                  Buffer of data that will be transmit to USB device.
  @param  DataLength            On input, the size, in bytes, of the data buffer; On
                                output, the number of bytes transferred.
  @param  DataToggle            On input, the initial data toggle to use; on output,
                                it is updated to indicate the next data toggle.
  @param  TimeOut               Maximum time, in second, to complete.
  @param  Interval              Interval time.
  @param  Translator            Transaction translator to use.
  @param  TransferResult        Variable to receive the transfer result.

  @return EFI_SUCCESS           The transfer was completed successfully.
  @return EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @return EFI_INVALID_PARAMETER Some parameters are invalid.
  @return EFI_TIMEOUT           The transfer failed due to timeout.
  @return EFI_DEVICE_ERROR      The failed due to host controller or device error
**/
EFI_STATUS
PhytiumHostInterruptTransferOut (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  UINTN                               Interval,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Times;
  DMA_TRB     *TrbAddr;
  HOST_REQ    Req;
  VOID        *Buffer;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  Times = TimeOut;
  TrbAddr = (DMA_TRB *) AllocatePool (sizeof (DMA_TRB));
  if (TrbAddr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  Buffer = AllocatePool (*DataLength);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcError;
  }
  CopyMem (Buffer, Data, *DataLength);

  //Host Config
  Req.Type = UsbRequestInt;
  Req.FrameInterval = Interval;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = EpNum;
  Req.IsIn = TRANSFER_OUT;
  Req.Toggle = *DataToggle;
  HostRequestProgram (Ctrl, &Req);
  //Dma Config
  TrbAddr->DmaAddr = (UINT32)(UINT64) (Buffer);
  TrbAddr->DmaSize = *DataLength;
  TrbAddr->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb Addr : 0x%x\n", (UINT64)TrbAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", TrbAddr->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", TrbAddr->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", TrbAddr->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)TrbAddr
                  );
  //Wait Transfer Complete
  while (Times != 0) {
    MicroSecondDelay (1 * 1000);
    if (HostCheckTransferComplete (Ctrl, Req.EpNum, Req.IsIn, TransferResult)) {
      if ((*TransferResult) != EFI_USB_NOERROR) {
        Status = EFI_DEVICE_ERROR;
        goto ProcError;
      } else {
        break;
      }
    }
    Times--;
    if (Times == 0) {
      *TransferResult = EFI_USB_ERR_TIMEOUT;
      Status = EFI_TIMEOUT;
      goto ProcError;
    }
  }
  *TransferResult = EFI_USB_NOERROR;

ProcError:
  //Update DataToggle
  *DataToggle = HostGetToggle (Ctrl, Req.EpNum);
  //Release DMA Channel
  Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, Req.EpNum, Req.IsIn);
  if (TrbAddr != NULL) {
    FreePool (TrbAddr);
  }

  return Status;
}

/**
  Submits synchronous interrupt transfer to an interrupt endpoint
  of a USB device, need to wait the completion.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the target endpoint is capable
                                of sending or receiving.
  @param  TransferDirection     Specifies the data direction for the data stage
  @param  Data                  Buffer of data that will be transmitted to  USB
                                device or received from USB device.
  @param  DataLength            On input, the size, in bytes, of the data buffer; On
                                output, the number of bytes transferred.
  @param  DataToggle            On input, the initial data toggle to use; on output,
                                it is updated to indicate the next data toggle.
  @param  TimeOut               Maximum time, in second, to complete.
  @param  Translator            Transaction translator to use.
  @param  TransferResult        Variable to receive the transfer result.

  @return EFI_SUCCESS           The transfer was completed successfully.
  @return EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @return EFI_INVALID_PARAMETER Some parameters are invalid.
  @return EFI_TIMEOUT           The transfer failed due to timeout.
  @return EFI_DEVICE_ERROR      The failed due to host controller or device error
**/
EFI_STATUS
PhytiumHostInterruptTransfer (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DATA_DIRECTION              TransferDirection,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  EFI_STATUS  Status;
  UINT32      Index;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbDataIn) &&
      (TransferDirection != EfiUsbDataOut)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "[%d] : 0x%x\n", Index, *((UINT8*)Data + Index)));
  }
  switch (TransferDirection) {
  case EfiUsbDataOut:
    Status = PhytiumHostInterruptTransferOut (
               Ctrl,
               DeviceAddress,
               EpNum,
               MaximumPacketLength,
               Data,
               DataLength,
               DataToggle,
               TimeOut,
               0,
               Translator,
               TransferResult
               );
    break;
  case EfiUsbDataIn:
    Status = PhytiumHostInterruptTransferIn (
               Ctrl,
               DeviceAddress,
               EpNum,
               MaximumPacketLength,
               Data,
               DataLength,
               DataToggle,
               TimeOut,
               0,
               Translator,
               TransferResult
               );
    break;
  default:
    break;
  }

  for (Index = 0; Index < *DataLength; Index++) {
    //DEBUG ((DEBUG_INFO, "[%d] : 0x%x\n", Index, *((UINT8*)Data + Index)));
  }

  return Status;
}

/**
  Submits asynchronous interrupt transfer to an interrupt endpoint
  of a USB device, only transmit in packet, not need to wait the completion.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the target endpoint is capable
                                of sending or receiving.
  @param  Data                  Buffer of data that will be transmitted to  USB
                                device or received from USB device.
  @param  DataLength            On input, the size, in bytes, of the data buffer; On
                                output, the number of bytes transferred.
  @param  DataToggle            On input, the initial data toggle to use; on output,
                                it is updated to indicate the next data toggle.
  @param  Interval              Interval time.
  @param  Translator            Transaction translator to use.
  @param  Trb                   DMA trb to transmit.

  @return EFI_SUCCESS           The transfer was completed successfully.
  @return EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @return EFI_INVALID_PARAMETER Some parameters are invalid.
**/
EFI_STATUS
PhytiumHostAsyncInterruptTransfer (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EpNum,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               Interval,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN  DMA_TRB                             *Trb
  )
{
  HOST_REQ    Req;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Trb == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //Host Config
  Req.Type = UsbRequestInt;
  Req.FrameInterval = Interval;
  Req.DeviceAddress = DeviceAddress;
  Req.MaximumPacketLength = MaximumPacketLength;
  Req.EpNum = EpNum;
  Req.IsIn = TRANSFER_IN;
  Req.Toggle = *DataToggle;
  HostRequestProgram (Ctrl, &Req);
  //Dma Config
  Trb->DmaAddr = (UINT32)(UINT64) (Data);
  Trb->DmaSize = *DataLength;
  Trb->Ctrl = BIT10 | BIT0 | BIT5;
  //DEBUG ((DEBUG_INFO, "Trb DmaAddr : 0x%x\n", Trb->DmaAddr));
  //DEBUG ((DEBUG_INFO, "Trb DmaSize : 0x%x\n", Trb->DmaSize));
  //DEBUG ((DEBUG_INFO, "Trb Ctrl : 0x%x\n", Trb->Ctrl));
  Ctrl->DmaDrv->DmaChannelProgram (
                  Ctrl->DmaController,
                  Req.EpNum,
                  Req.IsIn,
                  (UINT32)(UINT64)Trb
                  );

  return EFI_SUCCESS;
}

/**
  Check whether the async transmission is completed or not.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  EpNum                 Endpoint number.
  @param  Toggle                Updated to indicate the next data toggle.
  @param  TransferResult        Variable to receive the transfer result.

  @retval TRUE                  Completed.
  @retval FALSE                 Not completed.
**/
BOOLEAN
PhytiumHostAsyncCheckComplete (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               EpNum,
  OUT UINT8                               *Toggle,
  OUT UINT32                              *TransferResult
  )
{
  BOOLEAN  State;
  UINT8    Code;
  UINT8    Error;
  //
  //not completed default
  //
  State = FALSE;
  *TransferResult = EFI_USB_NOERROR;

  if (PhytiumUsbRead16 (&Ctrl->Regs->Rxirq) & BIT(EpNum)) {
    //clear ep rx interrupt
    State = TRUE;
    PhytiumUsbWrite16 (&Ctrl->Regs->Rxirq, BIT(EpNum));
    Code = (PhytiumUsbRead8 (&Ctrl->Regs->EpExt[EpNum - 1].RxErr) >> 2) & 0x7;
    //Check ep rx error irq
    if (PhytiumUsbRead16 (&Ctrl->Regs->Rxerrirq) & BIT(EpNum)) {
      Error = ParseErrorCode (Code);
      //clear ep rx error irq
      PhytiumUsbWrite16 (&Ctrl->Regs->Rxerrirq, BIT(EpNum));
      if (Error != EFI_USB_NOERROR) {
        *TransferResult = Error;
      }
    }
    //Update DataToggle
    *Toggle = HostGetToggle (Ctrl, EpNum);
    //Release DMA Channel
    Ctrl->DmaDrv->DmaChannelRelease (Ctrl->DmaController, EpNum, TRANSFER_IN);
  }

  return State;
}


/**
  Get PHYTIUM_USB2_HOST_OBJ protocol.

  @retval  PHYTIUM_USB2_HOST_OBJ instance.
**/
PHYTIUM_USB2_HOST_OBJ *
HOSTGetInstance (
  VOID
  )
{
  return &mHostDrv;
}
