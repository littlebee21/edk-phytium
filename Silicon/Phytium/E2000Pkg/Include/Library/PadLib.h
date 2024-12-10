/**

**/
#ifndef    _PAD_LIB_H_
#define    _PAD_LIB_H_

#define PLAT_INFO_BASE       ((CONFIG_SYS_TEXT_BASE - 0x180000) + 0xFA000)//704k
#define CONFIG_SYS_TEXT_BASE 0x38180000
#define PARAMETER_PAD_MAGIC  0x54460016

//
//Mio
//
#define    MIO_REGISTER_BASE              0x28014000
#define    MIO_BASE_OFFSET                0x2000
#define    MIO_CONFIG_BASE                0x1000
#define    MIO_SELECT_REG                 0x0
#define    MIO_MAX_COUNT                  16
#define    MIO_I2C_SELECT                 0
#define    MIO_UART_SELECT                1
#define    AUTO_DETECT_SATA               0
#define    AUTO_DETECT_PCIE               1

typedef struct _PAD_CONFIG {
  UINT32  Addr;
  UINT32  Value;
} PAD_CONFIG;

typedef struct _PAD_INFO {
  UINT32      Size;
  PAD_CONFIG  List[1];
} PAD_INFO;

typedef struct _PHY_CONFIG {
  UINT32  PhySel;
  UINT32  MacMode;
  UINT32  PhySpeed;
} PHY_CONFIG;


/**
  Config pad information to regitsters according parameter table.

  @retval    EFI_SUCCESS
             EFI_NOT_FOUND
**/
EFI_STATUS
ConfigPad (
  VOID
  );

/**
  Set pad configuration with board type.

  @param[in]  BoardType    Boart type.
                           4 : Phytium D/S A.
                           5 : Phytium D/S B.
                           8 : Great Wall F080-E.

  @retval     Null.
**/
VOID
ConfigPadWithBoardType (
  UINT32  BoardType
  );

/**
  Config phy through SMC message according to the board type.

  @param[in]  BoardType    Boart type.
                           4 : Phytium D/S A.
                           5 : Phytium D/S B.
                           8 : Great Wall F080-E.

  @retval     Null.
**/
VOID
PhyConfigWithBoardType (
  UINT32  BoardType
  );

/**
  Configure phy according to the parameter table.

  @retval    EFI_SUCCESS    Success.
  @retval    EFI_NOT_FOUND  Not fount phy information.
**/
EFI_STATUS
PhyConfigWithParTable (
  VOID
  );

/**

**/
VOID
LsdDmaChannelConfig (
  VOID
  );

/**
  Configure mio selection according to the parameter table.

  @retval    EFI_SUCCESS    Success.
  @retval    EFI_NOT_FOUND  Not fount mio information.
**/
EFI_STATUS
MioConfigWithParTable (
  VOID
  );
  
  
  
VOID QosConfig(
UINT32 arg1, 
UINT32 arg2
);
  
#endif
