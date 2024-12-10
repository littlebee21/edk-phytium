/** @file
  Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.
**/
Scope (_SB) {
  Device (Dev0) {
    Name (_HID, "ESSX8336")
    Name (_UID, Zero)
    Name (_CRS, ResourceTemplate () {
      I2CSerialBusV2 (0x10, ControllerInitiated, 100000,AddressingMode7Bit, "\\_SB.I2C4", 0, ResourceConsumer, ,)
      //GpioIo (Exclusive, PullUp, 0, 0, IoRestrictionOutputOnly, "\\_SB.PCI0.RP01.BRG1.BRG2.Gp00", 0, ResourceConsumer) { 26 }
      GpioInt (Edge, ActiveHigh, ExclusiveAndWake, PullUp,,"\\_SB.Gp02") {5}
    })

    Name (_DSD, Package()
    {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"det-gpios", Package(){^Dev0, 0, 0, 0}},
          //Package () {"sel-gpios", Package () {^Dev0, 0, 0, 0}},
        }
    })

    Name (_STA, 0xF)
  }

  //machine node
  Device (Dev1)
  {
    Name (_HID, "PHYT8005")

    Name (_DSD, Package()
    {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"dmic-exist", 1},
          Package () {"headset-mic", 2},
          Package () {"internal-mic", 0},
          Package () {"headphone-exist", 1},
          Package () {"speaker-exist", 1},
        }
    })

    Name (_STA, 0xF)
  }

  //Corresponding I2s
  Device (I2S0) {
    Name (_HID, "PHYT0016")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x28009000, 0x1000)
      Memory32Fixed (ReadWrite, 0x28005000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {109}
    })

    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () {"dai-name", "phytium-i2s-lsd"},
          Package () {"i2s_clk", 600000000},
        }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }
}
