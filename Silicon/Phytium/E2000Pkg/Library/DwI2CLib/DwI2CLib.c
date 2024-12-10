/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/I2CLib.h>

#include "DwI2CLib.h"

/**
  Enable I2C.

  @param[in]    Info    A pointer to I2C_INFO

  @retval    Null.
**/
STATIC
VOID
I2cEnable (
  IN I2C_INFO  *Info
  )
{
  UINT32 Enbl;

  //
  //Enable i2c
  //
  Enbl = MmioRead32 (Info->BusAddress + DW_IC_ENABLE);
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (Info->BusAddress + DW_IC_ENABLE, Enbl);
}

/**
  Disable I2C.

  @param[in]    Info    A pointer to I2C_INFO

  @retval    Null.
**/
STATIC
VOID
I2cDisable (
  IN I2C_INFO  *Info
  )
{
  UINT32 Enbl;

  //
  //Disable i2c
  //
  Enbl = MmioRead32 (Info->BusAddress + DW_IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (Info->BusAddress + DW_IC_ENABLE, Enbl);
}

/**
  Set i2c speed, standard, high or fast. Config the high and low level duration
  according to the speed.

  @param[in]    Info    A pointer to I2C_INFO

  @retval    Null
**/
STATIC
VOID
SetSpeed (
  IN I2C_INFO  *Info
  )
{
  UINT32 Cntl;
  UINT32 Hcnt;
  UINT32 Lcnt;
  UINT32 Enbl;

  //
  //to set speed cltr must be disabled
  //
  Enbl = MmioRead32 (Info->BusAddress + DW_IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (Info->BusAddress + DW_IC_ENABLE, Enbl);

  Cntl = (MmioRead32 (Info->BusAddress + DW_IC_CON) & (~IC_CON_SPD_MSK));

  switch (Info->Speed) {
  case I2C_MAX_SPEED:
    Cntl |= IC_CON_SPD_HS;
    Hcnt = IC_CLK_FREQ / (I2C_MAX_SPEED * 2);
    MmioWrite32 (Info->BusAddress + DW_IC_HS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (Info->BusAddress + DW_IC_HS_SCL_LCNT, Lcnt);
    break;
  case I2C_STANDARD_SPEED:
    Cntl |= IC_CON_SPD_SS;
    Hcnt = IC_CLK_FREQ / (I2C_STANDARD_SPEED * 2);
    MmioWrite32 (Info->BusAddress + DW_IC_SS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (Info->BusAddress + DW_IC_SS_SCL_LCNT, Lcnt);
    break;
  case I2C_FAST_SPEED:
  default:
    Cntl |= IC_CON_SPD_FS;
    Hcnt = IC_CLK_FREQ / (I2C_FAST_SPEED * 2);
    MmioWrite32 (Info->BusAddress + DW_IC_FS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (Info->BusAddress + DW_IC_FS_SCL_LCNT, Lcnt);
    break;
  }

  MmioWrite32 (Info->BusAddress + DW_IC_CON, Cntl);
  //
  //Enable back i2c now speed set
  //
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (Info->BusAddress + DW_IC_ENABLE, Enbl);
}

/**
  Fix and set i2c speed to bus, standard, high or fast.

  @param[in]    Info    A pointer to I2C_INFO

  @retval     EFI_SCEESSS    Success.
**/
EFI_STATUS
I2cSetBusSpeed (
  IN I2C_INFO  *Info
  )
{
  if (Info->Speed >= I2C_MAX_SPEED) {
    Info->Speed = I2C_MAX_SPEED;
    SetSpeed (Info);
  } else if (Info->Speed >= I2C_FAST_SPEED) {
    Info->Speed = I2C_FAST_SPEED;
    SetSpeed (Info);
  } else {
    Info->Speed = I2C_STANDARD_SPEED;
    SetSpeed(Info);
  }

  return EFI_SUCCESS;
}

/*
  Get i2c bus speed, standard, fast or high.

  @param[in]    Info    A pointer to I2C_INFO

  @retval    I2C_MaX_SPEED - 3.4M
             I2C_STANDARD_SPEED - 100K
             I2C_FAST_SPEED - 100K
 */
UINT32
I2cGetBusSpeed (
  IN I2C_INFO  *Info
  )
{
  UINT32 Cntl;

  Cntl = (MmioRead32 (Info->BusAddress + DW_IC_CON) & IC_CON_SPD_MSK);

  if (Cntl == IC_CON_SPD_HS) {
    return I2C_MAX_SPEED;
  } else if (Cntl == IC_CON_SPD_FS) {
    return I2C_FAST_SPEED;
  } else if (Cntl == IC_CON_SPD_SS) {
    return I2C_STANDARD_SPEED;
  }

  return 0;
}

/*
  Set i2c slave device address to controller.

  @param[in]    Info    A pointer to I2C_INFO

  @retval  Null.
 */
STATIC
VOID
I2cSetaddress (
  IN I2C_INFO  *Info
  )
{
  MmioWrite32 (Info->BusAddress + DW_IC_TAR, Info->SlaveAddress);
}

/*
  Flushes the i2c RX FIFO.

  @param[in]    Info    A pointer to I2C_INFO

  @retval  Null.
 */
STATIC
VOID
I2cFlushRxfifo (
  IN I2C_INFO  *Info
  )
{
  while (MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_RFNE) {
    MmioRead32 (Info->BusAddress + DW_IC_DATA_CMD);
  }
}

/*
  Waits for bus busy.

  @param[in]    Info    A pointer to I2C_INFO

  @retval  Null.
 */
STATIC
EFI_STATUS
I2cWaitForBb (
  IN I2C_INFO  *Info
  )
{
  UINT32  Timeout;

  Timeout = I2C_BYTE_TO_BB;

  while ((MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_MA) ||
         !(MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_TFE)) {
    //
    //Evaluate timeout
    //
    MicroSecondDelay (1000);
    if (Timeout-- == 0) {
      /*DEBUG ((DEBUG_ERROR, "Timed out. i2c i2c_wait_for_bb Failed\n"));*/
      return EFI_TIMEOUT;
    }
  }

  return EFI_SUCCESS;
}

/**
  Check parameters for I2cRead and I2cWrite.

  @param[in]  Addr    Slave address.
  @param[in]  Alen    Slave address length.
  @param[in]  Buffer  Data to write.
  @param[in]  Len     Length of data to write.

  @retval     0       Success.
              Other   Failed.
**/
STATIC
UINT32
CheckParams (
  IN UINT32 Addr,
  IN UINT32 Alen,
  IN UINT8  *Buffer,
  IN UINT32 Len)
{
  if (Buffer == NULL) {
    //DEBUG ((DEBUG_ERROR, "Buffer is invalid\n"));
    return 1;
  }

  if (Alen > 1) {
    //DEBUG ((DEBUG_ERROR, "addr len %d not supported\n", alen));
    return 1;
  }

  if (Addr + Len > 256) {
    //DEBUG ((DEBUG_ERROR, "address out of range\n"));
    return 1;
  }

  return 0;
}

/**
  Transfer init.

  @param[in] Info  A pointer to I2C_INFO.
  @param[in] Addr  Address to read from.
  @param[in] Alen  Address length.

  @retval    0     Success.
**/
STATIC
UINT32
I2cXferInit (
  IN I2C_INFO  *Info,
  IN UINT32    Addr,
  IN UINT32    Alen)
{
  UINT8   Index;

  if (I2cWaitForBb (Info)) {
    return 1;
  }

  I2cDisable (Info);
  I2cSetaddress (Info);
  I2cEnable (Info);
  /// REG addr
  for (Index = Alen; Index > 0; Index--){
    //DEBUG((DEBUG_INFO,"the number i is %d \n",i));
    Addr = (Addr >> (Index-1) * 8) & 0xFF;
    MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, Addr);
  }
  ///
  return 0;
}

/**
  I2c transfer finish.

  @param[in] Info  A pointer to I2C_INFO.

  @retval  Null.
**/
STATIC
UINT32
I2cXferFinish (
  I2C_INFO  *Info
  )
{
  UINTN  Timeout;

  Timeout = I2C_STOPDET_TO;
  while (1) {
    if ((MmioRead32 (Info->BusAddress + DW_IC_RAW_INTR_STAT) & IC_STOP_DET)) {
      MmioRead32 (Info->BusAddress + DW_IC_CLR_STOP_DET);
      break;
    } else {
      MicroSecondDelay (1000);
      if (Timeout-- == 0) {
        //DEBUG ((DEBUG_ERROR, "Timed out. i2c i2c_xfer_finish Failed\n"));
        break;
      }
    }
  }

  if (I2cWaitForBb (Info)) {
    //DEBUG ((DEBUG_ERROR, "Timed out waiting for bus\n"));
    return 1;
  }

  I2cFlushRxfifo (Info);

  /* Wait for read/write operation to complete on actual memory */
  MicroSecondDelay (10);

  return 0;
}

/**
  Rtc i2c Read.

  @param[in]     Info    A pointer to I2C_INFO.
  @param[in]     Addr    Address to read from
  @param[in]     Alen    Address length.
  @param[in,out] Buffer  Buffer for read data
  @param[in]     Len     Number of bytes to be read

  @retval        0       Success.
                 Other   Failed.
 */
UINT32
I2cRead (
  IN I2C_INFO  *Info,
  IN UINT64    Addr,
  IN UINT32    Alen,
  IN UINT8     *Buffer,
  IN UINT32    Len
  )
{
  UINT32 Timeout;

  if (I2cXferInit(Info, Addr , Alen)) {
    return 1;
  }

  Timeout = I2C_BYTE_TO;
  while (Len) {
    if (Len == 1) {
      MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, IC_CMD | IC_STOP);
    } else {
      MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, IC_CMD);
    }
    if (MmioRead32(Info->BusAddress + DW_IC_STATUS) & IC_STATUS_RFNE) {

      *Buffer  = (UINT8) MmioRead32 (Info->BusAddress + DW_IC_DATA_CMD);
      Len--;
      Buffer++;
      Timeout = I2C_BYTE_TO;
    } else {
      MicroSecondDelay(1000);
      //DEBUG ((DEBUG_ERROR, "need time delay-------------\n"));
      if (Timeout-- == 0) {
        //DEBUG ((DEBUG_ERROR, "Timed out. i2c read Failed\n"));
        return 1;
      }
      if (MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_RFNE) {
           //DEBUG ((DEBUG_ERROR, "FIFO HAVE DATA 2  -------------\n"));
           *Buffer = (UINT8) MmioRead32 (Info->BusAddress + DW_IC_DATA_CMD);
          Len--;
          Buffer ++;
          Timeout = I2C_BYTE_TO;
      }
    }
  }
  return I2cXferFinish (Info);
}

VOID
SpdSetpage (
  IN I2C_INFO  *Info,
  IN UINT32    PageNum
  )
{
  I2cEnable (Info);
  if (PageNum == 0x1) {
    MmioWrite32 (Info->BusAddress + DW_IC_TAR, 0x37);
  } else {
    MmioWrite32 (Info->BusAddress + DW_IC_TAR, 0x36);
  }

  MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, 0x0);
  MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, 0x0|IC_STOP);

  /*******************For NoAck Response************************/

  while (MmioRead32 (Info->BusAddress + DW_IC_STATUS) != 0x6);

  if (MmioRead32 (Info->BusAddress+DW_IC_INTR_STAT) & 0x40) {
    volatile UINT32 Index = 0;
    for (Index = 0; Index < 100; Index++);
    MmioRead32(Info->BusAddress + DW_IC_INTR_STAT);
    MmioRead32(Info->BusAddress + DW_IC_STATUS);
    MmioRead32(Info->BusAddress + DW_IC_CLR_TX_ABRT);
    MmioRead32(Info->BusAddress + DW_IC_INTR_STAT);
    MmioRead32(Info->BusAddress + DW_IC_STATUS);
  }

}

/**
  Rtc i2c write data.

  @param[in] Info     A pointer to I2C_INFO
  @param[in] Addr     Address to read from
  @param[in] Alen     Address length.
  @param[in] Buffer   Buffer for read data
  @param[in] Len      Number of bytes to be read

  @retval    0        Success.
             Other    Failed.
**/
UINT32
I2cWrite (
  IN I2C_INFO  *Info,
  IN UINT64    Addr,
  IN UINT32    Alen,
  IN UINT8     *Buffer,
  IN UINT32    Len
  )
{
  UINT32  Nb;
  UINT32  Timeout;
  UINT32  Cnt;

  Nb = Len;
  Cnt = 0;

  if (CheckParams (Addr, Alen, Buffer, Len))
    return 1;

  if (I2cXferInit (Info, Addr, Alen)) {
    return 1;
  }

  Timeout = Nb * I2C_BYTE_TO;
  while (Len) {
    if (MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_TFNF) {
      if (Len > 1){  //not last one
        if(((Cnt + 1) % 8 == 0)) {
          //DEBUG((DEBUG_ERROR, " send stop ,num is %d.len is %d \n",cnt,len));
          MmioWrite32(Info->BusAddress + DW_IC_DATA_CMD, *Buffer | IC_STOP);  //send stop
          MicroSecondDelay (100000);
        } else if (Cnt !=0 && Cnt % 8 == 0) {
          I2cXferInit (Info, Addr + Cnt, Alen);
          if (MmioRead32(Info->BusAddress + DW_IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, *Buffer);
          }
        } else {
          MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, *Buffer);
        }
      } else {  //last one
        if (Cnt !=0 && Cnt % 8 == 0) {
          I2cXferInit (Info, Addr + Cnt, Alen);
          if (MmioRead32 (Info->BusAddress + DW_IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, *Buffer| IC_STOP);
          }
        } else {
          //DEBUG((DEBUG_ERROR, " last one :send stop ,num is %d.len is %d \n",cnt,len));
          MmioWrite32 (Info->BusAddress + DW_IC_DATA_CMD, *Buffer| IC_STOP);
        }
      }
      Buffer++;
      Cnt++;
      Len--;
      Timeout = Nb * I2C_BYTE_TO;
    } else {
      MicroSecondDelay (1000);
      if (Timeout-- == 0) {
        //DEBUG ((DEBUG_ERROR, "Timed out. i2c write Failed\n"));
        return 1;
      }
    }
  }
  return I2cXferFinish (Info);
}

/**
  I2c controller init.

  @param[in] Info     A pointer to I2C_INFO

  @retval     Null.
**/
VOID
I2cInit (
  IN I2C_INFO  *Info
  )
{
  /* Disable i2c */
  I2cDisable (Info);

  MmioWrite32 (Info->BusAddress + DW_IC_CON, IC_CON_SD | IC_CON_RE | IC_CON_MM);

  MmioWrite32 (Info->BusAddress + DW_IC_RX_TL, IC_RX_TL);
  MmioWrite32 (Info->BusAddress + DW_IC_TX_TL, IC_TX_TL);

  I2cSetBusSpeed (Info);

  MmioWrite32 (Info->BusAddress + DW_IC_INTR_MASK, IC_STOP_DET);
  MmioWrite32 (Info->BusAddress + DW_IC_SAR, Info->SlaveAddress);

  I2cEnable (Info);
}

/**
  Config I2c as slave mode.

  @param[in] Info     A pointer to I2C_INFO

  @retval    EFI_SUCCESS    Success.
**/
EFI_STATUS
EnableI2cAsSlaver (
  IN I2C_INFO  *Info
  )
{
  I2cDisable (Info);
  //
  //set slave address
  //
  MmioWrite32 (Info->BusAddress + DW_IC_SAR, Info->SlaveAddress);
  //
  //config slave mode
  //
  MmioWrite32 (Info->BusAddress + DW_IC_CON, 0);
  //
  //set speed
  //
  SetSpeed (Info);
  I2cEnable (Info);

  return EFI_SUCCESS;
}
