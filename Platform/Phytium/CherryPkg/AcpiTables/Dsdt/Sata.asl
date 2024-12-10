/**
**/
Scope(_SB)
{
  //SATA0
  Device(AHC0) {
    Name (_HID, "PHYT0020")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x31A40000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {74}
    })
    Name (_CLS, Package (0x3) {
      0x01,
      0x06,
      0x01
    })
    Method (_STA, 0, NotSerialized) {
     Return (0x0F)
    //  Return (0)
    }
  }

  //SATA1
  Device(AHC1) {
    Name (_HID, "PHYT0020")
    Name (_UID, 1)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate() {
      Memory32Fixed (ReadWrite, 0x32014000, 0x2000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {75}
    })
    Name (_CLS, Package (0x3) {
      0x01,
      0x06,
      0x01
    })
    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }
  }
}
