/**
**/
Scope(_SB)
{
  //usb3.0-0
  Device(XHC0) {
    Name (_HID, "PHYT0039")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31A08000, 0x18000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {48}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"reset-on-resume", " "},
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }

  //usb3.0-1
  Device(XHC1) {
    Name (_HID, "PHYT0039")
    Name (_UID, 1)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31A28000, 0x18000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {49}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"reset-on-resume", " "},
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }

  //usb2.0-0
  Device(USB0) {
    Name (_HID, "PHYT0037")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x32800000, 0x40000)
      Memory32Fixed (ReadWrite, 0x32880000, 0x40000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {46}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"dr_mode", "host"}, // "peripheral", "otg"
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }

  //usb2.0-1
  Device(USB1) {
    Name (_HID, "PHYT0037")
    Name (_UID, 1)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x32840000, 0x40000)
      Memory32Fixed (ReadWrite, 0x328C0000, 0x40000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {47}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"dr_mode", "host"}, // "peripheral", "otg"
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }

  //usb2.0 otg-0
  Device(USB2) {
    Name (_HID, "PHYT0037")
    Name (_UID, 2)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31800000, 0x80000)
      Memory32Fixed (ReadWrite, 0x31990000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {64}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"dr_mode", "host"}, // "peripheral", "otg"
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }

  //usb2.0 otg-1
  Device(USB3) {
    Name (_HID, "PHYT0037")
    Name (_UID, 3)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31880000, 0x80000)
      Memory32Fixed (ReadWrite, 0x319A0000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {65}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"dr_mode", "host"}, // "peripheral", "otg"
      }
    })

    Method (_STA, 0, NotSerialized) {
      //Return (0x0F)
      Return (0)
    }
  }

  //usb2.0 otg-2
  Device(USB4) {
    Name (_HID, "PHYT0037")
    Name (_UID, 2)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31900000, 0x80000)
      Memory32Fixed (ReadWrite, 0x319B0000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {66}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"dr_mode", "host"}, // "peripheral", "otg"
      }
    })

    Method (_STA, 0, NotSerialized) {
      //Return (0x0F)
      Return (0)
    }
  }
}
