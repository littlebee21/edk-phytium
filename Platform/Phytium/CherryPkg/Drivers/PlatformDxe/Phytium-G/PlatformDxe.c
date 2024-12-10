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

#include <Platform.h>
#include <Phy.h>
#include "../Type.h"

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

VOID
GmuRegister (
  VOID
  )
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;

  Status = EFI_NOT_FOUND;
  //
  //GMU0 Register
  //
  Handle = NULL;
  Status = RegisterNoDevicepath (
             &gGmuNonDiscoverableDeviceGuid,
             mGmu0Desc,
             &Handle);
  ASSERT_EFI_ERROR (Status);
}

STATIC
VOID
PlatDeviceRegister (
  VOID
  )
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;

  Status = EFI_NOT_FOUND;
  //
  //Soc USB Xhci Controller Register 0 and 1
  //
  Handle = NULL;
  Status = RegisterDevice (
             &gPsuUsb3NonDiscoverableDeviceGuid,
             mPsuUsb30Desc,
             &mPsuUsb30DevicePath,
             &Handle);
  ASSERT_EFI_ERROR (Status);
  Handle = NULL;
  Status = RegisterDevice (
             &gPsuUsb3NonDiscoverableDeviceGuid,
             mPsuUsb31Desc,
             &mPsuUsb31DevicePath,
             &Handle);
  ASSERT_EFI_ERROR (Status);
  //
  //GSD SATA - SATA1
  //
  Handle = NULL;
  Status = RegisterDevice (
             &gSocSataNonDiscoverableDeviceGuid,
             mGsdSataDesc,
             &mGsdSataDevicePath,
             &Handle);
  ASSERT_EFI_ERROR (Status);
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
  UINT64       Value;
  DCDP_CONFIG  Config;
  EFI_STATUS   Status;
  UINTN        VarSize;
  EFI_HANDLE   Handle;

  VarSize = sizeof (DCDP_CONFIG);
  Value = FixedPcdGet64 (PcdDcDpConfig);
  DEBUG ((EFI_D_INFO, "Dc Dp Config : %x\n", Value));
  ZeroMem (&Config, sizeof (DCDP_CONFIG));
  if (Value & 0x1) {
    Config.Enable[0] = 1;
  }
  if ((Value >> 1) & 0x1) {
    Config.Enable[1] = 1;
  }
  Status = gRT->SetVariable (
                  DCDP_CONFIG_VARIABLE,
                  &gSocDcDpConfigGuid,
                  DCDP_VARIABLE_FLAG,
                  VarSize,
                  &Config
                  );
  ASSERT_EFI_ERROR(Status);
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

EFI_STATUS
EFIAPI
PlatformDxeInitialise (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS          Status;
  DEBUG ((EFI_D_INFO, "Phytium-G Board!\n"));
  Status = EFI_SUCCESS;
  PlatDeviceRegister ();
  DcDpResourceInit ();
  //PhyConfigWithBoardType (FixedPcdGet32 (PcdPhytiumBoardType));
  PhyConfigWithParTable ();
  return Status;
}
