/** @file
** Phy layer.
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>

#include "GenPhy.h"
#include "Mac.h"
#include "Phy.h"
#include "Snp.h"
#include "Type.h"


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
  )
{
  EFI_STATUS   Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __FUNCTION__));

  //
  // initialize the phyaddr
  //
  PhyDriver->PhyAddr = 0;
  PhyDriver->PhyCurrentLink = LINK_DOWN;
  PhyDriver->PhyOldLink = LINK_DOWN;
  PhyDriver->PhyInterface = Mac->PhyInterface;

  if (PhyDriver->PhyDetected == FALSE) {
    Status = PhyDetectDevice (Mac, PhyDriver);
    if (EFI_ERROR (Status)) {
      PhyDriver->PhyDetected = 0;
      return EFI_NOT_FOUND;
    }

    //
    //Install Phy function interface.
    //
    PhyDriver->PhyConfig = GenPhyConfig;
    PhyDriver->PhySoftReset = GenPhySoftReset;
    PhyDriver->PhyAutoNego = GenPhyAutoNego;
    PhyDriver->PhyLinkAdjustEmacConfig = GenPhyLinkAdjustEmacConfig;
    PhyDriver->PhyCheckLinkStatus = GenPhyCheckLinkStatus;
    PhyDriver->PhyReadCapability = GenPhyReadCapability;
    PhyDriver->PhyDisplayAbility = GenPhyDisplayAbility;
    PhyDriver->PhyDetected = TRUE;
  }

  PhyDriver->PhyConfig (Mac, PhyDriver);

  return EFI_SUCCESS;
}

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
  )
{
  UINT32       PhyAddr;
  EFI_STATUS   Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __FUNCTION__));

  for (PhyAddr = 0; PhyAddr < 32; PhyAddr++) {
    Status = PhyReadId (PhyAddr, Mac, PhyDriver);
    if (EFI_ERROR(Status)) {
      continue;
    }

    PhyDriver->PhyAddr = PhyAddr;
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "SNP:PHY: Fail to detect Ethernet PHY!\r\n"));
  return EFI_NOT_FOUND;
}

/**
  Read the phy devices ID.

  @param[in] PhyAddr    The phy address
  @param[in] Mac        A point to MAC_DEVICE structure
. @param[out] PhyDriver A point to PHY_DRIVER structure

  @retval EFI_SUCCESS    Success to read phy ID.
**/
EFI_STATUS
EFIAPI
PhyReadId (
  IN  UINT32       PhyAddr,
  IN  MAC_DEVICE   *Mac,
  OUT PHY_DRIVER   *PhyDriver
  )
{
  UINT16        PhyId1;
  UINT16        PhyId2;

  PhyId1 = PHY_INVALID_ID;
  PhyId2 = PHY_INVALID_ID;
  PhyId1 = MacMdioRead (Mac, PhyAddr, PHY_ID1);
  PhyId2 = MacMdioRead (Mac, PhyAddr, PHY_ID2);

  if (PhyId1 == PHY_INVALID_ID || PhyId2 == PHY_INVALID_ID) {
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "SNP:PHY: Ethernet PHY detected. PHY_ID1=0x%04X, PHY_ID2=0x%04X, PHY_ADDR=0x%02X\r\n",
          PhyId1, PhyId2, PhyAddr));
  PhyDriver->PhyId1 = PhyId1;
  PhyDriver->PhyId2 = PhyId2;
#if 0
  if ((PhyId1 == 0x78) && (PhyId2 == 0x2013)) {
    DEBUG((DEBUG_INFO, "Detected Mar1-s Phy Chip \n"));
    PhyWrite(PhyAddr, 0x1e, 0x27,MacBaseAddress);
    DEBUG((DEBUG_INFO,"THE MacBaseAddress is------ 0x%lx\n",MacBaseAddress));
    PhyRead(PhyAddr,  0x1f,&value,MacBaseAddress);
    value = value & (~(1 << 15));
    DEBUG((DEBUG_INFO, "sleep config:%x", value));
    PhyWrite(PhyAddr, 0x1e, 0x27,MacBaseAddress);
    PhyWrite(PhyAddr, 0x1f, value,MacBaseAddress);
    PhyWrite(PhyAddr, 0x1e, 0x27,MacBaseAddress);
    mars_set_link_timer_2_6ms(PhyAddr,MacBaseAddress);
  }
#endif
  return EFI_SUCCESS;
}
