/** @file
Phytium usb2 type and struct description.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PHYTIUM_USB2_H_
#define  PHYTIUM_USB2_H_

#include <Uefi.h>

#include <IndustryStandard/Usb.h>

#include <Protocol/HardwareInterrupt2.h>
#include <Protocol/NonDiscoverableDevice.h>
#include <Protocol/Usb2HostController.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include <Protocol/HardwareInterrupt2.h>
#include <Protocol/NonDiscoverableDevice.h>
#include <Protocol/Usb2HostController.h>

#include <Uefi.h>

#include "Common.h"
#include "ComponentName.h"
#include "Dma.h"
#include "Host.h"

typedef struct _PHYTIUM_USB2_CONTROLLER     PHYTIUM_USB2_CONTROLLER;
typedef struct _PHYTIUM_USB_REQUEST_ASYNC   PHYTIUM_USB_REQUEST_ASYNC;

extern EFI_COMPONENT_NAME_PROTOCOL  gPhytiumUsb2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gPhytiumUsb2ComponentName2;

#define PHYTIUM_USB2_INSTANCE_SIG              SIGNATURE_32 ('p', 'u', 's', 'b')
#define PHYTIUM_USB2_FROM_THIS(a) \
          CR(a, PHYTIUM_USB2_CONTROLLER, Usb2Hc, PHYTIUM_USB2_INSTANCE_SIG)
#define UHC_TPL                                TPL_NOTIFY
//
// Iterate through the double linked list. NOT delete safe
//
#define EFI_LIST_FOR_EACH(Entry, ListHead)    \
  for(Entry = (ListHead)->ForwardLink; Entry != (ListHead); Entry = Entry->ForwardLink)

//
// Iterate through the double linked list. This is delete-safe.
// Do not touch NextEntry
//
#define EFI_LIST_FOR_EACH_SAFE(Entry, NextEntry, ListHead)            \
  for(Entry = (ListHead)->ForwardLink, NextEntry = Entry->ForwardLink;\
      Entry != (ListHead); Entry = NextEntry, NextEntry = Entry->ForwardLink)

#define EFI_LIST_CONTAINER(Entry, Type, Field) BASE_CR(Entry, Type, Field)

#define ASYNC_TRB_MAX_COUNT         512

struct _PHYTIUM_USB_REQUEST_ASYNC {
  LIST_ENTRY                         List;
  PHYTIUM_USB2_CONTROLLER            *Usb2Hc;
  UINT32                             FrameInterval;
  EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator;
  UINT8                              DeviceSpeed;
  UINT8                              DeviceAddress;
  UINTN                              MaximumPacketLength;
  EFI_USB_DATA_DIRECTION             TransferDirection;
  VOID                               *Data;
  UINTN                              DataLength;
  UINT8                              EpNum;
  UINT32                             TransferResult;
  EFI_ASYNC_USB_TRANSFER_CALLBACK    CallbackFunction;
  VOID                               *CallbackContext;
  UINTN                              TimeOut;
  UINT8                              Toggle;
  UINT32                             TrbIndex;
  DMA_TRB                            Trb[ASYNC_TRB_MAX_COUNT];
};

struct _PHYTIUM_USB2_CONTROLLER {
  UINT32                    Signature;
  NON_DISCOVERABLE_DEVICE   *Dev;
  UINT64                    OriginalPciAttributes;

  EFI_USB2_HC_PROTOCOL      Usb2Hc;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_EVENT                 ExitBootServiceEvent;
  EFI_EVENT                 PollTimer;

  UINT64                    RegsAddr;
  UINT64                    PhyRegsAddr;
  UINT32                    Irq;
  EFI_USB_HC_STATE          HcState;
  //
  //Host
  //
  PHYTIUM_USB2_HOST_OBJ     *HostObj;
  HOST_CFG                  HostCfg;
  HOST_CTRL                 HostCtrl;
  //
  //Dma
  //
  PHYTIUM_USB2_DMA_OBJ      *DmaObj;
  DMA_CFG                   DmaCfg;
  BOOLEAN                   IsVhubHost;
  DMA_CONTROLLER            DmaController;
  //
  // Misc
  //
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
  //
  //Async Interrup Request List
  //
  LIST_ENTRY                AsyncReqList;
};

extern EFI_DRIVER_BINDING_PROTOCOL gPhytiumUsb2DriverBinding;

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
  );

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
  );

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
  );

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
  );

/**
  Retrieves the current status of a USB root hub port.

  @param[in]   This                  This EFI_USB2_HC_PROTOCOL instance.
  @param[in]   PortNumber            The root hub port to retrieve the state from.
                                This value is zero-based.
  @param[out]  PortStatus            Variable to receive the port state.

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Submits an asynchronous interrupt transfer to an
  interrupt endpoint of a USB device.

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
  @param[out]     Context               Context to CallBackFunction.

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

#endif
