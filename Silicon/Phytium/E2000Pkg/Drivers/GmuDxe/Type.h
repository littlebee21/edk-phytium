/** @file
** Type.h
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  _GMU_TPYE_H_
#define  _GMU_TPYE_H_

//#define  PLD_TEST

#define  BIT_MASK_10    (0x3FF)
#define  BIT_MASK_8     (0XFF)
#define  BIT_MASK_18    (0X3FFFF)
#define  BIT_MASK_16    (0xFFFF)

typedef struct _PHY_DRIVER PHY_DRIVER;
typedef struct _MAC_DEVICE MAC_DEVICE;

typedef struct {
  MAC_ADDR_DEVICE_PATH      MacAddrDP;
  EFI_DEVICE_PATH_PROTOCOL  End;
} SIMPLE_NETWORK_DEVICE_PATH;

typedef enum {
  PhyUsxgmii = 0,
  PhyXgmii,
  PhySgmii,
  PhyRgmii,
  PhyRmii,
} PHY_INTERFACE;

typedef struct {
  EFI_PHYSICAL_ADDRESS  AddrMap;
  VOID                  *Mapping;
} MAP_INFO;

typedef struct _MAC_DMA_DESC {
  UINT32  Addr;
  UINT32  Ctrl;
} MAC_DMA_DESC;

typedef struct _MAC_STATS {
  UINT32 TxOctets31_0;
  UINT32 TxOctets47_32;
  UINT32 TxFrames;
  UINT32 TxBroadcastFrames;
  UINT32 TxMulticastFrames;
  UINT32 TxPauseFrames;
  UINT32 Tx64ByteFrames;
  UINT32 Tx65_127ByteFrames;
  UINT32 Tx128_255ByteFrames;
  UINT32 Tx256_511ByteFrames;
  UINT32 Tx512_1023ByteFrames;
  UINT32 Tx1024_1518ByteFrames;
  UINT32 TxGreaterThan1518ByteFrames;
  UINT32 TxUnderrun;
  UINT32 TxSingleCollisionFrames;
  UINT32 TxMultipleCollisionFrames;
  UINT32 TxExcessiveCollisions;
  UINT32 TxLateCollisions;
  UINT32 TxDeferredFrames;
  UINT32 TxCarrierSenseErrors;
  UINT32 RxOctets31_0;
  UINT32 RxOctets47_32;
  UINT32 RxFrames;
  UINT32 RxBroadcastFrames;
  UINT32 RxMulticastFrames;
  UINT32 RxPauseFrames;
  UINT32 Rx64ByteFrames;
  UINT32 Rx65_127ByteFrames;
  UINT32 Rx128_255ByteFrames;
  UINT32 Rx256_511ByteFrames;
  UINT32 Rx512_1023ByteFrames;
  UINT32 Rx1024_1518ByteFrames;
  UINT32 RxGreaterThan1518ByteFrames;
  UINT32 RxUndersizedFrames;
  UINT32 RxOversizeFrames;
  UINT32 RxJabbers;
  UINT32 RxFrameCheckSequenceErrors;
  UINT32 RxLengthFieldFrameErrors;
  UINT32 RxSymbolErrors;
  UINT32 RxAlignmentErrors;
  UINT32 RxResourceErrors;
  UINT32 RxOverruns;
  UINT32 RxIpHeaderChecksumErrors;
  UINT32 RxTcpChecksumErrors;
  UINT32 RxUdpChecksumErrors;
} MAC_STATS;

struct _MAC_DEVICE {
  CHAR8            *TxBuffer;
  CHAR8            *RxBuffer;
  UINT64           Base;
  BOOLEAN          Wrapped;
  MAC_DMA_DESC    *TxdescRing;
  MAC_DMA_DESC    *RxdescRing;
  UINT32           RxBufferSize;

  UINT64           TxTotalFrame;
  UINT64           TxTotalBytes;
  UINT64           TxDropped;
  UINT64           RxTotalFrame;
  UINT64           RxTotalBytes;
  UINT64           RxDropped;

  MAC_DMA_DESC    *DummyDesc;

  UINT16           PhyAddr;
  PHY_INTERFACE    PhyInterface;
  UINT32           PhySpeed;
  UINT32           Duplex;
  BOOLEAN          Autoneg;
  EFI_MAC_ADDRESS  MacAddr;

  UINT32           TxCurrentDescriptorNum;
  UINT32           TxNextDescriptorNum;
  UINT32           RxCurrentDescriptorNum;
  UINT32           RxNextDescriptorNum;
};

/**
  Phy devices config,common config ,AR8035 or KSZ9031 config.
  1.phy soft reset
  2.phy special config
  3.Configure AN and Advertise

  @param[in] Mac               A point to MAC_DEVICE structure.
  @param[in] PhyDriver          A point to Phy dirver structureM

  @retval EFI_SUCCESS           Success to config phy.
  @retval EFI_DEVICE_ERROR      Fail to config phy.
**/
typedef
EFI_STATUS
(*PHY_CONFIG)(
  IN  MAC_DEVICE  *Mac,
  IN  PHY_DRIVER   *PhyDriver
  );

/**
  Perform PHY software reset.

  @param[in] Mac        A point to MAC_DEVICE structure.
  @param[in] PhyDriver   A point to Phy dirver structure

  @retval EFI_SUCCESS    Success to soft reset phy.
  @retval EFI_TIMEOUT    timeout to soft reset phy.
**/
typedef
EFI_STATUS
(*PHY_SOFT_RESET)(
  IN  PHY_DRIVER  *PhyDriver,
  IN  MAC_DEVICE  *Mac
  );

/**
  Do phy auto-negotiation.
  1.Read PHY Status
  2.Read PHY Auto-Nego Advertise capabilities register for 10/100 Base-T
  3.Set Advertise capabilities for 10Base-T/10Base-T full-duplex/100Base-T/100Base-T full-duplex
  4.Set Advertise capabilities for 1000 Base-T/1000 Base-T full-duplex
  5.Enable and restart Auto-Negotiation.

  @param[in] Mac        A point to MAC_DEVICE structure.
  @param[in] PhyDriver   A point to Phy dirver structure

  @retval EFI_SUCCESS       auto-negotiation config success.
  @retval EFI_DEVICE_ERROR  Auto-negotiation is not supported
**/
typedef
EFI_STATUS
(*PHY_AUTO_NEGO)(
  IN  MAC_DEVICE *Mac,
  IN  PHY_DRIVER  *PhyDriver
  );

/**
  Phy link adjust config.
  1.check phy link status.
  2.read phy link capability.
  3.GMAC adjust config.

  @param[in] Mac        A point to MAC_DEVICE structure.
  @param[in] PhyDriver   A point to Phy dirver structure

  @retval EFI_SUCCESS     phy link up.
  @retval EFI_NOT_READY   phy link down.
**/
typedef
EFI_STATUS
(*PHY_LINKADJUST_EMAC_CONFIG)(
  IN PHY_DRIVER   *PhyDriver,
  IN MAC_DEVICE  *Mac
  );

/**
  Check phy link status.
  1.check phy link existed or not.
  2.wait until Auto Negotiation completed.

  @param[in] PhyDriver   A point to Phy dirver structure
  @param[in] Mac        A point to MAC_DEVICE structure.

  @retval EFI_SUCCESS     phy link up.
  @retval EFI_TIMEOUT     phy link down.
**/
typedef
EFI_STATUS
(*PHY_CHECK_LINK_STATUS)(
  IN PHY_DRIVER   *PhyDriver,
  IN  MAC_DEVICE *Mac
  );

/**
  Read phy capability, speed and duplex.

  @param[in]  PhyDriver   A point to Phy dirver structure
  @param[in]  Mac        A point to MAC_DEVICE structure.
  @param[out] speed       ethernet speed,10M/100M/1000M
  @param[out] duplex      Duplex mode,half/full

  @retval EFI_SUCCESS   Read success
**/
typedef
EFI_STATUS
(*PHY_READ_CAPABILITY)(
  IN PHY_DRIVER    *PhyDriver,
  IN  MAC_DEVICE  *Mac,
  OUT UINT32       *Speed,
  OUT UINT32       *Duplex
  );

/**
  Display phy capability, speed and duplex.

  @param Speed   ethernet speed,10M/100M/1000M
  @param Duplex  Duplex mode,half/full
**/
typedef
VOID
(*PHY_DISPLAY_ABILITY)(
  IN UINT32   Speed,
  IN UINT32   Duplex
  );

struct _PHY_DRIVER {
  UINT32         PhyAddr;
  UINT32         PhyCurrentLink;
  UINT32         PhyOldLink;
  UINT16         PhyId1;
  UINT16         PhyId2;
  BOOLEAN        PhyDetected;
  PHY_INTERFACE  PhyInterface;
  //
  //phy function define
  //
  PHY_CONFIG     PhyConfig;
  PHY_SOFT_RESET PhySoftReset;
  PHY_AUTO_NEGO  PhyAutoNego;
  PHY_LINKADJUST_EMAC_CONFIG  PhyLinkAdjustEmacConfig;
  PHY_CHECK_LINK_STATUS  PhyCheckLinkStatus;
  PHY_READ_CAPABILITY  PhyReadCapability;
  PHY_DISPLAY_ABILITY  PhyDisplayAbility;
};

#endif
