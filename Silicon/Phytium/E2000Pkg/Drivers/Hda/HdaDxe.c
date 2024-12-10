/** @file
 *
**/
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/ArmSmcLib.h>

#include <Protocol/Cpu.h>

#define HDA_BASE           0x28006000

#define ALC665_DEVICE_ID   0x10EC0665
#define ALC662_DEVICE_ID   0x10EC0662
#define ALC897_DEVICE_ID   0x10EC0897
#define ALC897_DEVICE_LAPTOP_ID   0x10EC0897

#pragma pack (push, 1)
typedef struct {
  UINT16  VendorId;             ///< Codec Vendor ID
  UINT16  DeviceId;             ///< Codec Device ID
  UINT8   RevisionId;           ///< Revision ID of the codec. 0xFF matches any revision.
  UINT8   SdiNum;               ///< SDI number, 0xFF matches any SDI.
  UINT16  DataDwords;           ///< Number of data DWORDs following the header.
} PCH_HDA_VERB_TABLE_HEADER;

typedef struct  {
  PCH_HDA_VERB_TABLE_HEADER  Header;
  UINT32                     Data[];
} HDAUDIO_VERB_TABLE;

#pragma pack(pop)

HDAUDIO_VERB_TABLE ALC665 = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x665,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    68      // Data size in DWORDs
  },
  {
  //===================================================================================================
  //
  //                               Realtek Semiconductor Corp.
  //
  //===================================================================================================
  //
  //Realtek High Definition Audio Configuration - Version : 5.0.2.4
  //Realtek HD Audio Codec : ALC665
  //PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_000010EC
  //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0665&SUBSYS_10EC0000
  //The number of verb command block : 17
  //    NID 0x11 : 0x40040300
  //    NID 0x12 : 0x411111F0
  //    NID 0x13 : 0x411111F0
  //    NID 0x14 : 0x411111F0
  //    NID 0x15 : 0x01813020
  //    NID 0x16 : 0x411111F0
  //    NID 0x17 : 0x411111F0
  //    NID 0x18 : 0x01A1902F
  //    NID 0x19 : 0x02A15030
  //    NID 0x1A : 0x01014012
  //    NID 0x1B : 0x0201E010
  //    NID 0x1D : 0x4025EC01
  //    NID 0x1E : 0x411111F0
  //    NID 0x21 : 0x411111F0
  //===== HDA Codec Subsystem ID Verb-table =====
  //HDA Codec Subsystem ID  : 0x10EC0000
  0x00172000,
  0x00172100,
  0x001722EC,
  0x00172310,
  //===== Pi,n Widget Verb-table =====
  //Widget n,ode 0x01 :
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  //Pin widg,et 0x11 - S/PDIF-OUT2
  0x01171C00,
  0x01171D03,
  0x01171E04,
  0x01171F40,
  //Pin widg,et 0x12 - DMIC-1/2
  0x01271CF0,
  0x01271D11,
  0x01271E11,
  0x01271F41,
  //Pin widg,et 0x13 - DMIC-3/4
  0x01371CF0,
  0x01371D11,
  0x01371E11,
  0x01371F41,
  //Pin widg,et 0x14 - FRONT (Port-D)
  0x01471CF0,
  0x01471D11,
  0x01471E11,
  0x01471F41,
  //Pin widg,et 0x15 - SURR (Port-A)
  0x01571C20,
  0x01571D30,
  0x01571E81,
  0x01571F01,
  //Pin widg,et 0x16 - CEN/LFE (Port-G)
  0x01671CF0,
  0x01671D11,
  0x01671E11,
  0x01671F41,
  //Pin widg,et 0x17 - MONO-OUT (Port-H)
  0x01771CF0,
  0x01771D11,
  0x01771E11,
  0x01771F41,
  //Pin widg,et 0x18 - MIC1 (Port-B)
  0x01871C2F,
  0x01871D90,
  0x01871EA1,
  0x01871F01,
  //Pin widg,et 0x19 - MIC2 (Port-F)
  0x01971C30,
  0x01971D50,
  0x01971EA1,
  0x01971F02,
  //Pin widg,et 0x1A - LINE1 (Port-C)
  0x01A71C12,
  0x01A71D40,
  0x01A71E01,
  0x01A71F01,
  //Pin widg,et 0x1B - LINE2 (Port-E)
  0x01B71C10,
  0x01B71DE0,
  0x01B71E01,
  0x01B71F02,
  //Pin widg,et 0x1D - BEEP-IN
  0x01D71C01,
  0x01D71DEC,
  0x01D71E25,
  0x01D71F40,
  //Pin widg,et 0x1E - S/PDIF-OUT1
  0x01E71CF0,
  0x01E71D11,
  0x01E71E11,
  0x01E71F41,
  //Pin widg,et 0x21 - HP-OUT (Port-I)
  0x02171CF0,
  0x02171D11,
  0x02171E11,
  0x02171F41,
  //Widget n,ode 0x20 :
  0x0205000D,
  0x02044050,
  0x0205000D,
  0x02044050,
  }
};

HDAUDIO_VERB_TABLE ALC662 = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x0662,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    60      // Data size in DWORDs
  },
  {
  //===================================================================================================
  //
  //                               Realtek Semiconductor Corp.
  //
  //===================================================================================================
  //
  //Realtek High Definition Audio Configuration - Version : 5.0.3.2
  //Realtek HD Audio Codec : ALC662-VD
  //PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_000010EC
  //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0662&SUBSYS_10EC0000
  //The number of verb command block : 15
  //
  //    NID 0x12 : 0x40000000
  //    NID 0x14 : 0x01214010
  //    NID 0x15 : 0x411111F0
  //    NID 0x16 : 0x411111F0
  //    NID 0x18 : 0x01A19030
  //    NID 0x19 : 0x02A19050
  //    NID 0x1A : 0x01813040
  //    NID 0x1B : 0x02214020
  //    NID 0x1C : 0x411111F0
  //    NID 0x1D : 0x40400001
  //    NID 0x1E : 0x411111F0
  //
  //
  //===== HDA Codec Subsystem ID Verb-table =====
  //HDA Codec Subsystem ID  : 0x10EC0000
  0x00172000,
  0x00172100,
  0x001722EC,
  0x00172310,
  //
  //
  //===== Pin Widget Verb-table =====
  //Widget node 0x01 :
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  //Pin widget 0x12 - DMIC
  0x01271C00,
  0x01271D00,
  0x01271E13,
  0x01271F40,
  //Pin widget 0x14 - FRONT (Port-D)
  0x01471C10,
  0x01471D40,
  0x01471E01,
  0x01471F01,
  //Pin widget 0x15 - SURR (Port-A)
  0x01571CF0,
  0x01571D11,
  0x01571E11,
  0x01571F41,
  //Pin widget 0x16 - CEN/LFE (Port-G)
  0x01671CF0,
  0x01671D11,
  0x01671E11,
  0x01671F41,
  //Pin widget 0x18 - MIC1 (Port-B)
  0x01871C30,
  0x01871D90,
  0x01871EA1,
  0x01871F01,
  //Pin widget 0x19 - MIC2 (Port-F)
  0x01971C40,
  0x01971D90,
  0x01971EA1,
  0x01971F02,
  //Pin widget 0x1A - LINE1 (Port-C)
  0x01A71C3F,
  0x01A71D30,
  0x01A71E81,
  0x01A71F01,
  //Pin widget 0x1B - LINE2 (Port-E)
  0x01B71C20,
  0x01B71D40,
  0x01B71E21,
  0x01B71F02,
  //Pin widget 0x1C - CD-IN
  0x01C71CF0,
  0x01C71D11,
  0x01C71E11,
  0x01C71F41,
  //Pin widget 0x1D - BEEP-IN
  0x01D71C01,
  0x01D71DC6,
  0x01D71E44,
  0x01D71F40,
  //Pin widget 0x1E - S/PDIF-OUT
  0x01E71CF0,
  0x01E71D11,
  0x01E71E11,
  0x01E71F41,
  //Widget node 0x20 :
  0x02050004,
  0x02040001,
  0x02050004,
  0x02040001,
  //Widget node 0x20 - 1 :
  0x02050005,
  0x02040080,
  0x02050001,
  0x0204A9B8,
  }
};

HDAUDIO_VERB_TABLE ALC897 = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x0897,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    0x6C      // Data size in DWORDs
  },
  {
  //===================================================================================================
  //
  //                               Realtek Semiconductor Corp.
  //
  //===================================================================================================
  //
  //Realtek High Definition Audio Configuration - Version : 5.0.3.2
  //Realtek HD Audio Codec : ALC897
  //PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_089710EC
  //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0897&SUBSYS_10EC0897
  //The number of verb command block : 16
  //
  //    NID 0x11 : 0x40000000
  //    NID 0x12 : 0x411111F0
  //    NID 0x14 : 0x01014010
  //    NID 0x15 : 0x411111F0
  //    NID 0x16 : 0x411111F0
  //    NID 0x17 : 0x411111F0
  //    NID 0x18 : 0x01A19030
  //    NID 0x19 : 0x02A19031
  //    NID 0x1A : 0x0181303F
  //    NID 0x1B : 0x02214020
  //    NID 0x1C : 0x411111F0
  //    NID 0x1D : 0x4024C601
  //    NID 0x1E : 0x411111F0
  //    NID 0x1F : 0x411111F0


  //===== HDA Codec Subsystem ID Verb-table =====
  //HDA Codec Subsystem ID  : 0x10EC0897
  0x001720A6,
  0x00172112,
  0x001722EC,
  0x00172310,

  //===== Pin Widget Verb-table =====
  //Widget node 0x01 :
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  //Pin widget 0x11 - S/PDIF-OUT2
  0x01171C00,
  0x01171D00,
  0x01171E00,
  0x01171F40,
  //Pin widget 0x12 - DMIC
  0x01271CF0,
  0x01271D11,
  0x01271E11,
  0x01271F41,
  //Pin widget 0x14 - FRONT (Port-D)
  0x01471C10,
  0x01471D40,
  0x01471E01,
  0x01471F01,
  //Pin widget 0x15 - SURR (Port-A)
  0x01571CF0,
  0x01571D11,
  0x01571E11,
  0x01571F41,
  //Pin widget 0x16 - CEN/LFE (Port-G)
  0x01671CF0,
  0x01671D11,
  0x01671E11,
  0x01671F41,
  //Pin widget 0x17 - SIDESURR (Port-H)
  0x01771CF0,
  0x01771D11,
  0x01771E11,
  0x01771F41,
  //Pin widget 0x18 - MIC1 (Port-B)
  0x01871C30,
  0x01871D90,
  0x01871EA1,
  0x01871F01,
  //Pin widget 0x19 - MIC2 (Port-F)
  0x01971C31,
  0x01971D90,
  0x01971EA1,
  0x01971F02,
  //Pin widget 0x1A - LINE1 (Port-C)
  0x01A71C3F,
  0x01A71D30,
  0x01A71E81,
  0x01A71F01,
  //Pin widget 0x1B - LINE2 (Port-E)
  0x01B71C20,
  0x01B71D40,
  0x01B71E21,
  0x01B71F02,
  //Pin widget 0x1C - CD-IN
  0x01C71CF0,
  0x01C71D11,
  0x01C71E11,
  0x01C71F41,
  //Pin widget 0x1D - BEEP-IN
  0x01D71C01,
  0x01D71DC6,
  0x01D71E24,
  0x01D71F40,
  //Pin widget 0x1E - S/PDIF-OUT1
  0x01E71CF0,
  0x01E71D11,
  0x01E71E11,
  0x01E71F41,
  //Pin widget 0x1F - S/PDIF-IN
  0x01F71CF0,
  0x01F71D11,
  0x01F71E11,
  0x01F71F41,
  //Pin widget 0x20 -1
  0x0205003B,
  0x02040080,
  0x02050039,
  0x02044031,
  //Pin widget 0x20 -2
  0x02050013,
  0x02040053,
  0x02050030,
  0x020492D1,
  //Pin widget 0x20 -3
  0x02050021,
  0x02040000,
  0x02050023,
  0x02040000,
  //Pin widget 0x20 -4
  0x02050025,
  0x02040000,
  0x02050027,
  0x02040000,
  //Pin widget 0x20 -5
  0x02050029,
  0x02040000,
  0x0205002A,
  0x02041640,
  //Pin widget 0x20 -6
  0x0205002B,
  0x02041640,
  0x02050021,
  0x02040C00,
  //Pin widget 0x20 -7
  0x02050023,
  0x02040C00,
  0x02050025,
  0x02040C00,
  //Pin widget 0x20 -8
  0x02050027,
  0x02040C00,
  0x02050029,
  0x02040C00,
  //Pin widget 0x20 -9
  0x0205002A,
  0x02041641,
  0x0205002B,
  0x02041641,
  //Pin widget 0x20 -10
  0x02050030,
  0x02049251,
  0x0205002A,
  0x02041649,
  //Pin widget 0x20 -11
  0x0205002B,
  0x02041649,
  0x0205000C,
  0x02043F06,
 }
};

HDAUDIO_VERB_TABLE ALC897_LAPTOP = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x0897,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    0x74      // Data size in DWORDs
  },
  {
  //===================================================================================================
  //
  //                               Realtek Semiconductor Corp.
  //
  //===================================================================================================
  //
  //Realtek High Definition Audio Configuration - Version : 5.0.3.3
  //Realtek HD Audio Codec : ALC897
  //PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_000010EC
  //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0897&SUBSYS_10EC0000
  //The number of verb command block : 16
  //
  //    NID 0x11 : 0x40000000
  //    NID 0x12 : 0x90A60140
  //    NID 0x14 : 0x90170110
  //    NID 0x15 : 0x411111F0
  //    NID 0x16 : 0x411111F0
  //    NID 0x17 : 0x411111F0
  //    NID 0x18 : 0x411111F0
  //    NID 0x19 : 0x02A19030
  //    NID 0x1A : 0x411111F0
  //    NID 0x1B : 0x02214020
  //    NID 0x1C : 0x411111F0
  //    NID 0x1D : 0x40240E05
  //    NID 0x1E : 0x411111F0
  //    NID 0x1F : 0x411111F0


  //===== HDA Codec Subsystem ID Verb-table =====
  //HDA Codec Subsystem ID  : 0x10EC0000
  0x00172000,
  0x00172100,
  0x001722EC,
  0x00172310,

  //===== Pin Widget Verb-table =====
  //Widget node 0x01 :
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  0x0017FF00,
  //Pin widget 0x11 - S/PDIF-OUT2
  0x01171C00,
  0x01171D00,
  0x01171E00,
  0x01171F40,
  //Pin widget 0x12 - DMIC
  0x01271C40,
  0x01271D01,
  0x01271EA6,
  0x01271F90,
  //Pin widget 0x14 - FRONT (Port-D)
  0x01471C10,
  0x01471D01,
  0x01471E17,
  0x01471F90,
  //Pin widget 0x15 - SURR (Port-A)
  0x01571CF0,
  0x01571D11,
  0x01571E11,
  0x01571F41,
  //Pin widget 0x16 - CEN/LFE (Port-G)
  0x01671CF0,
  0x01671D11,
  0x01671E11,
  0x01671F41,
  //Pin widget 0x17 - SIDESURR (Port-H)
  0x01771CF0,
  0x01771D11,
  0x01771E11,
  0x01771F41,
  //Pin widget 0x18 - MIC1 (Port-F)
  0x01871CF0,
  0x01871D11,
  0x01871E11,
  0x01871F41,
  //Pin widget 0x19 - MIC2 (Port-B)
  0x01971C30,
  0x01971D90,
  0x01971EA1,
  0x01971F02,
  //Pin widget 0x1A - LINE1 (Port-C)
  0x01A71CF0,
  0x01A71D11,
  0x01A71E11,
  0x01A71F41,
  //Pin widget 0x1B - LINE2 (Port-E)
  0x01B71C20,
  0x01B71D40,
  0x01B71E21,
  0x01B71F02,
  //Pin widget 0x1C - CD-IN
  0x01C71CF0,
  0x01C71D11,
  0x01C71E11,
  0x01C71F41,
  //Pin widget 0x1D - BEEP-IN
  0x01D71C05,
  0x01D71D0E,
  0x01D71E24,
  0x01D71F40,
  //Pin widget 0x1E - S/PDIF-OUT1
  0x01E71CF0,
  0x01E71D11, 0x01E71E11, 0x01E71F41, //Pin widget 0x1F - S/PDIF-IN
  0x01F71CF0,
  0x01F71D11,
  0x01F71E11,
  0x01F71F41,
  //Pin widget 0x20 -0
  0x0205000F,
  0x0204403E,
  0x0205000F,
  0x0204003E,
  //Pin widget 0x20 -1
  0x0205003B,
  0x02040080,
  0x02050039,
  0x02044031,
  //Pin widget 0x20 -2
  0x02050013,
  0x020400C1,
  0x02050030,
  0x020492D1,
  //Pin widget 0x20 -3
  0x02050021,
  0x02040000,
  0x02050023,
  0x02040000,
  //Pin widget 0x20 -4
  0x02050025,
  0x02040000,
  0x02050027,
  0x02040000,
  //Pin widget 0x20 -5
  0x02050029,
  0x02040000,
  0x0205002A,
  0x02041640,
  //Pin widget 0x20 -6
  0x0205002B,
  0x02041640,
  0x02050021,
  0x02040C00,
  //Pin widget 0x20 -7
  0x02050023,
  0x02040C00,
  0x02050025,
  0x02040C00,
  //Pin widget 0x20 -8
  0x02050027,
  0x02040C00,
  0x02050029,
  0x02040C00,
  //Pin wid,et 0x20 -9
  0x0205002A,
  0x02041641,
  0x0205002B,
  0x02041641,
  //Pin widget 0x20 -10
  0x02050030,
  0x02049251,
  0x0205002A,
  0x02041649,
  //Pin widget 0x20 -11
  0x0205002B,
  0x02041649,
  0x0205000C,
  0x02043F06,
  //Pin widget 0x20 -12
  0x02050004,
  0x02040523,
  0x02050004,
  0x02040523,
  }
};

HDAUDIO_VERB_TABLE GW_VERBT = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x665,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    64      // Data size in DWORDs
  },
  //
  //Verb Table Data
  //
  {
    0x00172000,
    0x00172100,
    0x001722ec,
    0x00172310,
    0x0017ff00,
    0x0017ff00,
    0x0017ff00,
    0x0017ff00,
    0x01271c40,
    0x01271d01,
    0x01271ea6,
    0x01271f90,
    0x01471c10,
    0x01471d01,
    0x01471e17,
    0x01471f90,
    0x01771c00,
    0x01771d00,
    0x01771e00,
    0x01771f40,
    0x01871c30,
    0x01871d90,
    0x01871ea1,
    0x01871f02,
    0x01971cf0,
    0x01971d11,
    0x01971e11,
    0x01971f41,
    0x01a71cf0,
    0x01a71d11,
    0x01a71e11,
    0x01a71f41,
    0x01b71cf0,
    0x01b71d11,
    0x01b71e11,
    0x01b71f41,
    0x01d71c05,
    0x01d71d1e,
    0x01d71ef5,
    0x01d71f40,
    0x01e71cf0,
    0x01e71d11,
    0x01e71e11,
    0x01e71f41,
    0x02171c20,
    0x02171d40,
    0x02171e21,
    0x02171f02,
    0x02050011,
    0x02040e10,
    0x02050012,
    0x02041000,
    0x0205000d,
    0x02044440,
    0x0205000d,
    0x02044440,
    0x02050002,
    0x0204aab8,
    0x02050008,
    0x02040300,
    0x02050017,
    0x020400af,
    0x02050007,
    0x02040040,
  }
};
HDAUDIO_VERB_TABLE YD_ALC269 = {
  //
  //  VerbTable: (Realtek ALC298) for PantherMtn
  //  Revision ID = 0xff
  //  Codec Verb Table for Panther Mountain and StarBrook
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0298
  //
  {
    0x10EC,     // Vendor ID
    0x0269,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    68      // Data size in DWORDs
  },
  {
  //===================================================================================================
  //
  //                               Realtek Semiconductor Corp.
  //
  //===================================================================================================
  //
  //Realtek High Definition Audio Configuration - Version : 5.0.3.2
  //Realtek HD Audio Codec : ALC662-VD
  //PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_000010EC
  //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0662&SUBSYS_10EC0000
  //The number of verb command block : 15
  //
  //    NID 0x12 : 0x40000000
  //    NID 0x14 : 0x01214010
  //    NID 0x15 : 0x411111F0
  //    NID 0x16 : 0x411111F0
  //    NID 0x18 : 0x01A19030
  //    NID 0x19 : 0x02A19050
  //    NID 0x1A : 0x01813040
  //    NID 0x1B : 0x02214020
  //    NID 0x1C : 0x411111F0
  //    NID 0x1D : 0x40400001
  //    NID 0x1E : 0x411111F0
  //
  //
  //===== HDA Codec Subsystem ID Verb-table =====
  //HDA Codec Subsystem ID  : 0x10EC0000
0x00172000,
0x00172100,
0x001722EC,
0x00172310,


//===== Pin Widget Verb-table =====
//Widget node 0x01 :
0x0017FF00,
0x0017FF00,
0x0017FF00,
0x0017FF00,
//Pin widget 0x12 - DMIC
0x01271C00,
0x01271D00,
0x01271E00,
0x01271F40,
//Pin widget 0x14 - SPEAKER-OUT (Port-D)
0x01471C10,
0x01471D01,
0x01471E17,
0x01471F90,
//Pin widget 0x15 - HP-OUT (Port-A)
0x01571C20,
0x01571D40,
0x01571E2B,
0x01571F04,
//Pin widget 0x17 - MONO-OUT (Port-H)
0x01771CF0,
0x01771D11,
0x01771E11,
0x01771F41,
//Pin widget 0x18 - MIC1 (Port-B)
0x01871C40,
0x01871D90,
0x01871EAB,
0x01871F04,
//Pin widget 0x19 - MIC2 (Port-F)
0x01971C30,
0x01971D01,
0x01971EA7,
0x01971F90,
//Pin widget 0x1A - LINE1 (Port-C)
0x01A71CF0,
0x01A71D11,
0x01A71E11,
0x01A71F41,
//Pin widget 0x1B - LINE2 (Port-E)
0x01B71CF0,
0x01B71D11,
0x01B71E11,
0x01B71F41,
//Pin widget 0x1D - PC-BEEP
0x01D71C05,
0x01D71D85,
0x01D71E44,
0x01D71F40,
//Pin widget 0x1E - S/PDIF-OUT
0x01E71CF0,
0x01E71D11,
0x01E71E11,
0x01E71F41,
//Widget node 0x20 :
0x02050018,
0x02040184,
0x0205001C,
0x02040800,
//Widget node 0x20 - 1 :
0x02050024,
0x02040000,
0x02050004,
0x02040080,
//Widget node 0x20 - 2 :
0x02050008,
0x02040300,
0x0205000C,
0x02043F00,
//Widget node 0x20 - 3 :
0x02050015,
0x02048002,
0x02050015,
0x02048002,
//Widget node 0x0C :
0x00C37080,
0x00270610,
0x00D37080,
0x00370610,
  }
};

VOID
RegConfig (
  IN EFI_PHYSICAL_ADDRESS CorbBase,
  IN EFI_PHYSICAL_ADDRESS RirbLBase
  )
{
  //1.写 ’h8寄存器为32‘h0，确认hda控制器处于复位状态11
  MmioWrite32(HDA_BASE + 0x8, 0x0);
  gBS->Stall(1000);
  //3.写 ’h8寄存器为32‘h1，使得hda控制器跳出复位状态
  MmioWrite32 (HDA_BASE + 0x8, 0x1);
  gBS->Stall (2000);

  //5.读取‘he寄存器，及states寄存器，读到的是32‘h1，表示sdi0信号线上挂了一颗codec
  DEBUG ((DEBUG_ERROR, "Hda States = %x \n", MmioRead32(HDA_BASE + 0xc)));

  gBS->Stall (1000);

  //7.写‘hc寄存器为32’h000f0000，清除states寄存器
  MmioWrite32 (HDA_BASE + 0xc, 0xf0000);

  //8.写’h20寄存器为32‘h0，清除中断输出控制，即当前hda不会输出中断
  MmioWrite32 (HDA_BASE + 0x20, 0x0);

  //9.写‘h40寄存器，设定corb的低32位地址（低7bits要为0，128Byte对齐）
  MmioWrite32 (HDA_BASE + 0x40, (UINT32)CorbBase);
  //10. 写‘h44寄存器，设定corb的高32位地址
  MmioWrite32 (HDA_BASE + 0x44, 0x0);
  //11.写’h48寄存器为32‘h80000000，复位corb的写指针
  MmioWrite32 (HDA_BASE + 0x48, 0x80000000);

  //12.写‘h4c寄存器为32’h00420000 设定corb size是256个条目
  MmioWrite32 (HDA_BASE + 0x4c, 0x420000);

  //13.写‘h4c寄存器为32’h00420002，使能corb的dma功能
  MmioWrite32 (HDA_BASE + 0x4c, 0x420002);

  //14.写‘h50寄存器，设定rirb的低32位地址，注意低7位仍要是0
  MmioWrite32 (HDA_BASE + 0x50, (UINT32)RirbLBase);

  //15. 写’h54寄存器，设定rirb的高32位地址
  MmioWrite32 (HDA_BASE + 0x54, 0x0);

  //16.写‘h58寄存器为32’h00018000，复位rirbwp
  MmioWrite32 (HDA_BASE + 0x58, 0x8000);

  //17.写‘h5c寄存器为32’h00420000，设定rirb的size
  MmioWrite32 (HDA_BASE + 0x5c, 0x420000);

  //18. 写‘h5c寄存器为32’h00420003，使能能rirb的dma，以及rirb中断
  MmioWrite32 (HDA_BASE + 0x5c, 0x420002);
}

UINT32
GetCodecId (
  IN EFI_PHYSICAL_ADDRESS   RirbLBase,
  IN EFI_CPU_ARCH_PROTOCOL  *gCpu
  )
{
  EFI_STATUS           Status;
  UINT32               CodecId;
  EFI_PHYSICAL_ADDRESS Id_Cmd;

  Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 1, &Id_Cmd);
  ASSERT_EFI_ERROR (Status);

  MmioWrite32 (Id_Cmd, 0x0);
  MmioWrite32 (Id_Cmd + 4, 0xf0000);
  MmioWrite32 (Id_Cmd + 8, 0xf0000);
  gCpu->FlushDataCache (gCpu, Id_Cmd, 0x20, EfiCpuFlushTypeWriteBack);
  RegConfig (Id_Cmd,RirbLBase);
  MmioWrite32 (HDA_BASE + 0x48, 1);
  MmioWrite32 (HDA_BASE + 0x48, 2);
  gBS->Stall (1000);
  CodecId = MmioRead32 (RirbLBase +  8);
  gBS->FreePages (Id_Cmd, 1);

  return CodecId;
}

EFI_STATUS
EFIAPI
InitializeHdaDxe (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  UINT32                 Index;
  EFI_PHYSICAL_ADDRESS   CorbBase;
  EFI_PHYSICAL_ADDRESS   RirbLBase;
  EFI_CPU_ARCH_PROTOCOL  *gCpu;
  HDAUDIO_VERB_TABLE     *VerbTable;
  //ARM_SMC_ARGS           ArmSmcArgs;

  Index = 0;

  //MmioWrite32(SDC_BASE + 0x3c, 0x1001);
  //MmioWrite32(SDC_BASE + 0xa8, 0x12221222);

  Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 1, &CorbBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a() L%d : %r\n",__FUNCTION__, __LINE__, Status));
    return Status;
  }

  Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, 1, &RirbLBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a() L%d : %r\n",__FUNCTION__, __LINE__, Status));
    return Status;
  }

  DEBUG((EFI_D_INFO, "%x\n", CorbBase));
  DEBUG((EFI_D_INFO, "%x\n", RirbLBase));

  // Get the Cpu protocol for later use
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&gCpu);
  ASSERT_EFI_ERROR(Status);

  //CodecId = GetCodecId (RirbLBase, gCpu);
  //DEBUG((EFI_D_INFO, "HD Audio Codec ID:0x%x\n", CodecId));

  //switch (CodecId) {
  //case ALC662_DEVICE_ID:
  //  VerbTable = &ALC662;
  //  break;
  //case ALC665_DEVICE_ID:
  //  VerbTable = &ALC665;
  //  break;
  //case ALC897_DEVICE_ID:
  //  VerbTable = &ALC897;
  //  break;
  //default:
  //  VerbTable = &ALC662;
  //}
  VerbTable = &GW_VERBT;
  CopyMem ((VOID *)(CorbBase + 4), (VOID *)VerbTable->Data, VerbTable->Header.DataDwords * 4);
  //
  //Flush Dcache
  //
  gCpu->FlushDataCache (gCpu, CorbBase, 0x1000, EfiCpuFlushTypeWriteBack);

  RegConfig(CorbBase,RirbLBase);
  for (Index = 1; Index <= VerbTable->Header.DataDwords; Index++) {
    MmioWrite32 (HDA_BASE + 0x48, Index);
    gBS->Stall (1000);
    DEBUG ((EFI_D_INFO, "cmd = %x \n", MmioRead32(HDA_BASE + 0x214)));
    gBS->Stall (1000);
    DEBUG ((EFI_D_INFO, "reponds = %x  \n", MmioRead32((RirbLBase + Index * 8))));
  }

  //1. 写‘h4c寄存器为32’h00420000，停止corb的dma功能
  MmioWrite32 (HDA_BASE + 0x4c, 0x420000);
  //2. 写‘h5c寄存器为32‘h00420000，停止rirb的dma功能
  MmioWrite32 (HDA_BASE + 0x5c, 0x420000);
  MmioWrite32(HDA_BASE + 0x8, 0x0);

  //ArmSmcArgs.Arg0 = 0xc300fff4;
  //ArmSmcArgs.Arg1 = (1 << 5);
  //ArmCallSmc (&ArmSmcArgs);

  gBS->FreePages (CorbBase, 1);
  gBS->FreePages (RirbLBase, 1);

  return Status;
}



