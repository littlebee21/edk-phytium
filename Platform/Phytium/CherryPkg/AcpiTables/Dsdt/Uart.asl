/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/
Scope(_SB)
{
    //UART 1
    Device(UAR1) {
      Name(_HID, "ARMH0011")
      Name(_UID, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2800d000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {116}
      })

      Method (_STA, 0, NotSerialized) {
        Return(0x0F)
      }
    }

    //UART 0
    Device(UAR0) {
      Name(_HID, "ARMH0011")
      Name(_UID, 0)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2800c000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {115}
      })

      Method (_STA, 0, NotSerialized) {
      Return(0x0F)
      }
    }

    //UART 2
    Device(UAR2) {
      Name(_HID, "ARMH0011")
      Name(_UID, 2)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2800e000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {117}
      })

      Method (_STA, 0, NotSerialized) {
      Return(0x0F)
      }
    }

    //UART 3
    Device(UAR3) {
      Name(_HID, "ARMH0011")
      Name(_UID, 3)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2800f000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {118}
      })

      Method (_STA, 0, NotSerialized) {
      Return(0x0F)
      }
    }

    //Method(TCOM, 1, Serialized) {
    //  OperationRegion(COM0, SystemMemory, FixedPcdGet64 (PcdSerialRegisterBase), 0x1000)
    //  Field(COM0, DWordAcc, NoLock, Preserve) {
    //    DAT8, 8,
    //    Offset(0x18),
    //    ,5,
    //    TRDY, 1,
    //  }

    //  Add(SizeOf(Arg0), One, Local0)
    //  Name(BUF0, Buffer(Local0){})
    //  Store(Arg0, BUF0)
    //  store(0, Local1)
    //  Decrement(Local0)
    //  While(LNotEqual(Local1, Local0)){
    //    while(LEqual(TRDY, ONE)){}
    //    Store(DerefOf(Index(BUF0, Local1)), DAT8)
    //    Increment(Local1)
    //  }
    //}

    //Method(DBGC, 3, Serialized) {      // DBGC(count, string, int)
    //  Name(CRLF, Buffer(2){0x0D, 0x0A})
    //  TCOM(Arg1)
    //  if(LEqual(Arg0, 2)){
    //    TCOM(ToHexString(Arg2))
    //  }
    //  TCOM(CRLF)
    //}
}
