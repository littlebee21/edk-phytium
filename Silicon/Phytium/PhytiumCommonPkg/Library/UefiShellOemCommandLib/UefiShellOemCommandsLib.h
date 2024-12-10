/** @file
The header files of OEM command library.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef UEFI_SHELL_OEM_COMMANDS_LIB_H_
#define UEFI_SHELL_OEM_COMMANDS_LIB_H_
#include <Pi/PiFirmwareFile.h>

#include <Guid/ShellLibHiiGuid.h>
#include <Guid/GlobalVariable.h>

#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/ShellLib.h>
#include <Library/SortLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ServiceBinding.h>
#include <Protocol/Shell.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/UnicodeCollation.h>

#include <Uefi.h>

extern EFI_HANDLE gShellOemHiiHandle;

SHELL_STATUS
EFIAPI
ShellCommandRunBootefi (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

SHELL_STATUS
EFIAPI
ShellCommandRunMemCp (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#endif // UEFI_SHELL_OEM_COMMANDS_LIB_H
