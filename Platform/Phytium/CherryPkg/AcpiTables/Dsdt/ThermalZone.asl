/** @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
**/

Scope(_SB)
{
  ThermalZone (TZ0) {
    Method (_PSV) { Return (_C2K(70)) }
    Name (_PSL, Package (){
      \_SB.CLU0.CPU0,
      \_SB.CLU0.CPU1,
      \_SB.CLU1.CPU2,
      \_SB.CLU2.CPU3
    })
    Method (_HOT) { Return (_C2K(85)) }
    Method (_CRT) { Return (_C2K(95)) }
    Method (_TMP) {
      //MHU Address
      OperationRegion(MHUB, SystemMemory, 0x32A00000, 0x1000)
      Field(MHUB, DWordAcc, NoLock, Preserve){
        Offset (0x20),
        SUST, 32,           //0x20 : SCP_UBOOT_STAT
        Offset (0x28),
        SUSE, 32,           //0x28 : SCP_UBOOT_SET
        Offset (0x30),
        SUCL, 32,           //0x30 : SCP_UBOOT_CLEAR
        Offset (0x120),
        AUST, 32,           //0x120 : AP_UBOOT_STAT
        Offset (0x128),
        AUSE, 32,           //0x128 : AP_UBOOT_SET
        Offset (0x130),
        AUCL, 32,           //0x130 : AP_UBOOT_CLEAR
        Offset (0x508),
        AOCG, 32,           //0x508 : AP_OS_CONFIG
        AUCG, 32,           //0x50C : AP_UBOOT_CONFIG
      }
      //Share Memory Address Uboot channel
      OperationRegion(SHRM, SystemMemory, 0x32A10C00, 0x1000)
      Field(SHRM, DWordAcc, NoLock, Preserve){
        RESA, 32,           //Reserved A
        STAT, 32,           //Status
        RESB, 32,           //Reserved B
        RESC, 32,           //Reserved C
        FLAG, 32,           //Flags
        LENG, 32,           //Length
        HEAD, 32,           //Message Header
        PAY0, 32,           //Payload[0]
        PAY1, 32,           //Payload[1]
        PAY2, 32,           //Payload[2]
      }
      Store (SUST, Local0)
      Local0 &= 0x1
      Local1 = 0x1F4                //timeout : 500x10us
      while (LGreater (Local1, 0x0)) {
        Store (SUST, Local0)
        Local0 &= 0x1
        If (LEqual (Local0, 0x0)) {  //Check channel clear
          break;
        }
        If (LEqual (Local1, 0x0)) {
          return (0xFFFFFFFF)
        }
        Local1 -= 1
        Sleep (10)
      }
      Store (0x1, AUCG)
      Store (12, LENG)        //Length = sizeof (MessageHeader) + 8
      Store (0, FLAG)         //Flags = 0
      Store ((0x15 << 10) | 0x6, HEAD)          //Protocol ID 0x15, Message ID 0x6
      Store (0, PAY0)         //Sensor 0
      Store (0, PAY1)         //sync
      Store (STAT, Local0)
      Local0 &= ~(0x1)
      Store (Local0, STAT)    //Mark channel busy
      Sleep (10)
      Store ((0x15 << 10) | 0x6, AUSE)
      Sleep (10)
      Local1 = 0x1F4                       //timeout : 500x10 us
      While (LGreater (Local1, 0x0)) {
       Store (STAT, Local2)
       Local2 &= 0x1
       If (LEqual (Local2, 0x1)) {
         break;
       }
       If (LEqual (Local1, 0x0)) {
         Store (0xFFFFFFFF, SUCL)               //clear channel
         return (0xFFFFFFFF)
       }
       Local1 -= 1
       Sleep (10)
      }
      Store (PAY0, Local3)
      If (LNotEqual (Local3, 0x0)) {
        Return (Local3)
      }
      Store (PAY1, Local4)
      Store (0xFFFFFFFF, SUCL)               //clear channel
      Return (_C2K (Local4))
    }
  }

  Method (_C2K, 1, Serialized) {
    Add (Multiply (Arg0, 0x0A), 0xAAC, Local0)
    If (LLessEqual (Local0, 0x0AAC)) {
      Store (0x0C8C, Local0)
    } Else {
      If (LGreater (Local0, 0x0FAC)) {
        Store (0xC8C, Local0)
      }
    }
    Return (Local0)
  }
}
