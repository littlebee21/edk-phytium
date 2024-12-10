/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device (VPU0) {
    Name (_HID, "PHYT0019")
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x32B00000, 0x20000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {44}
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }
}

