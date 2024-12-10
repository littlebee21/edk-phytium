/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
    //Device (MMC0) {
    //  Name (_HID, "PHYT0017")
    //  Name (_UID, 0)
    //  Name (_CCA, 1)
    //  Name (_CRS, ResourceTemplate () {
    //    Memory32Fixed (ReadWrite, 0x28000000, 0x1000)
    //    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {104}
    //  })

    //  Name (_DSD, Package () {
    //      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    //      Package () {
    //        Package () {"bus-width", 4},
    //        Package () {"max-frequency", 25000000},
    //        Package () {"cap-sdio-irq", 1},
    //        Package () {"cap-sd-highspeed", 1},
    //        Package () {"no-mmc", 1}
    //      }
    //  })

    //  Method (_STA, 0, NotSerialized) {
    //    Return(0x0F)
    //  }
    //}

    Device (MMC0) {
      Name (_HID, "PHYT0017")
      Name (_UID, 0)
      Name (_CCA, 1)
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x28000000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {104}
      })

      Name (_DSD, Package () {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"bus-width", 8},
            Package () {"max-frequency", 50000000},
            Package () {"cap-mmc-hw-reset", 1},
            Package () {"cap-mmc-highspeed", 1},
		//	Package () {"mmc-hs200-1_8v", 1},
            Package () {"no-sdio", 1},
            Package () {"no-sd", 1},
            Package () {"non-removable", 1}
          }
      })

      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
    }

    //Device (MMC1) {
    //  Name (_HID, "PHYT0017")
    //  Name (_UID, 1)
    //  Name (_CCA, 1)
    //  Name (_CRS, ResourceTemplate () {
    //    Memory32Fixed (ReadWrite, 0x28001000, 0x1000)
    //    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {105}
    //  })

    //  Name (_DSD, Package () {
    //      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    //      Package () {
    //        Package () {"bus-width", 8},
    //        Package () {"max-frequency", 25000000},
    //        Package () {"cap-mmc-hw-reset", 1},
    //        Package () {"cap-mmc-highspeed", 1},
    //        Package () {"no-sdio", 1},
    //        Package () {"no-sd", 1},
    //        Package () {"non-removable", 1}
    //      }
    //  })

    //  Method (_STA, 0, NotSerialized) {
    //    Return(0x0F)
    //  }
    //}
}

