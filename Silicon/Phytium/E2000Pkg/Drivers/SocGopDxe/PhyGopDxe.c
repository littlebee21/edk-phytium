/* @file
** PhyGopDxe.c
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
*/

#include "PhyGopDxe.h"

EFI_GUID gSocDcDpNonDiscoverableDeviceGuid= \
  {0xa561612c, 0xbf86, 0x11ec, {0x89, 0xc1, 0xf7, 0x47, 0x79, 0x60, 0x5d, 0xd6}};
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
  )
{
  UINT32  Data;

  ASSERT (Offset < 0x1000);
  Data = MmioRead32 (Private->DcCtrlBaseAddr + Offset);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, "read reg : %08x, data : %08x\n", Private->DcCtrlBaseAddr + Offset, Data));
#endif

  return Data;
}

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
  )
{
  ASSERT (Offset < 0x1000);
  MmioWrite32 (Private->DcCtrlBaseAddr + Offset, Data);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, "write reg : %08x, data : %08x\n", Private->DcCtrlBaseAddr + Offset, Data));
#endif
  gBS->Stall (1000);
}

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
  )
{
  UINT32 Data;
  UINT64 Address;

  ASSERT (Num < 2);
  switch (Type) {
  case DcChOperate:
    Address = Private->DcChBaseAddr + Offset +  Num * 0x1000;
    break;
  case DpChOperate:
    Address = Private->DpChBaseAddr + Offset + Num * 0x1000;
    break;
  case DpPhyOperate:
    Address = Private->DpPhyBaseAddr + Offset + Num * 0x100000;
    break;
  default:
    ASSERT (0);
  }
  Data = MmioRead32 (Address);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO,"read reg : %08x, data : %08x\n", Address, Data));
#endif
  gBS->Stall (1000);
 return Data;
}

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
  )
{
  UINT64 Address;

  ASSERT (Num < 2);
  switch (Type) {
  case DcChOperate:
    Address = Private->DcChBaseAddr + Offset +  Num * 0x1000;
    break;
  case DpChOperate:
    Address = Private->DpChBaseAddr + Offset + Num * 0x1000;
    break;
  case DpPhyOperate:
    Address = Private->DpPhyBaseAddr + Offset + Num * 0x100000;
    break;
  default:
    ASSERT (0);
  }
  MmioWrite32 (Address, Data);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, "write reg : %08x, data : %08x\n", Address, Data));
#endif
  gBS->Stall (1000);
}

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
  )
{
  UINT32 Index;

  for (Index = 0; Index < Len; Index++) {
    MmioWrite32 (Private->BufferAddr + Offset + Index * 4, *((UINT32 *) (Buffer + Index * 4)));
  }
}

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
  )
{
  UINT32 Index;

  for (Index = 0; Index < Len; Index++) {
    *((UINT32 *) (Buffer + Index * 4)) = MmioRead32 (Private->BufferAddr + Offset + Index * 4);
  }
}

EFI_DRIVER_BINDING_PROTOCOL gPhyDriverBinding = {
  PhyControllerDriverSupported,
  PhyControllerDriverStart,
  PhyControllerDriverStop,
  0x10,     //Driver version
  NULL,
  NULL
};

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
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS          Status;
  NON_DISCOVERABLE_DEVICE    *Dev;

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

  if (CompareGuid (Dev->Type, &gSocDcDpNonDiscoverableDeviceGuid)) {
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
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  EFI_STATUS                    Status;
  PHY_PRIVATE_DATA              *Private;


  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the Graphics Output Protocol interface does not exist the driver is not started
    //
    return Status;
  }

  //
  // Get our private context information
  //
  Private = PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);

  //
  // Remove the Graphics Output Protocol interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UninstallMultipleProtocolInterfaces gEfiGraphicsOutputProtocolGuid,%r\n", Status));
    return Status;
  }

  //
  // Shutdown the hardware
  //
  PhyGraphicsOutputDestructor (Private);

  //
  // Free our instance data
  //
  gBS->FreePool (Private);
  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                  Status;
  PHY_PRIVATE_DATA            *Private;
  EFI_DEVICE_PATH_PROTOCOL    *GopDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
  NON_DISCOVERABLE_DEVICE     *Dev;
  ACPI_ADR_DEVICE_PATH        AcpiDeviceNode;

  //
  // Allocate Private context data for GOP inteface.
  //
  Private = NULL;
  Private = AllocateZeroPool(sizeof (PHY_PRIVATE_DATA));
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG((DEBUG_ERROR, "Allocate poll failed -\n"));
    goto Error;
  }

  //
  // Set up context record
  //
  Private->Signature  = PHY_PRIVATE_DATA_SIGNATURE;
  Private->Handle     = NULL;

  //
  // Open Non Discoverable Device Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  (VOID **) &Dev,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "OpenProtocol gEdkiiNonDiscoverableDeviceProtocolGuid failed -\n"));
    goto Error;
  }
  //
  // Open Device Path Protocol for on USB host controller
  //
  GopDevicePath = NULL;
  Status = gBS->HandleProtocol (
             Controller,
             &gEfiDevicePathProtocolGuid,
            (VOID **) &ParentDevicePath
            );
  //
  // Set Gop Device Path
  //
  if (RemainingDevicePath == NULL) {
    ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
    AcpiDeviceNode.Header.Type = ACPI_DEVICE_PATH;
    AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
    AcpiDeviceNode.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
    SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));

    GopDevicePath = AppendDevicePathNode (
                      ParentDevicePath,
                      (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDeviceNode
                      );
  } else if (!IsDevicePathEnd (RemainingDevicePath)) {
    //
    // If RemainingDevicePath isn't the End of Device Path Node,
    // only scan the specified device by RemainingDevicePath
    //
    GopDevicePath = AppendDevicePathNode (ParentDevicePath, RemainingDevicePath);
  }

  DEBUG ((DEBUG_INFO, "Remaining Device Path : %g\n", RemainingDevicePath ));
  DEBUG ((DEBUG_INFO, "Parent Device Path : %g\n", ParentDevicePath ));
  DEBUG ((DEBUG_INFO, "Gop Device Path : %g\n", GopDevicePath ));
  //
  // Start the Graphics Output Protocol software stack.
  //
  Status = PhyGraphicsOutputConstructor (Private, Dev, GopDevicePath);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PhyControllerDriverStart Status %r\n", Status));
    goto Error;
  }

  //
  // Publish the Graphics Output Protocol interface to the world
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiDevicePathProtocolGuid,
                  Private->GopDevicePath,
                  NULL
                  );
Error:
  if (EFI_ERROR (Status)) {
    if (Private) {
      gBS->FreePool (Private);
    }
  }
  DEBUG ((DEBUG_ERROR, "PhyControllerDriverStart %d, Status %r\n", __LINE__, Status));
  return Status;
}

/**
  The entry point for PhyGop driver which installs the driver binding and component
  name protocol on its ImageHandle.

  @param[in]  ImageHandle  The image handle of the driver.

  @param[in]  SystemTable  The system table.

  @retval EFI_SUCCES       The driver binding and component name protocols are
                           successfully installed.
  @retval Others           Other errors as indicated.
**/
EFI_STATUS
EFIAPI
PhyGraphicsOutputDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS      Status;
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gPhyDriverBinding,
             ImageHandle,
             &gPhyComponentName,
             NULL
             );
  ASSERT_EFI_ERROR (Status);
  return Status;
}
