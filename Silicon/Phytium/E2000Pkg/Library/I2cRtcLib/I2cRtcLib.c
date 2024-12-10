/** @file
**/
#include <I2cRtcLib.h>

STATIC EFI_EVENT     mRtcVirtualAddrChangeEvent;
STATIC UINTN         mRtcBase;
STATIC UINT32        mSlaveaddress;
STATIC UINT32        mSpeed;

/*
  Enable i2c controller master function.

  @retval  Null
 */
STATIC
VOID
Rtcenable(
  VOID
  )
{
  UINT32 Enbl;

  //
  //Enable i2c
  //
  Enbl = MmioRead32 (mRtcBase + DW_IC_ENABLE);
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (mRtcBase + DW_IC_ENABLE, Enbl);
}

/*
  Disable i2c controller master function.
 */
STATIC
VOID
Rtcdisable(
  VOID
  )
{
  UINT32 Enbl;

  //
  //Disable i2c
  //
  Enbl = MmioRead32 (mRtcBase + DW_IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (mRtcBase + DW_IC_ENABLE, Enbl);
}

/*
  Set i2c speed, standard, high or fast. Config the high and low level duration
  according to the speed.

  @param[in]  I2cSpd    I2C_MaX_SPEED - 3.4M
                        I2C_STANDARD_SPEED - 100K
                        I2C_FAST_SPEED - 100K

  @retval    Null
*/
STATIC
VOID
RtcSetSpeed (
  IN UINT32 I2cSpd
  )
{
  UINT32 Cntl;
  UINT32 Hcnt;
  UINT32 Lcnt;
  UINT32 Enbl;

  //
  //to set speed cltr must be disabled
  //
  Enbl = MmioRead32 (mRtcBase + DW_IC_ENABLE);
  Enbl &= ~IC_ENABLE_0B;
  MmioWrite32 (mRtcBase + DW_IC_ENABLE, Enbl);

  Cntl = (MmioRead32 (mRtcBase + DW_IC_CON) & (~IC_CON_SPD_MSK));
  switch (I2cSpd) {
  case I2C_MAX_SPEED:
    Cntl |= IC_CON_SPD_HS;
    Hcnt = IC_CLK_FREQ / (I2C_MAX_SPEED * 2);
    MmioWrite32 (mRtcBase + DW_IC_HS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (mRtcBase + DW_IC_HS_SCL_LCNT, Lcnt);
    break;
  case I2C_STANDARD_SPEED:
    Cntl |= IC_CON_SPD_SS;
    Hcnt = IC_CLK_FREQ / (I2C_STANDARD_SPEED * 2);
    MmioWrite32 (mRtcBase + DW_IC_SS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (mRtcBase + DW_IC_SS_SCL_LCNT, Lcnt);
    break;
  case I2C_FAST_SPEED:
  default:
    Cntl |= IC_CON_SPD_FS;
    Hcnt = IC_CLK_FREQ / (I2C_FAST_SPEED * 2);
    MmioWrite32 (mRtcBase + DW_IC_FS_SCL_HCNT, Hcnt);
    Lcnt = Hcnt;
    MmioWrite32 (mRtcBase + DW_IC_FS_SCL_LCNT, Lcnt);
    break;
  }
  //DEBUG ((DEBUG_INFO, "Speed : %d, Low : %d, High : %d\n", I2cSpd, Lcnt, Hcnt));
  MmioWrite32 (mRtcBase + DW_IC_CON, Cntl);
  //
  //Enable back i2c now speed set
  //
  Enbl |= IC_ENABLE_0B;
  MmioWrite32 (mRtcBase + DW_IC_ENABLE, Enbl);
}

/*
  Set i2c speed to bus, standard, high or fast.

  @param[in]  Speed          I2c bus speed.

  @retval     EFI_SCEESSS    Success.
 */
EFI_STATUS
RTCI2cSetBusSpeed(
  IN UINT32 Speed
  )
{
  if (Speed >= I2C_MAX_SPEED) {
    RtcSetSpeed (I2C_MAX_SPEED);
  } else if (Speed >= I2C_FAST_SPEED) {
    RtcSetSpeed (I2C_FAST_SPEED);
  } else {
    RtcSetSpeed (I2C_STANDARD_SPEED);
  }

  return EFI_SUCCESS;
}

/*
  Get i2c bus speed, standard, fast or high.

  @retval    I2C_MaX_SPEED - 3.4M
             I2C_STANDARD_SPEED - 100K
             I2C_FAST_SPEED - 100K
 */
UINT32
RTCI2cGetBusSpeed (
  VOID
  )
{
  UINT32 Cntl;

  Cntl = (MmioRead32 (mRtcBase + DW_IC_CON) & IC_CON_SPD_MSK);

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

  @param[in]  I2cAddr    Slave address.

  @retval  Null.
 */
STATIC
VOID
RTCI2cSetAddress (
  IN UINT32  I2cAddr
  )
{
  MmioWrite32 (mRtcBase + DW_IC_TAR, I2cAddr);
}

/*
  Flushes the i2c RX FIFO.

  @retval  Null.
 */
STATIC
VOID
RTCI2cFlushRxfifo (
  VOID
  )
{
  while (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_RFNE) {
    MmioRead32 (mRtcBase + DW_IC_DATA_CMD);
  }
}

/*
  Waits for bus busy.

  @retval  Null.
 */
STATIC
EFI_STATUS
RTCI2cWaitForBusy (
  VOID
  )
{
  UINT32 Timeout;

  Timeout= I2C_BYTE_TO_BB;

  while ((MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_MA) ||
     !(MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_TFE)) {
    //
    //Evaluate timeout
    //
    MicroSecondDelay(1000);
    if (Timeout-- == 0) {
      /*DEBUG ((DEBUG_ERROR, "Timed out. i2c RTCI2cWaitForBusy Failed\n"));*/
      return EFI_TIMEOUT;
    }
  }

  return EFI_SUCCESS;
}

/**
  Check parameters for RTCI2cRead and RTCI2cWrite.

  @param[in]  Addr    Slave address.
  @param[in]  Alen    Slave address length.
  @param[in]  Buffer  Data to write.
  @param[in]  Len     Length of data to write.

  @retval     0       Success.
              Other   Failed.
**/
STATIC
UINT32
RTCCheckParams (
  IN UINT32  Addr,
  IN UINT32  Alen,
  IN UINT8   *Buffer,
  IN UINT32  Len
  )
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

  @param[in] Chip  Target i2c address.
  @param[in] Addr  Address to read from.
  @param[in] Alen  Address length.

  @retval    0     Success.
**/
STATIC
UINT32
RTCI2cXferInit (
  IN UINT8  Chip,
  IN UINT32 Addr,
  IN UINT32 Alen
  )
{
  UINT8 Index;

  if (RTCI2cWaitForBusy ()) {
    return 1;
  }

#if 0
  Rtcdisable ();
  RTCI2cSetAddress(Chip);
  Rtcenable();
#else
//
//Reinitialize at every transmision initialization.
//
  Rtcdisable ();

  MmioWrite32 (mRtcBase + DW_IC_CON, (IC_CON_SD | IC_CON_RE | IC_CON_MM));

  MmioWrite32 (mRtcBase + DW_IC_RX_TL, IC_RX_TL);
  MmioWrite32 (mRtcBase + DW_IC_TX_TL, IC_TX_TL);

  RTCI2cSetBusSpeed (mSpeed);
  RTCI2cSetAddress(Chip);

  MmioWrite32 (mRtcBase + DW_IC_INTR_MASK, IC_STOP_DET);
  MmioWrite32 (mRtcBase + DW_IC_SAR, mSlaveaddress);

  Rtcenable ();
#endif
  //
  // REG addr
  //
  for (Index = Alen; Index > 0; Index--) {
    //DEBUG((DEBUG_INFO,"the number i is %d \n",i));
    Addr = (Addr >> (Index-1) * 8) & 0xFF;
    MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, Addr);
  }

  return 0;
}

/**
  I2c transfer finish.

  @retval  Null.
**/
STATIC
UINT32
RTCI2cXferFinish (
  VOID
  )
{
  UINTN  Timeout;

  Timeout = I2C_STOPDET_TO;
  while (1) {
    if ((MmioRead32 (mRtcBase + DW_IC_RAW_INTR_STAT) & IC_STOP_DET)) {
      MmioRead32 (mRtcBase + DW_IC_CLR_STOP_DET);
      break;
    } else {
      MicroSecondDelay(1000);
      if (Timeout-- == 0) {
        //DEBUG ((DEBUG_ERROR, "Timed out. i2c RTCI2cXferFinish Failed\n"));
        break;
      }
    }
  }

  if (RTCI2cWaitForBusy ()) {
      //DEBUG ((DEBUG_ERROR, "Timed out waiting for bus\n"));
      //DEBUG((DEBUG_INFO,"ABRT:%x\n",readl(mRtcBase + DW_IC_TX_ABRT_SOURCE)));
      return 1;
  }
  RTCI2cFlushRxfifo ();
  //
  //Wait for read/write operation to complete on actual memory
  //
  MicroSecondDelay (10);

  return 0;
}


/**
  Rtc i2c Read.

  @param[in]     Chip    Target i2c address
  @param[in]     Addr    Address to read from
  @param[in]     Alen    Address length.
  @param[in,out] Buffer  Buffer for read data
  @param[in]     Len     Number of bytes to be read

  @retval        0       Success.
                 Other   Failed.
 */
UINT32
RTCI2cRead (
  IN     UINT32  Chip,
  IN     UINT16  Addr,
  IN     UINT32  Alen,
  IN OUT UINT8   *Buffer,
  IN     UINT32  Len
  )
{
  UINT32 Timeout;
  //DEBUG((DEBUG_INFO, " mRtcBase is 0x%x Slaveaddress is 0x%x Reg is 0x%x %a line %d:\n", mRtcBase, Chip, Addr,__FUNCTION__,__LINE__));

  if (RTCI2cXferInit (Chip, Addr, Alen)) {
    return 1;
  }

  //DEBUG((DEBUG_INFO, " mRtcBase is 0x%x Slaveaddress is 0x%x Reg is 0x%x %a line %d:\n", mRtcBase, Chip, Addr,__FUNCTION__,__LINE__));
  Timeout = I2C_BYTE_TO;
  while (Len) {
    if (Len == 1) {
      MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, IC_CMD | IC_STOP);
    } else {
      MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, IC_CMD);
    }
    if (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_RFNE) {
      *Buffer  = (UINT8) MmioRead32 (mRtcBase + DW_IC_DATA_CMD);
      Len--;
      Buffer++;
      Timeout = I2C_BYTE_TO;
    } else {
      MicroSecondDelay (1000);
      //DEBUG ((DEBUG_ERROR, "need time delay------Timeout : %d-------\n", Timeout));
      if (Timeout-- == 0) {
        //DEBUG ((DEBUG_ERROR, "Timed out. i2c read Failed\n"));
        return 1;
      }
      if (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_RFNE) {
        //DEBUG ((DEBUG_ERROR, "FIFO HAVE DATA 2  -------------\n"));
        *Buffer = (UINT8) MmioRead32 (mRtcBase + DW_IC_DATA_CMD);
        Len--;
        Buffer++;
        Timeout = I2C_BYTE_TO;
      }
    }
  }

  return RTCI2cXferFinish();
}

#if 0
VOID
RTCSpdSetPage (
  UINT8  Chip,
  UINT32 PageNum
  )
{
  UINT32  Index;

  Rtcenable ();
  if (PageNum == 0x1) {
    MmioWrite32 (mRtcBase + DW_IC_TAR, 0x37);
  } else {
    MmioWrite32 (mRtcBase + DW_IC_TAR, 0x36);
  }

  MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, 0x0);
  MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, 0x0 | IC_STOP);

  /*******************For NoAck Response************************/

  while (MmioRead32 (mRtcBase + DW_IC_STATUS) != 0x6);

  if (MmioRead32 (mRtcBase+DW_IC_INTR_STAT) & 0x40){
    for (Index = 0; Index < 100; Index++);
    MmioRead32 (mRtcBase + DW_IC_INTR_STAT);
    MmioRead32 (mRtcBase + DW_IC_STATUS);
    MmioRead32 (mRtcBase + DW_IC_CLR_TX_ABRT);
    MmioRead32 (mRtcBase + DW_IC_INTR_STAT);
    MmioRead32 (mRtcBase + DW_IC_STATUS);
  }
}
#endif

/**
  Rtc i2c write data.

  @param[in] Chip     Target i2c address
  @param[in] Addr     Address to read from
  @param[in] Alen     Address length.
  @param[in] Buffer   Buffer for read data
  @param[in] Len      Number of bytes to be read

  @retval    0        Success.
             Other    Failed.
**/
UINT32
RTCI2cWrite (
  IN UINT32  Chip,
  IN UINT64  Addr,
  IN UINT32  Alen,
  IN UINT8   *Buffer,
  IN UINT32  Len
  )
{
  UINT32 Nb;
  UINT32 Timeout;
  UINT32 Cnt;

  Cnt = 0;
  Nb = Len;
  //DEBUG((DEBUG_INFO, " mRtcBase is 0x%x Slaveaddress is 0x%x Reg is 0x%x %a line %d:\n", mRtcBase, Chip, Addr,__FUNCTION__,__LINE__));
  if (RTCCheckParams (Addr, Alen, Buffer, Len)) {
    return 1;
  }

  //DEBUG((DEBUG_INFO, " mRtcBase is 0x%x Slaveaddress is 0x%x Reg is 0x%x %a line %d:\n", mRtcBase, Chip, Addr,__FUNCTION__,__LINE__));
  if (RTCI2cXferInit (Chip, Addr, Alen)) {
    return 1;
  }

  //DEBUG((DEBUG_INFO, " mRtcBase is 0x%x Slaveaddress is 0x%x Reg is 0x%x %a line %d:\n", mRtcBase, Chip, Addr,__FUNCTION__,__LINE__));
  Timeout = Nb * I2C_BYTE_TO;
  while (Len) {
    if (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_TFNF) {
      if (Len > 1) {  //not last one
        if (((Cnt + 1) % 8 == 0)){
          //DEBUG((DEBUG_ERROR, " send stop ,num is %d.len is %d \n",cnt,len));
          MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, *Buffer | IC_STOP);  //send stop
          MicroSecondDelay (100 * 1000);
        } else if ((Cnt !=0) && ((Cnt % 8) == 0)) {
          //DEBUG((DEBUG_ERROR, "re send start ,num is %d.\n",cnt));
          RTCI2cXferInit (Chip, Addr + Cnt, Alen);
          if (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, *Buffer);
          }
        } else {
          MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, *Buffer);
        }
      } else {  //last one
        if ((Cnt !=0) && ((Cnt % 8) == 0)) {
          //DEBUG((DEBUG_ERROR, " last one: restart and stop ,num is %d.len is %d \n",cnt,len));
          RTCI2cXferInit (Chip, Addr + Cnt, Alen);
          if (MmioRead32 (mRtcBase + DW_IC_STATUS) & IC_STATUS_TFNF) {
            MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, *Buffer| IC_STOP);
          }
        } else {
          //DEBUG((DEBUG_ERROR, " last one :send stop ,num is %d.len is %d \n",cnt,len));
          MmioWrite32 (mRtcBase + DW_IC_DATA_CMD, *Buffer| IC_STOP);
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

  return RTCI2cXferFinish();
}

/**
  I2c controller init.

  @param[in]  Speed         I2C_MaX_SPEED - 3.4M
                            I2C_STANDARD_SPEED - 100K
                            I2C_FAST_SPEED - 100K
  @param[in]  SlaveAddress  Slave address.

  @retval     Null.
**/
VOID
RTCI2cInit (
  IN UINT32  Speed,
  IN UINT32  SlaveAddress
  )
{
  //
  //Disable i2c
  //
  Rtcdisable ();

  MmioWrite32 (mRtcBase + DW_IC_CON, (IC_CON_SD | IC_CON_RE | IC_CON_MM));

  MmioWrite32 (mRtcBase + DW_IC_RX_TL, IC_RX_TL);
  MmioWrite32 (mRtcBase + DW_IC_TX_TL, IC_TX_TL);

  RTCI2cSetBusSpeed (Speed);

  MmioWrite32 (mRtcBase + DW_IC_INTR_MASK, IC_STOP_DET);
  MmioWrite32 (mRtcBase + DW_IC_SAR, SlaveAddress);

  Rtcenable ();
}

#if 0
UINT32
RTCEnableI2cAsSlaver (
  UINT8 SlaveAddress
  )
{

  Rtcdisable ();
  //
  //set slave address
  //
  MmioWrite32 (mRtcBase + DW_IC_SAR, SlaveAddress);
  //
  //config slave mode
  //
  MmioWrite32 (mRtcBase + DW_IC_CON, 0);
  //
  //set speed
  //
  RtcSetSpeed (I2C_FAST_SPEED);
  Rtcenable ();

  return 0;
}
#endif

/**
  BCD to BIN.

  @param[in]  Val    Bcd data.

  @retval     BIN
**/
UINT32
Bcd2bin (
  IN UINT8 Val
  )
{
  return ((Val) & 0x0f) + ((Val) >> 4) * 10;
}

/**
  BIN to BCD.

  @param[in]  Val    BIN

  @retval     BCD
**/
UINT8
Bin2bcd (
  UINT32 Val
  )
{
  return (((Val / 10) << 4) | (Val % 10));
}

/**
  Rtc register read.

  @param[in]  Reg        Register to read.
  @param[in]  SlaveAddr  I2c slave address.

  @retval     Buf        Register value.
**/
UINT8
RtcRead (
  IN UINT16  Reg,
  IN UINT32  SlaveAddr
  )
{
  UINT8 Buf;

  RTCI2cRead (SlaveAddr, Reg, 1, &Buf, 1);
  //DEBUG ((DEBUG_INFO, "%a(), Reg : %x, Data : %x\n", __FUNCTION__, Reg, Buf));
  return Buf;
}

/**
  Rtc register write.

  @param[in]  Reg        Register to write.
  @param[in]  Val        Data to write.
  @param[in]  SlaveAddr  I2c slave address.

  @retval     Null
**/
VOID
RtcWrite (
  IN UINT16  Reg,
  IN UINT8   Val,
  IN UINT32  SlaveAddr
  )
{
  //DEBUG ((DEBUG_INFO, "%a(), Reg : %x, Data : %x\n", __FUNCTION__, Reg, Val));
  RTCI2cWrite (SlaveAddr, Reg, 1, &Val, 1);
}

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param[out]  Time              A pointer to storage to receive a snapshot of the current time.
  @param[out]  Capabilities      An optional pointer to a buffer to receive the real time clock
                                 device's capabilities.
  @param[in]   Slaveaddress      Rtc device i2c slave address.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.
**/
EFI_STATUS
EFIAPI
GetRtcTime (
  OUT EFI_TIME                  *Time,
  OUT EFI_TIME_CAPABILITIES     *Capabilities,
  IN UINT32                      Slaveaddress
  )
{
  EFI_STATUS  Status;
  RTC_TIME    RtcTime;
  UINT8       Sec;
  UINT8       Min;
  UINT8       Hour;
  UINT8       Mday;
  UINT8       MonCent;
  UINT8       Year;

  Status = EFI_SUCCESS;

  if (Time == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto EXIT;
  }

  Sec      = RtcRead (RTC_SEC_REG_ADDR,  Slaveaddress);
  Min      = RtcRead (RTC_MIN_REG_ADDR,  Slaveaddress);
  Hour     = RtcRead (RTC_HR_REG_ADDR,   Slaveaddress);
  Mday     = RtcRead (RTC_DATE_REG_ADDR, Slaveaddress);
  MonCent  = RtcRead (RTC_MON_REG_ADDR,  Slaveaddress);
  Year     = RtcRead (RTC_YR_REG_ADDR,   Slaveaddress);

  //DEBUG ((DEBUG_INFO, "%a()\n", __FUNCTION__));
  //DEBUG ((DEBUG_INFO, "Sec : %d\n", Sec));
  //DEBUG ((DEBUG_INFO, "Min : %d\n", Min));
  //DEBUG ((DEBUG_INFO, "Hour : %d\n", Hour));
  //DEBUG ((DEBUG_INFO, "Mday : %d\n", Mday));
  //DEBUG ((DEBUG_INFO, "MonCent : %d\n", MonCent));
  //DEBUG ((DEBUG_INFO, "Year : %d\n", Year));

  RtcTime.TmSec  = Bcd2bin (Sec & 0x7F);
  RtcTime.TmMin  = Bcd2bin (Min & 0x7F);
  RtcTime.TmHour = Bcd2bin (Hour & 0x3F);
  RtcTime.TmMday = Bcd2bin (Mday & 0x3F);
  RtcTime.TmMon  = Bcd2bin (MonCent & 0x1F);
  RtcTime.TmYear = Bcd2bin (Year) + 2000;
  RtcTime.TmYday = 0;
  RtcTime.TmIsdst= 0;

  Time->Year       = (UINT16)RtcTime.TmYear;
  Time->Month      = (UINT8)RtcTime.TmMon;
  Time->Day        = (UINT8)RtcTime.TmMday;
  Time->Hour       = (UINT8)RtcTime.TmHour;
  Time->Minute     = (UINT8)RtcTime.TmMin;
  Time->Second     = (UINT8)RtcTime.TmSec;
  Time->Nanosecond = 0;

EXIT:
  return Status;
}

/**
  Sets the current local time and date information.

  @param[in]  Time              A pointer to the current time.
  @param[in]  Slaveaddress      Rtc device i2c slave address.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.
**/
EFI_STATUS
EFIAPI
SetRtcTime (
  IN EFI_TIME              *Time,
  IN UINT32                 Slaveaddress
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  //DEBUG ((DEBUG_INFO, "%a()\n", __FUNCTION__));
  //DEBUG ((DEBUG_INFO, "year : %d\n", Time->Year));
  //DEBUG ((DEBUG_INFO, "Month : %d\n", Time->Month));
  //DEBUG ((DEBUG_INFO, "day : %d\n", Time->Day));
  //DEBUG ((DEBUG_INFO, "hour : %d\n", Time->Hour));
  //DEBUG ((DEBUG_INFO, "minute : %d\n", Time->Minute));
  //DEBUG ((DEBUG_INFO, "Second : %d\n", Time->Second));
  // Check the input parameters are within the range specified by UEFI
  if (!IsTimeValid (Time)) {
    Status = EFI_INVALID_PARAMETER;
    goto EXIT;
  }

  //
  // Because the PL031 is a 32-bit counter counting seconds,
  // the maximum time span is just over 136 years.
  // Time is stored in Unix Epoch format, so it starts in 1970,
  // Therefore it can not exceed the year 2106.
  //
  if ((Time->Year < 1970) || (Time->Year >= 2106)) {
    Status = EFI_UNSUPPORTED;
    goto EXIT;
  }

  RtcWrite (RTC_YR_REG_ADDR, Bin2bcd (Time->Year % 100), Slaveaddress);
  if (Slaveaddress == CONFIG_RTC_DS1337) {
    RtcWrite (RTC_MON_REG_ADDR, Bin2bcd (Time->Month) | 0x80, Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068) {
    RtcWrite (RTC_MON_REG_ADDR, Bin2bcd (Time->Month), Slaveaddress);
  }
  RtcWrite (RTC_DATE_REG_ADDR, Bin2bcd (Time->Day), Slaveaddress);
  if (Slaveaddress == CONFIG_RTC_DS1337) {
    RtcWrite (RTC_HR_REG_ADDR, Bin2bcd (Time->Hour), Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068) {
    RtcWrite (RTC_HR_REG_ADDR, Bin2bcd (Time->Hour) | 0x80, Slaveaddress);
  }
  RtcWrite (RTC_MIN_REG_ADDR, Bin2bcd (Time->Minute), Slaveaddress);
  RtcWrite (RTC_SEC_REG_ADDR, Bin2bcd (Time->Second), Slaveaddress);

EXIT:
  return Status;
}

/**
  Sets the system wakeup alarm clock time.

  @param[in]   Enabled          Enable or disable the wakeup alarm.
  @param[out]  Time             If Enable is TRUE, the time to set the wakeup alarm for.
  @param[in]   Slaveaddress     Rtc device i2c slave address.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
**/
EFI_STATUS
EFIAPI
SetRtcWakeUpTime (
  IN BOOLEAN     Enabled,
  OUT EFI_TIME   *Time,
  IN UINT32      Slaveaddress
  )
{
  EFI_STATUS         Status;
  UINT8              Temp;
  Temp   = 0;
  Status = EFI_SUCCESS;

  if (Slaveaddress == CONFIG_RTC_DS1337){
    //
    //disable alarm
    //
    if (!Enabled) {
      Temp = RtcRead (RTC_CTL_REG_ADDR, Slaveaddress);
      Temp &= ~(1 << 0);
      RtcWrite (RTC_CTL_REG_ADDR, Temp, Slaveaddress);
      return EFI_SUCCESS;
    }

    if (Time == NULL || !IsTimeValid (Time)) {
      return EFI_INVALID_PARAMETER;
    }
    //
    //enable alarm
    //
    RtcWrite (RTC_CTL_REG_ADDR, RTC_CTL_BIT_A1IE | RTC_CTL_BIT_INTCN, Slaveaddress);
    //
    //set alarm time
    //
    RtcWrite (RTC_A1M1_SECONDS_REG, Bin2bcd (Time->Second), Slaveaddress);
    RtcWrite (RTC_A1M2_MINUTES_REG, Bin2bcd (Time->Minute), Slaveaddress);
    RtcWrite (RTC_A1M3_HOURS_REG,   Bin2bcd (Time->Hour), Slaveaddress);
    RtcWrite (RTC_A1M4_DAYS_REG,    Bin2bcd (Time->Day), Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068){
    //
    //disable alarm
    //
    if (!Enabled){
      RtcWrite (RTC_CTL_REG_ADDR, 0x0, Slaveaddress);
      Temp = RtcRead(RTC_TC_REG_ADDR, Slaveaddress);
      Temp &= ~(1 << 1);
      RtcWrite (RTC_TC_REG_ADDR, Temp, Slaveaddress);
      return EFI_SUCCESS;
    }
    if (Time == NULL || !IsTimeValid (Time)) {
      return EFI_INVALID_PARAMETER;
    }
    RtcWrite (RTC_TC_REG_ADDR, 0x92, Slaveaddress);        //10h    1001 0010 IM=0 INTS1=0 INTS0=1 INTAE=1
    RtcWrite (RTC_STAT_REG_ADDR, 0xFF, Slaveaddress);
    RtcWrite (RTC_CTL_REG_ADDR, 0x17, Slaveaddress);        //enable alarm

    RtcWrite (RTC_A1M1_SECONDS_REG, Bin2bcd (Time->Second), Slaveaddress);
    RtcWrite (RTC_A1M2_MINUTES_REG, Bin2bcd (Time->Minute), Slaveaddress);
    RtcWrite (RTC_A1M3_HOURS_REG, Bin2bcd (Time->Hour), Slaveaddress);
    RtcWrite (0x0B, Bin2bcd (Time->Day), Slaveaddress);
    RtcWrite (0x0C, Bin2bcd (Time->Month), Slaveaddress);
    RtcWrite (0x0D, Bin2bcd (Time->Year % 100), Slaveaddress);
  }

  return Status;
}

/**
  Returns the current wakeup alarm clock setting.

  @param[out]  Enabled          Indicates if the alarm is currently enabled or disabled.
  @param[out]  Pending          Indicates if the alarm signal is pending and requires acknowledgement.
  @param[out]  Time             The current alarm setting.
  @param[in]   Slaveaddress     Rtc device i2c slave address.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
**/
EFI_STATUS
EFIAPI
GetRtcWakeUpTime (
  OUT BOOLEAN    *Enabled,
  OUT BOOLEAN    *Pending,
  OUT EFI_TIME   *Time,
  IN  UINT32     Slaveaddress
  )
{
  EFI_STATUS  Status;
  UINT16      Year;
  UINT8       Mounth;
  UINT8       Second;
  UINT8       Minute;
  UINT8       Hour;
  UINT8       Day;

  Status = EFI_SUCCESS;

  if (Time == NULL || Enabled == NULL || Pending == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Slaveaddress == CONFIG_RTC_DS1337){
    *Enabled = RtcRead (RTC_CTL_REG_ADDR, Slaveaddress) & RTC_CTL_BIT_A1IE;
    *Pending = RtcRead (RTC_STAT_REG_ADDR, Slaveaddress) & RTC_STAT_BIT_A1F;
    if (*Pending) {
      RtcWrite (RTC_STAT_REG_ADDR, RTC_STAT_BIT_A1F,Slaveaddress);
    }
    Day = RtcRead (RTC_A1M4_DAYS_REG, Slaveaddress);
  } else if (Slaveaddress == CONFIG_RTC_SD3068) {
    if (RtcRead (RTC_TC_REG_ADDR, Slaveaddress) & 0x2) {
      *Enabled = 1;
    } else {
      *Enabled = 0;
    }
    if (RtcRead (RTC_STAT_REG_ADDR, Slaveaddress) & 0x20) {
      *Pending = 1;
    } else {
      *Pending = 0;
    }
    if (*Pending) {
      RtcWrite (RTC_STAT_REG_ADDR, 0x20, Slaveaddress);
    }
     Day     = RtcRead (0x0B, Slaveaddress);
     Mounth  = RtcRead (0x0C, Slaveaddress);
     Year    = RtcRead (0x0D, Slaveaddress);
  }

  Second = RtcRead (RTC_A1M1_SECONDS_REG, Slaveaddress);
  Minute = RtcRead (RTC_A1M2_MINUTES_REG, Slaveaddress);
  Hour   = RtcRead (RTC_A1M3_HOURS_REG, Slaveaddress);

  Time->Second  = Bcd2bin (Second & 0x7F);
  Time->Minute  = Bcd2bin (Minute & 0x7F);
  Time->Hour    = Bcd2bin (Hour & 0x3F);
  Time->Day     = Bcd2bin (Day & 0x3F);
  Time->Month   = Bcd2bin (Mounth & 0x1F);
  Time->Year    = Bcd2bin (Year) + 2000;
  Time->Nanosecond = 0;

  return Status;
}


/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param[out]  Time              A pointer to storage to receive a snapshot of the current time.
  @param[out]  Capabilities      An optional pointer to a buffer to receive the real time clock
                                 device's capabilities.

  @retval EFI_SUCCESS            The operation completed successfully.
  @retval EFI_INVALID_PARAMETER  Time is NULL.
  @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
  @retval EFI_SECURITY_VIOLATION The time could not be retrieved due to an authentication failure.
**/
EFI_STATUS
EFIAPI
LibGetTime (
  OUT EFI_TIME                *Time,
  OUT EFI_TIME_CAPABILITIES   *Capabilities
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = GetRtcTime (Time, Capabilities, mSlaveaddress);

  return Status;
}

/**
  Sets the current local time and date information.

  @param[in]  Time              A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.
**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN  EFI_TIME                *Time
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = SetRtcTime (Time, mSlaveaddress);

  return Status;
}

/**
  Returns the current wakeup alarm clock setting.

  @param[out]  Enabled          Indicates if the alarm is currently enabled or disabled.
  @param[out]  Pending          Indicates if the alarm signal is pending and requires acknowledgement.
  @param[out]  Time             The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.

**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  EFI_STATUS  Status;

  Status = EFI_UNSUPPORTED;
  Status = GetRtcWakeUpTime (Enabled, Pending, Time, mSlaveaddress);

  return Status;
}


/**
  Sets the system wakeup alarm clock time.

  @param[in]   Enabled          Enable or disable the wakeup alarm.
  @param[out]  Time             If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibSetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
  EFI_STATUS Status;

  Status = EFI_UNSUPPORTED;
  Status = SetRtcWakeUpTime (Enabled, Time, mSlaveaddress);

  return Status;
}

/**
  Fixup internal data so that EFI can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  //
  // Only needed if you are going to support the OS calling RTC functions in virtual mode.
  // You will need to call EfiConvertPointer (). To convert any stored physical addresses
  // to virtual address. After the OS transitions to calling in virtual mode, all future
  // runtime calls will be made in virtual mode.
  //
  EfiConvertPointer (0x0, (VOID**) &mRtcBase);

  return;
}

/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS       Status;
  EFI_HANDLE       Handle;
  UINT32           Speed;

  // Initialize RTC Base Address
  mRtcBase      = PcdGet64 (PcdRtcI2cControllerBaseAddress);
  mSlaveaddress = PcdGet32 (PcdRtcI2cControllerSlaveAddress);
  Speed         = PcdGet32 (PcdRtcI2cControllerSpeed);
  mSpeed        = PcdGet32 (PcdRtcI2cControllerSpeed);

  DEBUG ((DEBUG_INFO, "mRtcBase : 0x%x\n", mRtcBase));
  DEBUG ((DEBUG_INFO, "mSlaveaddress : 0x%x\n", mSlaveaddress));
  RTCI2cInit (Speed, mSlaveaddress);
  //
  // Declare the controller as EFI_MEMORY_RUNTIME
  //
  Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeMemoryMappedIo,
                  mRtcBase,
                  SIZE_4KB,
                  EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gDS->SetMemorySpaceAttributes (
                  mRtcBase,
                  SIZE_4KB, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install the protocol
  //
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for the virtual address change event
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  LibRtcVirtualNotifyEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mRtcVirtualAddrChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
