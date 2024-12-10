/** @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
**/

Scope(_SB)
{
  Device (CLK3) {
    Name (_HID, "PHYT8002")
    Name (_CID, "FTCK0002") /* compatible with v1.0 */
    Name (_UID, 0x0)
    Name (_DSD, Package() {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package(2) {"clock-frequency", 200000000}
        }
      })

    Method (FREQ, 0x0, NotSerialized) {
      Return (200000000)
    }
  }
  //can0
  Device(CAN0) {
    Name (_HID, "PHYT000A")
    Name (_CID, "FTCN0001") /* compatible with v1.0 */
    Name(_UID, 0)
    Name(_CRS, ResourceTemplate() {
      Memory32Fixed(ReadWrite, 0x2800A000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {113}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clocks", Package (){"\\_SB.CLK3"}},
        Package () {"clock-frequency", 200000000},
        Package () {"clock-names", "can_clk"},
        Package () {"tx-fifo-depth", 0x40},
        Package () {"rx-fifo-depth", 0x40},
        Package () {"mode-select", "canfd"}        //can or canfd
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }

  //can1
  Device(CAN1) {
    Name (_HID, "PHYT000A")
    Name (_CID, "FTCN0001") /* compatible with v1.0 */
    Name(_UID, 1)
    Name(_CRS, ResourceTemplate() {
      Memory32Fixed(ReadWrite, 0x2800B000, 0x1000)
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {114}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clocks", Package (){"\\_SB.CLK3"}},
        Package () {"clock-frequency", 200000000},
        Package () {"clock-names", "can_clk"},
        Package () {"tx-fifo-depth", 0x40},
        Package () {"rx-fifo-depth", 0x40},
        Package () {"mode-select", "canfd"}        //can or canfd
      }
    })

    Method (_STA, 0, NotSerialized) {
      Return(0x0F)
    }
  }
}
