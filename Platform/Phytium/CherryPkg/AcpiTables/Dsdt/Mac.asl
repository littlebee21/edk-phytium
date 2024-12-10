/** @file
    Copyright (c) 2022, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device (ETH0) {
    Name (_HID, "PHYT0036")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x3200C000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {87}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {88}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {89}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {90}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {60}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {61}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {62}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {63}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"magic-packet", 1},
          Package () {"support-tsn", 1},
          Package () {"phy-mode", "sgmii"},
          Package () {"use-mii", 1},
        }
    })

    Method (_STA, 0, NotSerialized) {
      //Return(0x0F)
	  Return(0x0)
    }
  }

  Device (ETH1) {
    Name (_HID, "PHYT0036")
    Name (_UID, 1)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x3200E000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {91}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {92}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {93}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {94}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"magic-packet", 1},
          Package () {"support-tsn", 1},
          Package () {"phy-mode", "sgmii"},
          Package () {"use-mii", 1},
        }
    })

    Method (_STA, 0, NotSerialized) {
      //Return(0x0F)
	  Return(0x0)
    }
  }

  Device (ETH2) {
    Name (_HID, "PHYT0036")
    Name (_UID, 2)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x32010000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {96}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {97}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {98}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {99}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"magic-packet", 1},
          Package () {"support-tsn", 1},
          Package () {"phy-mode", "rgmii"},
          Package () {"use-mii", 1},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
	        //Return(0x0)
    }
  }

  Device (ETH3) {
    Name (_HID, "PHYT0036")
    Name (_UID, 3)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x32012000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {100}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {101}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {102}
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {103}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"magic-packet", 1},
          Package () {"support-tsn", 1},
          Package () {"phy-mode", "rgmii"},
          Package () {"use-mii", 1},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }
}
