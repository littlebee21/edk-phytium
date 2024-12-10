/** @file
Phytium usb otg controller driver.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Usb2.h"

PHYTIUM_USB2_CONTROLLER  *gPhytiumUsb2CtrlTemplate = NULL;

EFI_DRIVER_BINDING_PROTOCOL gPhytiumUsb2DriverBinding = {
  PhytiumUsb2DriverBindingSupported,
  PhytiumUsb2DriverBindingStart,
  PhytiumUsb2DriverBindingStop,
  0x20,
  NULL,
  NULL
};

EFI_USB2_HC_PROTOCOL  gPhytiumUsb2HcTemplate = {
  PhytiumUsb2GetCapability,
  PhytiumUsb2Reset,
  PhytiumUsb2GetState,
  PhytiumUsb2SetState,
  PhytiumUsb2ControlTransfer,
  PhytiumUsb2BulkTransfer,
  PhytiumUsb2AsyncInterruptTransfer,
  PhytiumUsb2SyncInterruptTransfer,
  PhytiumUsb2IsochronousTransfer,
  PhytiumUsb2AsyncIsochronousTransfer,
  PhytiumUsb2GetRootHubPortStatus,
  PhytiumUsb2SetRootHubPortFeature,
  PhytiumUsb2ClearRootHubPortFeature,
  0x2,
  0x0
};

/**
  Find the specified sync request in the AsyncReqList.

  @param[in]  Usb2Hc           This EFI_USB2_HC_PROTOCOL instance.
  @param[in]  DeviceAddress    The target device address.
  @param[in]  EndPointAddress  Endpoint number and its direction in bit 7.

  @retval     The result of finding.
**/
STATIC
PHYTIUM_USB_REQUEST_ASYNC *
FindAsyncIntTransferReq (
  IN PHYTIUM_USB2_CONTROLLER  *Usb2Hc,
  IN UINT8                    DeviceAddress,
  IN UINT8                    EndPointAddress
  )
{
  LIST_ENTRY                 *Entry;
  PHYTIUM_USB_REQUEST_ASYNC  *Req;

  EFI_LIST_FOR_EACH (Entry, &Usb2Hc->AsyncReqList) {
    Req = EFI_LIST_CONTAINER (Entry, PHYTIUM_USB_REQUEST_ASYNC, List);
    if (Req->DeviceAddress == DeviceAddress &&
        Req->EpNum == (EndPointAddress & 0xF) &&
        Req->TransferDirection == EfiUsbDataIn) {
      return Req;
    }
  }

  return NULL;
}

/**
  Retrieves the capability of root hub ports.

  @param[in]   This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[out]  MaxSpeed              Max speed supported by the controller.
  @param[out]  PortNumber            Number of the root hub ports.
  @param[out]  Is64BitCapable        Whether the controller supports 64-bit memory
                                addressing.

  @retval EFI_SUCCESS           Host controller capability were retrieved successfully.
  @retval EFI_INVALID_PARAMETER Either of the three capability pointer is NULL.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2GetCapability (
  IN  EFI_USB2_HC_PROTOCOL  *This,
  OUT UINT8                 *MaxSpeed,
  OUT UINT8                 *PortNumber,
  OUT UINT8                 *Is64BitCapable
  )
{
  if ((MaxSpeed == NULL) || (PortNumber == NULL) || (Is64BitCapable == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *MaxSpeed = EFI_USB_SPEED_SUPER;
  *PortNumber = 1;
  *Is64BitCapable = 1;

  return EFI_SUCCESS;
}

/**
  Provides software reset for the USB host controller.

  @param[in]  This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]  Attributes            A bit mask of the reset operation to perform.

  @retval EFI_SUCCESS           The reset operation succeeded.
  @retval EFI_INVALID_PARAMETER Attributes is not valid.
  @retval EFI_UNSUPPOURTED      The type of reset specified by Attributes is
                                not currently supported by the host controller.
  @retval EFI_DEVICE_ERROR      Host controller isn't halted to reset.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2Reset (
  IN EFI_USB2_HC_PROTOCOL  *This,
  IN UINT16                Attributes
  )
{
  return EFI_SUCCESS;
}

/**
  Retrieve the current state of the USB host controller.

  @param[in]   This                   This EFI_USB2_HC_PROTOCOL instance.
  @param[out]  State                  Variable to return the current host controller
                                 state.

  @retval EFI_SUCCESS            Host controller state was returned in State.
  @retval EFI_INVALID_PARAMETER  State is NULL.
  @retval EFI_DEVICE_ERROR       An error was encountered while attempting to
                                 retrieve the host controller's current state.
**/
EFI_STATUS
EFIAPI
PhytiumUsb2GetState (
  IN   EFI_USB2_HC_PROTOCOL  *This,
  OUT  EFI_USB_HC_STATE      *State
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;

  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  *State = Usb2Hc->HcState;

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**
  Sets the USB host controller to a specific state.

  @param[in]  This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]  State                 The state of the host controller that will be set.

  @retval EFI_SUCCESS           The USB host controller was successfully placed
                                in the state specified by State.
  @retval EFI_INVALID_PARAMETER State is invalid.
  @retval EFI_DEVICE_ERROR      Failed to set the state due to device error.
**/
EFI_STATUS
EFIAPI
PhytiumUsb2SetState (
  IN EFI_USB2_HC_PROTOCOL  *This,
  IN EFI_USB_HC_STATE      State
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;

  OldTpl = gBS->RaiseTPL (UHC_TPL);
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  switch (State) {
  case EfiUsbHcStateHalt:
  case EfiUsbHcStateOperational:
  case EfiUsbHcStateSuspend:
    Usb2Hc->HcState = State;
    Status = EFI_SUCCESS;
    break;

  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Retrieves the current status of a USB root hub port.

  @param[in]  This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]  PortNumber            The root hub port to retrieve the state from.
                                    This value is zero-based.
  @param[out] PortStatus            Variable to receive the port state.

  @retval EFI_SUCCESS           The status of the USB root hub port specified.
                                by PortNumber was returned in PortStatus.
  @retval EFI_INVALID_PARAMETER PortNumber is invalid.
  @retval EFI_DEVICE_ERROR      Can't read register.
**/
EFI_STATUS
EFIAPI
PhytiumUsb2GetRootHubPortStatus (
  IN   EFI_USB2_HC_PROTOCOL  *This,
  IN   UINT8                 PortNumber,
  OUT  EFI_USB_PORT_STATUS   *PortStatus
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;

  if (PortStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  PortStatus->PortStatus = 0;
  PortStatus->PortChangeStatus = 0;


  Usb2Hc->HostObj->HostOtgIrq (&Usb2Hc->HostCtrl);

  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_CONNECTION) {
    PortStatus->PortStatus |= USB_PORT_STAT_CONNECTION;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_ENABLE) {
    PortStatus->PortStatus |= USB_PORT_STAT_ENABLE;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_SUSPEND) {
    PortStatus->PortStatus |= USB_PORT_STAT_SUSPEND;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_OVERCURRENT) {
    PortStatus->PortStatus |= USB_PORT_STAT_OVERCURRENT;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_RESET) {
    PortStatus->PortStatus |= USB_PORT_STAT_RESET;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_POWER) {
    PortStatus->PortStatus |= USB_PORT_STAT_POWER;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_LOW_SPEED) {
    PortStatus->PortStatus |= USB_PORT_STAT_LOW_SPEED;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_HIGH_SPEED) {
    PortStatus->PortStatus |= USB_PORT_STAT_HIGH_SPEED;
  }
  if (Usb2Hc->HostCtrl.PortStatus & USB_PORT_STAT_SUPER_SPEED) {
    PortStatus->PortStatus |= USB_PORT_STAT_SUPER_SPEED;
  }
  if (Usb2Hc->HostCtrl.PortStatus & (USB_PORT_STAT_C_CONNECTION << 16)) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_CONNECTION;
  }
  if (Usb2Hc->HostCtrl.PortStatus & (USB_PORT_STAT_C_ENABLE << 16)) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_ENABLE;
  }
  if (Usb2Hc->HostCtrl.PortStatus & (USB_PORT_STAT_C_SUSPEND << 16)) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_SUSPEND;
  }
  if (Usb2Hc->HostCtrl.PortStatus & (USB_PORT_STAT_C_OVERCURRENT << 16)) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_OVERCURRENT;
  }
  if (Usb2Hc->HostCtrl.PortStatus & (USB_PORT_STAT_C_RESET << 16)) {
    PortStatus->PortChangeStatus |= USB_PORT_STAT_C_RESET;
  }

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**
  Sets a feature for the specified root hub port.

  @param[in]  This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]  PortNumber            Root hub port to set.
  @param[in]  PortFeature           Feature to set.

  @retval EFI_SUCCESS           The feature specified by PortFeature was set.
  @retval EFI_INVALID_PARAMETER PortNumber is invalid or PortFeature is invalid.
  @retval EFI_DEVICE_ERROR      Can't read register.
**/
EFI_STATUS
EFIAPI
PhytiumUsb2SetRootHubPortFeature (
  IN  EFI_USB2_HC_PROTOCOL  *This,
  IN  UINT8                 PortNumber,
  IN  EFI_USB_PORT_FEATURE  PortFeature
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  switch (PortFeature) {
  case EfiUsbPortEnable:
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortSuspend:
    Status = Usb2Hc->HostObj->HostHubPortSuspend (&Usb2Hc->HostCtrl, 1);
    break;
  case EfiUsbPortReset:
    Usb2Hc->HostObj->HostHubPortReset (&Usb2Hc->HostCtrl, 1);
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortPower:
    Usb2Hc->HostObj->HostStart (&Usb2Hc->HostCtrl);
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortOwner:
    //
    //root hub port don't has the owner bit, ignore the operation
    //
    Status = EFI_SUCCESS;
    break;

  default:
    Status = EFI_INVALID_PARAMETER;
  }

  if (!EFI_ERROR (Status)) {
    Usb2Hc->HostCtrl.PortStatus |= 1 << PortFeature;
  }

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**
  Clears a feature for the specified root hub port.

  @param[in]  This                  A pointer to the EFI_USB2_HC_PROTOCOL instance.
  @param[in]  PortNumber            Specifies the root hub port whose feature is
                                    requested to be cleared.
  @param[in]  PortFeature           Indicates the feature selector associated with the
                                    feature clear request.

  @retval EFI_SUCCESS           The feature specified by PortFeature was cleared
                                for the USB root hub port specified by PortNumber.
  @retval EFI_INVALID_PARAMETER PortNumber is invalid or PortFeature is invalid.
  @retval EFI_DEVICE_ERROR      Can't read register.
**/
EFI_STATUS
EFIAPI
PhytiumUsb2ClearRootHubPortFeature (
  IN  EFI_USB2_HC_PROTOCOL  *This,
  IN  UINT8                 PortNumber,
  IN  EFI_USB_PORT_FEATURE  PortFeature
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  switch (PortFeature) {
  case EfiUsbPortEnable:
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortSuspend:
    Status = Usb2Hc->HostObj->HostHubPortSuspend (&Usb2Hc->HostCtrl, 0);
    break;
  case EfiUsbPortReset:
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortPower:
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortOwner:
    //
    //root hub port don't has the owner bit, ignore the operation
    //
    Status = EFI_SUCCESS;
    break;
  case EfiUsbPortConnectChange:
    Status = EFI_SUCCESS;
    break;
  default:
    Status = EFI_INVALID_PARAMETER;
  }

  if (!EFI_ERROR (Status)) {
    Usb2Hc->HostCtrl.PortStatus &= ~(1 << PortFeature);
  }

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**
  Submits control transfer to a target USB device.

  @param[in]      This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress         The target device address.
  @param[in]      DeviceSpeed           Target device speed.
  @param[in]      MaximumPacketLength   Maximum packet size the default control transfer
                                        endpoint is capable of sending or receiving.
  @param[in]      Request               USB device request to send.
  @param[in]      TransferDirection     Specifies the data direction for the data stage.
  @param[in,out]  Data                  Data buffer to be transmitted or received from USB
                                        device.
  @param[in,out]  DataLength            The size (in bytes) of the data buffer.
  @param[in]      TimeOut               Indicates the maximum timeout, in millisecond.
  @param[in]      Translator            Transaction translator to be used by this device.
  @param[out]     TransferResult        Return the result of this control transfer.

  @retval EFI_SUCCESS           Transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resources.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           Transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      Transfer failed due to host controller or device error.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2ControlTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  EFI_USB_DEVICE_REQUEST              *Request,
  IN  EFI_USB_DATA_DIRECTION              TransferDirection,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;

  //
  // Validate parameters
  //
  if ((Request == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbDataIn) &&
      (TransferDirection != EfiUsbDataOut) &&
      (TransferDirection != EfiUsbNoData))
  {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection == EfiUsbNoData) &&
      ((Data != NULL) || (*DataLength != 0)))
  {
    return EFI_INVALID_PARAMETER;
  }

  if ((TransferDirection != EfiUsbNoData) &&
      ((Data == NULL) || (*DataLength == 0)))
  {
    return EFI_INVALID_PARAMETER;
  }

  if ((MaximumPacketLength != 8)  && (MaximumPacketLength != 16) &&
      (MaximumPacketLength != 32) && (MaximumPacketLength != 64))
  {
    return EFI_INVALID_PARAMETER;
  }

  if ((DeviceSpeed == EFI_USB_SPEED_LOW) && (MaximumPacketLength != 8)) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  Status = Usb2Hc->HostObj->HostControlTransfer (
                              &Usb2Hc->HostCtrl,
                              DeviceAddress,
                              DeviceSpeed,
                              MaximumPacketLength,
                              TransferDirection,
                              Request,
                              Data,
                              DataLength,
                              TimeOut,
                              Translator,
                              TransferResult
                              );

  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Submits bulk transfer to a bulk endpoint of a USB device.

  @param[in]      This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress         Target device address.
  @param[in]      EndPointAddress       Endpoint number and its direction in bit 7.
  @param[in]      DeviceSpeed           Device speed, Low speed device doesn't support bulk
                                        transfer.
  @param[in]      MaximumPacketLength   Maximum packet size the endpoint is capable of
                                        sending or receiving.
  @param[in]      DataBuffersNumber     Number of data buffers prepared for the transfer.
  @param[in,out]  Data                  Array of pointers to the buffers of data to transmit
                                        from or receive into.
  @param[in,out]  DataLength            The lenght of the data buffer.
  @param[in,out]  DataToggle            On input, the initial data toggle for the transfer;
                                        On output, it is updated to to next data toggle to
                                        use of the subsequent bulk transfer.
  @param[in]      TimeOut               Indicates the maximum time, in millisecond, which
                                        the transfer is allowed to complete.
  @param[in]      Translator            A pointr to the transaction translator data.
  @param[out]     TransferResult        A pointer to the detailed result information of the
                                    bulk transfer.

  @retval EFI_SUCCESS           The transfer was completed successfully.
  @retval EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_TIMEOUT           The transfer failed due to timeout.
  @retval EFI_DEVICE_ERROR      The transfer failed due to host controller error.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2BulkTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EndPointAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  UINT8                               DataBuffersNumber,
  IN  OUT VOID                            *Data[EFI_USB_MAX_BULK_BUFFER_NUM],
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;
  EFI_USB_DATA_DIRECTION   TransferDirection;
  UINT8                    EpNum;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (Data[0] == NULL) || (TransferResult == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  if (((EndPointAddress >> 7) & 0x1 ) == 1) {
    TransferDirection = EfiUsbDataIn;
  } else {
    TransferDirection = EfiUsbDataOut;
  }
  EpNum = EndPointAddress & 0xF;
  Status = Usb2Hc->HostObj->HostBulkTransfer (
                              &Usb2Hc->HostCtrl,
                              DeviceAddress,
                              EpNum,
                              MaximumPacketLength,
                              TransferDirection,
                              Data[0],
                              DataLength,
                              DataToggle,
                              TimeOut,
                              Translator,
                              TransferResult
                              );

  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Submits an asynchronous interrupt transfer to an interrupt endpoint of a USB
  device.

  @param[in]      This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress         Target device address.
  @param[in]      EndPointAddress       Endpoint number and its direction encoded in bit 7
  @param[in]      DeviceSpeed           Indicates device speed.
  @param[in]      MaximumPacketLength   Maximum packet size the target endpoint is capable
  @param[in]      IsNewTransfer         If TRUE, to submit an new asynchronous interrupt
                                        transfer If FALSE, to remove the specified
                                        asynchronous interrupt.
  @param[in,out]  DataToggle            On input, the initial data toggle to use; on output,
                                        it is updated to indicate the next data toggle.
  @param[in]      PollingInterval       The he interval, in milliseconds, that the transfer
                                        is polled.
  @param[in]      DataLength            The length of data to receive at the rate specified
                                        by  PollingInterval.
  @param[in]      Translator            Transaction translator to use.
  @param[in]      CallBackFunction      Function to call at the rate specified by
                                        PollingInterval.
  @param[in]      Context               Context to CallBackFunction.

  @retval EFI_SUCCESS           The request has been successfully submitted or canceled.
  @retval EFI_INVALID_PARAMETER Some parameters are invalid.
  @retval EFI_OUT_OF_RESOURCES  The request failed due to a lack of resources.
  @retval EFI_DEVICE_ERROR      The transfer failed due to host controller error.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2AsyncInterruptTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EndPointAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  BOOLEAN                             IsNewTransfer,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               PollingInterval,
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK     CallBackFunction,
  IN  VOID                                *Context OPTIONAL
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                   OldTpl;
  EFI_STATUS                Status;
  PHYTIUM_USB_REQUEST_ASYNC *FoundReq;
  PHYTIUM_USB_REQUEST_ASYNC *NewReq;
  VOID                      *Data;
  UINT8                     *Ptr;

  FoundReq = NULL;
  NewReq = NULL;
  Data = NULL;
  if ((EndPointAddress & USB_ENDPOINT_DIR_IN) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);
  FoundReq = FindAsyncIntTransferReq (Usb2Hc, DeviceAddress, EndPointAddress);

  if (IsNewTransfer) {
    if (FoundReq != NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }

    if (DataLength == 0) {
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }

    if ((*DataToggle != 1) && (*DataToggle != 0)) {
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }

    if ((PollingInterval > 255) || (PollingInterval < 1)) {
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }

    if (CallBackFunction == NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }
  }

  if (!IsNewTransfer) {
    if (FoundReq == NULL) {
      DEBUG ((DEBUG_ERROR, "%u:%u> transfer not found\n", DeviceAddress, EndPointAddress & 0xF));
      Status = EFI_INVALID_PARAMETER;
      goto ProcExit;
    }

    *DataToggle = FoundReq->Toggle;
    FreePool (FoundReq->Data);

    RemoveEntryList (&FoundReq->List);
    FreePool (FoundReq);

    Status = EFI_SUCCESS;
    goto ProcExit;
  }

  NewReq = AllocateZeroPool (sizeof (PHYTIUM_USB_REQUEST_ASYNC));
  if (NewReq == NULL) {
    DEBUG ((DEBUG_ERROR, "AsyncInterruptTransfer: failed to allocate req.\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }

  Data = AllocateZeroPool (DataLength * ASYNC_TRB_MAX_COUNT );
  if (Data == NULL) {
    DEBUG ((DEBUG_ERROR, "AsyncInterruptTransfer: failed to allocate buffer\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }

  InitializeListHead (&NewReq->List);

  NewReq->Usb2Hc = Usb2Hc;
  NewReq->FrameInterval = PollingInterval;
  NewReq->Translator = Translator;
  NewReq->DeviceSpeed = DeviceSpeed;
  NewReq->DeviceAddress = DeviceAddress;
  NewReq->MaximumPacketLength = MaximumPacketLength;
  if (((EndPointAddress >> 7) & 0x1 ) == 1) {
    NewReq->TransferDirection = EfiUsbDataIn;
  } else {
    NewReq->TransferDirection = EfiUsbDataOut;
  }
  NewReq->Data = Data;
  NewReq->DataLength = DataLength;
  NewReq->EpNum = EndPointAddress & 0xF;
  NewReq->CallbackFunction = CallBackFunction;
  NewReq->CallbackContext = Context;
  NewReq->Toggle = *DataToggle;
  NewReq->TrbIndex = 0;

  InsertTailList (&Usb2Hc->AsyncReqList, &NewReq->List);
  Status = EFI_SUCCESS;
  Ptr = (UINT8 *)NewReq->Data + DataLength * NewReq->TrbIndex;
  Status = Usb2Hc->HostObj->HostAsyncInterruptTransfer (
                              &Usb2Hc->HostCtrl,
                              NewReq->DeviceAddress,
                              NewReq->EpNum,
                              NewReq->MaximumPacketLength,
                              Ptr,
                              &NewReq->DataLength,
                              &NewReq->Toggle,
                              NewReq->FrameInterval,
                              NewReq->Translator,
                              &NewReq->Trb[NewReq->TrbIndex]
                              );
  NewReq->TrbIndex++;
  if (NewReq->TrbIndex >= ASYNC_TRB_MAX_COUNT) {
    NewReq->TrbIndex = 0;
  }
ProcExit:
  gBS->RestoreTPL (OldTpl);
  if (EFI_ERROR (Status)) {
    if (Data != NULL) {
      FreePool (Data);
    }
    if (NewReq != NULL) {
      FreePool (NewReq);
    }
  }

  return Status;
}

/**
  Submits synchronous interrupt transfer to an interrupt endpoint
  of a USB device.

  @param[in]      This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress         Target device address.
  @param[in]      EndPointAddress       Endpoint number and its direction encoded in bit 7
  @param[in]      DeviceSpeed           Indicates device speed.
  @param[in]      MaximumPacketLength   Maximum packet size the target endpoint is capable
                                        of sending or receiving.
  @param[in,out]  Data                  Buffer of data that will be transmitted to  USB
                                        device or received from USB device.
  @param[in,out]  DataLength            On input, the size, in bytes, of the data buffer; On
                                        output, the number of bytes transferred.
  @param[in,out]  DataToggle            On input, the initial data toggle to use; on output,
                                        it is updated to indicate the next data toggle.
  @param[in]      TimeOut               Maximum time, in second, to complete.
  @param[in]      Translator            Transaction translator to use.
  @param[out]     TransferResult        Variable to receive the transfer result.

  @return EFI_SUCCESS           The transfer was completed successfully.
  @return EFI_OUT_OF_RESOURCES  The transfer failed due to lack of resource.
  @return EFI_INVALID_PARAMETER Some parameters are invalid.
  @return EFI_TIMEOUT           The transfer failed due to timeout.
  @return EFI_DEVICE_ERROR      The failed due to host controller or device error

**/
EFI_STATUS
EFIAPI
PhytiumUsb2SyncInterruptTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EndPointAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  OUT VOID                            *Data,
  IN  OUT UINTN                           *DataLength,
  IN  OUT UINT8                           *DataToggle,
  IN  UINTN                               TimeOut,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;
  EFI_TPL                  OldTpl;
  EFI_STATUS               Status;
  EFI_USB_DATA_DIRECTION   TransferDirection;
  UINT8                    EpNum;

  //
  // Validate the parameters
  //
  if ((DataLength == NULL) || (*DataLength == 0) ||
      (Data == NULL) || (TransferResult == NULL) || (DataToggle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*DataToggle != 0) && (*DataToggle != 1)) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = gBS->RaiseTPL (UHC_TPL);

  Status = EFI_SUCCESS;
  Usb2Hc = PHYTIUM_USB2_FROM_THIS (This);

  if (((EndPointAddress >> 7) & 0x1 ) == 1) {
    TransferDirection = EfiUsbDataIn;
  } else {
    TransferDirection = EfiUsbDataOut;
  }
  EpNum = EndPointAddress & 0xF;
  Status = Usb2Hc->HostObj->HostSyncInterruptTransfer (
                              &Usb2Hc->HostCtrl,
                              DeviceAddress,
                              EpNum,
                              MaximumPacketLength,
                              TransferDirection,
                              Data,
                              DataLength,
                              DataToggle,
                              TimeOut,
                              Translator,
                              TransferResult
                              );

  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Submits isochronous transfer to a target USB device.

  @param[in]      This                 This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress        Target device address.
  @param[in]      EndPointAddress      End point address with its direction.
  @param[in]      DeviceSpeed          Device speed, Low speed device doesn't support this
                                       type.
  @param[in]      MaximumPacketLength  Maximum packet size that the endpoint is capable of
                                       sending or receiving.
  @param[in]      DataBuffersNumber    Number of data buffers prepared for the transfer.
  @param[in,out]  Data                 Array of pointers to the buffers of data that will
                                       be transmitted to USB device or received from USB
                                       device.
  @param[in]      DataLength           The size, in bytes, of the data buffer.
  @param[in]      Translator           Transaction translator to use.
  @param[out]     TransferResult       Variable to receive the transfer result.

  @return EFI_UNSUPPORTED      Isochronous transfer is unsupported.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2IsochronousTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EndPointAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  UINT8                               DataBuffersNumber,
  IN  OUT VOID                            *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  OUT UINT32                              *TransferResult
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Submits Async isochronous transfer to a target USB device.

  @param[in]      This                 This EFI_USB2_HC_PROTOCOL instance.
  @param[in]      DeviceAddress        Target device address.
  @param[in]      EndPointAddress      End point address with its direction.
  @param[in]      DeviceSpeed          Device speed, Low speed device doesn't support this
                                       type.
  @param[in]      MaximumPacketLength  Maximum packet size that the endpoint is capable of
                                       sending or receiving.
  @param[in]      DataBuffersNumber    Number of data buffers prepared for the transfer.
  @param[in,out]  Data                 Array of pointers to the buffers of data that will
                                       be transmitted to USB device or received from USB
                                       device.
  @param[in]      DataLength           The size, in bytes, of the data buffer.
  @param[in]      Translator           Transaction translator to use.
  @param[in]      IsochronousCallBack  Function to be called when the transfer complete.
  @param[in]      Context              Context passed to the call back function as
                                   parameter.

  @return EFI_UNSUPPORTED      Isochronous transfer isn't supported.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2AsyncIsochronousTransfer (
  IN  EFI_USB2_HC_PROTOCOL                *This,
  IN  UINT8                               DeviceAddress,
  IN  UINT8                               EndPointAddress,
  IN  UINT8                               DeviceSpeed,
  IN  UINTN                               MaximumPacketLength,
  IN  UINT8                               DataBuffersNumber,
  IN  OUT VOID                            *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
  IN  UINTN                               DataLength,
  IN  EFI_USB2_HC_TRANSACTION_TRANSLATOR  *Translator,
  IN  EFI_ASYNC_USB_TRANSFER_CALLBACK     IsochronousCallBack,
  IN  VOID                                *Context
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Entry point for EFI drivers.

  @param[in]  ImageHandle       EFI_HANDLE.
  @param[in]  SystemTable       EFI_SYSTEM_TABLE.

  @return EFI_SUCCESS       Success.
          EFI_DEVICE_ERROR  Fail.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2DriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EfiLibInstallDriverBindingComponentName2 (
           ImageHandle,
           SystemTable,
           &gPhytiumUsb2DriverBinding,
           ImageHandle,
           &gPhytiumUsb2ComponentName,
           &gPhytiumUsb2ComponentName2
           );
}

/**
  Test to see if this driver supports ControllerHandle. Any
  ControllerHandle that has Usb2HcProtocol installed will
  be supported.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  Controller           Handle of device to test.
  @param[in]  RemainingDevicePath  Not used.

  @return EFI_SUCCESS          This driver supports this device.
  @return EFI_UNSUPPORTED      This driver does not support this device.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  NON_DISCOVERABLE_DEVICE    *Dev;
  EFI_STATUS                 Status;
  //
  // Connect to the non-discoverable device
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  (VOID **)&Dev,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (CompareGuid (Dev->Type, &gPhytiumUsb2NonDiscoverableDeviceGuid)) {
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_FOUND;
  }

  //
  // Clean up.
  //
  gBS->CloseProtocol (Controller,
                      &gEdkiiNonDiscoverableDeviceProtocolGuid,
                      This->DriverBindingHandle,
                      Controller);

  return Status;
}

/**
  Set default configuration to PHYTIUM_USB2_CONTROLLER.

  @param[in]  Config    A pointer to PHYTIUM_USB2_CONTROLLER.

**/
STATIC
VOID
PhytiumHostSetDefaultCfg (
  IN PHYTIUM_USB2_CONTROLLER  *Config
  )
{
  Config->HostCfg.RegBase = (UINT64) Config->RegsAddr;
  Config->HostCfg.PhyRegBase = (UINT64) Config->PhyRegsAddr;
  Config->HostCfg.DmultEnabled = 1;
  Config->HostCfg.MemoryAlignment = 0;
  Config->HostCfg.DmaSupport = 1;
  Config->HostCfg.IsEmbeddedHost = 1;
}

/**
  One notified function to stop the Host Controller when gBS->ExitBootServices() called.

  @param[in]  Event                   Pointer to this event
  @param[in]  Context                 Event handler private data
**/
VOID
EFIAPI
PhytiumUsb2ExitBootService (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  PHYTIUM_USB2_CONTROLLER  *Usb2Hc;

  Usb2Hc = (PHYTIUM_USB2_CONTROLLER *)Context;

  Usb2Hc->HostObj->HostStop (&Usb2Hc->HostCtrl);

  gBS->SetTimer (Usb2Hc->PollTimer, TimerCancel, 0);
  if (Usb2Hc->PollTimer != NULL) {
    gBS->CloseEvent (Usb2Hc->PollTimer);
  }
}

/**
  Initialize the usb host controller.

  @param[in]  Config    A pointer to PHYTIUM_USB2_CONTROLLER.

  @retval     EFI_SUCCESS    Success.
**/
EFI_STATUS
PhytiumHostInitialize (
  IN PHYTIUM_USB2_CONTROLLER  *Config
  )
{
  EFI_STATUS  Status;

  if (Config == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PhytiumHostSetDefaultCfg (Config);
  Config->HostObj = HOSTGetInstance ();
  Config->DmaCfg.RegBase = Config->HostCfg.RegBase + 0x400;

  Status = Config->HostObj->HostInit (
                              &Config->HostCtrl,
                              &Config->HostCfg,
                              &Config->DmaController,
                              Config->IsVhubHost
                              );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Config->HostObj->HostStart (&Config->HostCtrl);
  return EFI_SUCCESS;
Error:
  Config->HostObj->HostDestroy (&Config->HostCtrl);
  return EFI_SUCCESS;
}

/**
  Async interrupt transfer periodic check handler.

  @param[in]  Event                   Pointer to this event
  @param[in]  Context                 Event handler private data
**/
VOID
UsbAsyncReqHandler (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  )
{
  PHYTIUM_USB2_CONTROLLER   *Usb2Hc;
  PHYTIUM_USB_REQUEST_ASYNC *Req;
  BOOLEAN                   State;
  LIST_ENTRY                *Entry;
  LIST_ENTRY                *NextEntry;
  UINT8                     *Ptr;
  UINT8                     *Ptr1;

  Usb2Hc = (PHYTIUM_USB2_CONTROLLER *) (Context);

  EFI_LIST_FOR_EACH_SAFE (Entry, NextEntry, &Usb2Hc->AsyncReqList) {
    Req = EFI_LIST_CONTAINER (Entry, PHYTIUM_USB_REQUEST_ASYNC, List);
    //
    //Check whegher complete
    //
    State = Usb2Hc->HostObj->HostAsyncCheckComplete (
                               &Usb2Hc->HostCtrl,
                               Req->EpNum,
                               &Req->Toggle,
                               &Req->TransferResult
                               );
    if (State == TRUE) {
      //
      //CallBack
      //
      Ptr = (UINT8 *)Req->Data + Req->DataLength * Req->TrbIndex;
      //
      //Resubmit Request
      //
      Req->TrbIndex++;
      if (Req->TrbIndex >= ASYNC_TRB_MAX_COUNT) {
        Req->TrbIndex = 0;
      }
      Ptr1 = (UINT8 *)Req->Data + Req->DataLength * Req->TrbIndex;
      Usb2Hc->HostObj->HostAsyncInterruptTransfer (
                         &Usb2Hc->HostCtrl,
                         Req->DeviceAddress,
                         Req->EpNum,
                         Req->MaximumPacketLength,
                         Ptr1,
                         &Req->DataLength,
                         &Req->Toggle,
                         Req->FrameInterval,
                         Req->Translator,
                         &Req->Trb[Req->TrbIndex]
                         );
      //
      //CallBack
      //
      Req->CallbackFunction (
             Ptr,
             Req->DataLength,
             Req->CallbackContext,
             Req->TransferResult
             );
    }
  }

}

/**
  Get configuration from the non discovery share memory.

  @param[in]    Address      Share memory base address.
  @param[out]   Uhc          A pointer to PHYTIUM_USB2_CONTROLLER instance.

 **/
STATIC
VOID
PhytiumUsb2GetConfig (
  IN  EFI_PHYSICAL_ADDRESS      Address,
  OUT PHYTIUM_USB2_CONTROLLER   *Uhc
  )
{
  UINT64  Temp;
  UINT32  Value;

  Temp = Address;
  Temp += 4;
  Value = MmioRead32 (Temp);
  Uhc->Irq = Value;
}

/**
  Create and initialize a PHYTIUM_USB2_CONTROLLER structure.

  @param[in]  Uhc                    A pointer to PHYTIUM_USB2_CONTROLLER.
  @param[in]  Dev                    A pointer to NON_DISCOVERABLE_DEVICE.
  @param[in]  DevicePath             The device path of host controller.

  @retval     EFI_SUCCESS            Success.
**/
EFI_STATUS
PhytiumUsb2CreateHc (
  IN PHYTIUM_USB2_CONTROLLER   *Uhc,
  IN NON_DISCOVERABLE_DEVICE   *Dev,
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  Uhc->Signature             = PHYTIUM_USB2_INSTANCE_SIG;
  Uhc->Dev                   = Dev;
  Uhc->DevicePath            = DevicePath;
  Uhc->RegsAddr              = Dev->Resources[0].AddrRangeMin;
  Uhc->PhyRegsAddr           = Dev->Resources[1].AddrRangeMin;
  Uhc->IsVhubHost           = TRUE;
  CopyMem (&Uhc->Usb2Hc, &gPhytiumUsb2HcTemplate, sizeof (EFI_USB2_HC_PROTOCOL));
  PhytiumUsb2GetConfig (Dev->Resources[2].AddrRangeMin, Uhc);

  gPhytiumUsb2CtrlTemplate = Uhc;
  Status = PhytiumHostInitialize (Uhc);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(), Error : %r\n", __FUNCTION__, Status));
    return Status;
  }

  //
  //Init Async Request List
  //
  InitializeListHead (&Uhc->AsyncReqList);
  //
  //create hotplug monitor event handle
  //
  Status = gBS->CreateEvent (
              EVT_NOTIFY_SIGNAL | EVT_TIMER,
              TPL_CALLBACK,
              UsbAsyncReqHandler,
              Uhc,
              &Uhc->PollTimer
              );
  Status = gBS->SetTimer (
                  Uhc->PollTimer,
                  TimerPeriodic,
                  10000
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Uhc->PollTimer);
  }

  return EFI_SUCCESS;
}

/**
  Starting the Usb EHCI Driver.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  Controller           Handle of device to test.
  @param[in]  RemainingDevicePath  Not used.

  @return EFI_SUCCESS          supports this device.
  @return EFI_UNSUPPORTED      do not support this device.
  @return EFI_DEVICE_ERROR     cannot be started due to device Error.
  @return EFI_OUT_OF_RESOURCES cannot allocate resources.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  NON_DISCOVERABLE_DEVICE     *Dev;
  EFI_DEVICE_PATH_PROTOCOL    *HcDevicePath;
  PHYTIUM_USB2_CONTROLLER     *Uhc;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  (VOID **) &Dev,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open Device Path Protocol for on USB host controller
  //
  HcDevicePath = NULL;
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &HcDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  Uhc = AllocateZeroPool (sizeof (PHYTIUM_USB2_CONTROLLER));
  if (Uhc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Status = PhytiumUsb2CreateHc (Uhc, Dev, HcDevicePath);
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  // Register for an ExitBootServicesEvent
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  PhytiumUsb2ExitBootService,
                  Uhc,
                  &Uhc->ExitBootServiceEvent
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Install the component name protocol, don't fail the start
  // because of something for display.
  //
  AddUnicodeString2 (
    "eng",
    gPhytiumUsb2ComponentName.SupportedLanguages,
    &gPhytiumUsb2CtrlTemplate->ControllerNameTable,
    L"Phytium Usb2 Host Controller",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gPhytiumUsb2ComponentName2.SupportedLanguages,
    &gPhytiumUsb2CtrlTemplate->ControllerNameTable,
    L"Phytium Usb2 Host Controller",
    FALSE
    );

  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gEfiUsb2HcProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &Uhc->Usb2Hc
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PhytiumUsb2DriverBindingStart: failed to install USB2_HC Protocol\n"));
    goto Error;
  }

  return EFI_SUCCESS;
Error:
  if (Uhc != NULL) {
    FreePool (Uhc);
  }
  return Status;
}

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  Controller           Handle of device to stop driver on.
  @param[in]  NumberOfChildren     Number of Children in the ChildHandleBuffer.
  @param[in]  ChildHandleBuffer    List of handles for the children we need to stop.

  @return EFI_SUCCESS          Success.
  @return EFI_DEVICE_ERROR     Fail.

**/
EFI_STATUS
EFIAPI
PhytiumUsb2DriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;
  EFI_USB2_HC_PROTOCOL        *Usb2Hc;
  PHYTIUM_USB2_CONTROLLER     *Uhc;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsb2HcProtocolGuid,
                  (VOID **) &Usb2Hc,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Uhc = PHYTIUM_USB2_FROM_THIS (Usb2Hc);
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiUsb2HcProtocolGuid,
                  &Uhc->Usb2Hc
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Uhc->HostObj->HostStop (&Uhc->HostCtrl);

  gBS->SetTimer (Uhc->PollTimer, TimerCancel, 0);
  if (Uhc->PollTimer != NULL) {
    gBS->CloseEvent (Uhc->PollTimer);
  }
  if (Uhc->ExitBootServiceEvent != NULL) {
    gBS->CloseEvent (Uhc->ExitBootServiceEvent);
  }
  //
  // Clean up.
  //
  gBS->CloseProtocol (
         Controller,
         &gEdkiiNonDiscoverableDeviceProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  FreePool (Uhc);

  return EFI_SUCCESS;
}
