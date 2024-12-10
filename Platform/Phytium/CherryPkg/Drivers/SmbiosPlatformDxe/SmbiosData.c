/** @file
  Phytium E2000 SMBIOS Data.

  Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <SmbiosData.h>

#include <IndustryStandard/SmBios.h>

#include <Protocol/Smbios.h>

//BIOS Information (Type 0)
ARM_TYPE0  BiosInfoType0 = {
  {
    { // SMBIOS_STRUCTURE Hdr
      EFI_SMBIOS_TYPE_BIOS_INFORMATION, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE0),      // UINT8 Length
      SMBIOS_HANDLE_BIOSINFOR
    },
    1,                                         //Vendor
    2,                                         //BiosVersion
    0,                                          //BiosSegment
    3,                                         //BiosReleaseDate
    4,                                         //BiosSize
    {                                          //BiosCharacteristics
      0,                                         // Reserved                          :2
      0,                                         // Unknown                           :1
      0,                                         // BiosCharacteristicsNotSupported   :1
      0,                                         // IsaIsSupported                    :1
      0,                                         // McaIsSupported                    :1
      0,                                         // EisaIsSupported                   :1
      1,                                         // PciIsSupported                    :1
      0,                                         // PcmciaIsSupported                 :1
      0,                                         // PlugAndPlayIsSupported            :1
      0,                                         // ApmIsSupported                    :1
      1,                                         // BiosIsUpgradable                  :1
      0,                                         // BiosShadowingAllowed              :1
      0,                                         // VlVesaIsSupported                 :1
      0,                                         // EscdSupportIsAvailable            :1
      0,                                         // BootFromCdIsSupported             :1
      1,                                         // SelectableBootIsSupported         :1
      0,                                         // RomBiosIsSocketed                 :1
      0,                                         // BootFromPcmciaIsSupported         :1
      0,                                         // EDDSpecificationIsSupported       :1
      0,                                         // JapaneseNecFloppyIsSupported      :1
      0,                                         // JapaneseToshibaFloppyIsSupported  :1
      0,                                         // Floppy525_360IsSupported          :1
      0,                                         // Floppy525_12IsSupported           :1
      0,                                         // Floppy35_720IsSupported           :1
      0,                                         // Floppy35_288IsSupported           :1
      0,                                         // PrintScreenIsSupported            :1
      0,                                         // Keyboard8042IsSupported           :1
      0,                                         // SerialIsSupported                 :1
      0,                                         // PrinterIsSupported                :1
      0,                                         // CgaMonoIsSupported                :1
      0,                                         // NecPc98                           :1
      0                                         // ReservedForVendor                 :3
    },
    {
      0x03,                                        //BIOSCharacteristicsExtensionBytes[0]
      //  {                                          //BiosReserved
      //    1,                                         // AcpiIsSupported                   :1
      //    1,                                         // UsbLegacyIsSupported              :1
      //    0,                                         // AgpIsSupported                    :1
      //    0,                                         // I20BootIsSupported                :1
      //    0,                                         // Ls120BootIsSupported              :1
      //    0,                                         // AtapiZipDriveBootIsSupported      :1
      //    0,                                         // Boot1394IsSupported               :1
      //    0                                          // SmartBatteryIsSupported           :1
      //  },
      0x0D                                         //BIOSCharacteristicsExtensionBytes[1]
      //  {                                          //SystemReserved
      //    1,                                         //BiosBootSpecIsSupported            :1
      //    0,                                         //FunctionKeyNetworkBootIsSupported  :1
      //    1,                                         //TargetContentDistributionEnabled   :1
      //    1,                                         //UefiSpecificationSupported         :1
      //    0,                                         //VirtualMachineSupported            :1
      //    0                                          //ExtensionByte2Reserved             :3
      //  },
    },
    0x01,                                     //SystemBiosMajorRelease;
    0x00,                                     //SystemBiosMinorRelease;
    0xFF,                                     //EmbeddedControllerFirmwareMajorRelease;
    0xFF,                                      //EmbeddedControllerFirmwareMinorRelease;
    {                                            //ExtendedBiosSize
      0,                                         //Size
      0                                           //Uint
    }
  },
  TYPE0_STRINGS
};



//System Information (Type 1).
ARM_TYPE1 SystemInfoType1 = {
  {
    {                                               // Hdr
      EFI_SMBIOS_TYPE_SYSTEM_INFORMATION,         // Type,
      sizeof (SMBIOS_TABLE_TYPE1),            // UINT8 Length
      SMBIOS_HANDLE_SYSEMINFO                   // Handle
    },
    1,                                              // Manufacturer
    2,                                              // ProductName
    3,                                              // Version
    4,                                              // SerialNumber
    {                                               // Uuid
      0x8322ea54, 0xd41b, 0x11ed, {0xb7, 0x8f, 0x73, 0xfb, 0x77, 0xae, 0x33, 0x58}
    },
    SystemWakeupTypePowerSwitch,                    // SystemWakeupType
    5,                                              // SKUNumber,
    6                                               // Family
  },

  TYPE1_STRINGS
};

//Base Board (or Module) Information (Type 2)
ARM_TYPE2 BaseboardInfoType2 = {
  {
    {                                                       // Hdr
      EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION,                // Type,
      sizeof (SMBIOS_TABLE_TYPE2),                          // UINT8 Length
      SMBIOS_HANDLE_MOTHERBOARD                               // Handle
    },
    1,                                                      // BaseBoardManufacturer
    2,                                                      // BaseBoardProductName
    3,                                                      // BaseBoardVersion
    4,                                                      // BaseBoardSerialNumber
    5,                                                      // BaseBoardAssetTag
    {                                                       // FeatureFlag
      1,                                                    // Motherboard           :1
      0,                                                    // RequiresDaughterCard  :1
      0,                                                    // Removable             :1
      1,                                                    // Replaceable           :1
      0,                                                    // HotSwappable          :1
      0                                                     // Reserved              :3
    },
    6,                                                      // BaseBoardChassisLocation
    0,                                                      // ChassisHandle;
    BaseBoardTypeMotherBoard,                               // BoardType;
    0,                                                      // NumberOfContainedObjectHandles;
    {
      0
    }                                                       // ContainedObjectHandles[1];
  },
  TYPE2_STRINGS
};

//System Enclosure or Chassis (Type 3)
ARM_TYPE3 SystemEnclosureType3 = {
  {
    {                                                       // Hdr
      EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE  ,                   // Type,
      sizeof (SMBIOS_TABLE_TYPE3),        // UINT8 Length
      SMBIOS_HANDLE_CHASSIS          // Handle
    },
    1,                                                      // Manufactrurer
    MiscChassisTypeDeskTop,                                 // Type
    2,                                                      // Version
    3,                                                      // SerialNumber
    4,                                                      // AssetTag
    ChassisStateSafe,                                       // BootupState
    ChassisStateSafe,                                       // PowerSupplyState
    ChassisStateSafe,                                       // ThermalState
    ChassisSecurityStatusNone,                              // SecurityState
    {
      0,                                                    // OemDefined[0]
      0,                                                    // OemDefined[1]
      0,                                                    // OemDefined[2]
      0                                                     // OemDefined[3]
    },
    2,                                                      // Height
    1,                                                      // NumberofPowerCords
    0,                                                      // ContainedElementCount
    0,                                                      // ContainedElementRecordLength
    {                                                       // ContainedElements[0]
      {
        0,                                                    // ContainedElementType
        0,                                                    // ContainedElementMinimum
        0                                                     // ContainedElementMaximum
      }
    }
  },
  TYPE3_STRINGS
};


//System Boot Information (Type 32)
ARM_TYPE32 SystemBootType32 = {
  {
    {                            // Hdr
      EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION,        // Type,
      sizeof (SMBIOS_TABLE_TYPE32),          // UINT8 Length
      SMBIOS_HANDLE_PI_RESERVED
    },
    {                            // Reserved[6]
      0,
      0,
      0,
      0,
      0,
      0
    },
    BootInformationStatusNoError              // BootInformationStatus
  },
  TYPE32_STRINGS
};

//Processor Infomation (Type 4)
ARM_TYPE4_EFFICIENCY ProcessorInfoType4Efficiency = {
  {
    {                        //Header
      EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION,    //Type
      sizeof(SMBIOS_TABLE_TYPE4),         //Length
      SMBIOS_HANDLE_CLUSTER          //Handle
    },
    1,                        //Socket
    CentralProcessor,                //ProcessorType
    ProcessorFamilyIndicatorFamily2,        //ProcessorFamily
    2,                        //ProcessorManufacture
    {                        //ProcessorId
      {                      //Signature
        0
      },
      {                      //FeatureFlags
        0
      }
    },
    3,                        //ProcessorVersion
    {                        //Voltage
      0
    },
    0,                 //ExternalClock
    CPU_MAX_SPEED,                  //MaxSpeed
    0,                        //CurrentSpeed
    0x41,                        //Status
    ProcessorUpgradeUnknown,            //ProcessorUpgrade
    SMBIOS_HANDLE_L1D,                     //L1Ins
    SMBIOS_HANDLE_L1I,                     //L1Data
    SMBIOS_HANDLE_L2,                       //L2
    4,                        //SerialNumber
    5,                        //AssetTag
    6,                        //PartNumber

    8,                        //CoreCount
    8,                        //EnabledCoreCount
    1,                        //ThreadCount
    0x00EC,                        //ProcessorCharacteristics

    ProcessorFamilyARMv8,                //ProcessorFamily2

    0,                        //CoreCount2
    0,                        //EnabledCoreCount2
    0                        //ThreadCount2
  },
  TYPE4_STRINGS_EFFICIENCY
};

//Processor Infomation (Type 4)
ARM_TYPE4_FUNCTION ProcessorInfoType4Func = {
  {
    {                        //Header
      EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION,    //Type
      sizeof(SMBIOS_TABLE_TYPE4),         //Length
      SMBIOS_HANDLE_CLUSTER_FUNC          //Handle
    },
    1,                        //Socket
    CentralProcessor,                //ProcessorType
    ProcessorFamilyIndicatorFamily2,        //ProcessorFamily
    2,                        //ProcessorManufacture
    {                        //ProcessorId
      {                      //Signature
        0
      },
      {                      //FeatureFlags
        0
      }
    },
    3,                        //ProcessorVersion
    {                        //Voltage
      0
    },
    0,                 //ExternalClock
    CPU_MAX_SPEED,                  //MaxSpeed
    0,                        //CurrentSpeed
    0x41,                        //Status
    ProcessorUpgradeUnknown,            //ProcessorUpgrade
    SMBIOS_HANDLE_L1D_FUNC,                     //L1Ins
    SMBIOS_HANDLE_L1I_FUNC,                     //L1Data
    SMBIOS_HANDLE_L2_FUNC,                       //L2
    4,                        //SerialNumber
    5,                        //AssetTag
    6,                        //PartNumber

    8,                        //CoreCount
    8,                        //EnabledCoreCount
    1,                        //ThreadCount
    0x00EC,                        //ProcessorCharacteristics

    ProcessorFamilyARMv8,                //ProcessorFamily2

    0,                        //CoreCount2
    0,                        //EnabledCoreCount2
    0                        //ThreadCount2
  },
  TYPE4_STRINGS_FUNCTION
};


//Cache Information (Type7) L1 DATA
ARM_TYPE7_L1DATA_EFFICIENCY L1DataType7Efficiency = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L1D              //Handle
    },
    1,                                              //SocketDesignation
    0x0180,                                              //CacheConfiguration
    0,                                              //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorSingleBit,                            //ErrorCorrectionType
    CacheTypeData,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    64,
    64
  },
  TYPE7_L1DATA_STRINGS_EFFICIENCY
};

//Cache Information (Type7) L1 INS
ARM_TYPE7_L1INS_EFFICIENCY L1InsType7Efficiency = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L1I              //Handle
    },
    1,                                              //SocketDesignation
    0x0180,                                              //CacheConfiguration
    0,                                              //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorParity,                            //ErrorCorrectionType
    CacheTypeInstruction,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    64,
    64
  },
  TYPE7_L1INS_STRINGS_EFFICIENCY
};

//Cache Information (Type7) L1 DATA
ARM_TYPE7_L1DATA_FUNCTION L1DataType7Function = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L1D_FUNC              //Handle
    },
    1,                                              //SocketDesignation
    0x0180,                                              //CacheConfiguration
    0,                                              //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorSingleBit,                            //ErrorCorrectionType
    CacheTypeData,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    64,
    64
  },
  TYPE7_L1DATA_STRINGS_FUNCTION
};

//Cache Information (Type7) L1 INS
ARM_TYPE7_L1INS_FUNCTION L1InsType7Function = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L1I_FUNC              //Handle
    },
    1,                                              //SocketDesignation
    0x0180,                                              //CacheConfiguration
    0,                                              //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorParity,                            //ErrorCorrectionType
    CacheTypeInstruction,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    96,
    96
  },
  TYPE7_L1INS_STRINGS_FUNCTION
};

//Cache Information (Type7) L2
ARM_TYPE7_L2_EFFICIENCY L2Type7Efficiency = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L2              //Handle
    },
    1,                                              //SocketDesignation
    0x0281,                                         //CacheConfiguration
    0,                                         //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorSingleBit,                            //ErrorCorrectionType
    CacheTypeUnified,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    256,
    256
  },
  TYPE7_L2_STRINGS_EFFICIENCY
};

//Cache Information (Type7) L2
ARM_TYPE7_L2_FUNCTION L2Type7Function = {
  {
    {                                               //Header
      EFI_SMBIOS_TYPE_CACHE_INFORMATION,          //Type
      sizeof(SMBIOS_TABLE_TYPE7),                 //Length
      SMBIOS_HANDLE_L2_FUNC              //Handle
    },
    1,                                              //SocketDesignation
    0x0281,                                         //CacheConfiguration
    0,                                         //MaximumCacheSize
    0,                                             //InstalledSize
    {                                               //SupportedSRAMType
      0,0,0,0,0,1,0,0
    },
    {                                               //CurrentSRAMType
      0,0,0,0,0,1,0,0
    },
    0,                                              //CacheSpeed
    CacheErrorSingleBit,                            //ErrorCorrectionType
    CacheTypeUnified,                                  //SystemCacheType
    CacheAssociativityDirectMapped,                          //Associativity
    2048,
    2048
  },
  TYPE7_L2_STRINGS_FUNCTION
};

//Bios Language Information (Type13)
ARM_TYPE13 BiosLangInfoType13 = {
  {
    { // SMBIOS_STRUCTURE Hdr
      EFI_SMBIOS_TYPE_BIOS_LANGUAGE_INFORMATION, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE13),      // UINT8 Length
      SMBIOS_HANDLE_PI_RESERVED
    },
    2,
    0,
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    2
  },
  TYPE13_STRINGS
};

//Physical Memory Array (Type 16)
ARM_TYPE16 PhyMemArrayType16 = {
  {
    { // SMBIOS_STRUCTURE Hdr
      EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE16),      // UINT8 Length
      SMBIOS_HANDLE_MEMORY
    },
    MemoryArrayLocationSystemBoard,
    MemoryArrayUseSystemMemory,
    MemoryErrorCorrectionNone,
    0x04000000,
    0xFFFE,
    2
  },
  TYPE16_STRINGS
};

//Memory Device (Type17)
ARM_TYPE17 MemDevType17 = {
  {
    { // SMBIOS_STRUCTURE Hdr
      EFI_SMBIOS_TYPE_MEMORY_DEVICE, // UINT8 Type
      sizeof (SMBIOS_TABLE_TYPE17),  // UINT8 Length
      SMBIOS_HANDLE_DIMM
    },
    SMBIOS_HANDLE_MEMORY, //array to which this module belongs
    0xFFFE,               //no errors
    64, //single DIMM, no ECC is 64bits (for ecc this would be 72)
    64, //data width of this device (64-bits)
    0x4000, //16GB
    0x09,   //FormFactor
    0,      //not part of a set
    1,      //right side of board
    2,      //bank 0
    MemoryTypeDdr4,                  //LP DDR4
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}, //unbuffered
    2400,                            //2400Mhz DDR
    3, //varies between diffrent production runs
    4, //serial
    5, //asset tag
    6, //part number
    0, //attrbute
    0x4000,      //  16G
    2400,      //2400MHz
    1500,      //Max V
    1500,      //Max V
    0,        //Configure V
  },
  TYPE17_STRINGS
};

//Memory Array Mapped Address (Type 19)
ARM_TYPE19 MemArrayMapAddrType19 = {
  {
    {  // SMBIOS_STRUCTURE Hdr
    EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, // UINT8 Type
    sizeof (SMBIOS_TABLE_TYPE19),         // UINT8 Length
    SMBIOS_HANDLE_PI_RESERVED
    },
    0, //invalid, look at extended addr field
    0x1FFFFFF,
    SMBIOS_HANDLE_MEMORY, //handle
    2,
    0,      //starting addr of first 2GB
    0,      //ending addr of first 2GB
  },
  TYPE19_STRINGS

};

