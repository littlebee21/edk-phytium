/**
  Some definitions of SCMI protocol and function interface of ScmiLib.

  Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  _SCMI_LIB_H_
#define  _SCMI_LIB_H_

//
//SCMI protocol error status enum
//
enum SCMI_ERROR {
    SCMI_SUCCESS            =  0,
    SCMI_NOT_SUPPORTED      = -1,
    SCMI_INVALID_PARAMETERS = -2,
    SCMI_DENIED             = -3,
    SCMI_NOT_FOUND          = -4,
    SCMI_OUT_OF_RANGE       = -5,
    SCMI_BUSY               = -6,
    SCMI_COMMS_ERROR        = -7,
    SCMI_GENERIC_ERROR      = -8,
    SCMI_HARDWARE_ERROR     = -9,
    SCMI_PROTOCOL_ERROR     = -10,
};

//
//scmi protocol id
//
#define SCMI_PROTOCOL_ID_BASE           0x10
#define SCMI_PROTOCOL_ID_PD             0x11
#define SCMI_PROTOCOL_ID_SYS_POWER      0x12
#define SCMI_PROTOCOL_ID_PERF           0x13
#define SCMI_PROTOCOL_ID_CLOCK          0x14
#define SCMI_PROTOCOL_ID_SENSOR         0x15
#define SCMI_PROTOCOL_ID_RESET          0x16
#define SCMI_PROTOCOL_ID_FT_DEFINED     0x81
#define SCMI_PROTOCOL_ID_OLD_SCPI       0x80


//
//scmi common message id
//
#define SCMI_PROTOCOL_VERSION             0x0
#define SCMI_PROTOCOL_ATTRIBUTES          0x001
#define SCMI_PROTOCOL_MESSAGE_ATTRIBUTES  0x002

//
//scmi base message id
//
#define SCMI_BASE_DISCOVER_VENDOR                 0x03
#define SCMI_BASE_DISCOVER_SUB_VENDOR             0x04
#define SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION 0x05
#define SCMI_BASE_DISCOVER_LIST_PROTOCOLS         0x06
#define SCMI_BASE_DISCOVER_AGENT                  0x07
#define SCMI_BASE_NOTIFY_ERRORS                   0x08

//
//scmi power domain message id
//
#define SCMI_PD_POWER_DOMAIN_ATTRIBUTES         0x03
#define SCMI_PD_POWER_STATE_SET                 0x04
#define SCMI_PD_POWER_STATE_GET                 0x05
#define SCMI_PD_POWER_STATE_NOTIFY              0x06

//
//scmi systerm power message id
//
#define SCMI_SYS_POWER_STATE_SET          0x03
#define SCMI_SYS_POWER_STATE_GET          0x04
#define SCMI_SYS_POWER_STATE_NOTIFY       0x05

//
//scmi performance domain
//
#define   SCMI_PERF_DOMAIN_ATTRIBUTES         0x03
#define   SCMI_PERF_DESCRIBE_LEVELS           0x04
#define   SCMI_PERF_LIMITS_SET                0x05
#define   SCMI_PERF_LIMITS_GET                0x06
#define   SCMI_PERF_LEVEL_SET                 0x07
#define   SCMI_PERF_LEVEL_GET                 0x08
#define   SCMI_PERF_NOTIFY_LIMITS             0x09
#define   SCMI_PERF_NOTIFY_LEVEL              0x0A

//
//scmi clock message id
//
#define   SCMI_CLOCK_ATTRIBUTES               0x03
#define   SCMI_CLOCK_DESCRIBE_RATES           0x04
#define   SCMI_CLOCK_RATE_SET                 0x05
#define   SCMI_CLOCK_RATE_GET                 0x06
#define   SCMI_CLOCK_CONFIG_SET               0x07

//
//scmi sensor message id
//
#define   SCMI_SENSOR_DESCRIPTION_GET         0x03
#define   SCMI_SENSOR_CONFIG_SET              0x04
#define   SCMI_SENSOR_TRIP_POINT_SET          0x05
#define   SCMI_SENSOR_READING_GET             0x06

//
//scmi sensor message id
//
#define   SCMI_RESET_ATTRIBUTES           0x03
#define   SCMI_RESET_SET                  0x04

//
//scmi ft_defined mwssage id
//
#define   PLLCONFIG_PRO_100               0x00
#define   WFI_ENABLE_101                  0x01
#define   PERIEQ_PWRUP_PWRDOWN_102        0x02
#define   CORE_POWER_STATE_GET_103        0x03
#define   CORE_POWER_STATE_NOTIFY_104     0x04
#define   S3_S4_S5_CTRL_105               0x05
#define   READ_REGISTER_EFUSE_106         0x06
#define   WRITE_REGISTER_EFUSE_107        0x07
#define   CPPC_ENABLE_SWITCH              0x09
#define   PMBUS_CONFIG_FUNC               0x12

typedef struct _SCMI_ADDRESS_BASE {
  UINT64  MhuConfigBase;
  UINT64  MhuBase;
  UINT64  ShareMemoryBase;
} SCMI_ADDRESS_BASE;

typedef struct _PMBUS_CONFIG {
  UINT8   SlaveAddr;
  UINT8   Reserve[3];
  UINT32  Mode;
  UINT32  Vlsb;
  UINT32  Ilsb;
  UINT32  Plsb;
} PMBUS_CONFIG;

/**
  Execute the SCMI command. Input protocol ID, message ID, the length and
  parameters of payload, return the length and results of payload.
  Execute the SCMI command. Input protocol ID, message ID, the length and
  parameters of payload, return the length and results of payload. The
  parameter ReturnValues must not be NULL, and large enough.

  @param[in]   Base            A poniter to SCMI_ADDRESS_BASE.
  @param[in]   ProtocolId      Protocol ID.
  @param[in]   MessageId       Message ID.
  @param[in]   LenIn           Number of input payload parameters.
  @param[in]   ParIn           A pointer of input payload.
  @param[out]  LenOut          A pointer of output payload parameter number.
  @param[out]  ReturnValues    A pointer of output payload buffer.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiCommandExecute (
  IN   SCMI_ADDRESS_BASE  *Base,
  IN   UINT32             ProtocolId,
  IN   UINT32             MessageId,
  IN   UINT32             LenIn,
  IN   UINT32             *ParIn,
  OUT  UINT32             *LenOut,
  OUT  UINT32             *ReturnValues
  );

/**
  Enable or Disable cppc freq function.

  @param[in]    Base        A poniter to SCMI_ADDRESS_BASE.
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
  );

/**
  Power control at S3 S4 or S5. Phytium SCMI private protocol :
  Protocol Id is 0x81, message ID is 0x5.

  @param[in]    Base        A poniter to SCMI_ADDRESS_BASE.
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
  );

/**
  PMBus config. Phytium SCMI private protocol : Protocol Id is 0x81, message ID
  is 0x12.

  @param[in]    Base        A poniter to SCMI_ADDRESS_BASE.
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
  );

#endif
