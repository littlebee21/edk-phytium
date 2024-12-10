/** @file


**/
#include "Type.h"

//
//Psu Usb3-0 DevicePath
//
HII_VENDOR_DEVICE_PATH  mPsuUsb30DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    PSU_USB30_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Psu Usb3-0 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuUsb30Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdPsuUsb30BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdPsuUsb30BaseAddress) + FixedPcdGet64 (PcdPsuUsb30Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdPsuUsb30Region),                // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Psu Usb3-1 DevicePath
//
HII_VENDOR_DEVICE_PATH  mPsuUsb31DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    PSU_USB31_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Psu Usb3-1 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuUsb31Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdPsuUsb31BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdPsuUsb31BaseAddress) + FixedPcdGet64 (PcdPsuUsb31Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdPsuUsb31Region),                // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Usb2 P2-0 DevicePath
//
HII_VENDOR_DEVICE_PATH  mUsb2P20DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    USB2_P20_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Usb2 P2-0 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P20Desc[] = {
  {
    //
    //Region 0 Controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x31800000,                                       // AddrRangeMin
    0x31800000  + 0x80000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x80000,                                          // AddrLen
  }, {
    //
    //Region 1 phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x31990000,                                       // AddrRangeMin
    0x31990000  + 0x10000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x10000,                                          // AddrLen
  }, {
    //
    //Region 2 Share Memory
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Usb2 P2-1 DevicePath
//
HII_VENDOR_DEVICE_PATH  mUsb2P21DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    USB2_P21_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Usb2 P2-1 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P21Desc[] = {
  {
    //
    //Region 0 Controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x31880000,                                       // AddrRangeMin
    0x31880000  + 0x80000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x80000,                                          // AddrLen
  }, {
    //
    //Region 1 phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x319A0000,                                       // AddrRangeMin
    0x319A0000  + 0x10000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x10000,                                          // AddrLen
  }, {
    //
    //Region 2 Share Memory
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Usb2 P2-2 DevicePath
//
HII_VENDOR_DEVICE_PATH  mUsb2P22DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    USB2_P22_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Usb2 P2-2 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P22Desc[] = {
  {
    //
    //Region 0 Controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x31900000,                                       // AddrRangeMin
    0x31900000  + 0x80000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x80000,                                          // AddrLen
  }, {
    //
    //Region 1 phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x319B0000,                                       // AddrRangeMin
    0x319B0000  + 0x10000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x10000,                                          // AddrLen
  }, {
    //
    //Region 2 Share Memory
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Usb2 P3 DevicePath
//
HII_VENDOR_DEVICE_PATH  mUsb2P3DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    USB2_P3_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Usb2 P3 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P3Desc[] = {
  {
    //
    //Region 0 Controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x32800000,                                       // AddrRangeMin
    0x32800000  + 0x40000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x40000,                                          // AddrLen
  }, {
    //
    //Region 1 phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x32880000,                                       // AddrRangeMin
    0x32880000  + 0x40000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x40000,                                          // AddrLen
  }, {
    //
    //Region 2 Share Memory
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Usb2 P4 DevicePath
//
HII_VENDOR_DEVICE_PATH  mUsb2P4DevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    USB2_P4_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Usb2 P4 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mUsb2P4Desc[] = {
  {
    //
    //Region 0 Controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x32840000,                                       // AddrRangeMin
    0x32840000  + 0x40000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x40000,                                          // AddrLen
  }, {
    //
    //Region 1 phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0x328C0000,                                       // AddrRangeMin
    0x328C0000  + 0x40000  - 1,                       // AddrRangeMax
    0,                                                // AddrTranslationOffset
    0x40000,                                          // AddrLen
  }, {
    //
    //Region 2 Share Memory
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Psu Sata DevicePath
//
HII_VENDOR_DEVICE_PATH  mPsuSataDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    PSU_SATA_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Psu Sata Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mPsuSataDesc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdPsuSataBaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdPsuSataBaseAddress) + FixedPcdGet64 (PcdPsuSataRegion) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdPsuSataRegion),                // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Gsd Sata DevicePath
//
HII_VENDOR_DEVICE_PATH  mGsdSataDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    GSD_SATA_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Gsd Sata Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGsdSataDesc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdGsdSataBaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdGsdSataBaseAddress) + FixedPcdGet64 (PcdGsdSataRegion) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdGsdSataRegion),                // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//Dc Dp Device Path
//
HII_VENDOR_DEVICE_PATH  mDcDpDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    DC_DP_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//
//Dc Dp Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mDcDpDesc[] = {
  {
    //
    //dc dp controller
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdDcDpBaseAddress),               // AddrRangeMin
    FixedPcdGet64 (PcdDcDpBaseAddress) + FixedPcdGet64 (PcdDcDpRegion) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdDcDpRegion),                    // AddrLen
  }, {
    //
    //dp phy
    //
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdDpPhyBaseAddress),               // AddrRangeMin
    FixedPcdGet64 (PcdDpPhyBaseAddress) + FixedPcdGet64 (PcdDpPhyRegion) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdDpPhyRegion),                   // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                 // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//GMU0 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu0Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdGmu0BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdGmu0BaseAddress) + FixedPcdGet64 (PcdGmu0Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdGmu0Region),                // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                 // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//GMU1 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu1Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdGmu1BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdGmu1BaseAddress) + FixedPcdGet64 (PcdGmu1Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdGmu1Region),                // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                 // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//GMU2 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu2Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdGmu2BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdGmu2BaseAddress) + FixedPcdGet64 (PcdGmu2Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdGmu2Region),                // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                 // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};

//
//GMU3 Description
//
EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR mGmu3Desc[] = {
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    FixedPcdGet64 (PcdGmu3BaseAddress),           // AddrRangeMin
    FixedPcdGet64 (PcdGmu3BaseAddress) + FixedPcdGet64 (PcdGmu3Region) - 1,    // AddrRangeMax
    0,                                                // AddrTranslationOffset
    FixedPcdGet64 (PcdGmu3Region),                // AddrLen
  }, {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                    // Desc
    sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3,   // Len
    ACPI_ADDRESS_SPACE_TYPE_MEM,                      // ResType
    0,                                                // GenFlag
    0,                                                // SpecificFlag
    32,                                               // AddrSpaceGranularity
    0,                                                 // AddrRangeMin
    0,                                                // AddrRangeMax
    0,                                                // AddrTranslationOffset
    4096,                                             // AddrLen
  }, {
    ACPI_END_TAG_DESCRIPTOR                           // Desc
  }
};
