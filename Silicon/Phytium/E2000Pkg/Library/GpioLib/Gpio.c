/**
  Phytium E2000 gpio controller library.

  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PhytiumGpioLib.h>

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
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - 5\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - 15\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Dir != GPIO_DIRECTION_IN) && (Dir != GPIO_DIRECTION_OUT)) {
    DEBUG ((DEBUG_ERROR, "The direction to set is not in and out\n"));
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Index) + PHYTIUM_GPIO_SWPORTA_DDR);
  if (Dir == GPIO_DIRECTION_OUT) {
    Value |= (0x1 << Port);
  } else {
    Value &= ~(0x1 << Port);
  }
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Index) + PHYTIUM_GPIO_SWPORTA_DDR, Value);

  return EFI_SUCCESS;
}

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
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - 5\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - 15\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Level != GPIO_LOW_LEVEL) && (Level != GPIO_HIGH_LEVEL)) {
    DEBUG ((DEBUG_ERROR, "Level to set is not high and low\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Index) + PHYTIUM_GPIO_SWPORTA_DR);
  if (Level == GPIO_HIGH_LEVEL) {
    Value |= (0x1 << Port);
  } else {
    Value &= ~(0x1 << Port);
  }
  MmioWrite32 (PHYTIUM_GPIO_N_BASE(Index) + PHYTIUM_GPIO_SWPORTA_DR, Value);

  return EFI_SUCCESS;
}

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
  )
{
  UINT32  Value;

  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - 5\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - 15\n"));
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (PHYTIUM_GPIO_N_BASE(Index) + PHYTIUM_GPIO_EXT_PORTA);

  *Level = ((Value & BIT(Port)) >> Port);

  return EFI_SUCCESS;
}

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
  )
{
  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - 5\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - 15\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Level != GPIO_LOW_LEVEL) && (Level != GPIO_HIGH_LEVEL)) {
    DEBUG ((DEBUG_ERROR, "Level to set is not high and low\n"));
    return EFI_INVALID_PARAMETER;
  }

  SetGpioDirect (Index, Port, GPIO_DIRECTION_OUT);
  SetGpioOutputLevel (Index, Port, Level);

  return EFI_SUCCESS;
}

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
  )
{
  if (Index >= GPIO_MAX_INDEX_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Index is not in the range 0 - 5\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Port >= GPIO_MAX_PORT_NUM) {
    DEBUG ((DEBUG_ERROR, "Gpio Port is not in the range 0 - 15\n"));
    return EFI_INVALID_PARAMETER;
  }

  SetGpioDirect (Index, Port, GPIO_DIRECTION_IN);
  GetGpioInputLevel (Index, Port, Level);

  return EFI_SUCCESS;
}