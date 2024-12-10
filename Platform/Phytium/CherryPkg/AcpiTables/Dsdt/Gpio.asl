/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device(GPI0) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 0)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28034000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {140}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {141}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {142}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {143}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {144}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {145}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {146}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {147}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {148}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {149}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {150}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {151}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {152}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {153}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {154}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {155}
    })

    Device(GP00) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }

  Device(GPI1) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 1)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28035000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {156}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {157}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {158}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {159}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {160}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {161}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {162}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {163}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {164}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {165}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {166}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {167}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {168}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {169}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {170}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {171}
    })

    Device(GP01) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }

  Device(GPI2) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 2)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28036000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {172}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {173}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {174}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {175}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {176}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {177}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {178}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {179}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {180}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {181}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {182}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {183}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {184}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {185}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {186}
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {187}
    })

    Device(GP02) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }

  Device(GPI3) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 3)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28037000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {188}
    })

    Device(GP03) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }

  Device(GPI4) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 4)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28038000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {189}
    })

    Device(GP03) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }

  Device(GPI5) {
    Name(_HID, "PHYT0001")
    Name(_CID, "FTGP0001")  /* compatible with v1.0 */
    Name(_UID, 5)
    Name (_CRS, ResourceTemplate ()  {
      Memory32Fixed (ReadWrite, 0x28039000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {190}
    })

    Device(GP03) {
      Name(_ADR, 0)
      Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"reg",0},
          Package () {"snps,nr-gpios",16},
          Package () {"nr-gpios",16},
        }
      })
    }
  }
}
