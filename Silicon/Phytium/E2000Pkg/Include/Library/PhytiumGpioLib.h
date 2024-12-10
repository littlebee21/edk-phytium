/**
  Some devinitaions of Phytium E2000 GPIO controller.

  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  _PHYTIUM_GPIO_H_
#define  _PHYTIUM_GPIO_H_

#define  PHYTIUM_GPIO_REGISTER_BASE      (0x28034000)
#define  PHYTIUM_GPIO_REGISTER_OFFSET    (0x1000)
#define  PHYTIUM_GPIO_N_BASE(Index)      (PHYTIUM_GPIO_REGISTER_BASE + Index * PHYTIUM_GPIO_REGISTER_OFFSET)

#define  GPIO_MAX_INDEX_NUM              6
#define  GPIO_MAX_PORT_NUM               16

#define  PHYTIUM_GPIO_SWPORTA_DR                 (0x0)
#define  PHYTIUM_GPIO_SWPORTA_DDR                (0x4)
#define  PHYTIUM_GPIO_EXT_PORTA                  (0x8)
#define  PHYTIUM_GPIO_INTEN                      (0x18)
#define  PHYTIUM_GPIO_INTMASK                    (0x1C)
#define  PHYTIUM_GPIO_INTTYPE_LEVEL              (0x20)
#define  PHYTIUM_GPIO_INT_POLARITY               (0x24)
#define  PHYTIUM_GPIO_INTSTATUS                  (0x28)
#define  PHYTIUM_GPIO_RAW_INTSTATUS              (0x2C)
#define  PHYTIUM_GPIO_LS_SYNC                    (0x30)
#define  PHYTIUM_GPIO_DEBOUNCE                   (0x34)
#define  PHYTIUM_GPIO_PORTA_EOI                  (0x38)

#define  GPIO_DIRECTION_IN               0
#define  GPIO_DIRECTION_OUT              1

#define  GPIO_INTERRUPT_LEVEL            0
#define  GPIO_INTERRUPT_EDGE             1

#define  GPIO_INTERRUPT_FALLING_LOW      0
#define  GPIO_INTERRUPT_RISING_HIGH      1

#define  GPIO_LOW_LEVEL                  0
#define  GPIO_HIGH_LEVEL                 1

#define  BIT(N)                          (0x1 << N)


/**
  Set a specify gpio pin direction, input or output.

  @param[in]    Index    Gpio group index, 0-5.
  @param[in]    Port     Gpio port number in a group, 0-15.
  @param[in]    Dir      Direction to set. 0-input, 1-output.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is not in the range 0 - 5.
                                       2.Gpio Port is not in the range 0 - 15.
                                       3.The direction to set is not in and out.
**/
EFI_STATUS
SetGpioDirect (
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Dir
  );

/**
  When the port is output state, set a specify gpio output level, high or low.

  @param[in]    Index    Gpio group index, 0-5.
  @param[in]    Port     Gpio port number in a group, 0-15.
  @param[in]    Level    Output level to set, 0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is not in the range 0 - 5.
                                       2.Gpio Port is not in the range 0 - 15.
                                       3.Level to set is not high and low.
**/
EFI_STATUS
SetGpioOutputLevel (
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Level
  );

/**
  When the port is input state, get a specify gpio input level, high or low.

  @param[in]    Index    Gpio group index, 0-5.
  @param[in]    Port     Gpio port number in a group, 0-15.
  @param[out]   Level    0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following two errors:
                                       1.Gpio Index is not in the range 0 - 5.
                                       2.Gpio Port is not in the range 0 - 15.
**/
EFI_STATUS
GetGpioInputLevel (
  IN  UINT8  Index,
  IN  UINT8  Port,
  OUT UINT8  *Level
  );

/**
  In a progress, set a specify gpio output level, high or low.
  1.Set the port direction to output state.
  2.Set the port level to high or low.

  @param[in]    Index    Gpio group index, 0-5.
  @param[in]    Port     Gpio port number in a group, 0-15.
  @param[in]    Level    Output level to set, 0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following three errors:
                                       1.Gpio Index is not in the range 0 - 5.
                                       2.Gpio Port is not in the range 0 - 15.
                                       3.Level to set is not high and low.
**/
EFI_STATUS
DirectSetGpioOutPutLevel (
  IN UINT8  Index,
  IN UINT8  Port,
  IN UINT8  Level
  );

/**
  In a progress, get a specify gpio input level, high or low.
  1.Set the port direction to input state.
  2.Get the port level.

  @param[in]    Index    Gpio group index, 0-5.
  @param[in]    Port     Gpio port number in a group, 0-15.
  @param[out]   Level    0-low, 1-high.

  @retval       EFI_SUCCESS            Success.
                EFI_INVALID_PARAMETER  One of the following two errors:
                                       1.Gpio Index is not in the range 0 - 5.
                                       2.Gpio Port is not in the range 0 - 15.
**/
EFI_STATUS
DirectGetGpioInputLevel (
  IN UINT8   Index,
  IN UINT8   Port,
  OUT UINT8  *Level
  );

#endif