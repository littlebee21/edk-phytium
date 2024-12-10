/** @file
Phytium usb2 general function description, such as register operation.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>

#include "Common.h"

/**
  Usb2 register 32-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT32
PhytiumUsbRead32 (
  IN UINT32  *Address
  )
{
  UINT32  Value;

  Value = MmioRead32 ((UINT64) Address);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif

  return Value;
}

/**
  Usb2 register 16-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT16
PhytiumUsbRead16 (
  IN UINT16  *Address
  )
{
  UINT16  Value;

  Value = MmioRead16 ((UINT64) Address);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif

  return Value;
}

/**
  Usb2 register 8-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT8
PhytiumUsbRead8 (
  IN UINT8  *Address
  )
{
  UINT16  Value;

  Value = MmioRead8 ((UINT64) Address);
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif

  return Value;
}

/**
  Usb2 register 32-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite32 (
  IN UINT32  *Address,
  IN UINT32  Value
  )
{
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif
  MmioWrite32 ((UINT64) Address, Value);
}

/**
  Usb2 register 16-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite16 (
  IN UINT16  *Address,
  IN UINT16  Value
  )
{
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif
  MmioWrite16 ((UINT64) Address, Value);
}

/**
  Usb2 register 8-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite8 (
  IN UINT8  *Address,
  IN UINT8   Value
  )
{
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, " %a, Address : 0x%08x, Value : 0x%08x\n",
            __FUNCTION__, (UINT64) Address, Value));
#endif
  MmioWrite8 ((UINT64) Address, Value);
}
