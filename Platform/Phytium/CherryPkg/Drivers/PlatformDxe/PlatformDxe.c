/** @file
  The Platform Initialization.
**/

#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/ParameterTable.h>
#include <Library/HobLib.h>

#include <Protocol/SpiNorFlashProtocol.h>

#include <Platform.h>
#include "Type.h"

#define GMU_SA1B             0x0088 /* Specific1 Bottom */
#define GMU_SA1T             0x008C /* Specific1 Top */
/*ETH*/
#define GMAC0_BASE_ADDR      0x3200c000
#define GMAC1_BASE_ADDR      0x3200e000
#define GMAC2_BASE_ADDR      0x32010000
#define GMAC3_BASE_ADDR      0x32012000

/**
  Get mac address.

  @param[in]  Base      Mac base addr.
  @param[out] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
MacReadHwaddr (
  IN  UINT32           Base,
  OUT EFI_MAC_ADDRESS  *MacAddr
  )
{
  UINT32  HwaddrBottom;
  UINT32  HwaddrTop;

  HwaddrBottom = MmioRead32 (Base + GMU_SA1B);
  HwaddrTop = MmioRead32 (Base + GMU_SA1T);

  MacAddr->Addr[0] = HwaddrBottom & 0xFF;
  MacAddr->Addr[1] = (HwaddrBottom >> 8) & 0xFF;
  MacAddr->Addr[2] = (HwaddrBottom >> 16) & 0xFF;
  MacAddr->Addr[3] = (HwaddrBottom >> 24) & 0xFF;
  MacAddr->Addr[4] = HwaddrTop & 0xFF;
  MacAddr->Addr[5] = (HwaddrTop >> 8) & 0xFF;
}

/**


**/
EFI_STATUS
StoreDdrTrainInfo (
  VOID
  )
{
  EFI_HOB_GUID_TYPE         *GuidHob;
  UINT64                    DdrInfoSize;
  UINT64                    *Temp;
  UINT64                    DdrInfoAddr;
  UINT8                     *DdrInfo;
  UINT64                    DdrInfoFlashAddr;
  EFI_STATUS                Status;
  EFI_NORFLASH_DRV_PROTOCOL *FlashHandle;

  FlashHandle = NULL;
  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid, NULL, (VOID*) &FlashHandle);
  if (EFI_ERROR(Status)){
    DEBUG ((DEBUG_ERROR, "Locate SpiFlashProtocol failed!\n"));
    return EFI_NOT_FOUND;
  }
  DdrInfoFlashAddr =  PcdGet64 (PcdDdrTrainInfoSaveBaseAddress);
  DEBUG ((DEBUG_INFO, "Pcd DdrInfoFlashAddr : %x\n", DdrInfoFlashAddr));
  ASSERT ((DdrInfoFlashAddr % SIZE_64KB) == 0);
  // Get  Information
  GuidHob = GetFirstGuidHob(&gDdrTrainInfoAddrHobGuid);
  if (GuidHob != NULL) {
    Temp = (UINT64 *)GET_GUID_HOB_DATA(GuidHob);
    CopyMem (&DdrInfoAddr, Temp, 8);
    DdrInfoSize = MmioRead64 (DdrInfoAddr);
    DEBUG ((DEBUG_ERROR,"DdrInfoAddr : %x, DdrInfoSize : %x\n", DdrInfoAddr, DdrInfoSize));
    DdrInfo = AllocatePages ((DdrInfoSize + 4) / 4096 + 1);
    if (DdrInfo == NULL) {
      DEBUG ((DEBUG_ERROR, "DdrInfo allcate failed!\n"));
      return EFI_NOT_FOUND;
    }
    SetMem32 (DdrInfo, 4, DDR_TRAIN_INFO_CHECK);
    CopyMem (DdrInfo +4, (VOID*)DdrInfoAddr, DdrInfoSize + 8);
    FlashHandle->Erase (DdrInfoFlashAddr, SIZE_64KB);
    FlashHandle->Write (DdrInfoFlashAddr, DdrInfo, DdrInfoSize + 4 + 8);
    FreePages (DdrInfo, (DdrInfoSize + 4) / 4096 + 1);
    DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
    return EFI_SUCCESS;
  }
  else {
    DEBUG ((DEBUG_INFO, "Ddr Info Hob Not Found\n"));
    return EFI_NOT_FOUND;
  }
}


/**
  Register non-discovery device with devicepath.

  @param[in]  TypeGuid    Device Guid.
  @param[in]  Desc        Device description.
  @param[in]  DevicePath  Device Path.
  @param[out] Handle      Handle after register

  @retval     EFI_OUT_OF_RESOURCES
              EFI_SUCCESS
**/
STATIC
EFI_STATUS
RegisterDevice (
  IN  EFI_GUID                            *TypeGuid,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Desc,
  IN  HII_VENDOR_DEVICE_PATH              *DevicePath,
  OUT EFI_HANDLE                          *Handle
  )
{
  NON_DISCOVERABLE_DEVICE             *Device;
  EFI_STATUS                          Status;

  Device = (NON_DISCOVERABLE_DEVICE *)AllocateZeroPool (sizeof (*Device));
  if (Device == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Device->Type = TypeGuid;
  Device->DmaType = NonDiscoverableDeviceDmaTypeNonCoherent;
  Device->Resources = Desc;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  Handle,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  Device,
                  &gEfiDevicePathProtocolGuid,
                  DevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto FreeDevice;
  }
  return EFI_SUCCESS;

FreeDevice:
  FreePool (Device);

  return Status;
}

/**
  Register non-discovery device without devicepath.

  @param[in]  TypeGuid    Device Guid.
  @param[in]  Desc        Device description.
  @param[out] Handle      Handle after register

  @retval     EFI_OUT_OF_RESOURCES
              EFI_SUCCESS
**/
STATIC
EFI_STATUS
RegisterNoDevicepath (
  IN  EFI_GUID                            *TypeGuid,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Desc,
  OUT EFI_HANDLE                          *Handle
  )
{
  NON_DISCOVERABLE_DEVICE             *Device;
  EFI_STATUS                          Status;

  Device = (NON_DISCOVERABLE_DEVICE *)AllocateZeroPool (sizeof (*Device));
  if (Device == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Device->Type = TypeGuid;
  Device->DmaType = NonDiscoverableDeviceDmaTypeNonCoherent;
  Device->Resources = Desc;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  Handle,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  Device,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto FreeDevice;
  }
  return EFI_SUCCESS;

FreeDevice:
  FreePool (Device);

  return Status;

}

/**
  Config Gmu share memory.
  Format:
    UINT32    Version
    UINT32    Interface : 0-Usxgmii, 1-Xgmii, 2-Sgmii, 3-rgmii, 4-rmii
    UINT32    Autoeng : 0-not auto, 1-auto
    UINT32    Speed : 10-10M, 100-100M, 1000-1G, 2500-2.5G, 10000-10G
    UINT32    Duplex : 0-Half, 1-Full
    UINT8*6   Mac Address.

  @param[in]  Addr       Share memory address.
  @param[in]  Interface  Phy interface mode.
  @param[in]  Auto       Autoeng disable or enable.
  @param[in]  Speed      Interface speed value.
  @param[in]  Duplex     Half or full duplex.
  @param[in]  Mac        A pointer to MAC_ADDRESS.

  @retval     Null
**/
STATIC
VOID
ConfigGmuSharedMem (
  IN EFI_PHYSICAL_ADDRESS  Addr,
  IN UINT32                Interface,
  IN UINT32                Auto,
  IN UINT32                Speed,
  IN UINT32                Duplex,
  IN EFI_MAC_ADDRESS       *Mac
  )
{
  UINT32 Index;
  //version
  MmioWrite32 (Addr, 0x100);
  Addr += 4;
  MmioWrite32 (Addr, Interface);
  DEBUG ((DEBUG_INFO, "Interface : %d\n", Interface));
  Addr += 4;
  MmioWrite32 (Addr, Auto);
  DEBUG ((DEBUG_INFO, "Autoeng : %d\n", Auto));
  Addr += 4;
  MmioWrite32 (Addr, Speed);
  DEBUG ((DEBUG_INFO, "Speed : %d\n", Speed));
  Addr += 4;
  MmioWrite32 (Addr, Duplex);
  DEBUG ((DEBUG_INFO, "Duplex : %d\n", Duplex));
  Addr += 4;
  for (Index = 0; Index < 6; Index++) {
    MmioWrite8 (Addr + Index, Mac->Addr[Index]);
  }
  DEBUG ((DEBUG_INFO, "Mac : %02x-%02x-%02x-%02x-%02x-%02x\n",
              Mac->Addr[0], Mac->Addr[1], Mac->Addr[2], Mac->Addr[3],
              Mac->Addr[4], Mac->Addr[5]));
}

/*
  Gmu device regist.

  @retval    Null.
**/
VOID
GmuRegister (
  VOID
  )
{
  EFI_HANDLE            Handle;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address[4];
  EFI_MAC_ADDRESS       MacAddr;
  UINT8                 PhyMode;
  UINT8                 Buffer[1024];
  BOARD_CONFIG          *BoardConfig;
  UINT32                Index;
  UINT8                 *Mac;
  UINT32                MacTemp;

  Status = EFI_NOT_FOUND;
  GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  BoardConfig = (BOARD_CONFIG *)Buffer;
  //
  //Mac 0
  //
  PhyMode = (BoardConfig->PhySelMode >> (2 * 2)) & 0x3;
  if ((PhyMode == 0) && (PcdGetBool (PcdPhytiumGmu0Enable))) {
    DEBUG ((DEBUG_INFO, "Gmu 0 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[0]
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address[0]));
    MacReadHwaddr (GMAC0_BASE_ADDR, &MacAddr);
    MacTemp = MmioRead32(GMAC0_BASE_ADDR + GMU_SA1B);
    if(0 == MacTemp)
    {
      Mac = PcdGetPtr (PcdPhytiumGmu0Mac);
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = Mac[Index];
      }
    }
    else
    {
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = MmioRead8((GMAC0_BASE_ADDR + GMU_SA1B) + Index);
        if(Index > 3)
          MacAddr.Addr[Index] = MmioRead8((GMAC0_BASE_ADDR + GMU_SA1T) + (Index-4));
      }
    }
    ConfigGmuSharedMem (
      Address[0],
      PcdGet32 (PcdPhytiumGmu0Interface),
      PcdGet32 (PcdPhytiumGmu0Autoeng),
      PcdGet32 (PcdPhytiumGmu0Speed),
      PcdGet32 (PcdPhytiumGmu0Duplex),
      &MacAddr);
    mGmu0Desc[1].AddrRangeMin = Address[0];
    mGmu0Desc[1].AddrRangeMax = Address[0] + 4096;
    Handle = NULL;
    Status = RegisterNoDevicepath (
               &gGmuNonDiscoverableDeviceGuid,
               mGmu0Desc,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Mac 1
  //
  PhyMode = (BoardConfig->PhySelMode >> (3 * 2)) & 0x3;
  if ((PhyMode == 0) && (PcdGetBool (PcdPhytiumGmu1Enable))) {
    DEBUG ((DEBUG_INFO, "Gmu 1 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[1]
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address[1]));
    MacReadHwaddr (GMAC1_BASE_ADDR, &MacAddr);
    MacTemp = MmioRead32(GMAC1_BASE_ADDR + GMU_SA1B);
    if(0 == MacTemp)
    {
      Mac = PcdGetPtr (PcdPhytiumGmu1Mac);
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = Mac[Index];
      }
    }
    else
    {
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = MmioRead8((GMAC1_BASE_ADDR + GMU_SA1B) + Index);
        if(Index > 3)
          MacAddr.Addr[Index] = MmioRead8((GMAC1_BASE_ADDR + GMU_SA1T) + (Index-4));
      }
    }
    ConfigGmuSharedMem (
      Address[1],
      PcdGet32 (PcdPhytiumGmu1Interface),
      PcdGet32 (PcdPhytiumGmu1Autoeng),
      PcdGet32 (PcdPhytiumGmu1Speed),
      PcdGet32 (PcdPhytiumGmu1Duplex),
      &MacAddr);
    mGmu1Desc[1].AddrRangeMin = Address[1];
    mGmu1Desc[1].AddrRangeMax = Address[1] + 4096;
    Handle = NULL;
    Status = RegisterNoDevicepath (
               &gGmuNonDiscoverableDeviceGuid,
               mGmu1Desc,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Mac 2
  //
  if (PcdGetBool (PcdPhytiumGmu2Enable)) {
    DEBUG ((DEBUG_INFO, "Gmu 2 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[2]
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address[2]));
    MacReadHwaddr (GMAC2_BASE_ADDR, &MacAddr);
    MacTemp = MmioRead32(GMAC2_BASE_ADDR + GMU_SA1B);
    if(0 == MacTemp)
    {
      Mac = PcdGetPtr (PcdPhytiumGmu2Mac);
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = Mac[Index];
      }
    }
    else
    {
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = MmioRead8((GMAC2_BASE_ADDR + GMU_SA1B) + Index);
        if(Index > 3)
          MacAddr.Addr[Index] = MmioRead8((GMAC2_BASE_ADDR + GMU_SA1T) + (Index-4));
      }
    }
    ConfigGmuSharedMem (
      Address[2],
      PcdGet32 (PcdPhytiumGmu2Interface),
      PcdGet32 (PcdPhytiumGmu2Autoeng),
      PcdGet32 (PcdPhytiumGmu2Speed),
      PcdGet32 (PcdPhytiumGmu2Duplex),
      &MacAddr);
    mGmu2Desc[1].AddrRangeMin = Address[2];
    mGmu2Desc[1].AddrRangeMax = Address[2] + 4096;
    Handle = NULL;
    Status = RegisterNoDevicepath (
               &gGmuNonDiscoverableDeviceGuid,
               mGmu2Desc,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Mac 3
  //
  if (PcdGetBool (PcdPhytiumGmu3Enable)) {
    DEBUG ((DEBUG_INFO, "Gmu 3 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[3]
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address[3]));
    MacReadHwaddr (GMAC3_BASE_ADDR, &MacAddr);
    MacTemp = MmioRead32(GMAC3_BASE_ADDR + GMU_SA1B);
    if(0 == MacTemp)
    {
      Mac = PcdGetPtr (PcdPhytiumGmu3Mac);
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = Mac[Index];
      }
    }
    else
    {
      for (Index = 0; Index < 6; Index++) {
        MacAddr.Addr[Index] = MmioRead8((GMAC3_BASE_ADDR + GMU_SA1B) + Index);
        if(Index > 3)
          MacAddr.Addr[Index] = MmioRead8((GMAC3_BASE_ADDR + GMU_SA1T) + (Index-4));
      }
    }
    ConfigGmuSharedMem (
      Address[3],
      PcdGet32 (PcdPhytiumGmu3Interface),
      PcdGet32 (PcdPhytiumGmu3Autoeng),
      PcdGet32 (PcdPhytiumGmu3Speed),
      PcdGet32 (PcdPhytiumGmu3Duplex),
      &MacAddr);
    mGmu3Desc[1].AddrRangeMin = Address[3];
    mGmu3Desc[1].AddrRangeMax = Address[3] + 4096;
    Handle = NULL;
    Status = RegisterNoDevicepath (
               &gGmuNonDiscoverableDeviceGuid,
               mGmu3Desc,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
}

/**
  Config Dp share memory.
  Format:
    UINT32    Version
    UINT32    Dp Used bit mask : 0 - unused, 1 - used
    UINT32    Edp bit mask : 0 - not, 1 - yes
    UINT32    Init row resolution
    UINT32    Init column resolution
    UINT32    mode : 0 - main ram, 1 - vedio ram
    UINT32    Edp light value
    UINT32    Downspread bit mask : 0 - not use, 1 - use.

  @param[in]  Addr       Share memory address.
  @param[in]  DpUsed     Dp Used bit mask
  @param[in]  IsEdp      Is Edp bit mask
  @param[in]  Row        Init row resolution
  @param[in]  Column     Init column resolution
  @param[in]  EdpValue   Edp value.

  @retval     Null
**/
STATIC
VOID
ConfigDpSharedMem (
  IN EFI_PHYSICAL_ADDRESS  Addr,
  IN UINT32                DpUsed,
  IN UINT32                IsEdp,
  IN UINT32                Row,
  IN UINT32                Column,
  IN UINT32                EdpValue
  )
{
  //version
  MmioWrite32 (Addr, 0x100);
  Addr += 4;
  MmioWrite32 (Addr, DpUsed);
  DEBUG ((DEBUG_INFO, "DpUsed : %x\n", DpUsed));
  Addr += 4;
  MmioWrite32 (Addr, IsEdp);
  DEBUG ((DEBUG_INFO, "IsEdp : %x\n", IsEdp));
  Addr += 4;
  MmioWrite32 (Addr, Row);
  DEBUG ((DEBUG_INFO, "Init row resolution : %d\n", Row));
  Addr += 4;
  MmioWrite32 (Addr, Column);
  DEBUG ((DEBUG_INFO, "Init column resolution : %d\n", Column));
  Addr += 4;
  MmioWrite32 (Addr, 0);
  Addr += 4;
  MmioWrite32 (Addr, EdpValue);
  DEBUG ((DEBUG_INFO, "EdpValue : %d\n", EdpValue));
  Addr += 4;
  MmioWrite32 (Addr, 0);
}

/**
  Initialize dcdp resource.
  1.Set dcdp config variable. Driver can get configuration(each path enable state)
    from this variable.
  2.Regitst device.

  @retval    NULL.
**/
STATIC
VOID
DcDpResourceInit (
  VOID
  )
{
  EFI_STATUS            Status;
  EFI_HANDLE            Handle;
  UINT32                DpUsed;
  EFI_PHYSICAL_ADDRESS  Address;
  UINT8                 Buffer[1024];
  BOARD_CONFIG          *BoardConfig;
  UINT8                 PhyMode0;
  UINT8                 PhyMode1;

  GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  BoardConfig = (BOARD_CONFIG *)Buffer;
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiBootServicesData,
                  1,
                  &Address
                  );
  ASSERT_EFI_ERROR (Status);
  //
  //Dp Used
  //
  DpUsed = 0;
  PhyMode0 = (BoardConfig->PhySelMode >> (4 * 2)) & 0x3;
  PhyMode1 = (BoardConfig->PhySelMode >> (5 * 2)) & 0x3;
  if ((PhyMode0 == 1) && PcdGetBool (PcdDcDpChanel0Enable)) {
    DpUsed |= 0x1;
  }
  if ((PhyMode1 == 1) && PcdGetBool (PcdDcDpChanel1Enable)) {
    DpUsed |= 0x2;
  }
  ConfigDpSharedMem (
    Address,
    DpUsed,
    0,
    PcdGet32 (PcdVideoHorizontalResolution),
    PcdGet32 (PcdVideoVerticalResolution),
    0
    );

  mDcDpDesc[2].AddrRangeMin = Address;
  mDcDpDesc[2].AddrRangeMax = Address + 4096;
  //
  //dc dp
  //
  Handle = NULL;
  Status = RegisterDevice (
             &gSocDcDpNonDiscoverableDeviceGuid,
             mDcDpDesc,
             &mDcDpDevicePath,
             &Handle
             );
  ASSERT_EFI_ERROR (Status);
}

/**
  Config Usb2 share memory.
  Format:
    UINT32    Version
    UINT32    Int num.

  @param[in]  Addr       Share memory address.
  @param[in]  IntNum     Interrupt number.

  @retval     Null
**/
STATIC
VOID
ConfigUsb2SharedMem (
  IN EFI_PHYSICAL_ADDRESS  Addr,
  IN UINT32                IntNum
  )
{
  //version
  MmioWrite32 (Addr, 0x100);
  //Interrupt Number
  Addr += 4;
  MmioWrite32 (Addr, IntNum);
  DEBUG ((DEBUG_INFO, "Interrupt Number  : %d\n", IntNum));
}

/**
  Usb 3.0 device register.

  @retval    Null.
**/
STATIC
VOID
UsbDeviceRegister (
  VOID
  )
{
  EFI_HANDLE            Handle;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Address;

  //
  //Usb3-0
  //
  Status = EFI_NOT_FOUND;
  //
  //Soc USB Xhci Controller Register
  //
  if (PcdGetBool (PcdPhytiumUsb30Enable)) {
    DEBUG ((DEBUG_INFO, "Usb3-0 Register!\n"));
    Handle = NULL;
    Status = RegisterDevice (
               &gPsuUsb3NonDiscoverableDeviceGuid,
               mPsuUsb30Desc,
               &mPsuUsb30DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Usb3-1
  //
  Status = EFI_NOT_FOUND;
  //
  //Soc USB Xhci Controller Register
  //
  if (PcdGetBool (PcdPhytiumUsb31Enable)) {
    DEBUG ((DEBUG_INFO, "Usb3-1 Register!\n"));
    Handle = NULL;
    Status = RegisterDevice (
               &gPsuUsb3NonDiscoverableDeviceGuid,
               mPsuUsb31Desc,
               &mPsuUsb31DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Usb2.0 p2-0
  //
  if (PcdGetBool (PcdUsb2P20Enable)) {
    DEBUG ((DEBUG_INFO, "Usb2.0 p2-0 Register!\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address));
    ConfigUsb2SharedMem (Address, 64);
    mUsb2P20Desc[2].AddrRangeMin = Address;
    mUsb2P20Desc[2].AddrRangeMax = Address + 4096;
    Handle = NULL;
    Status = RegisterDevice (
               &gPhytiumUsb2NonDiscoverableDeviceGuid,
               mUsb2P20Desc,
               &mUsb2P20DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Usb2.0 p3
  //
  if (PcdGetBool (PcdUsb2P3Enable)) {
    DEBUG ((DEBUG_INFO, "Usb2.0 p3 Register!\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address));
    ConfigUsb2SharedMem (Address, 46);
    mUsb2P3Desc[2].AddrRangeMin = Address;
    mUsb2P3Desc[2].AddrRangeMax = Address + 4096;
    Handle = NULL;
    Status = RegisterDevice (
               &gPhytiumUsb2NonDiscoverableDeviceGuid,
               mUsb2P3Desc,
               &mUsb2P3DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //Usb2.0 p4
  //
  if (PcdGetBool (PcdUsb2P4Enable)) {
    DEBUG ((DEBUG_INFO, "Usb2.0 p4 Register!\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address
                    );
    DEBUG ((DEBUG_INFO, "Share Memory Address : %llx\n", Address));
    ConfigUsb2SharedMem (Address, 47);
    mUsb2P4Desc[2].AddrRangeMin = Address;
    mUsb2P4Desc[2].AddrRangeMax = Address + 4096;
    Handle = NULL;
    Status = RegisterDevice (
               &gPhytiumUsb2NonDiscoverableDeviceGuid,
               mUsb2P4Desc,
               &mUsb2P4DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
}

/**
  Sata device register.

  @retval    Null.
**/
STATIC
VOID
GsdDeviceRegister (
  VOID
  )
{
  EFI_HANDLE       Handle;
  EFI_STATUS       Status;
  UINT8            Buffer[1024];
  BOARD_CONFIG     *BoardConfig;
  UINT8            Type;

  Status = EFI_NOT_FOUND;
  GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  BoardConfig = (BOARD_CONFIG *)Buffer;

  //sata 0 - psu sata
  Type = (BoardConfig->PhySelMode >> (1 * 2)) & 0x3;
  if (PcdGetBool (PcdPhytiumPsuSataEnable)) {
    Handle = NULL;
    Status = RegisterDevice (
               &gSocSataNonDiscoverableDeviceGuid,
               mPsuSataDesc,
               &mPsuSataDevicePath,
               &Handle
               );
    ASSERT_EFI_ERROR (Status);
  }
  //Sata 1 - gsd sata
  Type = (BoardConfig->PhySelMode >> (3 * 2)) & 0x3;
  if ((Type == 2) && PcdGetBool (PcdPhytiumGsdSataEnable)) {
    Handle = NULL;
    Status = RegisterDevice (
               &gSocSataNonDiscoverableDeviceGuid,
               mGsdSataDesc,
               &mGsdSataDevicePath,
               &Handle
               );
    ASSERT_EFI_ERROR (Status);
  }
}


EFI_STATUS
EFIAPI
PlatformDxeInitialise (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS          Status;

  Status = EFI_SUCCESS;

  StoreDdrTrainInfo ();
  //PhyConfigWithParTable ();
  UsbDeviceRegister ();
  GsdDeviceRegister ();
  GmuRegister ();
  DcDpResourceInit ();
  return Status;
}
