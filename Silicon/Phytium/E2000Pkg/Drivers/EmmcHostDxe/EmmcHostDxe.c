/** @file
Phytium eMMC host controller driver header. Only a single fixed eMMC
controller is supported, and the controller base address is specified by PCD.
The driver is the controller layer driver, and the upper layer is the mmcsd
protocol(BlockIo) layer.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright (c) 2014-2017, Linaro Limited. All rights reserved.
Copyright (c) 2011-2013, ARM Limited. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "EmmcHost.h"

EFI_MMC_HOST_PROTOCOL                 *gpMmcHost;
EMMC_IDMAC_DESCRIPTOR                 *gpIdmacDesc;
EMMC_HOST                             mEmmcHostInfo;
EFI_GUID gPhytiumEmmcDevicePathGuid = EFI_CALLER_ID_GUID;
STATIC UINT32                         mEmmcCommand;
STATIC UINT32                         mEmmcArgument;

/**
  Judge whether emmc is in power on state. Return TEUR forever.

  @retval    TRUE
**/
BOOLEAN
PhytiumEmmcIsPowerOn (
  VOID
  )
{
  return TRUE;
}

/**
  Set some specified bit of register to 1.

  @param[in]  Address    Register address to modify.
  @param[in]  Bs         Bits to be set to 1.
**/
STATIC
VOID
SetRegBits (
  IN UINT64  Address,
  IN UINT32  Bs
  )
{
  UINT32  Value;

  Value = MmioRead32 (Address);
  Value |= Bs;
  MmioWrite32 (Address, Value);
}

/**
  Clear some specified bit of register to 0.

  @param[in]  Address    Register address to modify.
  @param[in]  Bs         Bits to be clear to 0.
**/
STATIC
VOID
ClrRegBits (
  IN UINT64  Address,
  IN UINT32  Bs
  )
{
  UINT32  Value;

  Value = MmioRead32 (Address);
  Value &= ~Bs;
  MmioWrite32 (Address, Value);
}

/**
  Configure the UHS extension(UhsRegExt 0x108) register uhsregext. Configure
  the frequency division parameters. 20MHz - 0x302, 25MHz - 0x202, 50MHz - 0x202,
  and 400KHz - 0x502. The clock configuration is completed after the CIU clock
  is ready. If it is detected that the CIU clock cannot always be ready, it is
  stuck.

  @param[in]  Value    Value to configure.
**/
STATIC
VOID
PhytiumEmmcUpdateExtClk (
  IN UINT32  Value
  )
{
  MmioWrite32 (EMMC_UHS_REG_EXT, 0);
  MmioWrite32 (EMMC_UHS_REG_EXT, Value);
  while (!(MmioRead32 (EMMC_CCLK_READY) & 0x1));
}

/**
  Reset dma.
**/
STATIC
VOID
PhytiumEmmcDmaReset (
  VOID
  )
{
  MmioWrite32 (EMMC_BMOD, MmioRead32 (EMMC_BMOD) | EMMC_BUS_MODE_SWR);
}

/**
  Initialize DMA descriptor list. Configure the address of the next descriptor
  corresponding to each descriptor. The parameter 0 of each descriptor is
  configured, and the last descriptor is specially processed. The descriptor
  list will be modified according to the buffer address and the number of bytes
  actually transferred before DMA transfer.

  @param[in]  DescTable    A pointer to EMMC_IDMAC_DESCRIPTOR struct, DMA
                           descriptor list.
**/
STATIC
VOID
PhytiumEmmcInitDesTable (
  IN EMMC_IDMAC_DESCRIPTOR  *DescTable
  )
{
  UINT32                 Index;
  UINT64                 DmaAddr;
  EMMC_IDMAC_DESCRIPTOR  *Desc;

  Desc = DescTable;
  ZeroMem (DescTable, MAX_BD_NUM * sizeof (EMMC_IDMAC_DESCRIPTOR));
  for (Index = 0; Index < (MAX_BD_NUM - 1); Index++) {
    DmaAddr = (UINT64)DescTable + sizeof (EMMC_IDMAC_DESCRIPTOR) * (Index + 1);
    Desc[Index].Des0 = 0;
    Desc[Index].Des1 = 0;
    Desc[Index].Des2 = 0;
    Desc[Index].Des3 = 0;
    Desc[Index].Des6 = (UINT32) DmaAddr;
    Desc[Index].Des7 = (UINT32) (DmaAddr >> 32);
  }
  DmaAddr = (UINT64)DescTable;
  Desc[MAX_BD_NUM-1].Des0 = 0;
  Desc[MAX_BD_NUM-1].Des1 = 0;
  Desc[MAX_BD_NUM-1].Des2 = 0;
  Desc[MAX_BD_NUM-1].Des3 = 0;
  Desc[MAX_BD_NUM-1].Des6 = (UINT32) DmaAddr;
  Desc[MAX_BD_NUM-1].Des7 = (UINT32) (DmaAddr >> 32);
}

/**
  Reset the eMMC controller, and send a command to stop the clock. If the
  completion of FIFO and DMA reset is not detected, it is stuck.
**/
STATIC
VOID
PhytiumEmmcResetHw (
  VOID
  )
{
  SetRegBits (EMMC_CTRL, EMMC_CNTRL_FIFO_RESET |EMMC_CNTRL_DMA_RESET);
  while (MmioRead32 (EMMC_CTRL) & (EMMC_CNTRL_FIFO_RESET | EMMC_CNTRL_DMA_RESET));
  SendCommandInternal (BIT_CMD_UPDATE_CLOCK_ONLY, 0);
}

/**
  EMMC controller intialization.
  1.FIFO and card detection configuration.
  2.Clock configuration. The protocol requires the initial clock to be 400KHz.
  3.Reset.
  4.Interrupt configuration.

  @retval    EFI_SICCESS    Success.
**/
STATIC
EFI_STATUS
PhytiumEmmcInitialize (
  VOID
  )
{
  DEBUG ((DEBUG_BLKIO, "%a()\n", __FUNCTION__));
  //
  //step 1: FIFO and card detection configuration
  //
  MmioWrite32 (EMMC_FIFOTH, EMMC_SET_FIFOTH(0x2, 0x7, 0x100));
  MmioWrite32 (EMMC_CARDTHRCTL, 0x800001);
  //
  //step 2
  //Clock configuration. The protocol requires the initial clock to be 400KHz.
  //
  ClrRegBits (EMMC_CLKENA, EMMC_CLKENA_CCLK_ENABLE);
  PhytiumEmmcUpdateExtClk (0x502);

  SetRegBits (EMMC_PWREN, EMMC_PWREN_ENABLE);
  SetRegBits (EMMC_CLKENA, EMMC_CLKENA_CCLK_ENABLE);
  SetRegBits (EMMC_UHS_REG_EXT, EMMC_EXT_CLK_ENABLE);
  ClrRegBits (EMMC_UHSREG, EMMC_UHS_REG_VOLT);
  //
  //step 3 : reset.
  //
  PhytiumEmmcResetHw ();
  //
  //step 4 : Interrupt configuration.
  //
  //
  //nonremovable
  //
  SetRegBits (EMMC_CARD_RESET, EMMC_CARD_RESET_ENABLE);
  //ClrRegBits (EMMC_CARD_RESET, EMMC_CARD_RESET_ENABLE);
  MmioWrite32 (EMMC_INTMASK, 0);
  MmioWrite32 (EMMC_RINTSTS, MmioRead32 (EMMC_RINTSTS));
  MmioWrite32 (EMMC_INTR_EN, 0);
  MmioWrite32 (EMMC_DMAC_STATUS, MmioRead32 (EMMC_DMAC_STATUS));
  SetRegBits (EMMC_CTRL, EMMC_CNTRL_INT_ENABLE | EMMC_CNTRL_USE_INTERNAL_DMAC);
  MmioWrite32 (EMMC_TMOUT, 0xFFFFFFFF);
  DEBUG ((DEBUG_INFO, "Emmc hadrware init done\n"));

  return EFI_SUCCESS;
}

/**
  Card present detection. It cannot be removed defualt and return true.

  @param[in]  This    A pointer to EFI_MMC_HOST_PROTOCOL.

  @retval     TRUE    Present.
  @retval     FALSE   Not present.
**/
BOOLEAN
PhytiumEmmcIsCardPresent (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  //
  //if nonremovable, return true;
  //
  return TRUE;
}

/**
  Check whether the card is read-only. The default is not read-only.

  @param[in]  This    A pointer to EFI_MMC_HOST_PROTOCOL.

  @retval     TRUE    Read-only.
  @retval     FALSE   Not read-only.
**/
BOOLEAN
PhytiumEmmcIsReadOnly (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  return FALSE;
}

/**
  Check whether the controller supports DMA function. Default support.

  @param[in]  This    A pointer to EFI_MMC_HOST_PROTOCOL.

  @retval     TRUE    Support DMA.
  @retval     FALSE   Not support DMA.
**/
BOOLEAN
PhytiumEmmcIsDmaSupported (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  return TRUE;
}

/**
  Build device path for eMMC controller.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  DevicePath  A pointer to EFI_DEVICE_PATH_PROTOCOL. Device path
                          after building.

  @retval     EFI_SUCCESS    Success.
**/
EFI_STATUS
PhytiumEmmcBuildDevicePath (
  IN EFI_MMC_HOST_PROTOCOL      *This,
  IN EFI_DEVICE_PATH_PROTOCOL   **DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL *NewDevicePathNode;

  NewDevicePathNode = CreateDeviceNode (HARDWARE_DEVICE_PATH, HW_VENDOR_DP, sizeof (VENDOR_DEVICE_PATH));
  CopyGuid (& ((VENDOR_DEVICE_PATH*)NewDevicePathNode)->Guid, &gPhytiumEmmcDevicePathGuid);

  *DevicePath = NewDevicePathNode;
  return EFI_SUCCESS;
}

/**
  EMMC controller clock configuration. The clock parameter only support 50MHz,
  25MHz, 20MHz, 400KHz. When the clock is 0, stop the clock.
  1.Update ext clk.
  2.Stop clock through setting EMMC_CLKENA register and sending UPDATE_CLOCK
    command.
  3.Calculate the frequency division value and configurate EMMC_CLKDIV register.
  4.Enable clock.

  @param[in]  ClockFreq    Clock frequence, Hz unit.

  @retval     EFI_SUCCESS  Success.
**/
STATIC
EFI_STATUS
PhytiumEmmcSetClock (
  IN UINTN                     ClockFreq
  )
{
  UINT32  Div;
  UINT32  Drv;
  UINT32  Sample;
  UINT64  ClkRate;
  UINT32  MciCmdBits;
  UINT32  FirstUhsDiv;
  UINT32  TmpExtReg;

  Div = 0xFF;
  Drv = 0;
  Sample = 0;
  MciCmdBits = BIT_CMD_UPDATE_CLOCK_ONLY;


  if (ClockFreq) {
    if (ClockFreq >= 25000000) {
      TmpExtReg = 0x202;
    } else if (ClockFreq == 400000) {
      TmpExtReg = 0x502;
    } else {
      TmpExtReg = 0x302;
    }
    PhytiumEmmcUpdateExtClk (TmpExtReg);
    ClrRegBits (EMMC_CLKENA, EMMC_CLKENA_CCLK_ENABLE);
    SendCommandInternal (MciCmdBits, 0);

    ClkRate = EMMC_BASE_CLK_RATE_HZ;
    FirstUhsDiv = 1 + ((TmpExtReg >> 8) & 0xFF);
    Div = ClkRate / (2 * FirstUhsDiv * ClockFreq);
    if (Div > 2) {
      Sample = Div / 2 + 1;
      Drv = Sample - 1;
      MmioWrite32 (EMMC_CLKDIV, (Drv << 8) | (Sample << 16) | (Div & 0xFF));
    } else {
      Drv = 0;
      Sample = 1;
      MmioWrite32 (EMMC_CLKDIV, (Drv << 8) | (Sample << 16) | (Div & 0xFF));
    }
    SetRegBits (EMMC_CLKENA, EMMC_CLKENA_CCLK_ENABLE);
    SendCommandInternal (MciCmdBits, 0);
  } else {
    ClrRegBits (EMMC_CLKENA, EMMC_CLKENA_CCLK_ENABLE);
    ClrRegBits (EMMC_UHS_REG_EXT, EMMC_EXT_CLK_ENABLE);
  }

  return EFI_SUCCESS;
}

/**
  Notify the eMMC controller to operate according to different states. See
  MMC_STATE enumeration for supported states.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  State       Notify state.

  @retval     EFI_SUCCESS            Success.
  @retval     EFI_INVALID_PARAMETER  State is not supported.

**/
EFI_STATUS
PhytiumEmmcNotifyState (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_STATE                 State
  )
{
  switch (State) {
  case MmcInvalidState:
    return EFI_INVALID_PARAMETER;
  case MmcHwInitializationState:
    PhytiumEmmcInitialize ();
    PhytiumEmmcSetClock (400000);
    break;
  case MmcIdleState:
    break;
  case MmcReadyState:
    break;
  case MmcIdentificationState:
    break;
  case MmcStandByState:
    break;
  case MmcTransferState:
    break;
  case MmcSendingDataState:
    break;
  case MmcReceiveDataState:
    break;
  case MmcProgrammingState:
    break;
  case MmcDisconnectState:
    break;
  default:
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

/**
  Check whether the current command to be sent is a read command. If it is read
  command, need to prepare DMA buffer first before sending commands to MMC card

  @param[in]  MmcCmd    The command to sent.

  @retval     TRUE      Read command.
  @retval     FALSE     Not Read command.
**/
BOOLEAN
IsPendingReadCommand (
  IN MMC_CMD                    MmcCmd
  )
{
  UINTN  Mask;

  Mask = BIT_CMD_DATA_EXPECTED | BIT_CMD_READ;
  if ((MmcCmd & Mask) == Mask) {
    return TRUE;
  }
  return FALSE;
}

/**
  Check whether the current command to be sent is a read command. If it is write
  command, need to prepare DMA buffer first before sending commands to MMC card

  @param[in]  MmcCmd    The command to sent.

  @retval     TRUE      Write command.
  @retval     FALSE     Not write command.
**/
BOOLEAN
IsPendingWriteCommand (
  IN MMC_CMD                    MmcCmd
  )
{
  UINTN  Mask;

  Mask = BIT_CMD_DATA_EXPECTED | BIT_CMD_WRITE;
  if ((MmcCmd & Mask) == Mask) {
    return TRUE;
  }
  return FALSE;
}

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
  )
{
  UINT32      Data;
  UINT32      ErrMask;
  UINT32      TimeOut;

  TimeOut = EMMC_CMD_TIMEOUT / EMMC_TIMEOUT_INTERNAL;
  //
  // Wait until MMC is idle
  //
  do {
    Data = MmioRead32 (EMMC_STATUS);
  } while (Data & EMMC_STS_DATA_BUSY);
  //DEBUG ((DEBUG_INFO, "MmcCmd : %x, Argument : %x\n", MmcCmd, Argument));
  MmioWrite32 (EMMC_RINTSTS, 0xffffe);
  MmioWrite32 (EMMC_CMDARG, Argument);
  while (MmioRead32 (EMMC_STATUS) & EMMC_STATUS_CARD_BUSY);
  MmioWrite32 (EMMC_CMD, MmcCmd);
  while (MmioRead32 (EMMC_CMD) & BIT_CMD_START);

  ErrMask = EMMC_INT_EBE | EMMC_INT_HLE | EMMC_INT_RTO |
            EMMC_INT_RCRC | EMMC_INT_RE;
  ErrMask |= EMMC_INT_DCRC | EMMC_INT_DRT | EMMC_INT_SBE;
  SetRegBits (EMMC_INTMASK, ErrMask);
  do {
    MicroSecondDelay (EMMC_TIMEOUT_INTERNAL);
    Data = MmioRead32 (EMMC_RINTSTS);
    //DEBUG ((DEBUG_INFO, "CMD STATUS : %x\n", Data));
    //DEBUG ((DEBUG_INFO, "STATUS : %x\n", MmioRead32 (EMMC_STATUS)));
    if (Data & ErrMask) {
      return EFI_DEVICE_ERROR;
    }
    if (Data & EMMC_INT_DTO) {     // Transfer Done
      break;
    }
    TimeOut--;
    if (TimeOut == 0) {
      return EFI_TIMEOUT;
    }
  } while (!(Data & EMMC_INT_CMD_DONE));
  return EFI_SUCCESS;
}

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
  )
{
  MmioWrite32 (EMMC_CMDARG, Argument);
  while (MmioRead32 (EMMC_STATUS) & EMMC_STATUS_CARD_BUSY);
  MmioWrite32 (EMMC_CMD, MmcCmd | BIT_CMD_START);
  while (MmioRead32 (EMMC_CMD) & BIT_CMD_START);

  return EFI_SUCCESS;
}

/**
  Send a command with a specific command number to the card. If it is not a
  read/write command, it is sent directly. If is a read/write command, only
  need to keep the command to the global variable.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  MmcCmd      Command ID to card.
  @param[in]  Argument    Argument to card.

  @retval     EFI_SUCCESS       Command send successfully.
  @retval     EFI_TIMEOUT       Comnand response timeout.
  @retval     EFI_DEVICE_ERROR  Generate error interrupt status.
**/
EFI_STATUS
PhytiumEmmcSendCommand (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  )
{
  UINT32       Cmd;
  EFI_STATUS   Status;

  Cmd = 0;
  //DEBUG ((DEBUG_INFO, "Cmd : %d\n", MMC_GET_INDX(MmcCmd)));
  Status = EFI_SUCCESS;
  switch (MMC_GET_INDX(MmcCmd)) {
  case MMC_INDX(0):
    Cmd = BIT_CMD_SEND_INIT;
    break;
  case MMC_INDX(1):
    Cmd = BIT_CMD_RESPONSE_EXPECT;
    break;
  case MMC_INDX(2):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_LONG_RESPONSE |
           BIT_CMD_CHECK_RESPONSE_CRC | BIT_CMD_SEND_INIT;
    break;
  case MMC_INDX(3):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_SEND_INIT;
    break;
  case MMC_INDX(6):
    Cmd |= BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  case MMC_INDX(7):
    if (Argument)
        Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC;
    else
        Cmd = 0;
    break;
  case MMC_INDX(8):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
          //BIT_CMD_DATA_EXPECTED | BIT_CMD_READ |
          BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  case MMC_INDX(9):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_LONG_RESPONSE;
    break;
  case MMC_INDX(12):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_STOP_ABORT_CMD;
    break;
  case MMC_INDX(13):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  case MMC_INDX(16):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_DATA_EXPECTED | BIT_CMD_READ |
           BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  case MMC_INDX(17):
  case MMC_INDX(18):
    //Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
    //       BIT_CMD_DATA_EXPECTED | BIT_CMD_READ |
    //       BIT_CMD_USE_HOLD_REG;
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_DATA_EXPECTED | BIT_CMD_READ |
           BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  case MMC_INDX(24):
  case MMC_INDX(25):
    Cmd =  BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_DATA_EXPECTED | BIT_CMD_WRITE |
           BIT_CMD_USE_HOLD_REG;
    break;
  case MMC_INDX(30):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_DATA_EXPECTED;
    break;
  case MMC_INDX(41):
    Cmd = BIT_CMD_RESPONSE_EXPECT;
    break;
  case MMC_INDX(51):
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC |
           BIT_CMD_DATA_EXPECTED | BIT_CMD_READ |
           BIT_CMD_WAIT_PRVDATA_COMPLETE;
    break;
  default:
    Cmd = BIT_CMD_RESPONSE_EXPECT | BIT_CMD_CHECK_RESPONSE_CRC;
    break;
  }

  Cmd |= MmcCmd | BIT_CMD_USE_HOLD_REG | BIT_CMD_START;
  //DEBUG ((DEBUG_INFO, "Cmd Finish : 0x%x\n", Cmd));
  //DEBUG ((DEBUG_INFO, "Arg Finish : 0x%x\n", Argument));
  if (IsPendingReadCommand (Cmd) || IsPendingWriteCommand (Cmd)) {
    mEmmcCommand = Cmd;
    mEmmcArgument = Argument;
  } else {
    Status = SendCommand (Cmd, Argument);
  }
  return Status;
}

/**
  Get the reponse status of the protocol command according to the response type.
  R1, R1b, R3, R6, R7, or R2.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  Type        Response type.
  @param[in]  Buffer      A pointer to response result.

  @retval     EFI_SUCCESS    Get response successfully.
**/
EFI_STATUS
PhytiumEmmcReceiveResponse (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_RESPONSE_TYPE         Type,
  IN UINT32                    *Buffer
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (   (Type == MMC_RESPONSE_TYPE_R1)
      || (Type == MMC_RESPONSE_TYPE_R1b)
      || (Type == MMC_RESPONSE_TYPE_R3)
      || (Type == MMC_RESPONSE_TYPE_R6)
      || (Type == MMC_RESPONSE_TYPE_R7))
  {
    Buffer[0] = MmioRead32 (EMMC_RESP0);
  } else if (Type == MMC_RESPONSE_TYPE_R2) {
    Buffer[0] = MmioRead32 (EMMC_RESP0);
    Buffer[1] = MmioRead32 (EMMC_RESP1);
    Buffer[2] = MmioRead32 (EMMC_RESP2);
    Buffer[3] = MmioRead32 (EMMC_RESP3);
  }
  //DEBUG ((DEBUG_INFO, "Resp0: %x, Resp1 : %x, Resp2 : %x, Resp3 : %x\n",
  //          MmioRead32 (EMMC_RESP0),  MmioRead32 (EMMC_RESP1),
  //          MmioRead32 (EMMC_RESP2), MmioRead32 (EMMC_RESP3)));
  return EFI_SUCCESS;
}

/**
  PrePare the DMA descriptor for the next data transfer.

  @param[in]  IdmacDesc    A pointer to EMMC_IDMAC_DESCRIPTOR. The descriptor
                           list to transfer.
  @param[in]  Length       The length of buffer to transfer.
  @param[in]  Buffer       The buffer to transfer.

  @retval     EFI_SUCCESS  Success.
**/
EFI_STATUS
PrepareDmaData (
  IN EMMC_IDMAC_DESCRIPTOR    *IdmacDesc,
  IN UINTN                    Length,
  IN UINT8                    *Buffer
  )
{
  UINTN  Cnt;
  UINTN  Blks;
  UINTN  Idx;
  UINTN  LastIdx;

  Cnt = (Length + EMMC_DMA_BUF_SIZE - 1) / EMMC_DMA_BUF_SIZE;
  Blks = (Length + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
  Length = EMMC_BLOCK_SIZE * Blks;

  //DEBUG ((DEBUG_INFO, "Cnt : %d\n", Cnt));
  for (Idx = 0; Idx < Cnt; Idx++) {
    (IdmacDesc + Idx)->Des0 = EMMC_IDMAC_DES0_OWN | EMMC_IDMAC_DES0_CH |
                              EMMC_IDMAC_DES0_DIC;
    (IdmacDesc + Idx)->Des2 = EMMC_IDMAC_DES1_BS1(EMMC_DMA_BUF_SIZE);
    //
    //Buffer Address
    //
    (IdmacDesc + Idx)->Des4 = (UINT32)((UINTN)Buffer + EMMC_DMA_BUF_SIZE * Idx);
    (IdmacDesc + Idx)->Des5 = (UINT32)(((UINTN)Buffer + EMMC_DMA_BUF_SIZE * Idx) >> 32 );
    //
    //Next Descriptor Address
    //
    (IdmacDesc + Idx)->Des6 = (UINT32)((UINTN)IdmacDesc +
                                       (sizeof(EMMC_IDMAC_DESCRIPTOR) * (Idx + 1)));
    (IdmacDesc + Idx)->Des7 = (UINT32)(((UINTN)IdmacDesc +
                                       (sizeof(EMMC_IDMAC_DESCRIPTOR) * (Idx + 1))) >> 32);
    //DEBUG ((DEBUG_INFO, "Desc[%d]\n", Idx));
    //DEBUG ((DEBUG_INFO, "0 : %x\n", (IdmacDesc + Idx)->Des0));
    //DEBUG ((DEBUG_INFO, "1 : %x\n", (IdmacDesc + Idx)->Des1));
    //DEBUG ((DEBUG_INFO, "2 : %x\n", (IdmacDesc + Idx)->Des2));
    //DEBUG ((DEBUG_INFO, "3 : %x\n", (IdmacDesc + Idx)->Des3));
    //DEBUG ((DEBUG_INFO, "4 : %x\n", (IdmacDesc + Idx)->Des4));
    //DEBUG ((DEBUG_INFO, "5 : %x\n", (IdmacDesc + Idx)->Des5));
    //DEBUG ((DEBUG_INFO, "6 : %x\n", (IdmacDesc + Idx)->Des6));
    //DEBUG ((DEBUG_INFO, "7 : %x\n", (IdmacDesc + Idx)->Des7));
  }
  //
  //First Descriptor
  //
  IdmacDesc->Des0 |= EMMC_IDMAC_DES0_FS;
  //
  //Last Descriptor
  //
  LastIdx = Cnt - 1;
  (IdmacDesc + LastIdx)->Des0 |= EMMC_IDMAC_DES0_LD | EMMC_IDMAC_DES0_ER;
  (IdmacDesc + LastIdx)->Des0 &= ~(EMMC_IDMAC_DES0_DIC | EMMC_IDMAC_DES0_CH);
  (IdmacDesc + LastIdx)->Des2 = EMMC_IDMAC_DES1_BS1(Length - (LastIdx * EMMC_DMA_BUF_SIZE));
  //
  //Set the Next field of Last Descripto
  //
  (IdmacDesc + LastIdx)->Des6 = 0;
  (IdmacDesc + LastIdx)->Des7 = 0;
  //DEBUG ((DEBUG_INFO, "Desc[%d]\n", LastIdx));
  //DEBUG ((DEBUG_INFO, "0 : %x\n", (IdmacDesc + LastIdx)->Des0));
  //DEBUG ((DEBUG_INFO, "1 : %x\n", (IdmacDesc + LastIdx)->Des1));
  //DEBUG ((DEBUG_INFO, "2 : %x\n", (IdmacDesc + LastIdx)->Des2));
  //DEBUG ((DEBUG_INFO, "3 : %x\n", (IdmacDesc + LastIdx)->Des3));
  //DEBUG ((DEBUG_INFO, "4 : %x\n", (IdmacDesc + LastIdx)->Des4));
  //DEBUG ((DEBUG_INFO, "5 : %x\n", (IdmacDesc + LastIdx)->Des5));
  //DEBUG ((DEBUG_INFO, "6 : %x\n", (IdmacDesc + LastIdx)->Des6));
  //DEBUG ((DEBUG_INFO, "7 : %x\n", (IdmacDesc + LastIdx)->Des7));
  MmioWrite32 (EMMC_DESC_LIST_L, (UINT32)((UINTN) IdmacDesc));
  MmioWrite32 (EMMC_DESC_LIST_H, (UINT32)(((UINTN) IdmacDesc) >> 32));
  //
  //reset dma
  //
  PhytiumEmmcDmaReset ();

  return EFI_SUCCESS;
}

/**
  Wait for DMA transfer to complete.

  @retval    EFI_SUCCESS    DMA transfer complete.
  @retval    EFI_TIMEOUT    DMA transfer timeout.
**/
STATIC
EFI_STATUS
WaitDmaComplete (
  VOID
  )
{
  UINT32  Status;
  UINT32  TimeOut;

  TimeOut = EMMC_CMD_TIMEOUT / EMMC_TIMEOUT_INTERNAL;
  while (1) {
    MicroSecondDelay (EMMC_TIMEOUT_INTERNAL);
    Status = MmioRead32 (EMMC_RINTSTS);
    //DEBUG ((DEBUG_INFO, "%a(), Status : %x\n", __FUNCTION__, Status));
    Status = (Status >> 3) & 0x1;
    if (Status != 0 ) {
      break;
    }
    TimeOut--;
    if (TimeOut == 0) {
      DEBUG ((DEBUG_ERROR, "Dma transfer timeout!\n"));
      return EFI_TIMEOUT;
    }
  }
  return EFI_SUCCESS;
}

/**
  Start DMA transfer.

  @param  Length    Bytes of data to transfer.

  @retval    Null.
**/
STATIC
VOID
StartDma (
  UINTN    Length
  )
{
  UINT32 Data;
  Data = MmioRead32 (EMMC_CTRL);
  //Data |= EMMC_CTRL_INT_EN | EMMC_CTRL_DMA_EN | EMMC_CTRL_IDMAC_EN;
  Data |= EMMC_CTRL_INT_EN  | EMMC_CTRL_IDMAC_EN;
  MmioWrite32 (EMMC_CTRL, Data);
  Data = MmioRead32 (EMMC_BMOD);
  //Data |= EMMC_IDMAC_ENABLE | EMMC_IDMAC_FB;
  Data |= EMMC_IDMAC_ENABLE;
  MmioWrite32 (EMMC_BMOD, Data);

  MmioWrite32 (EMMC_BLKSIZ, EMMC_BLOCK_SIZE);
  MmioWrite32 (EMMC_BYTCNT, Length);
}

/**
  Read the data of the specified length(bytes) of the specified block of the
  card. Default block size is 512 bytes.

  @param[in]      This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]      Lba         Start Lba number to read.
  @param[in]      Length      Bytes to read.
  @param[in,out]  Buffer      Buffer after reading.

  @retval     EFI_SUCCESS       Read data successfully.
  @retval     EFI_TIMEOUT       Read data timeout.
  @retval     EFI_DEVICE_ERROR  Read data error.
**/
EFI_STATUS
PhytiumEmmcReadBlockData (
  IN     EFI_MMC_HOST_PROTOCOL     *This,
  IN     EFI_LBA                   Lba,
  IN     UINTN                     Length,
  IN OUT UINT32                    *Buffer
  )
{
  EFI_STATUS  Status;
  EFI_TPL     Tpl;
  UINT8       *Temp;

  Tpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  //Allocate Dma Buffer
  //
  Temp = (UINT8 *) AllocatePages (EFI_SIZE_TO_PAGES (Length));
  if (Temp == NULL) {
    DEBUG ((DEBUG_ERROR, "Failed to allocate pages for read data!\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto out;
  }
  Status = PrepareDmaData (gpIdmacDesc, Length, Temp);
  if (EFI_ERROR (Status)) {
    goto out;
  }
  StartDma (Length);
  Status = SendCommand (mEmmcCommand, mEmmcArgument);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "Failed to read data, mEmmcCommand:%x, mEmmcArgument:%x,Status:%r\n",
            mEmmcCommand, mEmmcArgument, Status));
    goto out;
  }
  Status = WaitDmaComplete ();
  if (EFI_ERROR (Status)) {
    goto out;
  }
  CopyMem ((VOID*) Buffer, Temp, Length);
out:
  if (Temp != NULL) {
    FreePages (Temp, EFI_SIZE_TO_PAGES (Length));
  }
  // Restore Tpl
  gBS->RestoreTPL (Tpl);
  return Status;
}

/**
  Write the data of the specified length(bytes) to the specified block of the
  card. Default block size is 512 bytes.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  Lba         Start Lba number to write.
  @param[in]  Length      Bytes to write.
  @param[in]  Buffer      Data to write.

  @retval     EFI_SUCCESS       Write data successfully.
  @retval     EFI_TIMEOUT       Write data timeout.
  @retval     EFI_DEVICE_ERROR  Write data error.

**/
EFI_STATUS
PhytiumEmmcWriteBlockData (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN EFI_LBA                    Lba,
  IN UINTN                      Length,
  IN UINT32*                    Buffer
  )
{
  EFI_STATUS  Status;
  EFI_TPL     Tpl;
  UINT8       *Temp;

  Tpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  //Allocate Dma Buffer
  //
  Temp = (UINT8 *) AllocatePages (EFI_SIZE_TO_PAGES (Length));
  if (Temp == NULL) {
    DEBUG ((DEBUG_ERROR, "Failed to allocate pages for read data!\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto out;
  }
  CopyMem (Temp, (VOID*) Buffer, Length);
  Status = PrepareDmaData (gpIdmacDesc, Length, Temp);
  if (EFI_ERROR (Status)) {
    goto out;
  }

  StartDma (Length);

  Status = SendCommand (mEmmcCommand, mEmmcArgument);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
            "Failed to write data, mEmmcCommand:%x, mEmmcArgument:%x, Status:%r\n",
            mEmmcCommand, mEmmcArgument, Status));
    goto out;
  }
  Status = WaitDmaComplete ();
  if (EFI_ERROR (Status)) {
    goto out;
  }

out:
  if (Temp != NULL) {
    FreePages (Temp, EFI_SIZE_TO_PAGES (Length));
  }
  // Restore Tpl
  gBS->RestoreTPL (Tpl);
  return Status;
}

/**
  Set eMMC controller clock, bus width, and timing mode (only support sdr).

  @param[in]  This            A pointer to EFI_MMC_HOST_PROTOCOL.
  @param[in]  BusClockFreq    Bus clock, only support 50MHz, 25MHz, 400KHz and 0.
  @param[in]  BusWidth        Bus width, 1, 4 or 8.
  @param[in]  TimingMode      Timing mode. Whatever the parameter, it will be
                              configured as SDR mode.

  @retval     EFI_SUCCESS  Success.
**/
EFI_STATUS
PhytiumEmmcSetIos (
  IN  EFI_MMC_HOST_PROTOCOL     *This,
  IN  UINT32                    BusClockFreq,
  IN  UINT32                    BusWidth,
  IN  UINT32                    TimingMode
  )
{
  EFI_STATUS Status;
  UINT32    Data;

  DEBUG ((DEBUG_INFO, "BusClockFreq : %d, BusWidth : %d, TimingMode : %d\n",
              BusClockFreq, BusWidth, TimingMode));

  Status = EFI_SUCCESS;
  //DEBUG ((DEBUG_INFO, "Clk : %d, BusWidth : %d, Time : %d\n", BusClockFreq, BusWidth, TimingMode));
  Data = MmioRead32 (EMMC_UHSREG);
  Data &= ~(1 << 16);
  MmioWrite32 (EMMC_UHSREG, Data);

  switch (BusWidth) {
  case 1:
    MmioWrite32 (EMMC_CTYPE, 0);
    break;
  case 4:
    MmioWrite32 (EMMC_CTYPE, 1);
    break;
  case 8:
    MmioWrite32 (EMMC_CTYPE, 1 << 16);
    break;
  default:
    return EFI_UNSUPPORTED;
  }
  if (BusClockFreq) {
    Status = PhytiumEmmcSetClock (BusClockFreq);
  }
  return Status;
}

/**
  Check whether eMMC controller supports multi block operation.
  Default support.

  @param[in]  This        A pointer to EFI_MMC_HOST_PROTOCOL.

  @retval     TRUE        Supported.
  @retval     FALSE       Not supported.
**/
BOOLEAN
PhytiumEmmcIsMultiBlock (
  IN EFI_MMC_HOST_PROTOCOL      *This
  )
{
  return TRUE;
}

EFI_MMC_HOST_PROTOCOL gMciHost = {
  MMC_HOST_PROTOCOL_REVISION,
  PhytiumEmmcIsCardPresent,
  PhytiumEmmcIsReadOnly,
  PhytiumEmmcBuildDevicePath,
  PhytiumEmmcNotifyState,
  PhytiumEmmcSendCommand,
  PhytiumEmmcReceiveResponse,
  PhytiumEmmcReadBlockData,
  PhytiumEmmcWriteBlockData,
  PhytiumEmmcSetIos,
  PhytiumEmmcIsMultiBlock
};

/**
  This function is the entry point of the eMMC host controller driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.
**/
EFI_STATUS
PhytiumEmmcDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    Handle;

  ZeroMem (&mEmmcHostInfo, sizeof (EMMC_HOST));
  DEBUG ((DEBUG_INFO, "Base Addr : %x\n", PcdGet64(PcdEmmcDxeBaseAddress)));
  //
  //Init descriptor
  //
  gpIdmacDesc = (EMMC_IDMAC_DESCRIPTOR *) AllocatePages
      (EFI_SIZE_TO_PAGES (MAX_BD_NUM * sizeof (EMMC_IDMAC_DESCRIPTOR)));
  if (gpIdmacDesc == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }
  PhytiumEmmcInitDesTable (gpIdmacDesc);
  //PhytiumEmmcInitialize ();
  mEmmcHostInfo.MaxSegs = MAX_BD_NUM;
  mEmmcHostInfo.MaxSegSize = 4 * 1024;
  mEmmcHostInfo.MaxBlkSize = 512;
  mEmmcHostInfo.MaxReqSize = 512 * 1024;
  mEmmcHostInfo.MaxBlkCount = mEmmcHostInfo.MaxReqSize / 512;

  Handle = NULL;

  //
  //Publish Component Name, BlockIO protocol interfaces
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gPhytiumMmcHostProtocolGuid,
                  &gMciHost,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
