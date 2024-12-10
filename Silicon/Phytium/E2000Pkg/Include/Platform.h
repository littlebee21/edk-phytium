#ifndef  _PLATFORM_H_
#define  _PLATFORM_H_

#include <IndustryStandard/Acpi.h>
#include <Pcie.h>

#define EFI_ACPI_6_1_GIC_ITS_INIT(GicITSHwId, GicITSBase) \
  { \
    EFI_ACPI_6_1_GIC_ITS, sizeof (EFI_ACPI_6_1_GIC_ITS_STRUCTURE), EFI_ACPI_RESERVED_WORD, \
    GicITSHwId, GicITSBase, EFI_ACPI_RESERVED_DWORD\
  }

#define EFI_ACPI_5_1_GICR_STRUCTURE_INIT(    \
    GicRBase, GicRlength)                                   \
  {                                                                                  \
    EFI_ACPI_5_1_GICR, sizeof (EFI_ACPI_5_1_GICR_STRUCTURE), EFI_ACPI_RESERVED_WORD,   \
     GicRBase, GicRlength \
  }

#define EFI_ACPI_6_1_GICC_AFFINITY_STRUCTURE_INIT(                                              \
    ProximityDomain, ACPIProcessorUID, Flags, ClockDomain)                                      \
  {                                                                                             \
    3, sizeof (EFI_ACPI_6_1_GICC_AFFINITY_STRUCTURE),ProximityDomain ,                          \
     ACPIProcessorUID,  Flags,  ClockDomain                                                     \
  }

#define EFI_ACPI_6_1_MEMORY_AFFINITY_STRUCTURE_INIT(                                              \
    ProximityDomain, AddressBaseLow, AddressBaseHigh, LengthLow, LengthHigh, Flags)               \
  {                                                                                               \
    1, sizeof (EFI_ACPI_6_1_MEMORY_AFFINITY_STRUCTURE),ProximityDomain , EFI_ACPI_RESERVED_WORD,  \
    AddressBaseLow, AddressBaseHigh, LengthLow, LengthHigh, EFI_ACPI_RESERVED_DWORD, Flags,       \
    EFI_ACPI_RESERVED_QWORD                                                                       \
  }

#define EFI_ACPI_6_1_GICC_STRUCTURE_INIT(GicId, AcpiCpuUid, Mpidr, Flags, PmuIrq,    \
    GicBase, GicVBase, GicHBase, GsivId, GicRBase, ProcessorPowerEfficiencyClass)    \
  {                                                                                  \
    EFI_ACPI_6_1_GIC, sizeof (EFI_ACPI_6_1_GIC_STRUCTURE), EFI_ACPI_RESERVED_WORD,   \
    GicId, AcpiCpuUid, Flags, 0, PmuIrq, 0, GicBase, GicVBase, GicHBase,             \
    GsivId, GicRBase, Mpidr, ProcessorPowerEfficiencyClass, {0, 0, 0}                \
  }

#define EFI_ACPI_6_1_GIC_DISTRIBUTOR_INIT(GicDistHwId, GicDistBase, GicDistVector, GicVersion) \
  { \
    EFI_ACPI_6_1_GICD, sizeof (EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE), EFI_ACPI_RESERVED_WORD, \
    GicDistHwId, GicDistBase, GicDistVector, GicVersion, \
    {EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE} \
  }

//
// Define the number of each table type.
// This is where the table layout is modified.
//
#define EFI_ACPI_PROCESSOR_LOCAL_GICC_AFFINITY_STRUCTURE_COUNT  64
#define EFI_ACPI_MEMORY_AFFINITY_STRUCTURE_COUNT                10

//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_ARM_OEM_ID           'P','H','Y','L','T','D'   // OEMID 6 bytes long
#define EFI_ACPI_ARM_OEM_TABLE_ID     SIGNATURE_64('P','H','Y','T','I','U','M','.') // OEM table id 8 bytes long
#define EFI_ACPI_ARM_OEM_REVISION     0x1
#define EFI_ACPI_ARM_CREATOR_ID       SIGNATURE_32('P','H','Y','T')
#define EFI_ACPI_ARM_CREATOR_REVISION 0x1

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define ARM_ACPI_HEADER(Signature, Type, Revision) {              \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    { EFI_ACPI_ARM_OEM_ID },        /* UINT8   OemId[6] */        \
    EFI_ACPI_ARM_OEM_TABLE_ID,      /* UINT64  OemTableId */      \
    EFI_ACPI_ARM_OEM_REVISION,      /* UINT32  OemRevision */     \
    EFI_ACPI_ARM_CREATOR_ID,        /* UINT32  CreatorId */       \
    EFI_ACPI_ARM_CREATOR_REVISION   /* UINT32  CreatorRevision */ \
  }

#define ACPI_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'A', 'C', 'P', 'I', 'R', 'A', 'M')
#define CPUS_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'C', 'P', 'U', 'S', 'R', 'A', 'M')

//
// Multiple APIC Description Table
//
#pragma pack (1)

typedef struct {
  EFI_ACPI_6_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER          Header;
  EFI_ACPI_6_0_MEMORY_AFFINITY_STRUCTURE                      Memory[EFI_ACPI_MEMORY_AFFINITY_STRUCTURE_COUNT];
  EFI_ACPI_6_0_GICC_AFFINITY_STRUCTURE                        Gicc[EFI_ACPI_PROCESSOR_LOCAL_GICC_AFFINITY_STRUCTURE_COUNT];
} EFI_ACPI_STATIC_RESOURCE_AFFINITY_TABLE;

//E2000 cpu core max count
#define GIC_INTERFACES_MAX_COUNT        4
#define CORCOUNT_PER_CLUSTER            4

typedef struct {
  EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER   Header;
  EFI_ACPI_6_1_GIC_STRUCTURE                            GicInterfaces[GIC_INTERFACES_MAX_COUNT];
  EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE                GicDistributor;
  EFI_ACPI_6_1_GIC_ITS_STRUCTURE                        GicITS[1];
} EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE;

typedef struct {
  UINT64      Signature;
  UINT32      RpIoBase[PCI_HOST_MAX_COUNT];
  UINT32      RpIoSize[PCI_HOST_MAX_COUNT];
  UINT32      RpMmioBase[PCI_HOST_MAX_COUNT];
  UINT32      RpMmioSize[PCI_HOST_MAX_COUNT];
  UINT64      RpMmio64Base[PCI_HOST_MAX_COUNT];
  UINT64      RpMmio64Size[PCI_HOST_MAX_COUNT];
  UINT8       RpCount;
  UINT8       RpBusBase[PCI_HOST_MAX_COUNT];
  UINT8       RpBusLimit[PCI_HOST_MAX_COUNT];
} EFI_ACPI_RAM_DATA;

#pragma pack ()

typedef struct _DCDP_CONFIG {
  //
  //dp0. dp1 enable : 0-disable, 1-enable
  //
  UINT8  Enable[2];
} DCDP_CONFIG;

#define    DCDP_CONFIG_VARIABLE    L"DcDpConfigVar"
#define    DCDP_VARIABLE_FLAG  (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

#define  PSU_USB30_VENDOR_GUID \
 {0x4306bc67, 0x67dd, 0x44f6, {0x89, 0x1, 0xc5, 0x16, 0xc9, 0x35, 0xf3, 0x9a}}

#define  PSU_USB31_VENDOR_GUID \
 {0xe1c51672, 0xb707, 0x11ec, {0x83, 0xcd, 0xf7, 0x05, 0x0f, 0x2c, 0x74, 0x9f}}

#define  PSU_SATA_VENDOR_GUID \
 {0x6a465504, 0xbafc, 0x11ec, {0x85, 0xc6, 0xbb, 0x70, 0x26, 0x0f, 0x01, 0xf0}}

#define  GSD_SATA_VENDOR_GUID \
 {0xad8312da, 0xbafc, 0x11ec, {0x80, 0xcf, 0xa7, 0xf8, 0x57, 0x06, 0x2e, 0x55}}

#define  DC_DP_VENDOR_GUID \
 {0x224c9658, 0xbf8b, 0x11ec, {0x83, 0xe4, 0x8f, 0x23, 0xdb, 0x29, 0x3e, 0x3b}}

#define  GMU_VENDOR_GUID \
 {0x5cf68726, 0xd64e, 0x11ec, {0xb4, 0x42, 0x8b, 0x1d, 0xab, 0x2b, 0x24, 0x6b}}

#define  USB2_P20_VENDOR_GUID \
 {0xb1a4ed0a, 0x6963, 0x11ed, {0xb4, 0xf5, 0x9f, 0x0b, 0x6f, 0xbb, 0x15, 0xf8}}

#define  USB2_P21_VENDOR_GUID \
 {0xdb6349a2, 0x6963, 0x11ed, {0xba, 0x93, 0xeb, 0xb0, 0x7b, 0x8b, 0xa7, 0xcc}}

#define  USB2_P22_VENDOR_GUID \
 {0x01a7c818, 0x6964, 0x11ed, {0x9a, 0x4f, 0x97, 0x97, 0x31, 0x2d, 0xcd, 0xac}}

#define  USB2_P3_VENDOR_GUID \
 {0x2687cd0e, 0x6964, 0x11ed, {0xbd, 0x1a, 0xbf, 0xe4, 0x23, 0xa2, 0xee, 0x04}}

#define  USB2_P4_VENDOR_GUID \
 {0x4a0d1892, 0x6964, 0x11ed, {0x9e, 0x37, 0x07, 0x5e, 0xa6, 0x90, 0x87, 0x26}}

#endif
