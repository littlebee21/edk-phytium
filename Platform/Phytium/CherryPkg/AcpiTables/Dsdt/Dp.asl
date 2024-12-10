/** @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

Scope(_SB)
{
  Device (DCP0) {
    Name (_HID, "PHYT0015")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (PIPE, 0x3)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x32000000, 0x8000)
      //CMA memory
      //disable : base = 0, size = 0
      //enable : update base and size in AcpiPlatformDxe
      Memory32Fixed (ReadWrite, 0x0, 0x0)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {76}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"pipe_mask", PIPE},
          Package () {"edp_mask", 0},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }

  //Corresponding DP0
  Device (IIS0) {
    Name (_HID, "PHYT0016")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x32009000, 0x1000)
      Memory32Fixed (ReadWrite, 0x32008000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {79}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"dai-name", "phytium-i2s-dp0"},
          Package () {"i2s_clk", 600000000},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }

  //Corresponding DP1
  Device (IIS1) {
    Name (_HID, "PHYT0016")
    Name (_UID, 1)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x3200B000, 0x1000)
      Memory32Fixed (ReadWrite, 0x3200A000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {80}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"dai-name", "phytium-i2s-dp1"},
          Package () {"i2s_clk", 600000000},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }

  //pmdk_dp virtual node
  Device (PMDK) {
    Name (_HID, "PHYT8006")
    Name (DPCN, 0x2)
    Name (PIPE, 0x3)
    Name (_DSD, Package() {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"num-dp", DPCN},
        Package () {"dp-mask", PIPE},
      }
    })
    Name (_STA, 0xF)
  }
}

