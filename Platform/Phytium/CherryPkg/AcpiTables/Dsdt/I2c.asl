/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/
Scope(_SB)
{
  Device (CLK2) {
    Name (_HID, "PHYT8002")
    Name (_CID, "FTCK0002") /* compatible with v1.0 */
    Name (_UID, 0x01)
    Name (_DSD, Package() {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package(2) {"clock-frequency", 50000000}
        }
      })

    Method (FREQ, 0x0, NotSerialized) {
      Return (50000000)
    }
  }

  //MIO3
  Device (I2C0) {
    Name (_HID, "PHYT0038")
    Name (_UID, Zero)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x2801A000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {127}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          250,
          250,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          62,
          62,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          7,
          7,
          0
      })
    }

    //Device DS3231
    Device (RTC0) {
      Name (_HID, "FTDS3231")
      Name (_CID, "DS3231")
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate () {
        I2CSerialBusV2 (0x68, ControllerInitiated, 400000,AddressingMode7Bit, "\\_SB.I2C0", 0, ResourceConsumer, ,)
      })
    }
  }

  //MIO1
  Device (I2C1) {
    Name (_HID, "PHYT0038")
    Name (_UID, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x28016000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {125}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          250,
          250,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          62,
          62,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          7,
          7,
          0
      })
    }

    //Device STK8321
    Device (IIO0) {
      Name (_HID, "STK8321")
      Name (_CID, "STK832X")
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate () {
        I2CSerialBusV2 (0x0F, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1", 0, ResourceConsumer, , Exclusive)
        GpioInt (Level, ActiveLow, ExclusiveAndWake, PullDefault, 0x0, "\\_SB.GPI0", 0, ResourceConsumer, ,) {2}
      })
    }
  }

  //MIO12
  Device (I2C2) {
    Name (_HID, "PHYT0038")
    Name (_UID, 2)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x2802C000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {136}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          250,
          250,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          62,
          62,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          7,
          7,
          0
      })
    }
    //Device STK3311-X
    Device (STK1) {
      Name (_HID, "STK3311")
      Name (_CID, "STK3311")
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate () {
        I2CSerialBusV2 (0x48, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C2", 0, ResourceConsumer, ,)
      })
    }
  }

  //MIO15
  Device (I2C3) {
    Name (_HID, "PHYT0038")
    Name (_UID, 3)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x28032000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {139}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          250,
          250,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          62,
          62,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          7,
          7,
          0
      })
    }
  }

  //MIO14
  Device (I2C4) {
    Name (_HID, "PHYT0038")
    Name (_UID, 4)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x28030000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {138}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 100000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          250,
          250,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          62,
          62,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          7,
          7,
          0
      })
    }
  }
}
