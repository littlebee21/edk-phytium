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
ConfigGmuSharedMem (
  EFI_PHYSICAL_ADDRESS  Addr,
  UINT32                Interface,
  UINT32                Auto,
  UINT32                Speed,
  UINT32                Duplex,
  EFI_MAC_ADDRESS       *Mac
  )
{
  UINT32 Index;
  //version
  MmioWrite32 (Addr, 0x100);
  Addr += 4;
  MmioWrite32 (Addr, Interface);
  DEBUG ((EFI_D_INFO, "Interface : %d\n", Interface));
  Addr += 4;
  MmioWrite32 (Addr, Auto);
  DEBUG ((EFI_D_INFO, "Autoeng : %d\n", Auto));
  Addr += 4;
  MmioWrite32 (Addr, Speed);
  DEBUG ((EFI_D_INFO, "Speed : %d\n", Speed));
  Addr += 4;
  MmioWrite32 (Addr, Duplex);
  DEBUG ((EFI_D_INFO, "Duplex : %d\n", Duplex));
  Addr += 4;
  for (Index = 0; Index < 6; Index++) {
    MmioWrite8 (Addr + Index, Mac->Addr[Index]);
  }
  DEBUG ((EFI_D_INFO, "Mac : %02x-%02x-%02x-%02x-%02x-%02x\n",
              Mac->Addr[0], Mac->Addr[1], Mac->Addr[2], Mac->Addr[3],
              Mac->Addr[4], Mac->Addr[5]));
}

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

  Status = EFI_NOT_FOUND;
  GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  BoardConfig = (BOARD_CONFIG *)Buffer;
  //
  //Mac 0
  //
  PhyMode = (BoardConfig->PhySelMode >> (2 * 2)) & 0x3;
  if ((PhyMode == 0) && (PcdGetBool (PcdPhytiumGmu0Enable))) {
    DEBUG ((EFI_D_INFO, "Gmu 0 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[0]
                    );
    DEBUG ((EFI_D_INFO, "Share Memory Address : %llx\n", Address[0]));
    Mac = PcdGetPtr (PcdPhytiumGmu0Mac);
    for (Index = 0; Index < 6; Index++) {
      MacAddr.Addr[Index] = Mac[Index];
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
    DEBUG ((EFI_D_INFO, "Gmu 1 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[1]
                    );
    DEBUG ((EFI_D_INFO, "Share Memory Address : %llx\n", Address[1]));
    Mac = PcdGetPtr (PcdPhytiumGmu1Mac);
    for (Index = 0; Index < 6; Index++) {
      MacAddr.Addr[Index] = Mac[Index];
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
  PhyMode = (BoardConfig->PhySelMode >> (4 * 2)) & 0x3;
  if ((PhyMode == 0) && (PcdGetBool (PcdPhytiumGmu2Enable))) {
    DEBUG ((EFI_D_INFO, "Gmu 2 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[2]
                    );
    DEBUG ((EFI_D_INFO, "Share Memory Address : %llx\n", Address[2]));
    Mac = PcdGetPtr (PcdPhytiumGmu2Mac);
    for (Index = 0; Index < 6; Index++) {
      MacAddr.Addr[Index] = Mac[Index];
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
  PhyMode = (BoardConfig->PhySelMode >> (5 * 2)) & 0x3;
  if ((PhyMode == 0) && (PcdGetBool (PcdPhytiumGmu3Enable))) {
    DEBUG ((EFI_D_INFO, "Gmu 3 Existed.\n"));
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    1,
                    &Address[3]
                    );
    DEBUG ((EFI_D_INFO, "Share Memory Address : %llx\n", Address[3]));
    Mac = PcdGetPtr (PcdPhytiumGmu3Mac);
    for (Index = 0; Index < 6; Index++) {
      MacAddr.Addr[Index] = Mac[Index];
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
  //Soc USB Xhci Controller Register
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

  BoardType = BOARD_TYPE_A;
  SetMem ((VOID*)&ArmSmcArgs, sizeof (ARM_SMC_ARGS), 0);
  ArmSmcArgs.Arg0 = 0xc2000015;
  ArmSmcArgs.Arg1 = 0x3f;
  ArmSmcArgs.Arg2 = 0x504;
  ArmSmcArgs.Arg3 = 0x3300;
  ArmSmcArgs.Arg4 = 0x3300;
  ArmCallSmc (&ArmSmcArgs);
}

/**
  Initialize pad.

  @retval    NULL.
**/
#if 1
VOID
PadInit (
  VOID
  )
{
  //
  //gmu mdio and mdc
  //
  //MmioWrite32 (0x32B300f0, 0x4 | (MmioRead32(0x32B300f0)));
  //MmioWrite32 (0x32B300f4, 0x4 | (MmioRead32(0x32B300f4)));
  MmioWrite32 (0x32B300f8, 0x73);
  MmioWrite32 (0x32B300fc, 0x73);
  MmioWrite32 (0x32B30100, 0x73);
  MmioWrite32 (0x32B30104, 0x73);
  //Mio3
  MmioWrite32 (0x32b30184, 0x71);
  MmioWrite32 (0x32b30188, 0x71);
  //dp
  //MmioWrite32 (0x32b300c0, 0x71);
  //MmioWrite32 (0x32b300c4, 0x71);
}
#endif

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
  //phy config
  //PhyInit ();
  PhyConfigWithParTable ();
  PlatDeviceRegister ();
  DcDpResourceInit ();
  return Status;
}
