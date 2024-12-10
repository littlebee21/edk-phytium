/** @file
A Library to support all BMC access via IPMI command during PEI Phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/PeiServicesLib.h>
#include <PiPei.h>
#include <Ppi/IpmiTransportPpi.h>

/**
  Initialize the global varible with the pointer of IpmiTransport Protocol.

  @return  EFI_SUCCESS    Always return success.
  @retval  Other          Failure.

**/
EFI_STATUS
InitializeIpmiBase (
  VOID
  )
{
  EFI_STATUS                     Status;
  PEI_IPMI_TRANSPORT_PPI         *IpmiTransport;

  Status = PeiServicesLocatePpi (
                      &gPeiIpmiTransportPpiGuid,
                      0,
                      NULL,
                      (VOID **) &IpmiTransport
                      );
  return Status;
}

/**
  Routine to send commands to BMC.

  @param[in]      NetFunction       Net function of the command.
  @param[in]      Command           IPMI Command.
  @param[in]      CommandData       Command Data.
  @param[in]      CommandDataSize   Size of CommandData.
  @param[out]     ResponseData      Response Data.
  @param[in,out]  ResponseDataSize  Response Data Size.

  @return EFI_NOT_AVAILABLE_YET   IpmiTransport Protocol is not installed yet.
  @retval  Other                  Failure.

**/
EFI_STATUS
IpmiSubmitCommand (
  IN UINT8        NetFunction,
  IN UINT8        Command,
  IN UINT8        *CommandData,
  IN UINT8        CommandDataSize,
  OUT UINT8       *ResponseData,
  IN OUT UINT8    *ResponseDataSize
  )
{
  EFI_STATUS                      Status;
  PEI_IPMI_TRANSPORT_PPI          *IpmiTransport;

  Status = PeiServicesLocatePpi (
                      &gPeiIpmiTransportPpiGuid,
                      0,
                      NULL,
                      (VOID **) &IpmiTransport
                      );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = IpmiTransport->IpmiSubmitCommand (
                            IpmiTransport,
                            NetFunction,
                            0,
                            Command,
                            CommandData,
                            CommandDataSize,
                            ResponseData,
                            ResponseDataSize
                            );
  return Status;
}


