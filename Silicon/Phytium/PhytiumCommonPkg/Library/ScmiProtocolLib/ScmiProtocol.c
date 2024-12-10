/**
  Realize sending and receiving SCMI messages at the protocol layer.

  Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ScmiLib.h>

/**
  Enable or Disable cppc freq function. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x9.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      1 - Enable, 0 - Disable.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiEnableCppc (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             Enable,
  OUT INT32              *ScmiStatus
  )
{
  UINT32  Payload[32];
  UINT32  Len;
  EFI_STATUS  Status;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             CPPC_ENABLE_SWITCH,
             1,
             &Enable,
             &Len,
             Payload
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = Payload[0];

  return Status;
}

/**
  Power control at S3 S4 or S5. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x5.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Enable      Power state, 0 - S0, 1 - S3, 2- S4, 3 - S5.
  @param[in]    WakeEnable  Wake enable bit mask. 1 - enable, 0 - disable.
                            bit 0 - USB0,
                            bit 1 - USB1,
                            bit 2 - MAC 0,
                            bit 3 - MAC 1,
                            bit 4 - MAC 2,
                            bit 5 - MAC 3,
                            bit 6 - GPIO0
                            bit 7 - keyboard,
                            bit 8 - primary core,
                            bit 31:9 - reserved.
  @param[in]    WakeGpio    Pad number of GPIO that support wakeup.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiPowerStateS3S4S5 (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  UINT32             PowerState,
  IN  UINT32             WakeEnable,
  IN  UINT32             WakeGpio,
  OUT INT32              *ScmiStatus
  )
{
  UINT32  PayloadIn[3];
  UINT32  PayloadOut[32];
  UINT32  Len;
  EFI_STATUS  Status;

  PayloadIn[0] = PowerState;
  PayloadIn[1] = WakeEnable;
  PayloadIn[2] = WakeGpio;

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             S3_S4_S5_CTRL_105,
             3,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}

/**
  PMBus config. Phytium SCMI private protocol : Protocol Id is 0x81, message ID
  is 0x12.

  @oaram[in]    Base        A pointer to SCMI_ADDRESS_BASE.
  @param[in]    Config      A pointer to PMBUS_CONFIG.
  @param[out]   ScmiStatus  A pointer to enum SCMI_ERROR.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiPmBusConfig (
  IN  SCMI_ADDRESS_BASE  *Base,
  IN  PMBUS_CONFIG       *Config,
  OUT INT32              *ScmiStatus
  )
{
  UINT32      PayloadIn[5];
  UINT32      PayloadOut[32];
  UINT32      Len;
  EFI_STATUS  Status;

  CopyMem (PayloadIn, Config, sizeof (PMBUS_CONFIG));

  Status = ScmiCommandExecute (
             Base,
             SCMI_PROTOCOL_ID_FT_DEFINED,
             PMBUS_CONFIG_FUNC,
             5,
             PayloadIn,
             &Len,
             PayloadOut
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ScmiStatus = PayloadOut[0];

  return Status;
}
