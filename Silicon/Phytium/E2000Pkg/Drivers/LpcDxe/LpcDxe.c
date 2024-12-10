#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/Lpc.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
//#include <Library/PhytiumPowerControlLib.h>
//
// Lpc Device Structure
//
#define LPC_DEVICE_SIGNATURE SIGNATURE_32 ('l', 'p', 'c', 'i')
#define PM_EC 1

typedef struct {
  UINT32                                    Signature;
  EFI_HANDLE                                Handle;
  UINTN                      				LpcBase;
} LPC_DEVICE;



EFI_LPC_PROTOCOL  glpcProtocol = {
  0x28000000,
};

EFI_STATUS
EFIAPI
InitializeLpcDxe (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  LPC_DEVICE	*LpcDeviec;
  CHAR16                   *DevicePathStr;
  //UINT32        S3Flag;

  DevicePathStr = NULL;
  //
  // Initialize the LPC_DEVICE structure
  //
  LpcDeviec = AllocateZeroPool (sizeof (LPC_DEVICE));
  if (LpcDeviec == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  LpcDeviec->Signature  = LPC_DEVICE_SIGNATURE;
  LpcDeviec->Handle     = NULL;

  /*S3Flag = pm_get_s3_flag_source();
  if (S3Flag != PM_EC) {
    DEBUG((EFI_D_ERROR, "EC not exist\n"));
    return EFI_SUCCESS;
  }*/

  //
  // Build device path
  //
  DEBUG ((EFI_D_INFO, "%s\n", DevicePathStr = ConvertDevicePathToText ((EFI_DEVICE_PATH_PROTOCOL *)&mKeyboardVendorDevicePath, FALSE, FALSE)));
  //
  // Create a child handle and install Device Path and Lpc protocols
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &LpcDeviec->Handle,
                  &gEfiDevicePathProtocolGuid,
                  &mKeyboardVendorDevicePath,
                  &gEfiLpcProtocolGuid,
                  &glpcProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG((EFI_D_ERROR, "%a() L%d controller = %x \n",__FUNCTION__, __LINE__, LpcDeviec->Handle));
  return Status;
}



