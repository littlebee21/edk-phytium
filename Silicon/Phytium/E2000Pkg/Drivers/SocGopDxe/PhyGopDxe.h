/* @file
** PhyGopDxe.h
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _PHY_GOP_DXE_H_
#define    _PHY_GOP_DXE_H_

#include  <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/TimerLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/NonDiscoverableDevice.h>

#include "Dc/DcStru.h"
#include "Dc/DcType.h"
#include "Dp/DpType.h"
#include "PhyGraphicsOutput.h"

//#define EDID_USE

#define DC_CTRL_BASE_OFFSET        0x0
#define DC_CHANNEL_BASE_OFFSET     0x1000
#define DP_CHANNEL_BASE_OFFSET     0x4000
#define DP_PHY_BASE_OFFSET         0x0

//#define  PLD_TEST
//#define  REG_DEBUG
//#define  DPCD_DEBUG

#define  DPSINK_HOTPLUG_CHECK_INTERVAL       (2000*10000)      //1s
//
// PHY Private Data Structure
//
#define PHY_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('S', 'G', 'O', 'P')

extern EFI_DRIVER_BINDING_PROTOCOL gPhyDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPhyComponentName;
extern EFI_GUID gSocDcDpNonDiscoverableDeviceGuid;

typedef struct _DTD_TABLE {
  UINT32  PixelClock;
  UINT32  HorPixel;
  UINT32  VerPixel;
  DC_SYNC DcSync;
  DP_SYNC DpSync;
} DTD_TABLE;

typedef enum {
  DcChOperate = 0,
  DpChOperate,
  DpPhyOperate,
} CHREG_OPERATE_TYPE;

typedef struct _DTD_LIST {
  UINT8     DtdListNum;
#define  DTD_LIST_MAX_NUM  4
  DTD_TABLE DtdTable[DTD_LIST_MAX_NUM];
} DTD_LIST;

typedef struct _PHY_PRIVATE_DATA {
  UINT64                          Signature;
  EFI_HANDLE                      Handle;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    GraphicsOutput;
  //
  // GOP Private Data
  //
  EFI_DEVICE_PATH_PROTOCOL        *GopDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *UgaDevicePath;
  EFI_EVENT                       DpSinkHotPlugEvent;
  BOOLEAN                         HardwareNeedsStarting;
  PHY_GOP_MODE                    ModeData[PHY_GOP_MAX_MODENUM];
  DC_SYNC                         DcSync[PHY_GOP_MAX_MODENUM];
  DP_SYNC                         DpSync[PHY_GOP_MAX_MODENUM];
  VOID                            *LineBuffer;
  UINT64                          BufferAddr;
  UINT8                           MaxMode;
  UINT8                           ModeNum;
  UINT64                          DcCtrlBaseAddr;
  UINT64                          DcChBaseAddr;
  UINT64                          DpChBaseAddr;
  UINT64                          DpPhyBaseAddr;
  UINT32                          InitHor;
  UINT32                          InitVer;
  UINT32                          DpIsHpdOn[DPDC_PATH_NUM];
  UINT32                          DpIsTrained[DPDC_PATH_NUM];
  UINT32                          DpConnect[DPDC_PATH_NUM];
  UINT32                          DpIsUsed[DPDC_PATH_NUM];
  UINT8                           IsEdp[DPDC_PATH_NUM];
  UINT8                           EdpBgLightOn[DPDC_PATH_NUM];
  UINT8                           EdpNeedInit[DPDC_PATH_NUM];
  DP_CONFIG                       DpConfig[DPDC_PATH_NUM];
  DC_CORE                         DcCore[DPDC_PATH_NUM];
  DTD_LIST                        DtdList[DPDC_PATH_NUM];
  UINT8                           DownSpreadEnable[DPDC_PATH_NUM];
} PHY_PRIVATE_DATA;

#define PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS(a) CR(a, PHY_PRIVATE_DATA, GraphicsOutput, PHY_PRIVATE_DATA_SIGNATURE)

/**
  Read Dc control register.

  @param[in]  Private    Pointer to PHY_PRIVATE_DATA structure.
  @param[in]  Offset     Register address offset.

  @retval     Data       Value read form Dc control register.
**/
UINT32
DcCtrlRegRead (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT64            Offset
  );

/**
  Write Dc control register.

  @param[in]  Private    Pointer to PHY_PRIVATE_DATA structure.
  @param[in]  Offset     Register address offset.
  @prram[in]  Data       Value to write into register

  @retval     Null
 **/
VOID
DcCtrlRegWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT64            Offset,
  IN UINT32            Data
  );

/**
  Read Dc, Dp or Phy channel register.

  @param[in]  Private      Pointer to PHY_PRIVATE_DATA structure.
  @param[in]  Num          The index of channel , 0 or 1.
  @param[in]  Offset       Register address offset in dc, dp or phy module.
  @param[in]  Type         Operate type, DcChOpertate, DpChOperate or DpPhyOperate.

  @retval     The value of register.
**/
UINT32
ChannelRegRead (
  IN PHY_PRIVATE_DATA    *Private,
  IN UINT8               Num,
  IN UINTN               Offset,
  IN CHREG_OPERATE_TYPE  Type
  );

/**
  @param[in]  Private      Pointer to private data structure.

  @param[in]  num          the index of dcdp , 0 , 1 or 2.

  @param[in]  Offset       Register address offset.

  @param[in]  Data         The value that you want to write to the register in a specified group.

  @retval     NULL
**/
VOID
ChannelRegWrite (
  IN PHY_PRIVATE_DATA    *Private,
  IN UINT8               Num,
  IN UINTN               Offset,
  IN CHREG_OPERATE_TYPE  Type,
  IN UINT32              Data
  );

/**
  Write video buffer data to video memory.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the selected BAR to start video memory.

  @param[in]  len          The number of data to write in byte unit.

  @param[in]  buffer       Pointer to the source buffer to write data from.

  @retval     NULL
**/
VOID
PhyFramebufferWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT64            Offset,
  IN UINTN             Len,
  IN VOID              *Buffer
  );

/**
  Read video buffer data from video memory.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the selected BAR to start video memory.

  @param[in]  Len          The number of data to read in byte unit.

  @param[in]  Buffer       Pointer to the destination buffer to store the results.

  @retval     NULL
**/
VOID
PhyFramebufferRead (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT64            Offset,
  IN  UINTN             Len,
  OUT VOID              *Buffer
  );

/**
  Constructor for the Graphics Output Protocol,initialize,specific variables, information.

  @param[in,out]  This          Pointer to Graphics Output protocol instance
  @param[in]      DevicePath    The device path of host controller.
  @param[in]      Dev           The non discoverable device of controller.

  @retval         EFI_SUCCESS   Initialization completes without error.
**/
EFI_STATUS
PhyGraphicsOutputConstructor (
  IN OUT PHY_PRIVATE_DATA      *Private,
  IN NON_DISCOVERABLE_DEVICE   *Dev,
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

/**
  Clean up after closing the Graphics Output Protocol.

  @param[in,out]  This          Pointer to Graphics Output protocol instance

  @retval         EFI_SUCCESS   Success.
**/
EFI_STATUS
PhyGraphicsOutputDestructor (
  IN OUT PHY_PRIVATE_DATA *Private
  );

/**
  Check if this device is supported.

  @param  This                   The driver binding protocol.

  @param  Controller             The controller handle to check.

  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.

  @retval EFI_UNSUPPORTED        This device isn't supported.
**/
EFI_STATUS
EFIAPI
PhyControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

/**
  Stop this driver on ControllerHandle.

  @param This               A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param Controller         A handle to the device being stopped.
  @param NumberOfChildren   The number of child device handles in ChildHandleBuffer.
  @param ChildHandleBuffer  An array of child handles to be freed.

  @retval EFI_SUCCESS       This driver is removed from this device.
  @retval other             Some error occurs when removing this driver from this device.

**/
EFI_STATUS
EFIAPI
PhyControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  );

/**
  Start to process the controller.

  @param  This                   The phytium soc gop driver binding instance.
  @param  Controller             The controller to check.
  @param  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.
  @retval EFI_ALREADY_STARTED    The controller is already controlled by phytium soc gop.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.
**/
EFI_STATUS
EFIAPI
PhyControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );
#endif
