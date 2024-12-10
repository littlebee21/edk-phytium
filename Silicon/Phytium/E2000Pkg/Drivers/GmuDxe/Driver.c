/** @file
** Driver.c
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/NonDiscoverableDevice.h>

#include "Mac.h"
#include "Snp.h"


EFI_GUID gGmuNonDiscoverableDeviceGuid= \
  {0x3b2deb7e, 0xd64f, 0x11ec, {0x92, 0x18, 0x47, 0xbe, 0x37, 0xc7, 0xf6, 0x02}};

/**
  Start to process the controller.

  @param[in]   Event               The Event handle.
  @param[out]  Context             The input protocol pointer.
**/
VOID
EFIAPI
SnpExitBootServiceEventNotify(
  IN EFI_EVENT Event,
  OUT VOID *Context
);



/**
  Check if this device is supported.

  @param[in]  This                   The driver binding protocol.
  @param[in]  Controller             The controller handle to check.
  @param[in]  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.
  @retval EFI_UNSUPPORTED        This device isn't supported.
**/
STATIC
EFI_STATUS
EFIAPI
DriverSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  Start to process the controller.

  @param[in]  This                   The phytium soc gop driver binding instance.
  @param[in]  Controller             The controller to check.
  @param[in]  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.
  @retval EFI_ALREADY_STARTED    The controller is already controlled by phytium soc gop.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.
**/
STATIC
EFI_STATUS
EFIAPI
DriverStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  Stop this driver on ControllerHandle.

  @param[in] This               A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in] Controller         A handle to the device being stopped.
  @param[in] NumberOfChildren   The number of child device handles in ChildHandleBuffer.
  @param[in] ChildHandleBuffer  An array of child handles to be freed.

  @retval EFI_SUCCESS       This driver is removed from this device.
  @retval other             Some error occurs when removing this driver from this device.
**/
STATIC
EFI_STATUS
EFIAPI
DriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  );

STATIC
EFI_DRIVER_BINDING_PROTOCOL mDriverBinding = {
  DriverSupported,
  DriverStart,
  DriverStop,
  0xa,
  NULL,
  NULL
};

STATIC
SIMPLE_NETWORK_DEVICE_PATH PathTemplate = {
  {
    {
      MESSAGING_DEVICE_PATH, MSG_MAC_ADDR_DP,
      {(UINT8)(sizeof (MAC_ADDR_DEVICE_PATH)), (UINT8)((sizeof (MAC_ADDR_DEVICE_PATH)) >> 8)}
    },
    {{ 0 }},
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {sizeof (EFI_DEVICE_PATH_PROTOCOL), 0}
  }
};

/**
  Check if this device is supported.

  @param[in]  This                   The driver binding protocol.
  @param[in]  Controller             The controller handle to check.
  @param[in]  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.
  @retval EFI_UNSUPPORTED        This device isn't supported.
**/
STATIC
EFI_STATUS
EFIAPI
DriverSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  NON_DISCOVERABLE_DEVICE    *Dev;
  EFI_STATUS                 Status;
  //
  // Connect to the non-discoverable device
  //
  Status = gBS->OpenProtocol (Controller,
                              &gEdkiiNonDiscoverableDeviceProtocolGuid,
                              (VOID **)&Dev,
                              This->DriverBindingHandle,
                              Controller,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  if (CompareGuid (Dev->Type, &gGmuNonDiscoverableDeviceGuid)) {
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
                      Controller
                      );

  DEBUG ((DEBUG_INFO, "%a(), %d, Status : %r\n", __FUNCTION__, __LINE__, Status));
  return Status;
}

/**
  Start to process the controller.

  @param[in]  This                   The phytium soc gop driver binding instance.
  @param[in]  Controller             The controller to check.
  @param[in]  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.
  @retval EFI_ALREADY_STARTED    The controller is already controlled by phytium soc gop.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.
**/
STATIC
EFI_STATUS
EFIAPI
DriverStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  EFI_STATUS                       Status;
  SIMPLE_NETWORK_DRIVER            *Snp;
  EFI_SIMPLE_NETWORK_MODE          *SnpMode;
  SIMPLE_NETWORK_DEVICE_PATH       *DevicePath;
  EFI_EVENT                        ExitBootServiceEvent;
  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  //
  // Allocate Resources
  //
  Snp = AllocatePages (EFI_SIZE_TO_PAGES (sizeof (SIMPLE_NETWORK_DRIVER)));
  if (Snp == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  Status = gBS->OpenProtocol (Controller,
                                &gEdkiiNonDiscoverableDeviceProtocolGuid,
                                (VOID **)&Snp->Dev,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER
                                );
  //
  // Size for transmit and receive buffer
  //
  DevicePath = (SIMPLE_NETWORK_DEVICE_PATH*) AllocateCopyPool (sizeof (SIMPLE_NETWORK_DEVICE_PATH), &PathTemplate);
  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialized signature (used by INSTANCE_FROM_SNP_THIS macro)
  //
  Snp->Signature = SNP_DRIVER_SIGNATURE;

  EfiInitializeLock (&Snp->Lock, TPL_CALLBACK);
  //
  // Initialize pointers
  //
  SnpMode = &Snp->SnpMode;
  Snp->Snp.Mode = SnpMode;

  //
  // Get MAC controller base address
  //
  Snp->MacBase = (UINTN)Snp->Dev->Resources[0].AddrRangeMin;
  ParseMacShareMem ((UINTN)Snp->Dev->Resources[1].AddrRangeMin, &Snp->MacDriver);
  DEBUG ((DEBUG_INFO, "Mac Base : 0x%08x\n", Snp->MacBase));
  Status = MacInitialize (Snp);
  SnpMode->CurrentAddress.Addr[0] = Snp->MacDriver.MacAddr.Addr[0];
  SnpMode->CurrentAddress.Addr[1] = Snp->MacDriver.MacAddr.Addr[1];
  SnpMode->CurrentAddress.Addr[2] = Snp->MacDriver.MacAddr.Addr[2];
  SnpMode->CurrentAddress.Addr[3] = Snp->MacDriver.MacAddr.Addr[3];
  SnpMode->CurrentAddress.Addr[4] = Snp->MacDriver.MacAddr.Addr[4];
  SnpMode->CurrentAddress.Addr[5] = Snp->MacDriver.MacAddr.Addr[5];
  //
  //if use the mac of configing on ATF,read the mac from Gmac register
  //
  //MacReadHwaddr (&Snp->MacDriver, &SnpMode->CurrentAddress);
  if(PcdGetBool(PcdPhytiumMacFromEfuseEnable))
    MacReadHwaddr (&Snp->MacDriver, &SnpMode->CurrentAddress);
  DisplayMacAddr (&SnpMode->CurrentAddress);
  //
  // Assign fields and func pointers
  //
  Snp->ControllerHandle = NULL;
  Snp->Snp.Revision = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Snp->Snp.WaitForPacket = NULL;
  Snp->Snp.Initialize = SnpInitialize;
  Snp->Snp.Start = SnpStart;
  Snp->Snp.Stop = SnpStop;
  Snp->Snp.Reset = SnpReset;
  Snp->Snp.Shutdown = SnpShutdown;
  Snp->Snp.ReceiveFilters = SnpReceiveFilters;
  Snp->Snp.StationAddress = SnpStationAddress;
  Snp->Snp.Statistics = SnpStatistics;
  Snp->Snp.MCastIpToMac = SnpMcastIptoMac;
  Snp->Snp.NvData = SnpNvData;
  Snp->Snp.GetStatus = SnpGetStatus;
  Snp->Snp.Transmit = SnpTransmit;
  Snp->Snp.Receive = SnpReceive;

  Snp->RecycledTxBuf = AllocatePool (sizeof (UINT64) * SNP_TX_BUFFER_INCREASE);
  if (Snp->RecycledTxBuf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  }

  Snp->MaxRecycledTxBuf = SNP_TX_BUFFER_INCREASE;
  Snp->RecycledTxBufCount = 0;

  //
  // Start completing simple network mode structure
  //
  SnpMode->State = EfiSimpleNetworkStopped;
  SnpMode->HwAddressSize = NET_ETHER_ADDR_LEN;    // HW address is 6 bytes
  SnpMode->MediaHeaderSize = sizeof (ETHER_HEAD);
  SnpMode->MaxPacketSize = EFI_PAGE_SIZE;         // Preamble + SOF + Ether Frame (with VLAN tag +4bytes)
  SnpMode->NvRamSize = 0;                         // No NVRAM with this device
  SnpMode->NvRamAccessSize = 0;                   // No NVRAM with this device

  //
  // Update network mode information
  //
  SnpMode->ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
                               EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST   |
                               EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  //
  // We do not intend to receive anything for the time being.
  //
  SnpMode->ReceiveFilterSetting = 0;

  //
  // EMAC has 64bit hash table, can filter 64 MCast MAC Addresses
  //
  SnpMode->MaxMCastFilterCount = MAX_MCAST_FILTER_CNT;
  SnpMode->MCastFilterCount = 0;
  ZeroMem (&SnpMode->MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS));

  //
  // Set the interface type (1: Ethernet or 6: IEEE 802 Networks)
  //
  SnpMode->IfType = NET_IFTYPE_ETHERNET;

  //
  // Mac address is changeable as it is loaded from erasable memory
  //
  SnpMode->MacAddressChangeable = TRUE;

  //
  // Can only transmit one packet at a time
  //
  SnpMode->MultipleTxSupported = FALSE;

  //
  // MediaPresent checks for cable connection and partner link
  //
  SnpMode->MediaPresentSupported = TRUE;
  SnpMode->MediaPresent = FALSE;

  //
  // Set broadcast address
  //
  SetMem (&SnpMode->BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  //
  // Create event to stop the HC when exit boot service.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  SnpExitBootServiceEventNotify,
                  &Snp->Snp,
                  &gEfiEventExitBootServicesGuid,
                  &ExitBootServiceEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(), %d\n", __FUNCTION__, __LINE__));
    return EFI_PROTOCOL_ERROR;
  }

  //
  // Assign fields for device path
  //
  //Snp->Snp.Mode->CurrentAddress.Addr[5] = 0x00;
  //Snp->Snp.Mode->CurrentAddress.Addr[4] = 0x07;
  //Snp->Snp.Mode->CurrentAddress.Addr[3] = 0x3e;
  //Snp->Snp.Mode->CurrentAddress.Addr[2] = 0x90;
  //Snp->Snp.Mode->CurrentAddress.Addr[1] = 0xdb;
  //Snp->Snp.Mode->CurrentAddress.Addr[0] = 0x00;
  if(!PcdGetBool(PcdPhytiumMacFromEfuseEnable))
     MacWriteHwaddr (&Snp->MacDriver, &Snp->Snp.Mode->CurrentAddress);
  CopyMem (&DevicePath->MacAddrDP.MacAddress, &Snp->Snp.Mode->CurrentAddress, NET_ETHER_ADDR_LEN);
  DevicePath->MacAddrDP.IfType = Snp->Snp.Mode->IfType;
  DisplayMacAddr (&DevicePath->MacAddrDP.MacAddress);
  Snp->PhyDriver.PhyDetected = FALSE;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &(Snp->Snp),
                  &gEfiDevicePathProtocolGuid,
                  DevicePath,
                  NULL
                  );
  if (EFI_ERROR(Status)) {
    gBS->CloseProtocol (Controller,
                        &gEdkiiNonDiscoverableDeviceProtocolGuid,
                        This->DriverBindingHandle,
                        Controller);

    FreePages (Snp, EFI_SIZE_TO_PAGES (sizeof (SIMPLE_NETWORK_DRIVER)));
  } else {
    Snp->ControllerHandle = Controller;
  }

  DEBUG ((DEBUG_INFO, "%a(), %d, Status : %r\n", __FUNCTION__, __LINE__, Status));
  return Status;
}

/**
  Stop this driver on ControllerHandle.

  @param[in] This               A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in] Controller         A handle to the device being stopped.
  @param[in] NumberOfChildren   The number of child device handles in ChildHandleBuffer.
  @param[in] ChildHandleBuffer  An array of child handles to be freed.

  @retval EFI_SUCCESS       This driver is removed from this device.
  @retval other             Some error occurs when removing this driver from this device.
**/
STATIC
EFI_STATUS
EFIAPI
DriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  )
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_NETWORK_PROTOCOL  *SnpProtocol;
  SIMPLE_NETWORK_DRIVER        *Snp;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID **)&SnpProtocol
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Snp = INSTANCE_FROM_SNP_THIS(SnpProtocol);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &Snp->Snp,
                  NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FreePool (Snp->RecycledTxBuf);
  FreePages (Snp, EFI_SIZE_TO_PAGES (sizeof (SIMPLE_NETWORK_DRIVER)));

  return Status;
}


/**
   UEFI Driver Entry Point API

   @param[in]  ImageHandle   EFI_HANDLE.
   @param[in]  SystemTable   EFI_SYSTEM_TABLE.

   @retval EFI_SUCCESS       Success.
           EFI_DEVICE_ERROR  Fail.
**/
EFI_STATUS
EFIAPI
GmuSnpDxeEntry (
  IN  EFI_HANDLE ImageHandle,
  IN  EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS        Status;

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &mDriverBinding,
             ImageHandle,
             &gGmuSnpComponentName,
             &gGmuSnpComponentName2
             );

  return Status;
}


/**
  Start to process the controller.

  @param[in]   Event               The Event handle.
  @param[out]  Context             The input protocol pointer.
**/
VOID
EFIAPI
SnpExitBootServiceEventNotify(
  IN EFI_EVENT Event,
  OUT VOID *Context
)
{
  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  SnpShutdown ((EFI_SIMPLE_NETWORK_PROTOCOL *)Context);
}
