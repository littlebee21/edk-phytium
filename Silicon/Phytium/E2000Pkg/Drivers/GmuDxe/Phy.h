/** @file
** Phy layer header file.
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef _PHY_DXE_H__
#define _PHY_DXE_H__

#include "Mac.h"
#include "Type.h"

//#define  PHY_AR8035

//
// PHY Registers
//
#define PHY_BASIC_CTRL                        0
#define PHY_BASIC_STATUS                      1
#define PHY_ID1                               2
#define PHY_ID2                               3
#define PHY_AUTO_NEG_ADVERT                   4
#define PHY_AUTO_NEG_LINK_ABILITY             5
#define PHY_AUTO_NEG_EXP                      6
#define PHY_1000BASE_T_CONTROL                9
#define PHY_1000BASE_T_STATUS                 10
#define PHY_MODE                              17
#define PHY_SPECIAL_MODES                     18
#define PHY_SPECIAL_CTLR                      27
#define PHY_INT_SRC                           29
#define PHY_INT_MASK                          30
#define PHY_SPECIAL_PHY_CTLR                  31

//
// PHY control register bits
//
#define PHYCTRL_COLL_TEST                     BIT7            // Collision test enable
#define PHYCTRL_DUPLEX_MODE                   BIT8            // Set Duplex Mode
#define PHYCTRL_RST_AUTO                      BIT9            // Restart Auto-Negotiation of Link abilities
#define PHYCTRL_PD                            BIT11           // Power-Down switch
#define PHYCTRL_AUTO_EN                       BIT12           // Auto-Negotiation Enable
#define PHYCTRL_SPEED_SEL                     BIT13           // Link Speed Selection
#define PHYCTRL_LOOPBK                        BIT14           // Set loopback mode
#define PHYCTRL_RESET                         BIT15           // Do a PHY reset

//
// PHY status register bits
//
#define PHYSTS_EXT_CAP                        BIT0            // Extended Capabilities Register capability
#define PHYSTS_JABBER                         BIT1            // Jabber condition detected
#define PHYSTS_LINK_STS                       BIT2            // Link Status
#define PHYSTS_AUTO_CAP                       BIT3            // Auto-Negotiation Capability
#define PHYSTS_REMOTE_FAULT                   BIT4            // Remote fault detected
#define PHYSTS_AUTO_COMP                      BIT5            // Auto-Negotiation Completed
#define PHYSTS_10BASET_HDPLX                  BIT11           // 10Mbps Half-Duplex ability
#define PHYSTS_10BASET_FDPLX                  BIT12           // 10Mbps Full-Duplex ability
#define PHYSTS_100BASETX_HDPLX                BIT13           // 100Mbps Half-Duplex ability
#define PHYSTS_100BASETX_FDPLX                BIT14           // 100Mbps Full-Duplex ability
#define PHYSTS_100BASE_T4                     BIT15           // Base T4 ability

//
// PHY Auto-Negotiation advertisement
//
#define PHYANA_SEL_MASK                       ((UINT32)0x1F)   // Link type selector
#define PHYANA_10BASET                        BIT5             // Advertise 10BASET capability
#define PHYANA_10BASETFD                      BIT6             // Advertise 10BASET Full duplex capability
#define PHYANA_100BASETX                      BIT7             // Advertise 100BASETX capability
#define PHYANA_100BASETXFD                    BIT8             // Advertise 100 BASETX Full duplex capability
#define PHYANA_PAUSE_OP_MASK                  (3 << 10)        // Advertise PAUSE frame capability
#define PHYANA_REMOTE_FAULT                   BIT13            // Remote fault detected

#define PHYLPA_SLCT                           0x001f           // Same as advertise selector
#define PHYLPA_10HALF                         0x0020           // Can do 10mbps half-duplex
#define PHYLPA_1000XFULL                      0x0020           // Can do 1000BASE-X full-duplex
#define PHYLPA_10FULL                         0x0040           // Can do 10mbps full-duplex
#define PHYLPA_1000XHALF                      0x0040           // Can do 1000BASE-X half-duplex
#define PHYLPA_100HALF                        0x0080           // Can do 100mbps half-duplex
#define PHYLPA_1000XPAUSE                     0x0080           // Can do 1000BASE-X pause
#define PHYLPA_100FULL                        0x0100           // Can do 100mbps full-duplex
#define PHYLPA_1000XPAUSE_ASYM                0x0100           // Can do 1000BASE-X pause asym
#define PHYLPA_100BASE4                       0x0200           // Can do 100mbps 4k packets
#define PHYLPA_PAUSE_CAP                      0x0400           // Can pause
#define PHYLPA_PAUSE_ASYM                     0x0800           // Can pause asymetrically
#define PHYLPA_RESV                           0x1000           // Unused
#define PHYLPA_RFAULT                         0x2000           // Link partner faulted
#define PHYLPA_LPACK                          0x4000           // Link partner acked us
#define PHYLPA_NPAGE                          0x8000           // Next page bit

#define PHYLPA_DUPLEX                         (LPA_10FULL | LPA_100FULL)
#define PHYLPA_100                            (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

//
// 1000BASE-T Status register
//
#define PHYLPA_1000FULL                       0x0800           // Link partner 1000BASE-T full duplex
#define PHYLPA_1000HALF                       0x0400           // Link partner 1000BASE-T half duplex

//
// 1000BASE-T Control register
//
#define PHYADVERTISE_1000FULL                 0x0200           // Advertise 1000BASE-T full duplex
#define PHYADVERTISE_1000HALF                 0x0100           // Advertise 1000BASE-T half duplex

#define SPEED_10000                           10000
#define SPEED_2500                            2500
#define SPEED_1000                            1000
#define SPEED_100                             100
#define SPEED_10                              10

#define DUPLEX_FULL                           1
#define DUPLEX_HALF                           0

//
// PHY Super Special control/status
//
#define PHYSSCS_HCDSPEED_MASK                 (7 << 2)        // Speed indication
#define PHYSSCS_AUTODONE                      BIT12           // Auto-Negotiation Done

//
// Flags for PHY reset
//
#define PHY_RESET_PMT                         BIT0
#define PHY_RESET_BCR                         BIT1
#define PHY_RESET_CHECK_LINK                  BIT2

//
// Flags for auto negotiation
//
#define AUTO_NEGOTIATE_COLLISION_TEST         BIT0
#define AUTO_NEGOTIATE_ADVERTISE_ALL          BIT1

//
// Micrel KSZ9031 Extended registers
//
#define PHY_KSZ9031RN_CONTROL_PAD_SKEW_REG    4
#define PHY_KSZ9031RN_RX_DATA_PAD_SKEW_REG    5
#define PHY_KSZ9031RN_TX_DATA_PAD_SKEW_REG    6
#define PHY_KSZ9031RN_CLK_PAD_SKEW_REG        8

//
// Data operations
//
#define PHY_KSZ9031_MOD_DATA_NO_POST_INC      0x1
#define PHY_KSZ9031_MOD_DATA_POST_INC_RW      0x2
#define PHY_KSZ9031_MOD_DATA_POST_INC_W       0x3

#define PHY_KSZ9031RN_MMD_CTRL_REG            0x0d
#define PHY_KSZ9031RN_MMD_REGDATA_REG         0x0e

#define PHY_KSZ9031RN_CLK_SKEW_CLR_MASK       0x3FF
#define PHY_KSZ9031RN_CONTROL_SKEW_CLR_MASK   0xFF
#define PHY_KSZ9031RN_RX_DATA_SKEW_CLR_MASK   0xFF
#define PHY_KSZ9031RN_TX_DATA_SKEW_CLR_MASK   0xFF

#define PHY_KSZ9031RN_CLK_PAD_SKEW_VALUE      0x3FC
#define PHY_KSZ9031RN_CONTROL_PAD_SKEW_VALUE  0x70
#define PHY_KSZ9031RN_RX_DATA_PAD_SKEW_VALUE  0x7777
#define PHY_KSZ9031RN_TX_DATA_PAD_SKEW_VALUE  0x0


#define PHY_KSZ9031RN_DEV_ADDR                0x2

//
// MMD Address 0h, Auto-Negotiation FLP burst transmit timing
//
#define PHY_KSZ9031RN_MMD_DEV_ADDR_00         0x00
#define PHY_KSZ9031RN_MMD_D0_FLP_LO_REG       3
#define PHY_KSZ9031RN_MMD_D0_FLP_16MS_LO      0x1A80
#define PHY_KSZ9031RN_MMD_D0_FLP_HI_REG       4
#define PHY_KSZ9031RN_MMD_D0_FLP_16MS_HI      0x0006

//
// HPS MII
//
#define MII_BUSY                              (1 << 0)
#define MII_WRITE                             (1 << 1)
#define MII_CLKRANGE_60_100M                  (0x0)
#define MII_CLKRANGE_100_150M                 (0x4)
#define MII_CLKRANGE_20_35M                   (0x8)
#define MII_CLKRANGE_35_60M                   (0xC)
#define MII_CLKRANGE_150_250M                 (0x10)
#define MII_CLKRANGE_250_300M                 (0x14)

#define MIIADDRSHIFT                          (11)
#define MIIREGSHIFT                           (6)
#define MII_REGMSK                            (0x1F << 6)
#define MII_ADDRMSK                           (0x1F << 11)

//
// Others
//
#define PHY_INVALID_ID                        0xFFFF
#define LINK_UP                               1
#define LINK_DOWN                             0
#define PHY_TIMEOUT                           100000

//
//add CTC
//
#define CTC_PHY_REG_SPACE  0
#define CTC_SDS_REG_SPACE  1
//
//Mask used for ID comparisons
//
#define CTC_PHY_ID_MASK             0xffffffff

//
//Known PHY IDs
//
#define CTC_PHY_ID_MARS1S_V1        0x00782013
#define CTC_PHY_ID_MARS1S           0x01E04013
#define CTC_PHY_ID_MARS1P_V1        0x00782011
#define CTC_PHY_ID_MARS1P           0x01E04011

/**
  Phy initialization config.
  1.detece phy devices
  2.phy devices config

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyDriver  A point to PHY_DRIVER structure.

  @retval EFI_SUCCESS       The phy interface was started.
  @retval EFI_NOT_FOUND     No phy detected.
**/
EFI_STATUS
EFIAPI
PhyDxeInitialization (
  IN MAC_DEVICE  *Mac,
  IN PHY_DRIVER   *PhyDriver
  );

/**
  Detect phy devices.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyDriver  A point to Phy dirver structureM

  @retval EFI_SUCCESS       The phy interface was detected.
  @retval EFI_DEVICE_ERROR  Fail to detect ethernet phy.
**/
EFI_STATUS
EFIAPI
PhyDetectDevice (
  IN MAC_DEVICE  *Mac,
  IN PHY_DRIVER   *PhyDriver
  );

/**
  Read the phy devices ID.

  @param[in] PhyAddr    The phy address
  @param[in] Mac       A point to MAC_DEVICE structure.

  @retval EFI_SUCCESS    Success to read phy ID.
**/
EFI_STATUS
EFIAPI
PhyReadId (
  IN  UINT32       PhyAddr,
  IN  MAC_DEVICE  *Mac,
  OUT PHY_DRIVER   *PhyDriver
  );

/**
  Phy devices config,common config ,AR8035 or KSZ9031 config.
  1.phy soft reset
  2.phy special config,AR8035 or KSZ9031
  3.Configure AN and Advertise

  @param[in] Mac               A point to MAC_DEVICE structure.
  @param[in] PhyDriver          A point to Phy dirver structureM

  @retval EFI_SUCCESS           Success to config phy.
  @retval EFI_DEVICE_ERROR      Fail to config phy.
**/
EFI_STATUS
EFIAPI
PhyConfig (
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
EFI_STATUS
EFIAPI
PhySoftReset (
  IN PHY_DRIVER    *PhyDriver,
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
EFI_STATUS
EFIAPI
PhyAutoNego (
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
EFI_STATUS
EFIAPI
PhyLinkAdjustEmacConfig (
  IN PHY_DRIVER  *PhyDriver,
  IN MAC_DEVICE *Mac
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
EFI_STATUS
EFIAPI
PhyCheckLinkStatus (
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
EFI_STATUS
EFIAPI
PhyReadCapability (
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
VOID
EFIAPI
PhyDisplayAbility (
  IN UINT32   Speed,
  IN UINT32   Duplex
  );

/**
  Function to update the media state.
  Read the link state,compare the PhyOldLink state and PhyCurrentLink state,if the state is changed,
  update the media link state.And if the state is changed to link-up,read the phy capability and
  config adjust.

  @param[in] PhyDriver   A point to Phy dirver structure
  @param[in] Mac        A point to MAC_DEVICE structure.

  @retval EFI_SUCCESS     link-up
  @retval EFI_TIMEOUT     link-down
**/
EFI_STATUS
EFIAPI
UpdateMediaState(
  IN PHY_DRIVER   *PhyDriver,
  IN MAC_DEVICE  *Mac
  );

#endif /* KSZ9031_PHY_DXE_H__ */
