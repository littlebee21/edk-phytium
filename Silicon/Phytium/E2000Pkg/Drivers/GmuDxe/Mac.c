/* @file
** Mac function.
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Uefi.h>

#include "Mac.h"
#include "Snp.h"

/**
  Write phy register through MDIO interface.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyAddr   Phy address.
  @param[in] Reg       Phy register.
  @param[in] Value     Value to write

  @retval    NULL
**/
VOID
MacMdioWrite (
  IN MAC_DEVICE  *Mac,
  IN UINT16      PhyAddr,
  IN UINT8       Reg,
  IN UINT16      Value
  )
{
  UINT64 Netctl;
  UINT64 Netstat;
  UINT64 Frame;

  Netctl = MacRead(Mac, NCR);
  Netctl |= MAC_BIT(MPE);
  MacWrite(Mac, NCR, Netctl);

  Frame = (MAC_BF(SOF, 1)
            | MAC_BF(RW, 1)
            | MAC_BF(PHYA, PhyAddr)
            | MAC_BF(REGA, Reg)
            | MAC_BF(CODE, 2)
            | MAC_BF(DATA, Value));
  MacWrite(Mac, MAN, Frame);

  do {
    Netstat = MacRead(Mac, NSR);
  } while (!(Netstat & MAC_BIT(IDLE)));

  Netctl = MacRead(Mac, NCR);
  Netctl &= ~MAC_BIT(MPE);
  MacWrite(Mac, NCR, Netctl);
  //DEBUG ((DEBUG_INFO, "%a(), Reg : %x, Data : %x\n", __FUNCTION__, Reg, Value));
}

/**
  Read phy register through MDIO interface.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyAddr   Phy address.
  @param[in] Reg       Phy register.

  @retval    Register value.
**/
UINT16
MacMdioRead (
  IN MAC_DEVICE  *Mac,
  IN UINT16       PhyAddr,
  IN UINT8        Reg
  )
{
  UINT64 Netctl;
  UINT64 Netstat;
  UINT64 Frame;

  Netctl = MacRead(Mac, NCR);
  Netctl |= MAC_BIT(MPE);
  MacWrite(Mac, NCR, Netctl);

  Frame = (MAC_BF(SOF, 1)
            | MAC_BF(RW, 2)
            | MAC_BF(PHYA, PhyAddr)
            | MAC_BF(REGA, Reg)
            | MAC_BF(CODE, 2));
  MacWrite(Mac, MAN, Frame);

  do {
    Netstat = MacRead(Mac, NSR);
  } while (!(Netstat & MAC_BIT(IDLE)));

  Frame = MacRead(Mac, MAN);

  Netctl = MacRead(Mac, NCR);
  Netctl &= ~MAC_BIT(MPE);
  MacWrite(Mac, NCR, Netctl);
  //DEBUG ((DEBUG_INFO, "%a(), Reg : %x, Data : %x\n", __FUNCTION__, Reg, MAC_BFEXT(DATA, Frame)));

  return MAC_BFEXT(DATA, Frame);
}

/**
  Initialize the transmit descriptor list using the dummy descriptor.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    NULL
**/
STATIC
VOID
GmacInitMultiQueues (
  IN MAC_DEVICE *Mac
  )
{
  UINT32 Index;
  UINT32 NumQueues;
  UINT32 QueueMask;

  NumQueues = 1;
  //
  //bit 0 is never set but queue 0 always exists
  //
  QueueMask = GmuRead(Mac, DCFG6) & 0xff;
  QueueMask |= 0x1;

  for (Index = 1; Index < MAC_MAX_QUEUES; Index++) {
    if (QueueMask & (1 << Index)) {
      NumQueues++;
    }
  }
  Mac->DummyDesc->Ctrl = TXBUF_USED;
  Mac->DummyDesc->Addr = 0;

  for (Index = 1; Index < NumQueues; Index++) {
    GmuWriteQueueTBQP(Mac,(UINT32)(UINT64) Mac->DummyDesc, Index - 1);
  }
}

/**
  Initialize the receive and transmit descriptor base address. Clear descriptor
  index number.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    NULL
**/
STATIC
VOID
MacDmaDescInit (
  IN MAC_DEVICE  *Mac
  )
{
  UINT32  Index;
  UINT32  Paddr;

  //
  //Rx Descriptor
  //
  for (Index = 0; Index < MAC_RX_RING_SIZE; Index++) {
    Paddr =(UINT32) (UINT64) (Mac->RxBuffer + Index * GMU_RX_BUFFER_SIZE);
    if (Index == (MAC_RX_RING_SIZE) - 1) {
      Paddr |= RXADDR_WRAP;
    }
    Mac->RxdescRing[Index].Addr = Paddr;
    Mac->RxdescRing[Index].Ctrl = 0;
    //DEBUG ((DEBUG_INFO, "Rx desc ring[%d] :\n", Index));
    //DEBUG ((DEBUG_INFO, "Addr : %08x\n", Mac->RxdescRing[Index].Addr));
    //DEBUG ((DEBUG_INFO, "Ctrl : %08x\n", Mac->RxdescRing[Index].Ctrl));
  }
  //
  //Tx Descriptor
  //
  for (Index = 0; Index < MAC_TX_RING_SIZE; Index++) {
    if (Index == (MAC_TX_RING_SIZE) - 1) {
      Mac->TxdescRing[Index].Ctrl = TXBUF_USED | TXBUF_WRAP;
    } else {
      Mac->TxdescRing[Index].Ctrl = TXBUF_USED;
    }
    Mac->TxdescRing[Index].Addr = (UINT32) (UINT64) (Mac->TxBuffer + Index * GMU_TX_BUFFER_SIZE);
    //DEBUG ((DEBUG_INFO, "Tx desc ring [%d]:\n", Index));
    //DEBUG ((DEBUG_INFO, "Addr : %08x\n", Mac->TxdescRing[Index].Addr));
    //DEBUG ((DEBUG_INFO, "Ctrl : %08x\n", Mac->TxdescRing[Index].Ctrl));
  }

  Mac->TxCurrentDescriptorNum = 0;
  Mac->TxNextDescriptorNum = 0;
  Mac->RxCurrentDescriptorNum = 0;
  Mac->RxNextDescriptorNum = 0;
  Paddr = (UINT32) (UINT64) Mac->TxdescRing;
  //DEBUG ((DEBUG_INFO, "Tx desc Base : 0x%08x\n", Paddr));
  MacWrite(Mac, TBQP, Paddr);
  Paddr = (UINT32) (UINT64) Mac->RxdescRing;
  //DEBUG ((DEBUG_INFO, "Rx desc Base : 0x%08x\n", Paddr));
  MacWrite(Mac, RBQP, Paddr);
}

/**
  Enable mac tx and rx.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @revtal    NULL.
 **/
VOID
MacStartTxRx (
  IN MAC_DEVICE  *Mac
  )
{
  UINT32  Value;

  Value = MacRead(Mac, NCR);
  Value |= MAC_BIT(TE) | MAC_BIT(RE);
  //
  //Enable TX and RX
  //
  MacWrite(Mac, NCR, Value);
}

/**
  Mac dma config.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
STATIC
VOID
MacDmaConfig (
  IN MAC_DEVICE  *Mac
  )
{
  UINT32  DmaCfg;

  DmaCfg = GmuRead(Mac, DMACFG) & ~GMU_BF(RXBS, -1L);
  //
  //burst 16
  //
  DmaCfg = GMU_BFINS(FBLDO, 16, DmaCfg);
  DmaCfg |= GMU_BIT(TXPBMS) | GMU_BF(RXBMS, -1L);
  //
  //packet and descriptor little endian, 32 bit address
  //
  DmaCfg &= ~GMU_BIT(ENDIA_DESC);
  DmaCfg &= ~GMU_BIT(ENDIA_PKT);
  DmaCfg &= ~GMU_BIT(ADDR64);
  //DEBUG ((DEBUG_INFO, "Dma Config : %08x\n", DmaCfg));

  GmuWrite(Mac, DMACFG, DmaCfg);
}

/**
  Phytium gmu interface init.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
STATIC
EFI_STATUS
PhytiumGmuInitInterface (
  IN  MAC_DEVICE  *Mac
  )
{
  UINT32  Ctrl;
  UINT32  PcsCtrl;
  UINT32  Speed;
  UINT32  Duplex;
  UINT32  NetworkCtrl;
  BOOLEAN Autoneg;

  Ctrl = 0;
  PcsCtrl = 0;
  NetworkCtrl = 0;
  Autoneg = Mac->Autoneg;
  Speed = Mac->PhySpeed;
  Duplex = Mac->Duplex;

  PhytiumGmuSelClk (Mac, Speed);
  if (Mac->PhyInterface == PhyXgmii) {
    Ctrl = MacRead(Mac, NCFGR);
    Ctrl &=~ GMU_BIT(PCSSEL);
    MacWrite(Mac, NCFGR, Ctrl);

    //
    //NCR bit[31] enable_hs_mac
    //
    Ctrl = MacRead(Mac, NCR);
    Ctrl |=  MAC_BIT(HSMAC);
    MacWrite(Mac, NCR, Ctrl);
  } else if (Mac->PhyInterface == PhyUsxgmii) {
    Ctrl = MacRead(Mac, NCFGR);
    Ctrl |= GMU_BIT(PCSSEL);
    MacWrite(Mac, NCFGR, Ctrl);

    //
    //NCR bit[31] enable_hs_mac
    //
    Ctrl = MacRead(Mac, NCR);
    Ctrl |=  MAC_BIT(HSMAC);
    MacWrite(Mac, NCR, Ctrl);

    Ctrl = GmuRead(Mac, USX_CONTROL);
    Ctrl &= ~(GMU_BIT(TX_SCR_BYPASS) | GMU_BIT(RX_SCR_BYPASS));
    Ctrl |= GMU_BIT(RX_SYNC_RESET);
    if (Speed == SPEED_10000) {
      Ctrl = GMU_BFINS(USX_CTRL_SPEED, GMU_SPEED_10000, Ctrl);
      Ctrl |= GMU_BIT(SERDES_RATE);
    }
    GmuWrite(Mac, USX_CONTROL, Ctrl);
    Ctrl = GmuRead(Mac, USX_CONTROL);
    Ctrl &= ~(GMU_BIT(RX_SYNC_RESET));
    Ctrl |= GMU_BIT(TX_EN);
    Ctrl |= GMU_BIT(SIGNAL_OK);
    GmuWrite(Mac, USX_CONTROL, Ctrl);
  } else if (Mac->PhyInterface == PhySgmii) {
    Ctrl = MacRead(Mac, NCFGR);
    Ctrl |= GMU_BIT(PCSSEL) | GMU_BIT(SGMIIEN);

    Ctrl &= ~(MAC_BIT(SPD) | MAC_BIT(FD));
    Ctrl &= ~GMU_BIT(GBE);

    if (Duplex) {
      Ctrl |= MAC_BIT(FD);
    }
    if (Speed == SPEED_100) {
      Ctrl |= MAC_BIT(SPD);
    }
    if (Speed == SPEED_1000) {
      Ctrl |= GMU_BIT(GBE);
    }
    if (Speed == SPEED_2500) {
      NetworkCtrl = MacRead(Mac, NCR);
      NetworkCtrl |= MAC_BIT(2PT5G);
      MacWrite(Mac, NCR, NetworkCtrl);
    }

    MacWrite(Mac, NCFGR, Ctrl);

    Ctrl = MacRead(Mac, NCR);
    Ctrl &=~  MAC_BIT(HSMAC);
    MacWrite(Mac, NCR, Ctrl);

    if (Autoneg) {
      PcsCtrl = GmuRead(Mac, PCSCTRL);
      PcsCtrl |= GMU_BIT(AUTONEG);
      GmuWrite(Mac, PCSCTRL, PcsCtrl);
    } else {
      PcsCtrl = GmuRead(Mac, PCSCTRL);
      PcsCtrl &=~ GMU_BIT(AUTONEG);
      GmuWrite(Mac, PCSCTRL, PcsCtrl);
    }
  } else {
    Ctrl = MacRead(Mac, NCFGR);
    Ctrl &=~ GMU_BIT(PCSSEL);
    Ctrl &= ~(MAC_BIT(SPD) | MAC_BIT(FD));
    Ctrl &= ~GMU_BIT(GBE);

    if (Duplex) {
      Ctrl |= MAC_BIT(FD);
    }
    if (Speed == SPEED_100) {
      Ctrl |= MAC_BIT(SPD);
    }
    if (Speed == SPEED_1000) {
      Ctrl |= GMU_BIT(GBE);
    }

    MacWrite(Mac, NCFGR, Ctrl);

    Ctrl = MacRead(Mac, NCR);
    Ctrl &= ~MAC_BIT(HSMAC);
    MacWrite(Mac, NCR, Ctrl);
  }

  return EFI_SUCCESS;
}
/**
  Mac initialization.
  1.Initialize receive and transmit descriptor.
  2.Initialize descriptor queue using dummy descriptor.
  3.Phy interface mode config.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
EFI_STATUS
MacInit (
  IN MAC_DEVICE *Mac
  )
{
  //
  //initialize DMA descriptors
  //
  MacDmaDescInit (Mac);
  MacDmaConfig (Mac);
  GmacInitMultiQueues (Mac);
#ifdef PLD_TEST
  Mac->Autoneg = 0;
  Mac->PhySpeed = SPEED_1000;
  Mac->PhyInterface = PhyRgmii;
  Mac->Duplex = DUPLEX_FULL;
#else
  //Mac->Autoneg = 1;
  //Mac->PhySpeed = SPEED_1000;
  //Mac->PhyInterface = PhySgmii;
  ////Mac->PhyInterface = PhyRgmii;
  //Mac->Duplex = DUPLEX_FULL;
#endif
  PhytiumGmuInitInterface (Mac);
  MicroSecondDelay (1000);

  return EFI_SUCCESS;
}

/**
  Halt the mac controller.
  Disable tx and rx, clear statistics.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
VOID
MacHalt (
  IN MAC_DEVICE *Mac
  )
{
  UINT32 Ncr;
  UINT32 Tsr;

  //
  //Halt the controller and wait for any ongoing transmission to end.
  //
  Ncr = MacRead(Mac, NCR);
  Ncr |= MAC_BIT(THALT);
  MacWrite(Mac, NCR, Ncr);

  do {
    Tsr = MacRead(Mac, TSR);
  } while (Tsr & MAC_BIT(TGO));

  //
  //Disable TX and RX, and clear statistics
  //
  MacWrite(Mac, NCR, MAC_BIT(CLRSTAT));
}

/**
  Set mac address.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
MacWriteHwaddr (
  IN MAC_DEVICE      *Mac,
  IN EFI_MAC_ADDRESS  *MacAddr
  )
{
  UINT32 HwaddrBottom;
  UINT16 HwaddrTop;

  //
  //set hardware address
  //
  HwaddrBottom = MacAddr->Addr[0] | (MacAddr->Addr[1] << 8) |
                 (MacAddr->Addr[2] << 16) | (MacAddr->Addr[3] << 24);
  GmuWrite(Mac, SA1B, HwaddrBottom);
  HwaddrTop = MacAddr->Addr[4] | (MacAddr->Addr[5] << 8);
  GmuWrite(Mac, SA1T, HwaddrTop);
}

/**
  Get mac address.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[out] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
MacReadHwaddr (
  IN  MAC_DEVICE      *Mac,
  OUT EFI_MAC_ADDRESS  *MacAddr
  )
{
  UINT32  HwaddrBottom;
  UINT32  HwaddrTop;

  HwaddrBottom = GmuRead(Mac, SA1B);
  HwaddrTop = GmuRead(Mac, SA1T);

  MacAddr->Addr[0] = HwaddrBottom & 0xFF;
  MacAddr->Addr[1] = (HwaddrBottom >> 8) & 0xFF;
  MacAddr->Addr[2] = (HwaddrBottom >> 16) & 0xFF;
  MacAddr->Addr[3] = (HwaddrBottom >> 24) & 0xFF;
  MacAddr->Addr[4] = HwaddrTop & 0xFF;
  MacAddr->Addr[5] = (HwaddrTop >> 8) & 0xFF;
}

/**
  Print mac address.

  @param[in] MacAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
DisplayMacAddr (
  IN EFI_MAC_ADDRESS  *MacAddr
  )
{
  DEBUG ((DEBUG_INFO, "%02x-%02x-%02x-%02x-%02x-%02x\n",
            MacAddr->Addr[0], MacAddr->Addr[1], MacAddr->Addr[2], MacAddr->Addr[3], MacAddr->Addr[4], MacAddr->Addr[5]));
}

/**
  Calulate mac clock division configuration.

  @param[in] Clk       Clock to divide.
  @param[in] Mac       A point to MAC_DEVICE structure.

  @retval    Clock division configuration.
**/
STATIC
UINT32
GmuMdcClkDiv (
  IN UINT64      Clk,
  IN MAC_DEVICE *Mac
  )
{
  UINT32 Config;

  if (Clk < 20000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV8);
  } else if (Clk < 40000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV16);
  } else if (Clk < 80000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV32);
  } else if (Clk < 120000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV48);
  } else if (Clk < 160000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV64);
  } else if (Clk <= 240000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV96);
  } else if (Clk <= 320000000) {
    Config = GMU_BF(CLK, GMU_CLK_DIV128);
  } else {
    Config = GMU_BF(CLK, GMU_CLK_DIV224);
  }

  return Config;
}

/**
  Get the DMA bus width field of the network configuration register that we
  should program. We find the width from decoding the design configuration
  register to find the maximum supported data bus width.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    DMA bus width configuration.
**/
STATIC
UINT32
MacDbw (
 IN MAC_DEVICE *Mac
 )
{
  switch (GMU_BFEXT(DBWDEF, GmuRead(Mac, DCFG1))) {
  case 4:
    return GMU_BF(DBW, GMU_DBW128);
  case 2:
    return GMU_BF(DBW, GMU_DBW64);
  case 1:
  default:
    return GMU_BF(DBW, GMU_DBW32);
  }
}

/**
  Get mac stats from hardware.

  @param[in]  Mac     A point to MAC_DRIVER.
  @param[out] Stats   A point to MAC_STATS.

  @retval    NULL
**/
STATIC
VOID
MacGetStatsFrmoHw (
  IN  MAC_DEVICE *Mac,
  OUT MAC_STATS   *Stats
  )
{
  Stats->TxOctets31_0 = GmuRead(Mac, OCTTXL);
  Stats->TxOctets47_32 = GmuRead(Mac, OCTTXH) & BIT_MASK_16;
  Stats->TxFrames = GmuRead(Mac, TXCNT);
  Stats->TxBroadcastFrames = GmuRead(Mac, TXBCCNT);
  Stats->TxMulticastFrames = GmuRead(Mac, TXMCCNT);
  Stats->TxPauseFrames = GmuRead(Mac, TXPAUSECNT) & BIT_MASK_16;
  Stats->Tx64ByteFrames = GmuRead(Mac, TX64CNT);
  Stats->Tx65_127ByteFrames = GmuRead(Mac, TX65CNT);
  Stats->Tx128_255ByteFrames = GmuRead(Mac, TX128CNT);
  Stats->Tx256_511ByteFrames = GmuRead(Mac, TX256CNT);
  Stats->Tx512_1023ByteFrames = GmuRead(Mac, TX512CNT);
  Stats->Tx1024_1518ByteFrames = GmuRead(Mac, TX1024CNT);
  Stats->TxGreaterThan1518ByteFrames = GmuRead(Mac, TX1519CNT);
  Stats->TxUnderrun = GmuRead(Mac, TXURUNCNT) & BIT_MASK_10;
  Stats->TxSingleCollisionFrames = GmuRead(Mac, SNGLCOLLCNT) & BIT_MASK_18;
  Stats->TxMultipleCollisionFrames = GmuRead(Mac, MULTICOLLCNT) & BIT_MASK_18;
  Stats->TxExcessiveCollisions = GmuRead(Mac, EXCESSCOLLCNT) & BIT_MASK_10;
  Stats->TxLateCollisions = GmuRead(Mac, LATECOLLCNT) & BIT_MASK_10;
  Stats->TxDeferredFrames = GmuRead(Mac, TXDEFERCNT) & BIT_MASK_18;
  Stats->TxCarrierSenseErrors = GmuRead(Mac, TXCSENSECNT) & BIT_MASK_10;
  Stats->RxOctets31_0 = GmuRead(Mac, OCTRXL);
  Stats->RxOctets47_32 = GmuRead(Mac, OCTRXH) & BIT_MASK_16;
  Stats->RxFrames = GmuRead(Mac, RXCNT);
  Stats->RxBroadcastFrames = GmuRead(Mac, RXBROADCNT);
  Stats->RxMulticastFrames = GmuRead(Mac, RXMULTICNT);
  Stats->RxPauseFrames = GmuRead(Mac, RXPAUSECNT) & BIT_MASK_16;
  Stats->Rx64ByteFrames = GmuRead(Mac, RXCNT);
  Stats->Rx65_127ByteFrames = GmuRead(Mac, RX65CNT);
  Stats->Rx128_255ByteFrames = GmuRead(Mac, RX128CNT);
  Stats->Rx256_511ByteFrames =GmuRead(Mac, RX256CNT);
  Stats->Rx512_1023ByteFrames = GmuRead(Mac, RX512CNT);
  Stats->Rx1024_1518ByteFrames = GmuRead(Mac, RX1024CNT);
  Stats->RxGreaterThan1518ByteFrames = GmuRead(Mac, RX1519CNT);
  Stats->RxUndersizedFrames = GmuRead(Mac, RXUNDRCNT) & BIT_MASK_10;
  Stats->RxOversizeFrames = GmuRead(Mac, RXOVRCNT) & BIT_MASK_10;
  Stats->RxJabbers = GmuRead(Mac, RXJABCNT) & BIT_MASK_10;
  Stats->RxFrameCheckSequenceErrors = GmuRead(Mac, RXFCSCNT) & BIT_MASK_10;
  Stats->RxLengthFieldFrameErrors = GmuRead(Mac, RXLENGTHCNT) & BIT_MASK_10;
  Stats->RxSymbolErrors = GmuRead(Mac, RXSYMBCNT) & BIT_MASK_10;
  Stats->RxAlignmentErrors = GmuRead(Mac, RXALIGNCNT) & BIT_MASK_10;
  Stats->RxResourceErrors = GmuRead(Mac, RXRESERRCNT) & BIT_MASK_18;
  Stats->RxOverruns = GmuRead(Mac, RXORCNT) & BIT_MASK_10;
  Stats->RxIpHeaderChecksumErrors = GmuRead(Mac, RXIPCCNT) & BIT_MASK_8;
  Stats->RxTcpChecksumErrors = GmuRead(Mac, RXTCPCCNT) & BIT_MASK_8;
  Stats->RxUdpChecksumErrors = GmuRead(Mac, RXUDPCCNT) & BIT_MASK_8;
}

/**
  The hash address register is 64 bits long and takes up two
  locations in the memory map.  The least significant bits are stored
  in EMAC_HSL and the most significant bits in EMAC_HSH.

  The unicast hash enable and the multicast hash enable bits in the
  network configuration register enable the reception of hash matched
  frames. The destination address is reduced to a 6 bit index into
  the 64 bit hash register using the following hash function.  The
  hash function is an exclusive or of every sixth bit of the
  destination address.

  hi[5] = da[5] ^ da[11] ^ da[17] ^ da[23] ^ da[29] ^ da[35] ^ da[41] ^ da[47]
  hi[4] = da[4] ^ da[10] ^ da[16] ^ da[22] ^ da[28] ^ da[34] ^ da[40] ^ da[46]
  hi[3] = da[3] ^ da[09] ^ da[15] ^ da[21] ^ da[27] ^ da[33] ^ da[39] ^ da[45]
  hi[2] = da[2] ^ da[08] ^ da[14] ^ da[20] ^ da[26] ^ da[32] ^ da[38] ^ da[44]
  hi[1] = da[1] ^ da[07] ^ da[13] ^ da[19] ^ da[25] ^ da[31] ^ da[37] ^ da[43]
  hi[0] = da[0] ^ da[06] ^ da[12] ^ da[18] ^ da[24] ^ da[30] ^ da[36] ^ da[42]

  da[0] represents the least significant bit of the first byte
  received, that is, the multicast/unicast indicator, and da[47]
  represents the most significant bit of the last byte received.  If
  the hash index, hi[n], points to a bit that is set in the hash
  register then the frame will be matched according to whether the
  frame is multicast or unicast.  A multicast match will be signalled
  if the multicast hash enable bit is set, da[0] is 1 and the hash
  index points to a bit set in the hash register.  A unicast match
  will be signalled if the unicast hash enable bit is set, da[0] is 0
  and the hash index points to a bit set in the hash register.  To
  receive all multicast frames, the hash register should be set with
  all ones and the multicast hash enable bit should be set in the
  network configuration register.

  @param[in]  Bitnr
  @param[in]  MacAddr    A point to EFI_MAC_ADDRESS.

  @retval     1
  @retval     0
**/
STATIC
UINT32
HashBitValue (
  IN  UINT32           Bitnr,
  IN  EFI_MAC_ADDRESS  *MacAddr
  )
{
  if (MacAddr->Addr[Bitnr / 8] & (1 << (Bitnr % 8))) {
    return 1;
  }
  return 0;
}

/**
  Return the hash index value for the specified address.

  @param[in]  MacAddr    A point to EFI_MAC_ADDRESS.

  @retval     HashIndex
 **/
STATIC
UINT32
HashGetIndex (
  IN  EFI_MAC_ADDRESS  *MacAddr
  )
{
  UINT32 I;
  UINT32 J;
  UINT32 Bitval;
  UINT32 HashIndex;

  HashIndex = 0;

  for (J = 0; J < 6; J++) {
    for (I = 0, Bitval = 0; I < 8; I++) {
      Bitval ^= HashBitValue (I * 6 + J, MacAddr);
    }
    HashIndex |= (Bitval << J);
  }

  return HashIndex;
}

/**
  Set mac filter hash table.

  @param[in]  Mac        A pointer to MAC_DEVICE.
  @param[in]  MFilter     Mac filter list.
  @param[in]  NumMfilter  Number of filter mac.i

  @retval    NULL.
**/
STATIC
VOID
MacSetHashTable (
  IN  MAC_DEVICE      *Mac,
  IN  EFI_MAC_ADDRESS  *Mfilter,
  IN  UINTN            NumMfilter
  )
{
  UINT32  McFilter[2];
  UINT32  Bitnr;
  UINT32  Index;

  McFilter[0] = 0;
  McFilter[1] = 0;

  for (Index = 0; Index < NumMfilter; Index++) {
    Bitnr = HashGetIndex (&Mfilter[Index]);
    McFilter[Bitnr >> 5] |= 1 << (Bitnr & 31);
  }

  GmuWrite(Mac, HRB, McFilter[0]);
  GmuWrite(Mac, HRT, McFilter[1]);
}

/**
  Rec mac filter function config.

  @param[in]  Mac                   A pointer to MAC_DEVICE.
  @param[in]  ReceiveFilterSetting  Filter mask bits.
  @param[in]  Reset                 Set to TRUE to reset the contents of the multicast
                                    receive filters on the network interface to their
                                    default values.
  @param[in]  NumMfilter            Number of filter mac.
  @param[in]  Mfilter               Filter mac list.

  @retval     EFI_SUCCESS           Success.
**/
EFI_STATUS
EFIAPI
MacRxFilters (
  IN  MAC_DEVICE      *Mac,
  IN  UINT32           ReceiveFilterSetting,
  IN  BOOLEAN          Reset,
  IN  UINTN            NumMfilter             OPTIONAL,
  IN  EFI_MAC_ADDRESS  *Mfilter               OPTIONAL
  )
{
  UINT32 MacFilter;

  //
  // If reset then clear the filter registers
  //
  if (Reset) {
    GmuWrite(Mac, HRB, 0x0);
    GmuWrite(Mac, HRT, 0x0);
  }

  MacFilter = GmuRead(Mac, NCFGR);
  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    MacFilter |= MAC_BIT(NCFGR_MTI);
    //
    // Set the hash tables
    //
    if ((NumMfilter > 0) && (!Reset)) {
      MacSetHashTable (Mac, Mfilter, NumMfilter);
    }
  }
  if ((ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) == 0) {
    MacFilter |= MAC_BIT(NBC);
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    MacFilter |= MAC_BIT(CAF);
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST) {
    GmuWrite(Mac, HRB, 0xFFFFFFFF);
    GmuWrite(Mac, HRT, 0xFFFFFFFF);
    MacFilter |= MAC_BIT(NCFGR_MTI);
  }

  GmuWrite(Mac, NCFGR, MacFilter);

  return EFI_SUCCESS;
}

/**
  Get mac statistic.

  @param[in]  Mac     A point to MAC_DEVICE
  @param[out] Stats   A point to EFI_NETWORK_STATISTICS

  @retval    NULL
**/
VOID
MacGetStatistic (
  IN  MAC_DEVICE             *Mac,
  OUT EFI_NETWORK_STATISTICS  *Stats
  )
{
  MAC_STATS  MacStats;

  ZeroMem (&MacStats, sizeof (MAC_STATS));
  MacGetStatsFrmoHw (Mac, &MacStats);

  Stats->RxTotalFrames = MacStats.RxFrames
                           + MacStats.RxFrameCheckSequenceErrors
                           + MacStats.RxAlignmentErrors
                           + MacStats.RxResourceErrors
                           + MacStats.RxOversizeFrames
                           + MacStats.RxOverruns
                           + MacStats.RxJabbers
                           + MacStats.RxUndersizedFrames
                           + MacStats.RxLengthFieldFrameErrors;
  Stats->RxGoodFrames = MacStats.RxFrames;
  Stats->RxUndersizeFrames = MacStats.RxUndersizedFrames;
  Stats->RxOversizeFrames = MacStats.RxOversizeFrames;
  Stats->RxDroppedFrames = Mac->RxDropped;
  //Stats->RxUnicastFrames;
  Stats->RxBroadcastFrames = MacStats.TxBroadcastFrames;
  Stats->RxMulticastFrames = MacStats.TxMulticastFrames;
  Stats->RxCrcErrorFrames = MacStats.RxFrameCheckSequenceErrors;
  Stats->RxTotalBytes = Mac->RxTotalBytes;

  Stats->TxTotalFrames = MacStats.TxFrames
                           + MacStats.TxUnderrun
                           + MacStats.TxExcessiveCollisions
                           + MacStats.TxCarrierSenseErrors;
  Stats->TxGoodFrames = MacStats.TxFrames;
  Stats->TxUndersizeFrames = MacStats.TxUnderrun;
  Stats->TxDroppedFrames = Mac->TxDropped;
  //Stats->TxUnicastFrames;
  Stats->TxBroadcastFrames = MacStats.TxBroadcastFrames;
  Stats->TxMulticastFrames = MacStats.TxMulticastFrames;
  Stats->TxTotalBytes = Mac->TxTotalBytes;
  Stats->Collisions = MacStats.TxSingleCollisionFrames
                        + MacStats.TxMultipleCollisionFrames
                        + MacStats.TxExcessiveCollisions;
  Stats->TxErrorFrames = MacStats.TxExcessiveCollisions
                           + MacStats.TxUnderrun
                           + MacStats.TxCarrierSenseErrors;
}

/**
  Allocate DMA space for transmit, receive, dummy descriptor and data receive
  buffer. Configure the GMU module clock to enable MDIO reading and writing.

  @param[in]    Snp    Point to the SIMPLE_NETWORK_DRIVER.

  @retval       EFI_SUCCESS  GMU initialization succeeded.
**/
EFI_STATUS
MacInitialize (
  IN SIMPLE_NETWORK_DRIVER  *Snp
  )
{
  UINT32                Ncfgr;
  EFI_STATUS            Status;
  MAC_DEVICE           *Mac;

  Status = EFI_SUCCESS;
  Snp->MacDriver.Base = Snp->MacBase;
  Snp->MacDriver.RxBufferSize = GMU_RX_BUFFER_SIZE;
  Mac = &Snp->MacDriver;
#ifdef PLD_TEST
  DEBUG ((DEBUG_INFO, "Pad Config!\n"));
  //rgmii
  MmioWrite32 (0x32b3018c, 0x1);
  MmioWrite32 (0x32b30190, 0x1);
  MmioWrite32 (0x32b30194, 0x1);
  MmioWrite32 (0x32b30198, 0x1);
  MmioWrite32 (0x32b3019c, 0x1);
  MmioWrite32 (0x32b301a0, 0x1);
  MmioWrite32 (0x32b301a4, 0x1);
  MmioWrite32 (0x32b301a8, 0x1);
  MmioWrite32 (0x32b301ac, 0x1);
  MmioWrite32 (0x32b301b0, 0x1);
  MmioWrite32 (0x32b301b4, 0x1);
  MmioWrite32 (0x32b301b8, 0x1);
  MmioWrite32 (0x32b301cc, 0x1);
  MmioWrite32 (0x32b301d0, 0x1);
  MmioWrite32 (0x32b301d4, 0x1);
  MmioWrite32 (0x32b301d8, 0x1);
  MmioWrite32 (0x32b301dc, 0x1);
  MmioWrite32 (0x32b301e0, 0x1);
  MmioWrite32 (0x32b301e4, 0x1);
  MmioWrite32 (0x32b301e8, 0x1);
  MmioWrite32 (0x32b301ec, 0x1);
  MmioWrite32 (0x32b301f0, 0x1);
  MmioWrite32 (0x32b301f4, 0x1);
  MmioWrite32 (0x32b301f8, 0x1);
#endif
  Snp->MacDriver.TxdescRing = (MAC_DMA_DESC *) AllocatePages (EFI_SIZE_TO_PAGES (MAC_TX_DMA_DESC_SIZE));
  //DEBUG ((DEBUG_INFO, "TxdescRing : 0x%p\n", Snp->MacDriver.TxdescRing));
  Snp->MacDriver.RxdescRing = (MAC_DMA_DESC *) AllocatePages (EFI_SIZE_TO_PAGES (MAC_RX_DMA_DESC_SIZE));
  //DEBUG ((DEBUG_INFO, "RxdescRing : 0x%p\n", Snp->MacDriver.RxdescRing));
  Snp->MacDriver.TxBuffer = (CHAR8 *) AllocatePages (EFI_SIZE_TO_PAGES (TX_TOTAL_BUFFERSIZE));
  //DEBUG ((DEBUG_INFO, "Tx Buffer : 0x%p\n", Snp->MacDriver.TxBuffer));
  Snp->MacDriver.RxBuffer = (CHAR8 *) AllocatePages (EFI_SIZE_TO_PAGES (RX_TOTAL_BUFFERSIZE));
  //DEBUG ((DEBUG_INFO, "Rx Buffer : 0x%p\n", Snp->MacDriver.RxBuffer));
  Snp->MacDriver.DummyDesc = (MAC_DMA_DESC *) AllocatePages (EFI_SIZE_TO_PAGES (sizeof (MAC_DMA_DESC)));
  //Ncfgr = GemMdcClkDiv (250000000, Mac);
  Ncfgr = GmuMdcClkDiv (48000000, Mac);
  Ncfgr |= MacDbw (Mac);

  MacWrite(Mac, NCFGR, Ncfgr);

  return Status;
}

/**
  Configura mac according to the current phy interface type ,speed and duplex.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[in]  Speed     Phy speed, SPEED_10000, SPEED_1000, SPEED_100 or SPEED_10.
  @param[in]  Duplex    Duplex mode, full or half.

  @retval    NULL
**/
VOID
MacConfigAdjust (
  IN MAC_DEVICE  *Mac,
  IN UINT32      Speed,
  IN UINT32      Duplex
  )
{
  UINT32  Value;

  DEBUG ((DEBUG_INFO, "Speed : %d, Duplex : %d\n", Speed, Duplex));
  Value = MacRead(Mac, NCFGR);
  switch (Speed) {
  case SPEED_10000:
    //
    //todo
    //
    break;
  case SPEED_1000:
    Value &= ~(MAC_BIT(SPD) | MAC_BIT(FD));
    Value |= GMU_BIT(GBE);
    if (Duplex == DUPLEX_FULL) {
      Value |= MAC_BIT(FD);
    }
    break;
  case SPEED_100:
    Value &= ~(MAC_BIT(SPD) | MAC_BIT(FD) | GMU_BIT(GBE));
    Value |= MAC_BIT(SPD);
    if (Duplex == DUPLEX_FULL) {
      Value |= MAC_BIT(FD);
    }
    break;
  case SPEED_10:
    Value &= ~(MAC_BIT(SPD) | MAC_BIT(FD) | GMU_BIT(GBE));
    if (Duplex == DUPLEX_FULL) {
      Value |= MAC_BIT(FD);
    }
    break;
  default:
    ASSERT (0);
    break;
  }

  MacWrite(Mac, NCFGR, Value);
  PhytiumGmuSelClk (Mac, Speed);
  MicroSecondDelay (1000);
}

/**
  Configure phytium gmu module clock parameters according to interface speed.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[in]  Speed     Phy interface speed.10000, 2500, 1000, 100.

  @retval     EFI_SUCCESS    Success.
**/
EFI_STATUS
PhytiumGmuSelClk (
  IN MAC_DEVICE  *Mac,
  IN UINT32      Speed
  )
{
  UINT32 HsCtrl;
  UINT32 GmuSpeed;

  HsCtrl = 0;
  GmuSpeed = 0;
  DEBUG ((DEBUG_INFO, "%a(), Speed :%d\n", __FUNCTION__, Speed));
  DEBUG ((DEBUG_INFO, "Interface : %d\n", Mac->PhyInterface));
  if ((Mac->PhyInterface == PhyUsxgmii) || (Mac->PhyInterface == PhyXgmii)) {
    if (Speed == SPEED_10000) {
      GmuSpeed = GMU_SPEED_10000;
      //GmuWrite(Mac, SRC_SEL_LN, 0x1); /*0x1c04*/
      GmuWrite(Mac, DIV_SEL0_LN, 0x4); /*0x1c08*/
      GmuWrite(Mac, DIV_SEL1_LN, 0x1); /*0x1c0c*/
      GmuWrite(Mac, PMA_XCVR_POWER_STATE, 0x1); /*0x1c10*/
    }
  } else if (Mac->PhyInterface == PhySgmii) {
    if (Speed == SPEED_2500) {
      GmuSpeed = GMU_SPEED_2500;
      //GmuWrite(Mac, SRC_SEL_LN, 0x0); /*0x1c04*/
      GmuWrite(Mac, DIV_SEL0_LN, 0x1); /*0x1c08*/
      GmuWrite(Mac, DIV_SEL1_LN, 0x2); /*0x1c0c*/
      GmuWrite(Mac, PMA_XCVR_POWER_STATE, 0x1); /*0x1c10*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x1); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x1); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x1); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x1); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x0); /*0x1c34*/
      GmuWrite(Mac, TX_CLK_SEL3_0, 0x0); /*0x1c70*/
      GmuWrite(Mac, TX_CLK_SEL4_0, 0x0); /*0x1c74*/
      GmuWrite(Mac, RX_CLK_SEL3_0, 0x0); /*0x1c78*/
      GmuWrite(Mac, RX_CLK_SEL4_0, 0x0); /*0x1c7c*/
    }else if (Speed == SPEED_1000) {
      GmuSpeed = GMU_SPEED_1000;
      //GmuWrite(Mac, SRC_SEL_LN, 0x0); /*0x1c04*/
      GmuWrite(Mac, DIV_SEL0_LN, 0x4); /*0x1c08*/
      GmuWrite(Mac, DIV_SEL1_LN, 0x8); /*0x1c0c*/
      GmuWrite(Mac, PMA_XCVR_POWER_STATE, 0x1); /*0x1c10*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x0); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x0); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x1); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x1); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x0); /*0x1c34*/
      GmuWrite(Mac, TX_CLK_SEL3_0, 0x0); /*0x1c70*/
      GmuWrite(Mac, TX_CLK_SEL4_0, 0x0); /*0x1c74*/
      GmuWrite(Mac, RX_CLK_SEL3_0, 0x0); /*0x1c78*/
      GmuWrite(Mac, RX_CLK_SEL4_0, 0x0); /*0x1c7c*/
    } else if ((Speed == SPEED_100) || (Speed == SPEED_10)) {
      GmuSpeed = GMU_SPEED_100;
      //GmuWrite(Mac, SRC_SEL_LN, 0x0); /*0x1c04*/
      GmuWrite(Mac, DIV_SEL0_LN, 0x4); /*0x1c08*/
      GmuWrite(Mac, DIV_SEL1_LN, 0x8); /*0x1c0c*/
      GmuWrite(Mac, PMA_XCVR_POWER_STATE, 0x1); /*0x1c10*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x0); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x1); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x1); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x1); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x0); /*0x1c34*/
      GmuWrite(Mac, TX_CLK_SEL3_0, 0x1); /*0x1c70*/
      GmuWrite(Mac, TX_CLK_SEL4_0, 0x0); /*0x1c74*/
      GmuWrite(Mac, RX_CLK_SEL3_0, 0x0); /*0x1c78*/
      GmuWrite(Mac, RX_CLK_SEL4_0, 0x1); /*0x1c7c*/
    }
  } else if (Mac->PhyInterface == PhyRgmii) {
    if (Speed == SPEED_1000) {
      GmuSpeed = GMU_SPEED_1000;
      GmuWrite(Mac, MII_SELECT, 0x1); /*0x1c18*/
      GmuWrite(Mac, SEL_MII_ON_RGMII, 0x0); /*0x1c1c*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x1); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x0); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x0); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x0); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x1); /*0x1c34*/
      GmuWrite(Mac, CLK_250M_DIV10_DIV100_SEL, 0x0); /*0x1c38*/
      GmuWrite(Mac, RX_CLK_SEL5, 0x1); /*0x1c48*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL0, 0x1); /*0x1c80*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL1, 0x0); /*0x1c84*/
    } else if (Speed == SPEED_100) {
      GmuWrite(Mac, MII_SELECT, 0x1); /*0x1c18*/
      GmuWrite(Mac, SEL_MII_ON_RGMII, 0x0); /*0x1c1c*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x1); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x0); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x0); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x0); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x1); /*0x1c34*/
      GmuWrite(Mac, CLK_250M_DIV10_DIV100_SEL, 0x0); /*0x1c38*/
      GmuWrite(Mac, RX_CLK_SEL5, 0x1); /*0x1c48*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL0, 0x0); /*0x1c80*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL1, 0x0); /*0x1c84*/
      GmuSpeed = GMU_SPEED_100;
    } else {
      GmuWrite(Mac, MII_SELECT, 0x1); /*0x1c18*/
      GmuWrite(Mac, SEL_MII_ON_RGMII, 0x0); /*0x1c1c*/
      GmuWrite(Mac, TX_CLK_SEL0, 0x0); /*0x1c20*/
      GmuWrite(Mac, TX_CLK_SEL1, 0x1); /*0x1c24*/
      GmuWrite(Mac, TX_CLK_SEL2, 0x0); /*0x1c28*/
      GmuWrite(Mac, TX_CLK_SEL3, 0x0); /*0x1c2c*/
      GmuWrite(Mac, RX_CLK_SEL0, 0x0); /*0x1c30*/
      GmuWrite(Mac, RX_CLK_SEL1, 0x1); /*0x1c34*/
      GmuWrite(Mac, CLK_250M_DIV10_DIV100_SEL, 0x1); /*0x1c38*/
      GmuWrite(Mac, RX_CLK_SEL5, 0x1); /*0x1c48*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL0, 0x0); /*0x1c80*/
      GmuWrite(Mac, RGMII_TX_CLK_SEL1, 0x0); /*0x1c84*/
      GmuSpeed = GMU_SPEED_100;
    }
  } else if (Mac->PhyInterface == PhyRmii) {
    GmuSpeed = GMU_SPEED_100;
    GmuWrite(Mac, RX_CLK_SEL5, 0x1); /*0x1c48*/
  }

  DEBUG ((DEBUG_INFO, "%a(), GmuSpeed :%d\n", __FUNCTION__, GmuSpeed));
  //
  //GMU_HSMAC(0x0050) provide rate to the external
  //
  HsCtrl = GmuRead(Mac, HSMAC);
  HsCtrl = GMU_BFINS(HSMACSPEED, GmuSpeed, HsCtrl);
  GmuWrite (Mac, HSMAC, HsCtrl);

  return EFI_SUCCESS;
}

VOID
ParseMacShareMem (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  OUT MAC_DEVICE            *Mac
  )
{
  UINT64  Temp;
  UINT32  Index;

  Temp = Address;
  DEBUG ((DEBUG_INFO, "Shared Memory Address : %llx\n", Address));
  Temp += 4;
  Mac->PhyInterface = MmioRead32 (Temp);
  DEBUG ((DEBUG_INFO, "Phy Interface : %d\n", Mac->PhyInterface));
  Temp += 4;
  Mac->Autoneg = MmioRead32 (Temp);
  DEBUG ((DEBUG_INFO, "Autoneg : %d\n", Mac->Autoneg));
  Temp += 4;
  Mac->PhySpeed = MmioRead32 (Temp);
  DEBUG ((DEBUG_INFO, "Phy Speed : %d\n", Mac->PhySpeed));
  Temp += 4;
  Mac->Duplex = MmioRead32 (Temp);
  DEBUG ((DEBUG_INFO, "Duplex : %d\n", Mac->Duplex));
  Temp += 4;
  for (Index = 0; Index < 6; Index++) {
    Mac->MacAddr.Addr[Index] = MmioRead8 (Temp + Index);
  }
}
