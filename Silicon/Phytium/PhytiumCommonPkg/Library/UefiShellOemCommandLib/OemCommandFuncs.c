/** @file
The implementation of the OEM commands.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/ArmSmcLib.h>

#include "UefiShellOemCommandsLib.h"

/**
  Function for 'MemCp' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).

  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).

  @retval EFI_SUCCESS       The entry point is executed successfully.

  @retval SHELL_INVALID_PARAMETER   There is an error with the parameter.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunMemCp (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status;
  SHELL_STATUS      ShellStatus;
  LIST_ENTRY        *Package;
  CHAR16            *ProblemParam;
  CONST CHAR16      *Param1;
  CONST CHAR16      *Param2;
  CONST CHAR16      *Param3;
  UINTN Destination;
  UINTN Source;
  UINTN Length;

  Status = ShellCommandLineParse (EmptyParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellOemHiiHandle, L"MemCp", ProblemParam);
      FreePool(ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  }

  //
  // check for "-?"
  //
  if (ShellCommandLineGetFlag(Package, L"-?")) {
    ASSERT(FALSE);
  } else if (ShellCommandLineGetRawValue(Package, 4) != NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), gShellOemHiiHandle, L"MemCp");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else if (ShellCommandLineGetRawValue(Package, 3) == NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW), gShellOemHiiHandle, L"MemCp");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else {
    //
    Param1 = ShellCommandLineGetRawValue (Package, 1);
    Param2 = ShellCommandLineGetRawValue (Package, 2);
    Param3 = ShellCommandLineGetRawValue (Package, 3);


    ShellConvertStringToUint64(Param1, &Destination, FALSE, FALSE);
    ShellConvertStringToUint64(Param2, &Source, FALSE, FALSE);
    ShellConvertStringToUint64(Param3, &Length, FALSE, FALSE);

    ShellPrintEx(-1, -1, L"Copy from [%x] to 0x[%x], Length:%d \r\n", Source, Destination, Length);
    gBS->CopyMem((VOID *)Destination, (VOID *)Source, Length);

    ShellPrintEx(-1, -1, L"Copy ok\r\n");

  }

  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  //
  // return the status
  //
  return (ShellStatus);

}

/**
  Function for 'Bootefi' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).

  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).

  @retval EFI_SUCCESS       The entry point is executed successfully.

  @retval SHELL_INVALID_PARAMETER   There is an error with the parameter.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunBootefi (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  CONST CHAR16      *BootArgs;
  UINTN             BootArgsSize;
  CHAR16            *ClibParam;
  EFI_DEVICE_PATH   *DevicePath;
  EFI_SHELL_PARAMETERS_PROTOCOL EfiShellParametersProtocol;
  UINTN             Index;
  UINT64            ImageAddress;
  CONST CHAR16      *ImageSize;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  EFI_HANDLE        MyImageHandle;
  LIST_ENTRY        *Package;
  CONST CHAR16      *Param1;
  CONST CHAR16      *ParamClib;
  UINTN             ParamNum;
  CHAR16            *ProblemParam;
  UINTN             Size;
  EFI_STATUS        Status;
  SHELL_STATUS      ShellStatus;

  BootArgsSize  = 0;
  ClibParam  = NULL;
  DevicePath    = NULL;
  ParamNum        = 0;
  ProblemParam  = NULL;
  MyImageHandle = NULL;
  ShellStatus   = SHELL_SUCCESS;

  ZeroMem (&EfiShellParametersProtocol, sizeof(EFI_SHELL_PARAMETERS_PROTOCOL));

  //
  // parse the command line
  //
  Status = ShellCommandLineParse (EmptyParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PROBLEM),
        gShellOemHiiHandle,
        L"Bootefi",
        ProblemParam
        );
      FreePool (ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  }

  //
  // check for "-?"
  //
  if (ShellCommandLineGetFlag (Package, L"-?")) {
    ASSERT (FALSE);
  } else if (ShellCommandLineGetRawValue (Package, 1) == NULL){
    ShellPrintHiiEx (
      -1,
      -1,
      NULL,
      STRING_TOKEN (STR_GEN_TOO_FEW),
      gShellOemHiiHandle,
      L"Bootefi"
      );
    ShellPrintEx(-1,
      -1,
      L"bootefi <address> (<clib> <(param1 ...paramx)>\r\n"
      );
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else {
    ParamNum = ShellCommandLineGetCount (Package);
    ShellPrintEx (-1, -1, L"ParamNum= %d\r\n", ParamNum);
    Param1 = ShellCommandLineGetRawValue (Package, 1);
    Status = ShellConvertStringToUint64 (Param1, &ImageAddress, FALSE, FALSE);
    if (EFI_ERROR(Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1
        );
      return SHELL_INVALID_PARAMETER;
    }

    ImageSize = ShellGetEnvironmentVariable (L"image_size");
    if (ImageSize == NULL) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_ENV_NOT_FOUND),
        gShellOemHiiHandle,
        L"Bootefi"
        );
      return SHELL_INVALID_PARAMETER;
    }

    Status = ShellConvertStringToUint64 (ImageSize, &Size, FALSE, FALSE);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1);
      return SHELL_INVALID_PARAMETER;
    }

    Status = gBS->LoadImage (
                    FALSE,
                    gImageHandle,
                    DevicePath,
                    (VOID*)ImageAddress,
                    Size,
                    &MyImageHandle
                    );
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_LOAD_FILE_FAIL),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1
        );
      return SHELL_LOAD_ERROR;
    }

    Status = gBS->HandleProtocol (
             MyImageHandle,
             &gEfiLoadedImageProtocolGuid,
             (VOID **)&LoadedImage
           );
    ASSERT_EFI_ERROR(Status);

    if (ParamNum < 3) {
      BootArgs = ShellGetEnvironmentVariable (L"bootargs");
      if (BootArgs != NULL)  {
        ShellPrintEx (-1, -1, L"BootArgs= %x\r\n", BootArgs);
        BootArgsSize = StrSize (BootArgs);
        LoadedImage->LoadOptions = (VOID *)BootArgs;
        LoadedImage->LoadOptionsSize = BootArgsSize;
        gBS->ReinstallProtocolInterface(
               MyImageHandle,
               &gEfiLoadedImageProtocolGuid,
               (VOID *)LoadedImage,
               (VOID *)LoadedImage
               );
      }
    }else if (ParamNum >= 3) {
      ParamClib = ShellCommandLineGetRawValue (Package, 2);
      ShellPrintEx (-1, -1, L"%s \n", ParamClib);
      if (StrCmp (ParamClib, L"clib") == 0) {
        ShellPrintEx(-1, -1, L"This is a Clib App.\n");

        Status = gBS->OpenProtocol (
                        gImageHandle,
                        &gEfiShellParametersProtocolGuid,
                        (VOID **)&EfiShellParametersProtocol,
                        gImageHandle,
                        NULL,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
        if (EFI_ERROR (Status)){
          ShellPrintEx (-1, -1, L"EfiShellParametersProtocol open failed.\r\n");
          return Status;
        }

        EfiShellParametersProtocol.Argc = (ParamNum - 2);
        EfiShellParametersProtocol.Argv = AllocatePool (
                                            sizeof (CHAR16 *) * 24
                                            );
        if (EfiShellParametersProtocol.Argv == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto UnloadImage;
        }

        ShellPrintEx (
          -1,
          -1,
          L"EfiShellParametersProtocol.Argc=%d\n",
          EfiShellParametersProtocol.Argc
          );

        for (Index = 0; Index < EfiShellParametersProtocol.Argc; Index++) {
          ClibParam = (CHAR16 *)ShellCommandLineGetRawValue (Package, Index + 2);
          EfiShellParametersProtocol.Argv[Index] = ClibParam;
        }

        Status = gBS->InstallProtocolInterface (
                        &MyImageHandle,
                        &gEfiShellParametersProtocolGuid,
                        EFI_NATIVE_INTERFACE,
                        &EfiShellParametersProtocol
                        );
        if (EFI_ERROR (Status)){
          ShellPrintEx (
            -1,
            -1,
            L"EfiShellParametersProtocol install failed\r\n"
            );
          return Status;
        }

        Status = gBS->StartImage (MyImageHandle, NULL, NULL);
        if (EFI_ERROR(Status)) {
          ShellPrintEx (-1, -1, L"StartImage failed.\r\n");
          return SHELL_INVALID_PARAMETER;
        }

        Status = gBS->UninstallProtocolInterface (
                    MyImageHandle,
                    &gEfiShellParametersProtocolGuid,
                    &EfiShellParametersProtocol
                    );
        if (EFI_ERROR(Status)) {
          ShellPrintEx (
            -1,
            -1,
            L"EfiShellParametersProtocol Uninstall failed.\r\n"
            );
          return SHELL_INVALID_PARAMETER;
        }

        // Free Argv.
        if (EfiShellParametersProtocol.Argv != NULL) {
          FreePool (EfiShellParametersProtocol.Argv);
        }
        goto Return;
      }else {
        ShellPrintEx (-1, -1, L"Clib parameter error.\r\n");
        goto Return;
      }
    }
    Status = gBS->StartImage (MyImageHandle, 0, NULL);
    if (EFI_ERROR (Status)) {
      ShellPrintEx (-1, -1, L"StartImage failed Status =%x.\r\n", Status);
      goto Return;
    }
  }

UnloadImage:
  gBS->UnloadImage (MyImageHandle);

Return:
  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  //
  // return the status
  //
  return (ShellStatus);
}
