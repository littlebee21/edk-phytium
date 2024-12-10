/** @file
** General phy function.
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>

#include "GenPhy.h"
#include "Mac.h"
#include "Snp.h"

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
EFI_STATUS
EFIAPI
GenPhyConfig (
  IN  MAC_DEVICE  *Mac,
  IN  PHY_DRIVER   *PhyDriver
  )
{
  EFI_STATUS  Status;
  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __FUNCTION__));

  Status = PhyDriver->PhySoftReset (PhyDriver, Mac);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Configure AN and Advertise
  //
  PhyDriver->PhyAutoNego (Mac, PhyDriver);

  return EFI_SUCCESS;
}


/**
  Perform PHY software reset.

  @param[in] Mac        A point to MAC_DEVICE structure.
  @param[in] PhyDriver   A point to Phy dirver structure

  @retval EFI_SUCCESS    Success to soft reset phy.
  @retval EFI_TIMEOUT    timeout to soft reset phy.
**/
EFI_STATUS
EFIAPI
GenPhySoftReset (
  IN PHY_DRIVER    *PhyDriver,
  IN  MAC_DEVICE  *Mac
  )
{
  UINT32        TimeOut;
  UINT16        Data;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __FUNCTION__));

  // PHY Basic Control Register reset
  MacMdioWrite (Mac, PhyDriver->PhyAddr, PHY_BASIC_CTRL, PHYCTRL_RESET);

  // Wait for completion
  TimeOut = 0;
  do {
    //
    // Read PHY_BASIC_CTRL register from PHY
    //
    Data = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_CTRL);
    //
    // Wait until PHYCTRL_RESET become zero
    //
    if ((Data & PHYCTRL_RESET) == 0) {
      break;
    }
    MicroSecondDelay(1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: ERROR! PhySoftReset timeout\n"));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

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
GenPhyAutoNego (
  IN  MAC_DEVICE *Mac,
  IN  PHY_DRIVER  *PhyDriver
  )
{
  UINT16        PhyControl;
  UINT16        PhyStatus;
  UINT16        Features;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __FUNCTION__));

  //
  // Read PHY Status
  //
  PhyStatus = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);

  // Check PHY Status if auto-negotiation is supported
  if ((PhyStatus & PHYSTS_AUTO_CAP) == 0) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: Auto-negotiation is not supported.\n"));
    return EFI_DEVICE_ERROR;
  }

  //
  // Read PHY Auto-Nego Advertise capabilities register for 10/100 Base-T
  //
  Features = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_AUTO_NEG_ADVERT);
  //
  // Set Advertise capabilities for 10Base-T/10Base-T full-duplex/100Base-T/100Base-T full-duplex
  //
  Features |= (PHYANA_10BASET | PHYANA_10BASETFD | PHYANA_100BASETX | PHYANA_100BASETXFD);
  MacMdioWrite (Mac, PhyDriver->PhyAddr, PHY_AUTO_NEG_ADVERT, Features);
  //
  // Read PHY Auto-Nego Advertise capabilities register for 1000 Base-T
  //
  Features = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL);

  // Set Advertise capabilities for 1000 Base-T/1000 Base-T full-duplex
  Features |= (PHYADVERTISE_1000FULL | PHYADVERTISE_1000HALF);
  MacMdioWrite (Mac, PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL, Features);
  //
  // Read control register
  //
  PhyControl = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_CTRL);
  //
  // Enable Auto-Negotiation
  //
  PhyControl |= PHYCTRL_AUTO_EN;
  //
  // Restart auto-negotiation
  //
  PhyControl |= PHYCTRL_RST_AUTO;
  //
  // Write this configuration
  //
  MacMdioWrite (Mac, PhyDriver->PhyAddr, PHY_BASIC_CTRL, PhyControl);

  return EFI_SUCCESS;
}

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
GenPhyLinkAdjustEmacConfig (
  IN PHY_DRIVER   *PhyDriver,
  IN MAC_DEVICE  *Mac
  )
{
  UINT32       Speed;
  UINT32       Duplex;
  EFI_STATUS   Status;

  Status = EFI_SUCCESS;
  Speed = SPEED_10;
  Duplex = DUPLEX_HALF;

  Status = PhyDriver->PhyCheckLinkStatus (PhyDriver, Mac);
  if (EFI_ERROR (Status)) {
    PhyDriver->PhyCurrentLink = LINK_DOWN;
    Status = EFI_NOT_READY;
  } else {
    PhyDriver->PhyCurrentLink = LINK_UP;
    Status = EFI_SUCCESS;
  }

  if (PhyDriver->PhyOldLink != PhyDriver->PhyCurrentLink) {
    if (PhyDriver->PhyCurrentLink == LINK_UP) {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Link is up - Network Cable is Plugged\r\n"));
      PhyDriver->PhyReadCapability (PhyDriver, Mac, &Speed, &Duplex);
      MacConfigAdjust (Mac, Speed, Duplex);
      Status = EFI_SUCCESS;
    } else {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Link is Down - Network Cable is Unplugged?\r\n"));
      Status = EFI_NOT_READY;
    }
  } else if (PhyDriver->PhyCurrentLink == LINK_DOWN) {
    Status = EFI_NOT_READY;
  }

  PhyDriver->PhyOldLink = PhyDriver->PhyCurrentLink;

  return Status;
}

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
GenPhyCheckLinkStatus (
  IN PHY_DRIVER   *PhyDriver,
  IN  MAC_DEVICE *Mac
  )
{
  UINT16        Data;
  UINTN         TimeOut;
  UINT16        PhyBasicStatus;

  //
  // Get the PHY Status
  //
  PhyBasicStatus = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);
  //
  // if Link is already up then dont need to proceed anymore
  //
  if (PhyBasicStatus & PHYSTS_LINK_STS) {
    return EFI_SUCCESS;
  }
  //
  // Wait until it is up or until Time Out
  //
  TimeOut = 0;
  do {
    //
    // Read PHY_BASIC_STATUS register from PHY
    //
    Data = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);
    //
    // Wait until PHYSTS_LINK_STS become one
    //
    if (Data & PHYSTS_LINK_STS) {
      //
      // Link is up
      //
      break;
    }
    MicroSecondDelay (1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    // Link is down
    return EFI_TIMEOUT;
  }

  // Wait until autonego process has completed
  TimeOut = 0;
  do {
    //
    // Read PHY_BASIC_STATUS register from PHY
    //
    Data = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);
    //
    // Wait until PHYSTS_AUTO_COMP become one
    //
    if (Data & PHYSTS_AUTO_COMP) {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Auto Negotiation completed\r\n"));
      break;
    }
    MicroSecondDelay (1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: Error! Auto Negotiation timeout\n"));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

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
GenPhyReadCapability (
  IN PHY_DRIVER    *PhyDriver,
  IN  MAC_DEVICE  *Mac,
  OUT UINT32       *Speed,
  OUT UINT32       *Duplex
  )
{
  UINT16  PartnerAbilityGb;
  UINT16  AdvertisingGb;
  UINT16  CommonAbilityGb;
  UINT16  PartnerAbility;
  //
  // For 1000 Base-T
  //
  PartnerAbilityGb = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_1000BASE_T_STATUS);
  AdvertisingGb = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL);
  CommonAbilityGb = PartnerAbilityGb & (AdvertisingGb << 2);
  //
  // For 10/100 Base-T
  //
  PartnerAbility = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_AUTO_NEG_LINK_ABILITY);
  //
  // Determine the Speed and Duplex
  //
  if (PartnerAbilityGb & (PHYLPA_1000FULL | PHYLPA_1000HALF)) {
    *Speed = SPEED_1000;
    if (CommonAbilityGb & PHYLPA_1000FULL) {
      *Duplex = DUPLEX_FULL;
    } else {
      *Duplex = DUPLEX_HALF;
    }
  } else {
    if (PartnerAbility & (PHYLPA_100FULL | PHYLPA_100HALF)){
      *Speed = SPEED_100;
      if (PartnerAbility & PHYLPA_100FULL) {
        *Duplex = DUPLEX_FULL;
      } else {
        *Duplex = DUPLEX_HALF;
      }
    } else  {
      *Speed = SPEED_10;
      if (PartnerAbility & PHYLPA_10FULL) {
        *Duplex = DUPLEX_FULL;
      } else {
        *Duplex = DUPLEX_HALF;
      }
    }
  }

  PhyDriver->PhyDisplayAbility (*Speed, *Duplex);

  return EFI_SUCCESS;
}

/**
  Display phy capability, speed and duplex.

  @param Speed   ethernet speed,10M/100M/1000M
  @param Duplex  Duplex mode,half/full
**/
VOID
EFIAPI
GenPhyDisplayAbility (
  IN UINT32   Speed,
  IN UINT32   Duplex
  )
{
  DEBUG ((DEBUG_INFO, "SNP:PHY: "));
  switch (Speed) {
    case SPEED_1000:
      DEBUG ((DEBUG_INFO, "1 Gbps - "));
      break;
    case SPEED_100:
      DEBUG ((DEBUG_INFO, "100 Mbps - "));
      break;
    case SPEED_10:
      DEBUG ((DEBUG_INFO, "10 Mbps - "));
      break;
    default:
      DEBUG ((DEBUG_INFO, "Invalid link speed"));
      break;
    }

  switch (Duplex) {
    case DUPLEX_FULL:
      DEBUG ((DEBUG_INFO, "Full Duplex\n"));
      break;
    case DUPLEX_HALF:
      DEBUG ((DEBUG_INFO, "Half Duplex\n"));
      break;
    default:
      DEBUG ((DEBUG_INFO, "Invalid duplex mode\n"));
      break;
    }
}

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
  )
{
  UINT32      Speed;
  UINT32      Duplex;
  EFI_STATUS  Status;
  UINT16      PhyBasicStatus;
  UINT16      LinkStatus;
  UINTN       TimeOut;
  UINT16      Data;
  UINT16      ANState;

  Status = EFI_SUCCESS;
  Speed = SPEED_10;
  Duplex = DUPLEX_HALF;

  PhyBasicStatus = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);
  if (PhyBasicStatus & PHYSTS_LINK_STS) {
    LinkStatus = LINK_UP;
  } else {
    LinkStatus = LINK_DOWN;
  }
  if (LinkStatus == PhyDriver->PhyOldLink) {
    PhyDriver->PhyCurrentLink = LinkStatus;
    PhyDriver->PhyOldLink = PhyDriver->PhyCurrentLink;
    return EFI_SUCCESS;
  } else {
    PhyDriver->PhyCurrentLink = LinkStatus;
    PhyDriver->PhyOldLink = PhyDriver->PhyCurrentLink;
  }
  if (LinkStatus == LINK_UP) {
    //
    // Wait until autonego process has completed
    //
    TimeOut = 0;
    ANState = 0;
    do {
      //
      // Read PHY_BASIC_STATUS register from PHY
      //
      Data = MacMdioRead (Mac, PhyDriver->PhyAddr, PHY_BASIC_STATUS);
      //
      // Wait until PHYSTS_AUTO_COMP become one
      //
      if (Data & PHYSTS_AUTO_COMP) {
        DEBUG ((DEBUG_INFO, "SNP:PHY: Auto Negotiation completed\r\n"));
        ANState = 1;
        break;
      }
      MicroSecondDelay (1);
    } while (TimeOut++ < 10000);
    if (ANState == 0) {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Error! Auto Negotiation timeout\n"));
      Status =  EFI_TIMEOUT;
    } else {
      PhyDriver->PhyCurrentLink = LinkStatus;
      DEBUG((DEBUG_INFO,"Speed and Duplex config!\n"));
      PhyDriver->PhyReadCapability (PhyDriver, Mac, &Speed, &Duplex);
      //
      //adjust mac speed and duplex
      //
      MacConfigAdjust (Mac, Speed, Duplex);
      Status = EFI_SUCCESS;
    }
  }
  return Status;
}
