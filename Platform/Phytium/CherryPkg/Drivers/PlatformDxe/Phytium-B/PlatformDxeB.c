/** @file
  The Platform Initialization.

**/

#include <Platform.h>
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
  //GMU3 Register
  //
  Handle = NULL;
  Status = RegisterNoDevicepath (
             &gGmuNonDiscoverableDeviceGuid,
             mGmu3Desc,
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
  //Soc USB Xhci Controller Register, usb 3-0
  //
  if (1) {
    Handle = NULL;
    Status = RegisterDevice (
               &gPsuUsb3NonDiscoverableDeviceGuid,
               mPsuUsb30Desc,
               &mPsuUsb30DevicePath,
               &Handle);
    ASSERT_EFI_ERROR (Status);
  }
  //
  //psu sata
  //
  if (1) {
    Handle = NULL;
    Status = RegisterDevice (
               &gSocSataNonDiscoverableDeviceGuid,
               mPsuSataDesc,
               &mPsuSataDevicePath,
               &Handle
               );
    ASSERT_EFI_ERROR (Status);
  }
  //
  //GMU
  //
  GmuRegister ();
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

#define    BOARD_TYPE_A       0
#define    BOARD_TYPE_B       1
#define    BOARD_TYPE_C       2

VOID
PhyInit (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT32        BoardType;

  //BoardType = BOARD_TYPE_A;
  BoardType = BOARD_TYPE_B;
  SetMem ((VOID*)&ArmSmcArgs, sizeof (ARM_SMC_ARGS), 0);
  ArmSmcArgs.Arg0 = 0xc2000015;
  ArmSmcArgs.Arg1 = 0x3f;
  ArmSmcArgs.Arg2 = 0x100;
  ArmSmcArgs.Arg3 = 0x300000;
  ArmSmcArgs.Arg4 = 0x300000;
  ArmCallSmc (&ArmSmcArgs);
}

/**
  Initialize pad.

  @retval    NULL.
**/
VOID
PadInit (
  VOID
  )
{
  UINT32  Value;
#if 0
  PAD_INFO  *Info;
  UINT64    PadAddr;

  PadAddr = FixedPcdGet64 (PcdPadInfoAddr);
  DEBUG ((EFI_D_INFO, "Pad Infor Addr : 0x%x\n", PadAddr));
  Info = GetPadInfo (PadAddr);
  ConfigPad (Info);
  if (Info != NULL) {
    FreePool (Info);
  }
#else
//
//Phytium B board
//
// dp0_hpd, gpio51 00c0  fun0
  Value =  MmioRead32 (0x32B300c0);
  Value &= ~(0x7);
  MmioWrite32 (0x32B300c0, Value);
//
//RGMII1/Mac3
//
//rgmii
  MmioWrite32 (0x32b301cc, 0x41);
  MmioWrite32 (0x32b301d0, 0x41);
  MmioWrite32 (0x32b301d4, 0x41);
  MmioWrite32 (0x32b301d8, 0x41);
  MmioWrite32 (0x32b301dc, 0x41);
  MmioWrite32 (0x32b301e0, 0x41);
  MmioWrite32 (0x32b301e4, 0x41);
  MmioWrite32 (0x32b301e8, 0x41);
  MmioWrite32 (0x32b301ec, 0x41);
  //
  //rgmii tx clock
  //
  MmioWrite32 (0x32b301f0, 0x41);
  MmioWrite32 (0x32b311f0, 0x5f00);
  MmioWrite32 (0x32b301f4, 0x41);
  MmioWrite32 (0x32b301f8, 0x41);
//mdio/mdc
  Value = MmioRead32 (0x32b301fc);
  Value &= ~(0x7);
  Value |= 0x1;
  MmioWrite32 (0x32b301fc, Value);
  Value = MmioRead32 (0x32b30200);
  Value &= ~(0x7);
  Value |= 0x1;
  MmioWrite32 (0x32b30200, Value);
#endif
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
  //PadInit ();
  //PhyInit ();
  PlatDeviceRegister ();
  PhyConfigWithParTable ();
  DcDpResourceInit ();
  return Status;
}
