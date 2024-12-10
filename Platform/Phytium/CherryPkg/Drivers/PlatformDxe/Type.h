#ifndef    _PLATFORM_TYPE_H_
#define    _PLATFORM_TYPE_H_

#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PadLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/ArmSmcLib.h>

#include <Platform.h>

typedef struct {
  VENDOR_DEVICE_PATH              VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        End;
} HII_VENDOR_DEVICE_PATH;

extern HII_VENDOR_DEVICE_PATH  mPsuUsb30DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuUsb30Desc[];
extern HII_VENDOR_DEVICE_PATH  mPsuUsb31DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuUsb31Desc[];
extern HII_VENDOR_DEVICE_PATH  mPsuSataDevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuSataDesc[];
extern HII_VENDOR_DEVICE_PATH  mGsdSataDevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGsdSataDesc[];
extern HII_VENDOR_DEVICE_PATH  mDcDpDevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mDcDpDesc[];
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu0Desc[];
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu1Desc[];
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu2Desc[];
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu3Desc[];
extern HII_VENDOR_DEVICE_PATH  mUsb2P20DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P20Desc[];
extern HII_VENDOR_DEVICE_PATH  mUsb2P21DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P21Desc[];
extern HII_VENDOR_DEVICE_PATH  mUsb2P22DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P22Desc[];
extern HII_VENDOR_DEVICE_PATH  mUsb2P3DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P3Desc[];
extern HII_VENDOR_DEVICE_PATH  mUsb2P4DevicePath;
extern EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P4Desc[];

#endif
