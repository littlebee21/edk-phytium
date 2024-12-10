/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device (HDA0) {
    Name (_HID, "PHYT0006")
    Name (_CID, "FTHD0001") /* compatible with v1.0 */
    Name (_UID, 0)
    Name (_CCA, 1)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x28006000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {110}
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }
}
