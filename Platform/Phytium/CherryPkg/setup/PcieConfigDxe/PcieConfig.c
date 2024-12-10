/** @file
  HII Config Access protocol implementation of pcie configuration module.
**/
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Smbios.h>
#include <OEMSvc.h>
#include <Library/HiiLib.h>
#include <ArmPlatform.h>
#include <Library/HobLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/PciIo.h>
#include "PcieConfig.h"
#include "PcieConfigFormGuid.h"
#include <Library/ParameterTable.h>

VOID                  *mStartOpCodeHandle = NULL;
VOID                  *mEndOpCodeHandle = NULL;
EFI_IFR_GUID_LABEL    *mStartLabel = NULL;
EFI_IFR_GUID_LABEL    *mEndLabel = NULL;

EFI_GUID mPcieConfigGuid = PCIECONFIG_FORMSET_GUID;
extern UINT8 PcieConfigUiBin[];
BOOLEAN  mFirstEnterPcieConfigForm = FALSE;

HII_VENDOR_DEVICE_PATH  mPcieConfigHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {0x86d4f08a, 0x2a61, 0x11ec, {0x99, 0x22, 0xf3, 0x58, 0x56, 0x47, 0xcf, 0xc7}}
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

PCIE_CONFIG_PRIVATE_DATA gPcieConfigPrivate = {
  PCIE_CONFIG_PRIVATE_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    PcieConfigExtractConfig,
    PcieConfigRouteConfig,
    PcieConfigCallback
  }
};

/**
  Pcie configuration restore defaults.

  @param[in,out]    Config    Point to pcie configuration struct.
**/
VOID
PcieConfigDefault (
  IN OUT PCIE_CONFIG_DATA  *Config
  )
{
  UINT8 Peu0SplitMode;
  UINT8 Peu1SplitMode;
  UINT8 Peu0C0Enable;
  UINT8 Peu0C1Enable;

  UINT8 Peu1C0Enable;
  UINT8 Peu1C1Enable;
  UINT8 Peu1C2Enable;
  UINT8 Peu1C3Enable;

  UINT8 Peu0C0LinkSpeed;
  UINT8 Peu0C1LinkSpeed;

  UINT8 Peu1C0LinkSpeed;
  UINT8 Peu1C1LinkSpeed;
  UINT8 Peu1C2LinkSpeed;
  UINT8 Peu1C3LinkSpeed;

  UINT8 Peu0C0EqualValue;
  UINT8 Peu0C1EqualValue;
  
  UINT8 Peu1C0EqualValue;
  UINT8 Peu1C1EqualValue;
  UINT8 Peu1C2EqualValue;
  UINT8 Peu1C3EqualValue;

  PARAMETER_PEU_CONFIG *ParameterPcieConfig;
  UINT8 Buffer[0x100];

  DEBUG((EFI_D_INFO, "fun:%a() line:%d\n", __FUNCTION__, __LINE__));
  GetParameterInfo(PM_PCIE, Buffer, sizeof(Buffer));
  ParameterPcieConfig = (PARAMETER_PEU_CONFIG *)Buffer;
  DEBUG((EFI_D_INFO, "functionCOnfig:0x%x\n", ParameterPcieConfig->FunctionConfig));
  Peu0SplitMode = (ParameterPcieConfig->FunctionConfig >> PEU0_SPLITMODE_OFFSET) & 0xf;
  Peu1SplitMode = (ParameterPcieConfig->FunctionConfig >> PEU1_SPLITMODE_OFFSET) & 0xf;

  /********************PEU0************************/
  Peu0C0Enable = (ParameterPcieConfig->Peu0ControllerConfig[0] >> ENABLE_OFFSET) & 0x1;
  Peu0C0LinkSpeed = (ParameterPcieConfig->Peu0ControllerConfig[0] >> SPEED_OFFSET) & 0xff;
  Peu0C0EqualValue = ParameterPcieConfig->Peu0ControllerEqualization[0];
  DEBUG((EFI_D_INFO, "Peu0C0Enable:0x%x, Peu0C0LinkSpeed:0x%x, Peu0C0EqualValue:0x%x \n", Peu0C0Enable, Peu0C0LinkSpeed, Peu0C0EqualValue));

  Peu0C1Enable = (ParameterPcieConfig->Peu0ControllerConfig[1] >> ENABLE_OFFSET) & 0x1;
  Peu0C1LinkSpeed = (ParameterPcieConfig->Peu0ControllerConfig[1] >> SPEED_OFFSET) & 0xff;
  Peu0C1EqualValue = ParameterPcieConfig->Peu0ControllerEqualization[1];

  /********************PEU1************************/
  Peu1C0Enable = (ParameterPcieConfig->Peu1ControllerConfig[0] >> ENABLE_OFFSET) & 0x1;
  Peu1C0LinkSpeed = (ParameterPcieConfig->Peu1ControllerConfig[0] >> SPEED_OFFSET) & 0xff;
  Peu1C0EqualValue = ParameterPcieConfig->Peu1ControllerEqualization[0];

  Peu1C1Enable = (ParameterPcieConfig->Peu1ControllerConfig[1] >> ENABLE_OFFSET) & 0x1;
  Peu1C1LinkSpeed = (ParameterPcieConfig->Peu1ControllerConfig[1] >> SPEED_OFFSET) & 0xff;
  Peu1C1EqualValue = ParameterPcieConfig->Peu1ControllerEqualization[1];

  Peu1C2Enable = (ParameterPcieConfig->Peu1ControllerConfig[2] >> ENABLE_OFFSET) & 0x1;
  Peu1C2LinkSpeed = (ParameterPcieConfig->Peu1ControllerConfig[2] >> SPEED_OFFSET) & 0xff;
  Peu1C2EqualValue = ParameterPcieConfig->Peu1ControllerEqualization[2];

  Peu1C3Enable = (ParameterPcieConfig->Peu1ControllerConfig[3] >> ENABLE_OFFSET) & 0x1;
  Peu1C3LinkSpeed = (ParameterPcieConfig->Peu1ControllerConfig[3] >> SPEED_OFFSET) & 0xff;
  Peu1C3EqualValue = ParameterPcieConfig->Peu1ControllerEqualization[3];

  //PEU 0
  Config->PcieConfig[0].SplitMode = Peu0SplitMode;
  Config->PcieConfig[0].PcieControl[0].DisEnable = Peu0C0Enable;
  Config->PcieConfig[0].PcieControl[0].Speed = Peu0C0LinkSpeed;
  Config->PcieConfig[0].PcieControl[0].EqualValue = Peu0C0EqualValue;
  Config->PcieConfig[0].PcieControl[1].DisEnable = Peu0C1Enable;
  Config->PcieConfig[0].PcieControl[1].Speed = Peu0C1LinkSpeed;
  Config->PcieConfig[0].PcieControl[1].EqualValue = Peu0C1EqualValue;

  //PEU 1
  Config->PcieConfig[1].SplitMode = Peu1SplitMode;
  Config->PcieConfig[1].PcieControl[0].DisEnable = Peu1C0Enable;
  Config->PcieConfig[1].PcieControl[0].Speed = Peu1C0LinkSpeed;
  Config->PcieConfig[1].PcieControl[0].EqualValue = Peu1C0EqualValue;
  Config->PcieConfig[1].PcieControl[1].DisEnable = Peu1C1Enable;
  Config->PcieConfig[1].PcieControl[1].Speed = Peu1C1LinkSpeed;
  Config->PcieConfig[1].PcieControl[1].EqualValue = Peu1C1EqualValue;
  Config->PcieConfig[1].PcieControl[2].DisEnable = Peu1C2Enable;
  Config->PcieConfig[1].PcieControl[2].Speed = Peu1C2LinkSpeed;
  Config->PcieConfig[1].PcieControl[2].EqualValue = Peu1C3EqualValue;
  Config->PcieConfig[1].PcieControl[3].DisEnable = Peu1C3Enable;
  Config->PcieConfig[1].PcieControl[3].Speed = Peu1C3LinkSpeed;
  Config->PcieConfig[1].PcieControl[3].EqualValue = Peu1C3EqualValue;

  DEBUG((EFI_D_INFO, "PcieConfig0.splitMode:0x%x, pcieconfig1.splimode:0x%x\n", Config->PcieConfig[0].SplitMode, Config->PcieConfig[1].SplitMode));
}

/**
  Pcie configuration initialization.Get the current pice configuration from variable.
  If the variable does not exist, restore the default configuration and save the
  variable.

  @return    EFI_SUCCESS    Pcie configuration initialization successfully.
  @return    other          Pcie configuration initialization failed.
**/
EFI_STATUS
PcieConfigInit (
  VOID
  )
{
  EFI_STATUS        Status;
  PCIE_CONFIG_DATA  PcieConfigData;
  UINTN             VarSize;

  VarSize = sizeof(PCIE_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PCIE_CONFIG_NAME,
                  &gPcieConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PcieConfigData
                  );
  if (Status == EFI_NOT_FOUND) {
    PcieConfigDefault (&PcieConfigData);
    Status = gRT->SetVariable (
                    VAR_PCIE_CONFIG_NAME,
                    &gPcieConfigVarGuid,
                    PLATFORM_SETUP_VARIABLE_FLAG,
                    VarSize,
                    &PcieConfigData
                    );
    ASSERT_EFI_ERROR(Status);
  }

  return Status;
}

/**
  Get pcie configuration from variable and update uncommitted data in the form.

  @param[in]    PrivateData    Point to PCIE_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Update pcie configuration in the form successfully.
  @return       other          Update pcie configuration in the form failed.
**/
EFI_STATUS
GetPcieConfigData (
  IN PCIE_CONFIG_PRIVATE_DATA  *PrivateData
  )
{
  EFI_STATUS        Status;
  PCIE_CONFIG_DATA  PcieConfigData;
  UINTN             VarSize;

  Status = EFI_SUCCESS;
  VarSize = sizeof (PCIE_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PCIE_CONFIG_NAME,
                  &gPcieConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PcieConfigData
                  );
  if (Status == EFI_SUCCESS) {
    CopyMem (&PrivateData->PcieConfigData, &PcieConfigData, VarSize);
    Status = HiiSetBrowserData (
               &mPcieConfigGuid,
               PCIE_CONFIG_VARIABLE,
               sizeof (PCIE_CONFIG_DATA),
               (UINT8 *) &PrivateData->PcieConfigData,
               NULL
               );
  }
  return Status;
}

/**
  Get pcie configuration from variable.

  @param[in]    PrivateData    Point to PCIE_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Get pcie configuration from variable successfully.
  @return       other          Get pcie configuration from variable failed.
**/
EFI_STATUS
UpdatePcieConfig (
  IN PCIE_CONFIG_PRIVATE_DATA  *PrivateData
)
{
  EFI_STATUS        Status;
  PCIE_CONFIG_DATA  PcieConfigData;
  UINTN             VarSize;

  Status = EFI_SUCCESS;
  VarSize = sizeof (PCIE_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PCIE_CONFIG_NAME,
                  &gPcieConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PcieConfigData
                  );
  if (Status == EFI_SUCCESS) {
    CopyMem (&PrivateData->PcieConfigData, &PcieConfigData, VarSize);
  }
  return Status;
}

/**
  Get pcie configuration from the form and set the configuration to the variable.

  @param[in]    This           Point to EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]    PrivateData    Point to PCIE_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Update pcie configuration in the form successfully.
  @return       other          Update pcie configuration in the form failed.
**/
EFI_STATUS
PcieConfigSetupConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN PCIE_CONFIG_PRIVATE_DATA               *PrivateData
  )
{
  EFI_STATUS            Status;
  PCIE_CONFIG_DATA      PcieData;
//  UINT32                Index;
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mPcieConfigGuid, PCIE_CONFIG_VARIABLE, sizeof (PCIE_CONFIG_DATA), (UINT8 *) &PcieData)) {
    return EFI_NOT_FOUND;
  }
  /*
  for (Index = 0; Index < 2; Index++) {
    if (PcieData.PcieConfig[Index].SplitMode == 1) {
      PcieData.PcieConfig[Index].PcieControl[1].Speed = PcieData.PcieConfig[Index].PcieControl[0].Speed;
      PcieData.PcieConfig[Index].PcieControl[1].DisEnable = PcieData.PcieConfig[Index].PcieControl[0].DisEnable;
      PcieData.PcieConfig[Index].PcieControl[1].EqualValue = PcieData.PcieConfig[Index].PcieControl[0].EqualValue;
    }
  }*/
  CopyMem (&PrivateData->PcieConfigData, &PcieData, sizeof(PCIE_CONFIG_DATA));
  Status = EFI_SUCCESS;
  Status = gRT->SetVariable (
                  VAR_PCIE_CONFIG_NAME,
                  &gPcieConfigVarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (PCIE_CONFIG_DATA),
                  &PrivateData->PcieConfigData
                  );
  return Status;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
PcieConfigExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                     Status;
  PCIE_CONFIG_PRIVATE_DATA       *Private;
  EFI_STRING                     ConfigRequestHdr;
  EFI_STRING                     ConfigRequest;
  BOOLEAN                        AllocatedRequest;
  UINTN                          Size;
  UINTN                          BufferSize;

  Status = EFI_SUCCESS;
  BufferSize = sizeof (PCIE_CONFIG_DATA);

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = PCIE_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  UpdatePcieConfig (Private);
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mPcieConfigGuid, PCIE_CONFIG_VARIABLE)) {
    DEBUG((EFI_D_INFO,"%a(),%d,HiiIsConfigHdrMatch not match\n",__FUNCTION__,__LINE__));
    return EFI_NOT_FOUND;
  }
  ConfigRequestHdr = NULL;
  ConfigRequest  = NULL;
  AllocatedRequest = FALSE;

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&mPcieConfigGuid, PCIE_CONFIG_VARIABLE, Private->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
  }
  Status = gHiiConfigRouting->BlockToConfig (
                                gHiiConfigRouting,
                                ConfigRequest,
                                (UINT8 *) &Private->PcieConfigData,
                                sizeof (PCIE_CONFIG_DATA),
                                Results,
                                Progress
                                );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
    ConfigRequest = NULL;
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  }
  else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

/**
This function processes the results of changes in configuration.


@param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
@param Configuration   A null-terminated Unicode string in <ConfigResp> format.
@param Progress        A pointer to a string filled in with the offset of the most
                       recent '&' before the first failing name/value pair (or the
                       beginning of the string if the failure is in the first
                       name/value pair) or the terminating NULL if all was successful.

@retval  EFI_SUCCESS			The Results is processed successfully.
@retval  EFI_INVALID_PARAMETER	Configuration is NULL.
@retval  EFI_NOT_FOUND			Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
PcieConfigRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            Buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  PCIE_CONFIG_PRIVATE_DATA         *Private;

  if (Configuration == NULL || Progress == NULL) {
      return EFI_INVALID_PARAMETER;
  }
  Private = PCIE_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&ConfigRouting
                  );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (!HiiIsConfigHdrMatch (Configuration, &mPcieConfigGuid, PCIE_CONFIG_VARIABLE)) {
      return EFI_NOT_FOUND;
  }
  PcieConfigSetupConfig (This, Private);
  Buffsize = sizeof (PCIE_CONFIG_DATA);
  Status = ConfigRouting->ConfigToBlock (
                            ConfigRouting,
                            Configuration,
                            (UINT8 *) &Private->PcieConfigData,
                            &Buffsize,
                            Progress
                            );

  return Status;
}

/**
  This call back function is registered with Device Info infomation formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
PcieConfigCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  PCIE_CONFIG_PRIVATE_DATA  *Private;

  Private = PCIE_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
    if (QuestionId == 0x7101) {
      GetPcieConfigData (Private);
    }
  }
  return EFI_SUCCESS;
}

/**
  This function publish the Phytium PCIE configuration Form.

  @param[in, out]  PrivateData   Points to pcie configuration private data.

  @retval EFI_SUCCESS            HII Form is installed successfully.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource for HII Form installation.
  @retval Others                 Other errors as indicated.

**/
EFI_STATUS
InstallPcieConfigForm (
  IN OUT PCIE_CONFIG_PRIVATE_DATA  *PrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;

  DriverHandle = NULL;
  ConfigAccess = &PrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mPcieConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PrivateData->DriverHandle = DriverHandle;

  //
  // Publish the HII package list
  //
  HiiHandle = HiiAddPackages (
                &gPcieConfigFormSetGuid,
                DriverHandle,
                PcieConfigUiDxeStrings,
                PcieConfigUiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mPcieConfigHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           ConfigAccess,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->HiiHandle = HiiHandle;

  //
  // Init OpCode Handle and Allocate space for creation of Buffer
  //
  mStartOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (mStartOpCodeHandle == NULL) {
    UninstallPcieConfigForm (PrivateData);
    return EFI_OUT_OF_RESOURCES;
  }

  mEndOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (mEndOpCodeHandle == NULL) {
    UninstallPcieConfigForm (PrivateData);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  mStartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                         mStartOpCodeHandle,
                                         &gEfiIfrTianoGuid,
                                         NULL,
                                         sizeof (EFI_IFR_GUID_LABEL)
                                         );
  mStartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  mEndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                       mEndOpCodeHandle,
                                       &gEfiIfrTianoGuid,
                                       NULL,
                                       sizeof (EFI_IFR_GUID_LABEL)
                                       );
  mEndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  mEndLabel->Number       = LABEL_END;

  PcieConfigInit ();

  return EFI_SUCCESS;
}

/**
  This function removes phytium pcie configuration Form.

  @param[in, out]  PrivateData   Points to pcie configuration private data.

**/
VOID
UninstallPcieConfigForm (
  IN OUT PCIE_CONFIG_PRIVATE_DATA    *PrivateData
  )
{
  //
  // Uninstall HII package list
  //
  if (PrivateData->HiiHandle != NULL) {
    HiiRemovePackages (PrivateData->HiiHandle);
    PrivateData->HiiHandle = NULL;
  }

  //
  // Uninstall HII Config Access Protocol
  //
  if (PrivateData->DriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           PrivateData->DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mPcieConfigHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           &PrivateData->ConfigAccess,
           NULL
           );
    PrivateData->DriverHandle = NULL;
  }

  FreePool (PrivateData);

  if (mStartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (mStartOpCodeHandle);
  }

  if (mEndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (mEndOpCodeHandle);
  }
}
