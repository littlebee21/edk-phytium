#ifndef _MCU_INFO_H
#define _MCU_INFO_H

#include <Library/DebugLib.h>
#include <Library/ParameterTable.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PhytiumPowerControlLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/I2CLib.h>

#define PARAMETER_CPU_MAGIC       0x54460010


#define GPIO_0_BASE               0x28004000
#define PIN_REUSE_CTL             0x28180000

#define PWR_CTR0_REG              0x28180480
#define PWR_CTR1_REG              0x28180484

#define GPIO_O_A2_REUSE           0x0200
#define GPIO_O_A2_CONF            0x50000

#define GPIO_SWPORT_DR            0x00
#define GPIO_SWPORT_DDR           0x04
#define GPIO_EXT_PORTA            0x08

#define FUNC1                     0x01

#define NONE_RESISTOR             0x00
#define PULL_DOWN_RESISTOR        0x01
#define PULL_UP_RESISTOR          0x10

#define PULL_UP                   0x04
#define PULL_DOWN                 0x0
#define ERROR_TYPE0               0
#define ERROR_TYPE1               1
#define ERROR_TYPE2               2
#define ERROR_TYPE3               3
#define ERROR_TYPE4               4
#define ERROR_TYPE5               5
#define ERROR_TYPE6               6
#define ERROR_TYPE7               7

#define MISC_READ_SPD_FREQ_EN     (0x1 << 0)
#define MISC_USE_MARGIN_EN        (0x1 << 1)
#define MISC_S3_DEVINIT_EN        (0x1 << 2)
#define MISC_2T_MODE_EN           (0x1 << 3)
#define MISC_DUAL_DIMM_EN         (0x1 << 4)


#define PARAMETER_MCU_MAGIC       0x54460014
#define PARAM_MCU_VERSION         0x1
#define PARAM_MCU_SIZE            0x100
#define PARAM_CH_ENABLE           0x3
#define PARAM_ECC_ENABLE          0x3
#define PARAM_DM_ENABLE           0x3
#define PARAM_FORCE_SPD_DISABLE   0x0
#define PARAM_MCU_MISC_ENABLE     0x0
#define PARAM_TRAIN_DEBUF         0xff

//#define MCU_USE_MANUAL
//#define MCU_USE_PARAMETER

#define MASK_4BIT                 0xF

#define SPD_MTB                   125     //125ps
#define SPD_FTB                   1       //1ps

#define RDIMM_TYPE                1
#define UDIMM_TYPE                2
#define SODIMM_TYPE               3
#define LRDIMM_TYPE               4
#define DIMM_x4                   0x0
#define DIMM_x8                   0x1
#define DIMM_x16                  0x2
#define DIMM_x32                  0x3
#define MIRROR                    0x1
#define NO_MIRROR                 0x0
#define ECC_TYPE                  1
#define NO_ECC_TYPE               0
#define ROW_NUM                   16
#define COL_NUM                   10
#define DDR3_TYPE                 0xB
#define DDR4_TYPE                 0xC
#define LPDDR4_TYPE               0x10
#define DDR5_TYPE                 0x12

#define SAMSUNG_VENDOR            0x80CE
#define HYNIX_VENDOR              0x80AD
#define MICRON_VENDOR             0x802C
#define KINGSTON_VENDOR           0x0198
#define RAMAXEL_VENDOR            0x0443
#define LANQI_VENDOR              0x8632
#define CXMT_VENDOR               0x8A91
#define UNILC_VENDOR              0x081A

#define SPD_NUM                   256

#define LMU0_I2C_ID               0
#define LMU1_I2C_ID               0
#define LMU0_I2C_SLAVE_ADDR       0x50
#define LMU1_I2C_SLAVE_ADDR       0x51

#define MIO_BASE                  0x28000000 /* 1MB: Uart/i2c/wdt/gpio */
#define GPIO0_BASE                (MIO_BASE + 0x4000)
#define GPIO1_BASE                (MIO_BASE + 0x5000)
#define SWPORTA_DDR_OFFSET        0x4
#define SWPORTB_DDR_OFFSET        0x10
#define EXT_PORTA_OFFSET          0x8
#define EXT_PORTB_OFFSET          0x14


#define PWR_CTR0_REG              0x28180480
#define PWR_CTR1_REG              0x28180484

#define S3_CLEAN_CPLD             1
#define REBOOT_CPLD               4
#define S3_SETUP_CPLD             8
#define SHUTDOWN_CPLD             12

#define I2C0_BASE                 (MIO_BASE + 0x6000)

#if !defined(IC_CLK)
#define IC_CLK                    166
#endif
#define NANO_TO_MICRO             1000

/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME       4000
#define MIN_SS_SCL_LOWTIME        4700
#define MIN_FS_SCL_HIGHTIME       600
#define MIN_FS_SCL_LOWTIME        1300
#define MIN_HS_SCL_HIGHTIME       60
#define MIN_HS_SCL_LOWTIME        160

/* Worst case timeout for 1 byte is kept as 2ms */
#define I2C_BYTE_TO               (CONFIG_SYS_HZ/500)
#define I2C_STOPDET_TO            (CONFIG_SYS_HZ/500)
#define I2C_BYTE_TO_BB            (I2C_BYTE_TO * 16)

/* i2c control register definitions */
#define IC_CON_SD                 0x0040
#define IC_CON_RE                 0x0020
#define IC_CON_10BITADDRMASTER    0x0010
#define IC_CON_10BITADDR_SLAVE    0x0008
#define IC_CON_SPD_MSK            0x0006
#define IC_CON_SPD_SS             0x0002
#define IC_CON_SPD_FS             0x0004
#define IC_CON_SPD_HS             0x0006
#define IC_CON_MM                 0x0001

/* i2c target address register definitions */
#define TAR_ADDR                  0x0050

/* i2c slave address register definitions */
#define IC_SLAVE_ADDR             0x0002

/* i2c data buffer and command register definitions */
#define IC_CMD                    0x0100
#define IC_STOP                   0x0200

/* i2c interrupt status register definitions */
#define IC_GEN_CALL               0x0800
#define IC_START_DET              0x0400
#define IC_STOP_DET               0x0200
#define IC_ACTIVITY               0x0100
#define IC_RX_DONE                0x0080
#define IC_TX_ABRT                0x0040
#define IC_RD_REQ                 0x0020
#define IC_TX_EMPTY               0x0010
#define IC_TX_OVER                0x0008
#define IC_RX_FULL                0x0004
#define IC_RX_OVER                0x0002
#define IC_RX_UNDER               0x0001

/* fifo threshold register definitions */
#define IC_TL0                    0x00
#define IC_TL1                    0x01
#define IC_TL2                    0x02
#define IC_TL3                    0x03
#define IC_TL4                    0x04
#define IC_TL5                    0x05
#define IC_TL6                    0x06
#define IC_TL7                    0x07
#define IC_RX_TL                  IC_TL0
#define IC_TX_TL                  IC_TL0

/* i2c enable register definitions */
#define IC_ENABLE_0B              0x0001

/* i2c status register  definitions */
#define IC_STATUS_SA              0x0040
#define IC_STATUS_MA              0x0020
#define IC_STATUS_RFF             0x0010
#define IC_STATUS_RFNE            0x0008
#define IC_STATUS_TFE             0x0004
#define IC_STATUS_TFNF            0x0002
#define IC_STATUS_ACT             0x0001

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD    1
#define IC_SPEED_MODE_FAST        2
#define IC_SPEED_MODE_MAX         3

#define I2C_MAX_SPEED             3400000
#define I2C_FAST_SPEED            400000
#define I2C_STANDARD_SPEED        100000
#if 0
VOID
Alarm(
  UINTN  ErrorType
  );
#endif

VOID
GetDdrConfigParameter (
  IN OUT  DDR_CONFIG *DdrConfigData,
  IN      UINT8      S3Flag
  );
#endif
