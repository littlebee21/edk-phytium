/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

Scope(_SB)
{
    Device (CLU0) {
      Name(_HID, "ACPI0010")
      Name(_UID, 0)
      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
      Device(CPU0) {
        Name(_HID, "ACPI0007")
        Name(_UID, 0)
        Name (_DSD, Package () {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"clock-name","c0"},
            Package () {"clock-domain",0},
          }
        })

        Method (_STA, 0, NotSerialized) {
          Return(0x0F)
        }
        Name (_PSD, Package ()  // _PSD: Power State Dependencies
        {
          Package (0x05)
          {
            0x05,
            Zero,
            0x0,
            0xFD,
            0x2
          }
        })
        Name (_CPC, Package ()  // _CPC: Continuous Performance Control
        {
          0x17,
          0x03,
          ResourceTemplate ()
          {
            Register (PCC,          //1
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000000, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //2
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000004, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //3
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000008, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 4
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000000c, // Address
                0)
          },
          ResourceTemplate ()
          {
              Register (PCC,        // 5
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000010, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 6
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000014, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 7
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000018, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 8
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000001c, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 9
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000020, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 10
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000024, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 11
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000028, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 12
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000030, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 13
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000038, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 14
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000040, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 15
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000044, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 16
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000048, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 17
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000004c, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 18
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000050, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 19
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000054, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 20
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000058, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 21
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000005c, // Address
                0)
          }
        })
      }

      Device(CPU1) {
        Name(_HID, "ACPI0007")
        Name(_UID, 1)
        Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"clock-name","c0"},
            Package () {"clock-domain",0},
          }
        })
        Method (_STA, 0, NotSerialized) {
          Return(0x0F)
        }
        Name (_PSD, Package ()  // _PSD: Power State Dependencies
        {
          Package (0x05)
          {
            0x05,
            Zero,
            0x0,
            0xFD,
            0x2
          }
        })
        Name (_CPC, Package ()  // _CPC: Continuous Performance Control
        {
          0x17,
          0x03,
          ResourceTemplate ()
          {
            Register (PCC,          //1
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000000, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //2
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000004, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //3
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000008, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 4
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000000c, // Address
                0)
          },
          ResourceTemplate ()
          {
              Register (PCC,        // 5
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000010, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 6
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000014, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 7
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000018, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 8
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000001c, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 9
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000020, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 10
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000024, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 11
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000028, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 12
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000030, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 13
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000038, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 14
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000040, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 15
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000044, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 16
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000048, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 17
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000004c, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 18
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000050, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 19
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000054, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 20
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000058, // Address
                0)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 21
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000005c, // Address
                0)
          }
        })
      }
    }
    Device (CLU1) { // Cluster0 state
      Name(_HID, "ACPI0010")
      Name(_UID, 1)
      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
      Device(CPU2) {
        Name(_HID, "ACPI0007")
        Name(_UID, 2)
        Name (_DSD, Package () {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"clock-name","c1"},
            Package () {"clock-domain",1},
          }
        })
        Method (_STA, 0, NotSerialized) {
          Return(0x0F)
        }
        Name (_PSD, Package ()  // _PSD: Power State Dependencies
        {
          Package (0x05)
          {
            0x05,
            Zero,
            0x1,
            0xFD,
            0x1
          }
        })
        Name (_CPC, Package ()  // _CPC: Continuous Performance Control
        {
          0x17,
          0x03,
          ResourceTemplate ()
          {
            Register (PCC,          //1
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000000, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //2
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000004, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //3
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000008, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 4
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000000c, // Address
                1)
          },
          ResourceTemplate ()
          {
              Register (PCC,        // 5
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000010, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 6
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000014, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 7
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000018, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 8
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000001c, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 9
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000020, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 10
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000024, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 11
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000028, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 12
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000030, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 13
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000038, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 14
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000040, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 15
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000044, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 16
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000048, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 17
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000004c, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 18
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000050, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 19
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000054, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 20
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000058, // Address
                1)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 21
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000005c, // Address
                1)
          }
        })
      }
    }

    Device (CLU2) {
      Name(_HID, "ACPI0010")
      Name(_UID, 2)
      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
      Device(CPU3) {
        Name(_HID, "ACPI0007")
        Name(_UID, 3)
        Name (_DSD, Package () {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package () {
            Package () {"clock-name","c2"},
            Package () {"clock-domain",2},
          }
        })
        Method (_STA, 0, NotSerialized) {
          Return(0x0F)
        }
        Name (_PSD, Package ()  // _PSD: Power State Dependencies
        {
          Package (0x05)
          {
            0x05,
            Zero,
            0x2,
            0xFD,
            0x1
          }
        })
        Name (_CPC, Package ()  // _CPC: Continuous Performance Control
        {
          0x17,
          0x03,
          ResourceTemplate ()
          {
            Register (PCC,          //1
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000000, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //2
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000004, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          //3
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000008, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 4
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000000c, // Address
                2)
          },
          ResourceTemplate ()
          {
              Register (PCC,        // 5
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000010, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 6
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000014, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 7
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000018, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 8
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000001c, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 9
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000020, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 10
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000024, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 11
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000028, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 12
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000030, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 13
                0x40,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000038, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 14
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000040, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 15
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000044, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 16
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000048, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 17
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000004c, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 18
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000050, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 19
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000054, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 20
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x0000000000000058, // Address
                2)
          },
          ResourceTemplate ()
          {
            Register (PCC,          // 21
                0x20,               // Bit Width
                0x00,               // Bit Offset
                0x000000000000005c, // Address
                2)
          }
        })
      }
    }
}
