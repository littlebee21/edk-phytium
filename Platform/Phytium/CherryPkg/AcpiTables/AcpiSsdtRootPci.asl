/** @file
    Copyright (c) 2020, Phytium Limited. All rights reserved.
**/

#include "ArmPlatform.h"

/*
  See ACPI 6.1 Section 6.2.13

  There are two ways that _PRT can be used. ...

  In the first model, a PCI Link device is used to provide additional
  configuration information such as whether the interrupt is Level or
  Edge triggered, it is active High or Low, Shared or Exclusive, etc.

  In the second model, the PCI interrupts are hardwired to specific
  interrupt inputs on the interrupt controller and are not
  configurable. In this case, the Source field in _PRT does not
  reference a device, but instead contains the value zero, and the
  Source Index field contains the global system interrupt to which the
  PCI interrupt is hardwired.

  We use the first model with link indirection to set the correct
  interrupt type as PCI defaults (Level Triggered, Active Low) are not
  compatible with GICv2.
*/

#define PCI_OSC_SUPPORT() \
  Name(SUPP, Zero) /* PCI _OSC Support Field value */ \
  Name(CTRL, Zero) /* PCI _OSC Control Field value */ \
  Method(_OSC,4) { \
    If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) { \
      /* Create DWord-adressable fields from the Capabilities Buffer */ \
      CreateDWordField(Arg3,0,CDW1) \
      CreateDWordField(Arg3,4,CDW2) \
      CreateDWordField(Arg3,8,CDW3) \
      /* Save Capabilities DWord2 & 3 */ \
      Store(CDW2,SUPP) \
      Store(CDW3,CTRL) \
      /* Only allow native hot plug control if OS supports: */ \
      /* ASPM */ \
      /* Clock PM */ \
      /* MSI/MSI-X */ \
      If(LNotEqual(And(SUPP, 0x16), 0x16)) { \
        And(CTRL,0x1E,CTRL) \
      }\
      \
      /* Do not allow native PME, AER */ \
      /* Never allow SHPC (no SHPC controller in this system)*/ \
      And(CTRL,0x10,CTRL) \
      /*And(CTRL,0x1D,CTRL)*/ \
      If(LNotEqual(Arg1,One)) { /* Unknown revision */ \
        Or(CDW1,0x08,CDW1) \
      } \
      \
      If(LNotEqual(CDW3,CTRL)) { /* Capabilities bits were masked */ \
        Or(CDW1,0x10,CDW1) \
      } \
      \
      /* Update DWORD3 in the buffer */ \
      Store(CTRL,CDW3) \
      Return(Arg3) \
    } Else { \
      Or(CDW1,4,CDW1) /* Unrecognized UUID */ \
      Return(Arg3) \
    } \
  } // End _OSC

#define LNK_DEVICE(Unique_Id, Link_Name, irq)              \
  Device(Link_Name) {                  \
      Name(_HID, EISAID("PNP0C0F"))              \
      Name(_UID, Unique_Id)                \
      Name(_PRS, ResourceTemplate() {              \
          Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive) { irq }    \
      })                      \
      Method (_CRS, 0) { Return (_PRS) }              \
      Method (_SRS, 1) { }                \
      Method (_DIS) { }                  \
  }

#define PRT_ENTRY(Address, Pin, Link)                  \
        Package (4) {                                                                             \
            Address,    /* uses the same format as _ADR */                                        \
            Pin,        /* The PCI pin number of the device (0-INTA, 1-INTB, 2-INTC, 3-INTD). */  \
            Link,       /* Interrupt allocated via Link device. */                             \
            Zero        /* global system interrupt number (no used) */          \
          }

/*
  See Reference [1] 6.1.1
  "High word–Device #, Low word–Function #. (for example, device 3, function 2 is
   0x00030002). To refer to all the functions on a device #, use a function number of FFFF)."
*/
#define ROOT_PRT_ENTRY(Dev, Pin, Link)   PRT_ENTRY(Dev * 0x10000 + 0xFFFF, Pin, Link)


DefinitionBlock("SsdtPci.aml", "SSDT", 2, "PHYLTD", "PHYTIUM.", EFI_ACPI_ARM_OEM_REVISION) {
Scope(_SB) {
  //Device (IXIU) {
  //  Name (_HID, "PHYT0013")
  //  Name (_UID, 0)
  //  Name(_CRS, ResourceTemplate(){
  //     Memory32Fixed(ReadWrite, 0x29000000, 0x60000)
  //     Memory32Fixed(ReadWrite, 0x29100000, 0x2000)
  //  })

  //  Name (_DSD, Package () {
  //    ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
  //    Package () {
  //      Package () {"intx-spi-base", 36},
  //    }
  //  })
  //}

  External (\_SB.DBGC, MethodObj)
  LNK_DEVICE(1, LNKA, 36)
  LNK_DEVICE(2, LNKB, 37)
  LNK_DEVICE(3, LNKC, 38)
  LNK_DEVICE(4, LNKD, 39)

  // reserve ECAM memory range
  Device(RES0)
  {
     Name(_HID, EISAID("PNP0C02"))
     Name(_UID, 0)
     Name(_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
          0x0, // Granularity
          0x40000000, // Range Minimum
          0x4FFFFFFF, // Range Maximum
          0, // Translation Offset
          0x10000000, // Length
          ,,)
    })
  }

  Device(PCI0)
  {
      Name(_HID, EISAID("PNP0A08")) // PCI Express Root Bridge
      Name(_CID, EISAID("PNP0A03")) // Compatible PCI Root Bridge
      Name(_SEG, Zero) // PCI Segment Group number
      Name(_BBN, 0)    // PCI Base Bus Number
      Name(_CCA, 1)    // Initially mark the PCI coherent (for JunoR1)

    // PCI Routing Table
    Name(_PRT, Package() {
      ROOT_PRT_ENTRY(0, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(0, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(0, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(0, 3, LNKD),   // INTD

      ROOT_PRT_ENTRY(1, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(1, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(1, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(1, 3, LNKD),   // INTD

      ROOT_PRT_ENTRY(2, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(2, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(2, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(2, 3, LNKD),   // INTD

      ROOT_PRT_ENTRY(3, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(3, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(3, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(3, 3, LNKD),   // INTD

      ROOT_PRT_ENTRY(4, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(4, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(4, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(4, 3, LNKD),   // INTD

      ROOT_PRT_ENTRY(5, 0, LNKA),   // INTA
      ROOT_PRT_ENTRY(5, 1, LNKB),   // INTB
      ROOT_PRT_ENTRY(5, 2, LNKC),   // INTC
      ROOT_PRT_ENTRY(5, 3, LNKD),   // INTD

    })

    // Root complex resources
    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        WordBusNumber ( // Bus numbers assigned to this root
          ResourceProducer,
          MinFixed, MaxFixed, PosDecode,
          0,   // AddressGranularity
          0,   // AddressMinimum - Minimum Bus Number
          255, // AddressMaximum - Maximum Bus Number
          0,   // AddressTranslation - Set to 0
          256  // RangeLength - Number of Busses
        )

        DWordMemory ( // 32-bit BAR Windows
          ResourceProducer, PosDecode,
          MinFixed, MaxFixed,
          Cacheable, ReadWrite,
          0x00000000,               // Granularity
          0x58000000,               // Min Base Address
          0x7FFFFFFF,               // Max Base Address
          0x00000000,               // Translate
          0x28000000                // Length
        )

        QWordMemory ( // 64-bit BAR Windows
          ResourceProducer, PosDecode,
          MinFixed, MaxFixed,
          Cacheable, ReadWrite,
          0x00000000,               // Granularity
          0x1000000000,               // Min Base Address
          0x1FFFFFFFFF,               // Max Base Address
          0x0000000000,               // Translate
          0x1000000000                // Length
        )

        DWordIo ( // IO window
          ResourceProducer,
          MinFixed,
          MaxFixed,
          PosDecode,
          EntireRange,
          0x00000000,               // Granularity
          0x00000000,               // Min Base Address
          0x00efffff,                // Max Base Address
          0x50000000,               // Translate
          0x00f00000,                // Length
          ,,,TypeTranslation
        )
      }) // Name(RBUF)

      Return (RBUF)
    } // Method(_CRS)

    PCI_OSC_SUPPORT()
      Method (_DMA, 0)
      {
        Return (ResourceTemplate () {
          QWORDMemory (
            ResourceConsumer,
            PosDecode,
            MinFixed,
            MaxFixed,
            Prefetchable,
            ReadWrite,
            0x0,
            0x0,
            0xFFFFFFFFFF,
            0x0,
            0x10000000000,
            ,
            ,
            ,
          )
        })
      }
    Name (_STA, 0xF)
  } // PCI0
 }
}
