/*
 */

#ifndef _I2CLib_H_
#define _I2CLib_H_

typedef struct _I2C_INFO {
  UINT64  BusAddress;
  UINT32  Speed;
  UINT32  SlaveAddress;
} I2C_INFO;

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
  );

VOID
SpdSetpage (
  IN I2C_INFO  *Info,
  IN UINT32    PageNum
  );

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
  );

/**
  I2c controller init.

  @param[in] Info     A pointer to I2C_INFO

  @retval     Null.
**/
VOID
I2cInit (
  IN I2C_INFO  *Info
  );

#endif /* __I2CLib_H_ */
