/** @file
Phytium eMMC host controller driver header.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright (c) 2014-2017, Linaro Limited. All rights reserved.
Copyright (c) 2011-2013, ARM Limited. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef PYHTIUM_EMMC_H_
#define PHYTIUM_EMMC_H_

#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/MmcHost.h>

//
// Phytium MMC Registers
//
#define EMMC_CTRL             (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x000)
#define EMMC_PWREN            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x004)
#define EMMC_CLKDIV           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x008)
#define EMMC_CLKENA           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x010)
#define EMMC_TMOUT            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x014)
#define EMMC_CTYPE            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x018)
#define EMMC_BLKSIZ           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x01c)
#define EMMC_BYTCNT           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x020)
#define EMMC_INTMASK          (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x024)
#define EMMC_CMDARG           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x028)
#define EMMC_CMD              (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x02c)
#define EMMC_RESP0            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x030)
#define EMMC_RESP1            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x034)
#define EMMC_RESP2            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x038)
#define EMMC_RESP3            (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x03c)
#define EMMC_MASKED_INTS      (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x040)
#define EMMC_RINTSTS          (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x044)
#define EMMC_STATUS           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x048)
#define EMMC_FIFOTH           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x04c)
#define EMMC_CARD_DETECT_BIU  (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x050)
#define EMMC_CARD_WRITE_PRT   (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x054)
#define EMMC_CCLK_READY       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x058)
#define EMMC_TCBCNT           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x05c)
#define EMMC_TBBCNT           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x060)
#define EMMC_DEBNCE           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x064)
#define EMMC_UID              (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x068)
#define EMMC_VID              (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x06c)
#define EMMC_HCON             (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x070)
#define EMMC_UHSREG           (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x074)
#define EMMC_CARD_RESET       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x078)
#define EMMC_BMOD             (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x080)
#define EMMC_DESC_LIST_L      (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x088)
#define EMMC_DESC_LIST_H      (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x08c)
#define EMMC_DMAC_STATUS      (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x090)
#define EMMC_INTR_EN          (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x094)
#define EMMC_CUR_DESC_L       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x098)
#define EMMC_CUR_DESC_H       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x09c)
#define EMMC_CUR_BUFF_L       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x0a0)
#define EMMC_CUR_BUFF_H       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x0a4)
#define EMMC_CARDTHRCTL       (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x100)
#define EMMC_UHS_REG_EXT      (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x108)
#define EMMC_DDR              (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x10c)
#define EMMC_ENABLE_SHIFT     (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x110)
#define EMMC_DATA             (PcdGet64 (PcdEmmcDxeBaseAddress) + 0x200)

#define CMD_UPDATE_CLK                          0x80202000
#define CMD_START_BIT                           (1 << 31)

#define MMC_8BIT_MODE                           (1 << 16)

//
//CNTRL mask
//
#define  EMMC_CNTRL_CONTROLLER_RESET            (0x1 << 0)
#define  EMMC_CNTRL_FIFO_RESET                  (0x1 << 1)
#define  EMMC_CNTRL_DMA_RESET                   (0x1 << 2)
#define  EMMC_CNTRL_RES                         (0x1 << 3)
#define  EMMC_CNTRL_INT_ENABLE                  (0x1 << 4)
#define  EMMC_CNTRL_DMA_ENABLE                  (0x1 << 5)
#define  EMMC_CNTRL_READ_WAIT                   (0x1 << 6)
#define  EMMC_SEND_IRQ_RESPONSE                 (0x1 << 7)
#define  EMMC_ABORT_READ_DATA                   (0x1 << 8)
#define  EMMC_CNTRL_ENDIAN                      (0x1 << 11)
#define  EMMC_CNTRL_ENABLE_OD_PULLUP            (0x1 << 24)
#define  EMMC_CNTRL_USE_INTERNAL_DMAC           (0X1 << 25)

//
//PWREN mask
//
#define EMMC_PWREN_ENABLE                       (0x1 << 0)

//
//CLKENA mask
//
#define EMMC_CLKENA_CCLK_ENABLE                 (0x1 << 0)
#define EMMC_CLKENA_CCLK_LOW_POWER              (0x1 << 16)

//
//STATUS mask
//
#define EMMC_STATUS_FIFO_RX                     (0x1 << 0)
#define EMMC_STATUS_FIFO_TX                     (0x1 << 1)
#define EMMC_STATUS_FIFO_EMPTY                  (0x1 << 2)
#define EMMC_STATUS_FIFO_FULL                   (0x1 << 3)
#define EMMC_STATUS_CARD_STATUS                 (0x1 << 8)
#define EMMC_STATUS_CARD_BUSY                   (0x1 << 9)
#define EMMC_STATUS_DATA_BUSY                   (0x1 << 10)
#define EMMC_STATUS_DMA_ACK                     (0x1 << 30)
#define EMMC_STATUS_DMA_REQ                     (0x1 << 31)

//
//UHS_REG mask
//
#define EMMC_EXT_CLK_ENABLE                     (0x1 << 1)
#define EMMC_UHS_REG_VOLT                       (0x1 << 0)
#define EMMC_UHS_REG_DDR                        (0x1 << 16)

//
//CARD_RESET mask
//
#define EMMC_CARD_RESET_ENABLE                  (0x1 << 0)

//
//BUS_MODE mask
//
#define EMMC_BUS_MODE_SWR                       (0x1 << 0)
#define EMMC_BUS_MODE_FB                        (0x1 << 1)
#define EMMC_BUS_MODE_DE                        (0x1 << 7)

//
// CMD mask
//
#define BIT_CMD_RESPONSE_EXPECT                 (1 << 6)
#define BIT_CMD_LONG_RESPONSE                   (1 << 7)
#define BIT_CMD_CHECK_RESPONSE_CRC              (1 << 8)
#define BIT_CMD_DATA_EXPECTED                   (1 << 9)
#define BIT_CMD_READ                            (0 << 10)
#define BIT_CMD_WRITE                           (1 << 10)
#define BIT_CMD_BLOCK_TRANSFER                  (0 << 11)
#define BIT_CMD_STREAM_TRANSFER                 (1 << 11)
#define BIT_CMD_SEND_AUTO_STOP                  (1 << 12)
#define BIT_CMD_WAIT_PRVDATA_COMPLETE           (1 << 13)
#define BIT_CMD_STOP_ABORT_CMD                  (1 << 14)
#define BIT_CMD_SEND_INIT                       (1 << 15)
#define BIT_CMD_UPDATE_CLOCK_ONLY               (1 << 21)
#define BIT_CMD_READ_CEATA_DEVICE               (1 << 22)
#define BIT_CMD_CCS_EXPECTED                    (1 << 23)
#define BIT_CMD_ENABLE_BOOT                     (1 << 24)
#define BIT_CMD_EXPECT_BOOT_ACK                 (1 << 25)
#define BIT_CMD_DISABLE_BOOT                    (1 << 26)
#define BIT_CMD_MANDATORY_BOOT                  (0 << 27)
#define BIT_CMD_ALTERNATE_BOOT                  (1 << 27)
#define BIT_CMD_VOLT_SWITCH                     (1 << 28)
#define BIT_CMD_USE_HOLD_REG                    (1 << 29)
#define BIT_CMD_START                           (1 << 31)

//
// INITS mask
//
#define EMMC_INT_EBE                          (1 << 15)       /* End-bit Err */
#define EMMC_INT_SBE                          (1 << 13)       /* Start-bit  Err */
#define EMMC_INT_HLE                          (1 << 12)       /* Hardware-lock Err */
#define EMMC_INT_FRUN                         (1 << 11)       /* FIFO UN/OV RUN */
#define EMMC_INT_DRT                          (1 << 9)        /* Data timeout */
#define EMMC_INT_RTO                          (1 << 8)        /* Response timeout */
#define EMMC_INT_DCRC                         (1 << 7)        /* Data CRC err */
#define EMMC_INT_RCRC                         (1 << 6)        /* Response CRC err */
#define EMMC_INT_RXDR                         (1 << 5)
#define EMMC_INT_TXDR                         (1 << 4)
#define EMMC_INT_DTO                          (1 << 3)        /* Data trans over */
#define EMMC_INT_CMD_DONE                     (1 << 2)
#define EMMC_INT_RE                           (1 << 1)

//
// DMA descriptor0 mask
//
#define EMMC_IDMAC_DES0_DIC                   (1 << 1)
#define EMMC_IDMAC_DES0_LD                    (1 << 2)
#define EMMC_IDMAC_DES0_FS                    (1 << 3)
#define EMMC_IDMAC_DES0_CH                    (1 << 4)
#define EMMC_IDMAC_DES0_ER                    (1 << 5)
#define EMMC_IDMAC_DES0_CES                   (1 << 30)
#define EMMC_IDMAC_DES0_OWN                   (1 << 31)
#define EMMC_IDMAC_DES1_BS1(x)                ((x) & 0x1fff)
#define EMMC_IDMAC_DES2_BS2(x)                (((x) & 0x1fff) << 13)
#define EMMC_IDMAC_SWRESET                    (1 << 0)
#define EMMC_IDMAC_FB                         (1 << 1)
#define EMMC_IDMAC_ENABLE                     (1 << 7)

#define EMMC_FIX_RCA                            6

//
// EMMC_CTRL mask
//
#define EMMC_CTRL_RESET                       (1 << 0)
#define EMMC_CTRL_FIFO_RESET                  (1 << 1)
#define EMMC_CTRL_DMA_RESET                   (1 << 2)
#define EMMC_CTRL_INT_EN                      (1 << 4)
#define EMMC_CTRL_DMA_EN                      (1 << 5)
#define EMMC_CTRL_IDMAC_EN                    (1 << 25)
#define EMMC_CTRL_RESET_ALL                   (EMMC_CTRL_RESET | EMMC_CTRL_FIFO_RESET | EMMC_CTRL_DMA_RESET)

#define EMMC_STS_DATA_BUSY                    (1 << 9)

#define EMMC_FIFO_TWMARK(x)                   (x & 0xfff)
#define EMMC_FIFO_RWMARK(x)                   ((x & 0x1ff) << 16)
#define EMMC_DMA_BURST_SIZE(x)                ((x & 0x7) << 28)

#define EMMC_CARD_RD_THR(x)                   ((x & 0xfff) << 16)
#define EMMC_CARD_RD_THR_EN                   (1 << 0)

#define EMMC_GET_HDATA_WIDTH(x)               (((x) >> 7) & 0x7)

//
//FIFOTH register defines
//
#define EMMC_SET_FIFOTH(M, R, T)    (((M) & 0x7) << 28 | ((R) & 0xFFF) << 16 | ((T) & 0xFFF))

#define MAX_BD_NUM                    0x10000
#define EMMC_DESC_PAGE                1
#define EMMC_BLOCK_SIZE               512
#define EMMC_DMA_BUF_SIZE             (4096)
#define EMMC_MAX_DESC_PAGES           512
#define EMMC_BASE_CLK_RATE_HZ         (1200 * 1000 * 1000)

#define EMMC_TIMEOUT_INTERNAL         500
#define EMMC_CMD_TIMEOUT              (5 * 2000 * EMMC_TIMEOUT_INTERNAL)
#define EMMC_DATA_TIMEOUT             (10 * 2000 * EMMC_TIMEOUT_INTERNAL)

//
// DMA descriptor struct
//
typedef struct {
  UINT32                        Des0;
  UINT32                        Des1;
  UINT32                        Des2;
  UINT32                        Des3;
  UINT32                        Des4;
  UINT32                        Des5;
  UINT32                        Des6;
  UINT32                        Des7;
} EMMC_IDMAC_DESCRIPTOR;

//
// eMMC base information struct
//
typedef struct {
  UINT32  MaxSegs;
  UINT32  MaxSegSize;
  UINT32  MaxBlkSize;
  UINT32  MaxReqSize;
  UINT32  MaxBlkCount;
} EMMC_HOST;

/**
  Send a command to card and wait for response.

  @param[in]  MmcCmd    Command.
  @param[in]  Argument  Argument.

  @retval     EFI_SUCCESS       Command send successfully.
  @retval     EFI_TIMEOUT       Comnand response timeout.
  @retval     EFI_DEVICE_ERROR  Generate error interrupt status.
**/
EFI_STATUS
SendCommand (
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  );

/**
  Send a command to card and not need to wait for response.

  @param[in]  MmcCmd    Command.
  @param[in]  Argument  Argument.

  @retval     EFI_SUCCESS       Command send successfully.

**/
EFI_STATUS
SendCommandInternal (
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  );

#endif
