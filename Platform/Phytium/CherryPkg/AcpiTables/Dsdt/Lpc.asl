/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
  Device (LPC1) {
    Name (_HID, "PHYT0007")
    Name (_CID, "LPC0001") /* compatible with v1.0 */
    Name (_UID, Zero)
    Name (_CRS, ResourceTemplate () {
      Memory32Fixed (ReadWrite, 0x20000000, 0x8000000,)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, ) {119}
    })

    Device (BAT1) {
      Name (_HID, "PHYT000C") /* Battery */
      Name (_CID, "FTEC0002") /* compatible with v1.0 */
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate ()  {
      })
    }

    Device (SEN3) {
      Name (_HID, "PHYT000D") /* Sensor */
      Name (_CID, "FTEC0003") /* compatible with v1.0 */
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate ()  {
      })
    }

    Device(EC0) {
      Name (_HID, "PHYT000B") /* EC */
      Name (_CID, "FTEC0001") /* compatible with v1.0 */
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        GpioInt(Edge, ActiveLow, ExclusiveAndWake, PullUp,,"\\_SB.GPI1") {3}
      })

      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package(){"gpio-sci", Package() {^EC0, 0, 0, 1}},
        }
      })
    }

    //Device (PWM0)
    //{
    //    Name (_HID, "FTBL0001")
    //    Name (_UID, Zero)
    //    Name (_DSD, Package(0x2)
    //    {
    //        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    //        Package(0x2)
    //        {
    //            Package (0x2)
    //            {
    //                "index_display_brightness",
    //                0xE
    //            },
    //            Package(0x2)
    //            {
    //                "max_brightness",
    //                0x64
    //            }
    //        }
    //    })
    //}

    Name (_STA, 1)
  }
}
