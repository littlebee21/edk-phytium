/** @file
Phytium usb2 host type and struct description.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PHYTIUM_USB2_HOST_H_
#define  PHYTIUM_USB2_HOST_H_

#include <Protocol/Usb2HostController.h>

#include "Dma.h"
#include "Reg.h"

typedef enum _HOST_EP0_STAGE             HOST_EP0_STAGE;
typedef enum _HOST_OTGSTATE              HOST_OTGSTATE;
typedef enum _HOST_EP_STATE              HOST_EP_STATE;
typedef enum _USB_SPEED                  USB_SPEED;
typedef enum _USB_REQUEST_TYPE           USB_REQUEST_TYPE;
typedef struct _HOST_REQ                 HOST_REQ;
typedef struct _HOST_CFG                 HOST_CFG;
typedef struct _HOST_CTRL                HOST_CTRL;
typedef struct _PHYTIUM_USB2_HOST_OBJ    PHYTIUM_USB2_HOST_OBJ;

#define MAX_SUPPORTED_DEVICES        16
#define USB_PORT_STAT_RESUME         (1 << 31)
#define MAX_INSTANCE_EP_NUM          6

#define SIZE_PER_DMA_PACKET          512

#define HOST_ESTALL                  1
#define HOST_EUNHANDLED              2
#define HOST_EAUTOACK                3
#define HOST_ESHUTDOWN               4

#define HOST_EP_NUM                  16
#define TRANSFER_OUT                 0
#define TRANSFER_IN                  1

enum _HOST_OTGSTATE {
  HOST_OTG_STATE_A_IDLE,
  HOST_OTG_STATE_A_WAIT_VRISE,
  HOST_OTG_STATE_A_WAIT_BCON,
  HOST_OTG_STATE_A_HOST,
  HOST_OTG_STATE_A_SUSPEND,
  HOST_OTG_STATE_A_PERIPHERAL,
  HOST_OTG_STATE_A_VBUS_ERR,
  HOST_OTG_STATE_A_WAIT_VFALL,
  HOST_OTG_STATE_B_IDLE = 0x10,
  HOST_OTG_STATE_B_PERIPHERAL,
  HOST_OTG_STATE_B_WAIT_ACON,
  HOST_OTG_STATE_B_HOST,
  HOST_OTG_STATE_B_HOST_2,
  HOST_OTG_STATE_B_SRP_INT1,
  HOST_OTG_STATE_B_SRP_INT2,
  HOST_OTG_STATE_B_DISCHRG1,
  HOST_OTG_STATE_B_DISCHRG2,
  HOST_OTG_STATE_UNKNOWN
};

enum _HOST_EP0_STAGE {
  HOST_EP0_STAGE_IDLE,
  HOST_EP0_STAGE_SETUP,
  HOST_EP0_STAGE_IN,
  HOST_EP0_STAGE_OUT,
  HOST_EP0_STAGE_STATUSIN,
  HOST_EP0_STAGE_STATUSOUT,
  HOST_EP0_STAGE_ACK,
};

enum _HOST_EP_STATE {
  HOST_EP_FREE,
  HOST_EP_ALLOCATED,
  HOST_EP_BUSY,
  HOST_EP_NOT_IMPLEMENTED
};

enum _USB_SPEED {
  /** unknow speed - before enumeration */
  USB_SPEED_UNKNOWN = 0,
  /** (1,5Mb/s) */
  USB_SPEED_LOW = 1,
  /** usb 1.1 (12Mb/s) */
  USB_SPEED_FULL = 2,
  /** usb 2.0 (480Mb/s) */
  USB_SPEED_HIGH = 3,
  /** usb 2.5 wireless */
  USB_SPEED_WIRELESS = 4,
  /** usb 3.0 GEN 1  (5Gb/s) */
  USB_SPEED_SUPER = 5,
  /** usb 3.1 GEN2 (10Gb/s) */
  USB_SPEED_SUPER_PLUS = 6,
};

enum _USB_REQUEST_TYPE {
  UsbRequestControl = 0,
  UsbRequestBulk,
  UsbRequestInt,
};

struct _HOST_CFG {
  UINT64      RegBase;
  UINT64      PhyRegBase;
  UINT8       DmultEnabled;
  UINT8       MemoryAlignment;
  UINT8       DmaSupport;
  UINT8       IsEmbeddedHost;
};

struct _HOST_REQ {
  USB_REQUEST_TYPE        Type;
  UINT32                  FrameInterval;
  UINT8                   DeviceSpeed;
  UINT8                   DeviceAddress;
  UINTN                   MaximumPacketLength;
  EFI_USB_DATA_DIRECTION  TransferDirection;
  UINT8                   EpNum;
  UINT32                  TransferResult;
  UINT8                   Toggle;
  HOST_EP0_STAGE          Stage;
  UINT8                   IsIn;
};

struct _HOST_CTRL {
  HW_REGS                      *Regs;
  PHYTIUM_USB2_HOST_OBJ        *HostDrv;
  HOST_CFG                     HostCfg;
  UINT32                       PortStatus;
  UINT32                       Speed;
  HOST_OTGSTATE                OtgState;
  HOST_EP0_STAGE               Ep0State;
  UINT8                        VBusErrCnt;
  UINT8                        IsRemoteWakeup;
  UINT8                        IsSelfPowered;
  PHYTIUM_USB2_DMA_OBJ         *DmaDrv;
  VOID                         *DmaController;
  DMA_CFG                      DmaCfg;
  UINT8                        PortResetting;
  DMA_REG                      *DmaRegs;
  CUSTOM_REG                   *CustomRegs;
  VHUB_REG                     *VhubRegs;
};

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
  );

/**
  Handling of host controller to reset or not reset the port.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  On      1 - reset, 0 - not reset.

**/
VOID
PhytiumHubPortReset (
  IN HOST_CTRL *Ctrl,
  IN UINT8     On
  );

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
  );

/**
  Destroy the usb controller configuration.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
VOID
PhytiumHostDestroy (
  IN HOST_CTRL *Ctrl
  );

/**
  This function can be called only if firmware configuration supports OTG (isOtg)
  or Embedded Host (isEmbeddedHost) feature. The controller can work normally
  only after it starts.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
VOID
PhytiumHostStart (
  IN HOST_CTRL *priv
  );

/**

**/
VOID
PhytiumHostStop (
  IN HOST_CTRL *Ctrl
  );

/**
  Disable all endpoints of host controller.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

  @retval     EFI_SUCCESS Success.
**/
EFI_STATUS
PhytiumHostEpDisable (
  IN HOST_CTRL *Ctrl
  );

/**
  Handle to detect OTG interrupt state. Handling of controller state changes
  caused by some equipment states.

  @param[in]  Ctrl    A pointer to HOST_CTRL.

**/
VOID
PhytiumHostOtgIrq (
  IN HOST_CTRL *Ctrl
  );

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
  );

/**
  Submits bulk transfer to a bulk endpoint of a USB device.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the endpoint is capable of
                                sending or receiving.
  @param  TransferDirection     Specifies the data direction for the data stage
  @param  Data                  Data buffer to transmit or receive.
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
  );

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
  );

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
  );

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
  );

/**
  Get PHYTIUM_USB2_HOST_OBJ protocol.

  @retval  PHYTIUM_USB2_HOST_OBJ instance.
**/
PHYTIUM_USB2_HOST_OBJ *
HOSTGetInstance (
  VOID
  );

/**
  Initialize USB controller, clear all interrupt state. Get host protocol,
  DMA protocol and register base address.

  @param[in]  Ctrl        A pointer to HOST_CTRL.
  @param[in]  Config      A pointer to HOST_CFG.
  @param[in]  Ptr         A pointer to DMA_CONTROLLER.
  @param[in]  UsVhubHost  Vhub host state, 1 - host, 0 - not host.

  @retval     EFI_INVALID_PARAMETER   Some parameters are invalid.
  @retval     EFI_SUCCESS             Success.
**/
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_INIT) (
  IN HOST_CTRL      *Ctrl,
  IN HOST_CFG       *Config,
  IN DMA_CONTROLLER *Ptr,
  IN BOOLEAN        IsVhubHost
  );

/**
  Destroy the usb controller configuration.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
typedef
VOID
(*PHYTIUM_USB2_HOST_DESTROY) (
  IN HOST_CTRL *Ctrl
  );

/**
  This function can be called only if firmware configuration supports OTG (isOtg)
  or Embedded Host (isEmbeddedHost) feature. The controller can work normally
  only after it starts.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
typedef
VOID
(*PHYTIUM_USB2_HOST_START) (
  IN HOST_CTRL *priv
  );

/**
  Stop the host controller. Clear and disable all interrupts, power off Vbus.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

**/
typedef
VOID
(*PHYTIUM_USB2_HOST_STOP) (
  IN HOST_CTRL *Ctrl
  );

/**
  Handle to detect OTG interrupt state. Handling of controller state changes
  caused by some equipment states.

  @param[in]  Ctrl    A pointer to HOST_CTRL.

**/
typedef
VOID
(*PHYTIUM_USB2_HOST_OTG_IRQ) (
  IN HOST_CTRL *Ctrl
  );

/**
  Disable all endpoints of host controller.

  @param[in]  Ctrl        A pointer to HOST_CTRL.

  @retval     EFI_SUCCESS Success.
**/
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_EP_DISABLE) (
  IN HOST_CTRL *Ctrl
  );

/**
  Handling of host controller to suspend or not suspend the port.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  On      1 - suspend, 0 - not suspend.

  @retval     EFI_SUCCESS    Success.
**/
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_HUB_PORT_SUSPEND) (
  IN HOST_CTRL *Ctrl,
  IN UINT16    On
  );

/**
  Handling of host controller to reset or not reset the port.

  @param[in]  Ctrl    A pointer to HOST_CTRL.
  @param[in]  On      1 - reset, 0 - not reset.

**/
typedef
VOID
(*PHYTIUM_USB2_HOST_HUB_PORT_RESET) (
  IN HOST_CTRL *Ctrl,
  IN UINT8     On
  );

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
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_CONTROL_TRANSFER) (
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
  );

/**
  Submits bulk transfer to a bulk endpoint of a USB device.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  DeviceAddress         Target device address.
  @param  EpNum                 Endpoint number.
  @param  MaximumPacketLength   Maximum packet size the endpoint is capable of
                                sending or receiving.
  @param  TransferDirection     Specifies the data direction for the data stage
  @param  Data                  Data buffer to transmit or receive.
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
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_BULK_TRANSFER) (
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
  );

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
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_SYNC_INT_TRANSFER) (
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
  );

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
typedef
EFI_STATUS
(*PHYTIUM_USB2_HOST_ASYNC_INT_TRANSFER) (
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
  );

/**
  Check whether the async transmission is completed or not.

  @param  Ctrl                  This HOST_CTRL instance.
  @param  EpNum                 Endpoint number.
  @param  Toggle                Updated to indicate the next data toggle.
  @param  TransferResult        Variable to receive the transfer result.

  @retval TRUE                  Completed.
  @retval FALSE                 Not completed.
**/
typedef
BOOLEAN
(*PHYTIUM_USB2_HOST_ASYNC_CHECK_COMPLETE) (
  IN  HOST_CTRL                           *Ctrl,
  IN  UINT8                               EpNum,
  OUT UINT8                               *Toggle,
  OUT UINT32                              *TransferResult
  );

struct _PHYTIUM_USB2_HOST_OBJ {
  PHYTIUM_USB2_HOST_INIT                  HostInit;
  PHYTIUM_USB2_HOST_DESTROY               HostDestroy;
  PHYTIUM_USB2_HOST_START                 HostStart;
  PHYTIUM_USB2_HOST_STOP                  HostStop;
  PHYTIUM_USB2_HOST_OTG_IRQ               HostOtgIrq;
  PHYTIUM_USB2_HOST_EP_DISABLE            HostEpDisable;
  PHYTIUM_USB2_HOST_HUB_PORT_SUSPEND      HostHubPortSuspend;
  PHYTIUM_USB2_HOST_HUB_PORT_RESET        HostHubPortReset;
  PHYTIUM_USB2_HOST_CONTROL_TRANSFER      HostControlTransfer;
  PHYTIUM_USB2_HOST_BULK_TRANSFER         HostBulkTransfer;
  PHYTIUM_USB2_HOST_SYNC_INT_TRANSFER     HostSyncInterruptTransfer;
  PHYTIUM_USB2_HOST_ASYNC_INT_TRANSFER    HostAsyncInterruptTransfer;
  PHYTIUM_USB2_HOST_ASYNC_CHECK_COMPLETE  HostAsyncCheckComplete;
};

#endif
