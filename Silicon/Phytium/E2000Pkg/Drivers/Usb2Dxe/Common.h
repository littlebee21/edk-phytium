/** @file
Phytium usb2 general function description header file, such as register operation.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef  PHYTIUM_USB_COMMON_H_
#define  PHYTIUM_USB_COMMON_H_

//#define REG_DEBUG

/**
  Usb2 register 32-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT32
PhytiumUsbRead32 (
  IN UINT32  *Address
  );

/**
  Usb2 register 16-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT16
PhytiumUsbRead16 (
  IN UINT16  *Address
  );

/**
  Usb2 register 8-bit read.

  @param[in]  Address    Address to read.

  @retval     Value      Read value.
**/
UINT8
PhytiumUsbRead8 (
  IN UINT8  *Address
  );

/**
  Usb2 register 32-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite32 (
  IN UINT32  *Address,
  IN UINT32  Value
  );

/**
  Usb2 register 16-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite16 (
  IN UINT16  *Address,
  IN UINT16  Value
  );

/**
  Usb2 register 8-bit write.

  @param[in]  Address    Address to write.
  @param[in]  Value      Write value.

**/
VOID
PhytiumUsbWrite8 (
  IN UINT8   *Address,
  IN UINT8   Value
  );

#endif
