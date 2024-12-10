/** @file
  The description of PWM Ctroller

  Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

Scope(_SB)
{

    Device (PWM0) {
      Name (_HID, "PHYT0029")
      Name (_UID, 0)
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x2804A000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {205}
      })

      Name (_DSD, Package () {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"clock-frequency", 50000000},
            Package ()
            {
              "phytium,db",
              Package ()
              {
                0,  // 0:modulo mode,count loop 0-TxCCR 1:up-and-down mode,count between 0-TxCCR-0
                0,  // 0:capture mode  1:comparison mode
                0,  // Division mode parameter(div range in 1-4096)  0: 1 freq div  1: 2 freq div
                0,  // Rising edge delay period, range in 0-1023
                0,  // Falling edge delay period, range in 0-1023
                0   // Polarity selection control
                    // 00: active high(AH)pwm0_out and pwn1_out no overturn
                    // 01: active low complementary(ALC)pwm0_out overturn
                    // 10: active high complementary(AHC)pwm1_out overturn
                    // 11: active low(AL)pwm0_out and pwm1_out overturn
              }
            }
          }
      })

      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
    }

}

