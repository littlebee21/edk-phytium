/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device(SPI0) {
    Name(_HID, "PHYT000E")
    Name(_UID, 0)
    Name(_CRS, ResourceTemplate() {
      Memory32Fixed(ReadWrite, 0x2803C000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {193}
    })
    Name (_DSD, Package() {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package() {"clocks", 50000000},
          //Package() {"clocks-spi", 600000000},
          Package() {"num-cs", 4}
        }
    })

    Device(FLAS) {
      Name(_HID, "SPT0001")
      Name(_UID, 0)
      Name (_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"reg",0},
            //Package() {"spi-max-frequency", 48000000}
          }
      })

      Name(_CRS, ResourceTemplate () {
        // Index 0
        SPISerialBus (           // SCKL - GPIO 11 - Pin 23
                                 // MOSI - GPIO 10 - Pin 19
                                 // MISO - GPIO 9  - Pin 21
                                 // CE0  - GPIO 8  - Pin 24
          0,                     // Device selection (CE0)
          PolarityLow,           // Device selection polarity
          FourWireMode,          // WireMode
          8,                     // DataBit len
          ControllerInitiated,   // Slave mode
          4000000,               // Connection speed
          ClockPolarityLow,      // Clock polarity
          ClockPhaseFirst,       // Clock phase
          "\\_SB.SPI0",          // ResourceSource: SPI bus controller name
          0,                     // ResourceSourceIndex
                                 // Resource usage
                                 // DescriptorName: creates name for offset of resource descriptor
        )                        // Vendor Data
      })
    }
  }
}
