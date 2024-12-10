## @file
# This package provides common open source Phytium Platform modules.
#
# Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.
#
# SPDX-License-Identifier:BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = CherryPkg
  PLATFORM_GUID                  = 65b7d346-a4fe-11ec-8ccf-6f3838dcb398
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001c
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Phytium/CherryPkg/CherryPkg.fdf
  PLATFORM_PACKAGE               = Platform/Phytium/CherryPkg
  GENERAL_PACKAGE                = Silicon/Phytium/PhytiumCommonPkg
  SILICON_PACKAGE                = Silicon/Phytium/E2000Pkg

  RES_640_480                    = 0
  RES_1024_768                   = 1
  RES_1280_720                   = 2
  RES_1280_800                   = 3
  RES_1920_1080                  = 4
  RES_800_1280                   = 5
                     

  DEFINE SETUP_RESOLUTION        = RES_1024_768
  DEFINE BOARD_TYPE              = PHYTIUM_A
  #DEFINE BOARD_TYPE              = PHYTIUM_B
  #DEFINE BOARD_TYPE              = PHYTIUM_C
  #DEFINE BOARD_TYPE                = PHYTIUM_D
  #DEFINE BOARD_TYPE              = PHYTIUM_G
  #DEFINE BOARD_TYPE               = EBMC

  DEFINE DEBUG_ENABLE            = TRUE
  DEFINE PCI_USB_ENABLE          = TRUE
  #DT_ENABLE : true - use devicetree, false - use acpi
  DEFINE DT_OR_ACPI              = DT
 # DEFINE DT_OR_ACPI              = ACPI
  DEFINE NETWORK_ENABLE          = TRUE
  DEFINE SOC_USB3_ENABLE         = TRUE
  DEFINE I2C_RTC_ENABLE          = TRUE
  DEFINE HDA_ENABLE              = TRUE
  DEFINE THUNK_GOP               = FALSE
  DEFINE PHYTIUM_GOP             = TRUE
  DEFINE GMU_ENABLE              = TRUE
  DEFINE E2K_BMC_DEBUG           = FALSE
  DEFINE EMMC_ENABLE             = FALSE
  DEFINE X100_GOP                = FALSE
  DEFINE PS2_ENABLE              = FALSE

!include $(GENERAL_PACKAGE)/PhytiumCommonPkg.dsc.inc

[LibraryClasses.common]
  # Phytium Platform library
  ArmPlatformLib|$(SILICON_PACKAGE)/Library/PlatformLib/PlatformLib.inf

  #E2000Pkg RTC Driver
  #RealTimeClockLib|$(SILICON_PACKAGE)/Library/RealTimeClockLib/RealTimeClockLib.inf
!if $(I2C_RTC_ENABLE) == TRUE
  RealTimeClockLib|$(SILICON_PACKAGE)/Library/I2cRtcLib/I2cRtcLib.inf
!else
  RealTimeClockLib|EmbeddedPkg/Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
!endif
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
  #IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  #OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  #BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf

  # PL011 UART Driver and Dependency Libraries
  SerialPortLib|ArmPlatformPkg/Library/PL011SerialPortLib/PL011SerialPortLib.inf
  PL011UartClockLib|ArmPlatformPkg/Library/PL011UartClockLib/PL011UartClockLib.inf
  PL011UartLib|ArmPlatformPkg/Library/PL011UartLib/PL011UartLib.inf
  PhytiumPowerControlLib|$(SILICON_PACKAGE)/Library/PhytiumPowerControlLib/PhytiumPowerControlLib.inf
  ParameterTableLib|$(SILICON_PACKAGE)/Library/ParameterTableLib/ParameterTable.inf
  I2CLib|$(SILICON_PACKAGE)/Library/DwI2CLib/DwI2CLib.inf
  PlatformBootManagerLib|$(PLATFORM_PACKAGE)/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  DmaLib|EmbeddedPkg/Library/NonCoherentDmaLib/NonCoherentDmaLib.inf
  PadLib|$(SILICON_PACKAGE)/Library/PadLib/PadLib.inf
  AcpiHelperLib|DynamicTablesPkg/Library/Common/AcpiHelperLib/AcpiHelperLib.inf
  AcpiLib|EmbeddedPkg/Library/AcpiLib/AcpiLib.inf
  DynamicAmlLib|DynamicTablesPkg/Library/Common/AmlLib/AmlLib.inf
  AcpiHelperLib|DynamicTablesPkg/Library/Common/AcpiHelperLib/AcpiHelperLib.inf
  EcLib|$(GENERAL_PACKAGE)/Library/EcLib/Ec.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  #ScmiLib
  ScmiLib|$(GENERAL_PACKAGE)/Library/ScmiLib/ScmiLib.inf
  ScmiProtocolLib|$(GENERAL_PACKAGE)/Library/ScmiProtocolLib/ScmiProtocolLib.inf
 #EcLib
  PhytiumEcLib|$(GENERAL_PACKAGE)/Library/EcLib/Ec.inf
 #GpioLib
  GpioLib|$(SILICON_PACKAGE)/Library/GpioLib/Gpio.inf
   #
  #MM
  #
  MmFrameLib|$(GENERAL_PACKAGE)/Library/MmFrameLib/MmFrameLib.inf
  MmInterfaceLib|$(GENERAL_PACKAGE)/Library/MmInterfaceLib/MmInterfaceLib.inf
 #PwmLib
  PwmLib|$(SILICON_PACKAGE)/Library/PwmLib/Pwm.inf 

[LibraryClasses.common.DXE_DRIVER]
  # Pci dependencies
  #PciSegmentLib|$(SILICON_PACKAGE)/Library/PciSegmentLib/PciSegmentLib.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciHostBridgeLib|$(SILICON_PACKAGE)/Library/PciHostBridgeLib/PciHostBridgeLib.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFixedAtBuild.common]
  #!ifdef $(FIRMWARE_VER)
    gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"$(FIRMWARE_VER)"
  #!else
  #  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"Development Build Phytium E2k"
  #!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Cherry Platform"

  gArmTokenSpaceGuid.PcdVFPEnabled|1
  gArmPlatformTokenSpaceGuid.PcdCoreCount|1

  #
  # NV Storage PCDs.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0xe00000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase64|0xe10000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase64|0xe20000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|FALSE

  # Size of the region used by UEFI in permanent memory (Reserved 64MB)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x04000000

  # ParaTable Base
  gPhytiumPlatformTokenSpaceGuid.PcdParameterTableBase|0x380F5000
  #gPhytiumPlatformTokenSpaceGuid.PcdParameterTableBase|0x381A0000

  #
  # PL011 - Serial Terminal
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x2800D000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultReceiveFifoDepth|0
  #gArmPlatformTokenSpaceGuid.PL011UartClkInHz|48000000
  gArmPlatformTokenSpaceGuid.PL011UartClkInHz|100000000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
  gArmPlatformTokenSpaceGuid.PL011UartInterrupt|116

  #
  # ARM General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x30800000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x30880000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x30840000

  #
  # SCMI
  #
  gPhytiumPlatformTokenSpaceGuid.PcdMhuBaseAddress|0x32A00120
  gPhytiumPlatformTokenSpaceGuid.PcdMhuShareMemoryBase|0x32A10C00
  gPhytiumPlatformTokenSpaceGuid.PcdMhuConfigBaseAddress|0x32A00500

  # System IO space
  gPhytiumPlatformTokenSpaceGuid.PcdSystemIoBase|0x20000000
  gPhytiumPlatformTokenSpaceGuid.PcdSystemIoSize|0x20000000

  #
  # System Memory (2GB ~ 4GB - 64MB), the top 64MB is reserved for
  # PBF(the processor basic firmware, Mainly deals the initialization
  # of the chip).
  #
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x70000000

  # Stack Size
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x30C1A000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x10000

  #
  # Designware PCI Root Complex
  #
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0x40000000
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|28
  gPhytiumPlatformTokenSpaceGuid.PcdPciConfigBase|0x40000000
  gPhytiumPlatformTokenSpaceGuid.PcdPciConfigSize|0x10000000
  gArmTokenSpaceGuid.PcdPciBusMin|0
  gArmTokenSpaceGuid.PcdPciBusMax|255
  gArmTokenSpaceGuid.PcdPciIoBase|0x00000
  gArmTokenSpaceGuid.PcdPciIoSize|0xf00000
  gArmTokenSpaceGuid.PcdPciMmio32Base|0x58000000
  gArmTokenSpaceGuid.PcdPciMmio32Size|0x28000000
  gArmTokenSpaceGuid.PcdPciMmio64Base|0x1000000000
  gArmTokenSpaceGuid.PcdPciMmio64Size|0x1000000000
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDisableBusEnumeration|FALSE
  #
  #SBSA Watchdog
  #
  gArmTokenSpaceGuid.PcdGenericWatchdogControlBase|0x28041000
  gArmTokenSpaceGuid.PcdGenericWatchdogRefreshBase|0x28040000
  gArmTokenSpaceGuid.PcdGenericWatchdogEl2IntrNum|196

  #
  # SPI Flash Control Register Base Address and Size
  #
  #gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashBase|0x38000000
  #gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashSize|0x1000000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashBase|0
  gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashSize|0x1000000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerBase|0x28008000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerSize|0x1000
  #
  # DDR Info
  #
  gPhytiumPlatformTokenSpaceGuid.PcdSpdI2cControllerBaseAddress | 0x28020000
  gPhytiumPlatformTokenSpaceGuid.PcdSpdI2cControllerSpeed | 400000
  gPhytiumPlatformTokenSpaceGuid.PcdDdrI2cAddress             | {0x50}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrChannelCount           | 0x1

  gPhytiumPlatformTokenSpaceGuid.PcdType|3
  gPhytiumPlatformTokenSpaceGuid.PcdHeight|0
  gPhytiumPlatformTokenSpaceGuid.PcdBiosSize|0xFF

!if $(DT_OR_ACPI) == "DT"
  #Cppc function disable when using devicetree
  gPhytiumPlatformTokenSpaceGuid.PcdCppcFunctionEnable|FALSE
  gEmbeddedTokenSpaceGuid.PcdDefaultDtPref|TRUE
!else
  #Cppc function Enable when using ACPI
  gPhytiumPlatformTokenSpaceGuid.PcdCppcFunctionEnable|TRUE
  gEmbeddedTokenSpaceGuid.PcdDefaultDtPref|FALSE
!endif

  #gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28020000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerSlaveAddress|0x68
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerSpeed|400000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcBaseAddress|0x2803E000

  #
  #Soc Usb
  #
  gPhytiumPlatformTokenSpaceGuid.PcdPsuUsb30BaseAddress|0x31A08000
  gPhytiumPlatformTokenSpaceGuid.PcdPsuUsb30Region|0x18000
  gPhytiumPlatformTokenSpaceGuid.PcdPsuUsb31BaseAddress|0x31A28000
  gPhytiumPlatformTokenSpaceGuid.PcdPsuUsb31Region|0x18000
  #
  #gsd sata
  #
  gPhytiumPlatformTokenSpaceGuid.PcdGsdSataBaseAddress|0x32014000
  gPhytiumPlatformTokenSpaceGuid.PcdGsdSataRegion|0x2000
  #
  #psu sata
  #
  gPhytiumPlatformTokenSpaceGuid.PcdPsuSataBaseAddress|0x31A40000
  gPhytiumPlatformTokenSpaceGuid.PcdPsuSataRegion|0x2000
  #
  #usb 3.0 oc bypass
  #
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb30OverrunBypass|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb31OverrunBypass|TRUE
  #
  #usb 2.0 oc bypass
  #
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb20OverrunBypass|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb21OverrunBypass|TRUE
  #
  #dcdp config:
  #  bit 0 : dp0 enable, 0-disable, 1-enable
  #  bit 1 : dp1 enable, 0-disable, 1-enable
  #Board Type:
  #  4 - Phytium-A
  #  5 - Phytium-B
  #  8 - Phytium-G
  #  9 - Phytium-BMC
  #Phytium A Board
!if $(BOARD_TYPE) == "PHYTIUM_A"
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0x4
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28026000
  #Mac 0
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Interface|0x2
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Autoeng|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Speed|0x3E8
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Duplex|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Mac|{0x01, 0x11, 0x22, 0x33, 0x44, 0x55}
  #Mac 1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Enable|FALSE
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Interface|0x3
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Autoeng|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Speed|0x3E8
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Duplex|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Mac|{0x02, 0x11, 0x22, 0x33, 0x44, 0x55}
  #Mac 2
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Enable|FALSE
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Interface|0x3
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Autoeng|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Speed|0x3E8
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Duplex|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Mac|{0x03, 0x11, 0x22, 0x33, 0x44, 0x55}
  #Mac 3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Enable|FALSE
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Interface|0x3
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Autoeng|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Speed|0x64
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Duplex|0x1
  #gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Mac|{0x04, 0x11, 0x22, 0x33, 0x44, 0x55}
  #usb
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb30Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb31Enable|TRUE
  #sata
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumPsuSataEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGsdSataEnable|FALSE
  #dp
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel0Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel1Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdEmmcDxeBaseAddress|0x28000000
  #Phytium B Board
!elseif $(BOARD_TYPE) == "PHYTIUM_B"
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0x5
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x2801A000
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Interface|0x3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Autoeng|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Speed|0x3E8
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Duplex|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Mac|{0x03, 0x11, 0x22, 0x33, 0x44, 0x55}
  #usb
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb30Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb31Enable|TRUE
  #sata
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumPsuSataEnable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGsdSataEnable|FALSE
  #dp
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel0Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel1Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdEmmcDxeBaseAddress|0x28001000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28024000
!elseif $(BOARD_TYPE) == "PHYTIUM_G"
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0x8
  #gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28032000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x2801A000
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Enable|FALSE
  #usb
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb30Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb31Enable|TRUE
  #sata
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumPsuSataEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGsdSataEnable|TRUE
  #dp
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel0Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel1Enable|FALSE
    #eMMC
  gPhytiumPlatformTokenSpaceGuid.PcdEmmcDxeBaseAddress|0x28000000
!elseif $(BOARD_TYPE) == "PHYTIUM_D"
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0xa
  #gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28032000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28014000  #MIO0   low 4KB is control reg
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu0Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu1Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu2Enable|FALSE
  #usb
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb30Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumUsb31Enable|FALSE
  #sata
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumPsuSataEnable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGsdSataEnable|FALSE
  #dp
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel0Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpChanel1Enable|TRUE
#SATA0 and PCIe auto detect function
	gPhytiumPlatformTokenSpaceGuid.PcdSata0PcieAutoDetectEnableEnable|TRUE
	gPhytiumPlatformTokenSpaceGuid.PcdSata0PcieAutoDetectPort|0x0E  
  
	gPhytiumPlatformTokenSpaceGuid.PcdPhytiumQosConfig|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdEmmcDxeBaseAddress|0x28000000
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Interface|0x3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Autoeng|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Speed|0x3E8
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Duplex|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumGmu3Mac|{0x03, 0x11, 0x22, 0x33, 0x44, 0x55}

!elseif $(BOARD_TYPE) == "EBMC"
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0x9
  #gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x28032000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x2801A000
!else
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpConfig|0x3
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumBoardType|0x4
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x2801A000
!endif
  #edp backlight set
  gPhytiumPlatformTokenSpaceGuid.PcdEdpBacklightEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdBacklightEnableGpio|0x14
  gPhytiumPlatformTokenSpaceGuid.PcdBacklightPwrGpio|0x28
  gPhytiumPlatformTokenSpaceGuid.PcdBacklightPwmPort|3
  #pad table enable
  gPhytiumPlatformTokenSpaceGuid.PcdPhytiumPadTableEnable|TRUE
  #ddr training information save address
  gPhytiumPlatformTokenSpaceGuid.PcdDdrTrainInfoSaveBaseAddress|0x600000
  #eMMC bus width 1, 4 or 8
  gPhytiumPlatformTokenSpaceGuid.PcdEmmcBusWidth|4
  ##Mhu
  #gPhytiumPlatformTokenSpaceGuid.PcdMhuBaseAddress|0x32A00120
  #gPhytiumPlatformTokenSpaceGuid.PcdMhuShareMemoryBase|0x32A10C00
  #gPhytiumPlatformTokenSpaceGuid.PcdMhuConfigBaseAddress|0x32A00500
  #Dp Cma
  gPhytiumPlatformTokenSpaceGuid.PcdDcDpCmaEnable|TRUE
  #SATA0 and PCIe auto detect function
  gPhytiumPlatformTokenSpaceGuid.PcdSata0PcieAutoDetectEnableEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdSata0PcieAutoDetectPort|0x0E
  #Usb2.0
  gPhytiumPlatformTokenSpaceGuid.PcdUsb2P20Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdUsb2P21Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdUsb2P22Enable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdUsb2P3Enable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdUsb2P4Enable|TRUE
  #
  #MM Communication Buffer
  #
  gArmTokenSpaceGuid.PcdMmBufferBase|0xFC400000
  gArmTokenSpaceGuid.PcdMmBufferSize|0x100000

[PcdsDynamicDefault.common.DEFAULT]
  ## This PCD defines the video horizontal resolution.
  ##  This PCD could be set to 0 then video resolution could be at highest resolution.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|800
  ## This PCD defines the video vertical resolution.
  ##  This PCD could be set to 0 then video resolution could be at highest resolution.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|600

  #  This PCD could be set to 0 then video resolution could be at highest resolution.
  ## This PCD defines the Console output row and the default value is 80 according to UEFI spec.
  ##This PCD could be set to 0 then console output could be at max column and max row.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|128
  ## This PCD defines the Console output column and the default value is 25 according to UEFI spec.
  ##  This PCD could be set to 0 then console output could be at max column and max row.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40
  #
  ## Specify the video horizontal resolution of text setup.
  ## @Prompt Video Horizontal Resolution of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|800

  ## Specify the video vertical resolution of text setup.
  ## @Prompt Video Vertical Resolution of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|600
  # @Prompt Video Horizontal Resolution of Text Setup

  ## Specify the console output column of text setup.
  ## @Prompt Console Output Column of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
  ## Specify the console output row of text setup.
  ## @Prompt Console Output Row of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40

  !if $(SETUP_RESOLUTION) == RES_640_480
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|640
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|480
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|25

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|640
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|480
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|80
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|25
  !elseif $(SETUP_RESOLUTION) == RES_1024_768
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|128
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40
  !elseif $(SETUP_RESOLUTION) == RES_1280_720
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|720
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|160
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|37

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|720
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|160
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|37

  !elseif $(SETUP_RESOLUTION) == RES_1280_800
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|800
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|160
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|42

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|800
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|160
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|42

  !elseif $(SETUP_RESOLUTION) == RES_1920_1080
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|240
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|56

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|240
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|56

  !elseif $(SETUP_RESOLUTION) == RES_800_1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|800
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|100
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|67

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|800
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1280
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|100
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|67

  !else
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|640
    gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|480
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80
    gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|25

    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|640
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|480
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|80
    gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|25
  !endif

  ## The number of seconds that the firmware will wait before initiating the original default boot selection.
  #  A value of 0 indicates that the default boot selection is to be initiated immediately on boot.
  #  The value of 0xFFFF then firmware will wait for user input before booting.
  # @Prompt Boot Timeout (s)
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|3

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  #
  # PEI Phase modules
  #
  #ArmPlatformPkg/PrePeiCore/PrePeiCoreMPCore.inf
  ArmPlatformPkg/PrePeiCore/PrePeiCoreUniCore.inf
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  $(SILICON_PACKAGE)/PlatformPei/PlatformPeim.inf
  #ArmPlatformPkg/PlatformPei/PlatformPeim.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/CpuPei/CpuPei.inf
  UefiCpuPkg/CpuIoPei/CpuIoPei.inf
  MdeModulePkg/Universal/FaultTolerantWritePei/FaultTolerantWritePei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  ArmPlatformPkg/MemoryInitPei/MemoryInitPeim.inf
  #ArmPlatformPkg/PrePeiCore/PrePeiCoreUniCore.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf

  #
  # PCD database
  #
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
      OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

  #
  # Dxe core entry
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf

  #
  # DXE driver
  #
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf

  #
  # Common Arm Timer and Gic Components
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf

  #
  # security system
  #
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
  }

  #
  #Platform Init Dxe
  #
#!if $(BOARD_TYPE) == "PHYTIUM_A"
#  $(PLATFORM_PACKAGE)/Drivers/PlatformDxe/Phytium-A/PlatformDxeA.inf
#!elseif $(BOARD_TYPE) == "PHYTIUM_B"
#  $(PLATFORM_PACKAGE)/Drivers/PlatformDxe/Phytium-B/PlatformDxeB.inf
#!elseif $(BOARD_TYPE) == "GW_F080"
#  $(PLATFORM_PACKAGE)/Drivers/PlatformDxe/GreatWallF080/PlatformDxe.inf
#!else
#  $(PLATFORM_PACKAGE)/Drivers/PlatformDxe/Phytium-A/PlatformDxeA.inf
#!endif
  $(PLATFORM_PACKAGE)/Drivers/PlatformDxe/PlatformDxe.inf

!if $(HDA_ENABLE) == TRUE
  $(SILICON_PACKAGE)/Drivers/Hda/HdaDxe.inf
!endif

!if $(THUNK_GOP) == TRUE
  Drivers/BiosVideoThunkDxe/BiosVideo.inf
!endif
  #
  #
  #Ps2
  #
!if $(PS2_ENABLE) == TRUE
  $(SILICON_PACKAGE)/Drivers/Ps2KeyboardDxe/Ps2KeyboardDxe.inf
  $(SILICON_PACKAGE)/Drivers/LpcDxe/LpcDxe.inf
!endif
  #
  # network,  mod for https boot.
  #
!if $(NETWORK_ENABLE) == TRUE
  NetworkPkg/SnpDxe/SnpDxe.inf
  NetworkPkg/DpcDxe/DpcDxe.inf
  NetworkPkg/MnpDxe/MnpDxe.inf
  NetworkPkg/ArpDxe/ArpDxe.inf
  NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
  NetworkPkg/Ip4Dxe/Ip4Dxe.inf
  NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf
  NetworkPkg/Udp4Dxe/Udp4Dxe.inf
  NetworkPkg/VlanConfigDxe/VlanConfigDxe.inf

  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
  NetworkPkg/TcpDxe/TcpDxe.inf

  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf

  NetworkPkg/DnsDxe/DnsDxe.inf
  NetworkPkg/HttpUtilitiesDxe/HttpUtilitiesDxe.inf
  NetworkPkg/HttpDxe/HttpDxe.inf
!endif

  #
  # FV Filesystem
  #
  MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf

  #
  # Common Console Components
  # ConIn,ConOut,StdErr
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  #MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  #MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  #MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  $(GENERAL_PACKAGE)/Console/ConSplitterDxe/ConSplitterDxe.inf
  $(GENERAL_PACKAGE)/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  $(GENERAL_PACKAGE)/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf

  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf

  #
  # Hii database init
  #
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf

  #
  # Generic Watchdog Timer
  #
  ArmPkg/Drivers/GenericWatchdogDxe/GenericWatchdogDxe.inf

  #
  # Spi driver
  #
  $(SILICON_PACKAGE)/Drivers/SpiDxe/SpiDxe.inf

  #
  # NOR Flash driver
  #
  $(SILICON_PACKAGE)/Drivers/SpiNorFlashDxe/SpiNorFlashDxe.inf
  $(GENERAL_PACKAGE)/Drivers/FlashFvbDxe/FlashFvbDxe.inf
   # UpdateBios
   #
   $(SILICON_PACKAGE)/Drivers/UpdateBiosApp/UpdateBiosApp.inf   
  #
  #
  # Usb Support
  #
!if $(PCI_USB_ENABLE) == TRUE
  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
!endif

!if $(SOC_USB3_ENABLE) == TRUE
  $(SILICON_PACKAGE)/Drivers/SocXhciDxe/XhciDxe.inf
!endif
  $(SILICON_PACKAGE)/Drivers/Usb2Dxe/Usb2Dxe.inf

  #
  # IDE/AHCI Support
  #
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf
  $(SILICON_PACKAGE)/Drivers/SocAhciDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  $(SILICON_PACKAGE)/Drivers/SocAtaAtapiPassThru/AtaAtapiPassThru.inf

  #
  # PCI Support
  #
  ArmPkg/Drivers/ArmPciCpuIo2Dxe/ArmPciCpuIo2Dxe.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Pci/NonDiscoverablePciDeviceDxe/NonDiscoverablePciDeviceDxe.inf

  #
  # The following 2 module perform the same work except one operate variable.
  # Only one of both should be put into fdf.
  #
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf

  #
  # NVME Support
  #
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # SMBIOS
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  $(PLATFORM_PACKAGE)/Drivers/SmbiosPlatformDxe/SmbiosPlatformDxe.inf

  #
  #Gop
  #
!if $(PHYTIUM_GOP) == TRUE
  $(SILICON_PACKAGE)/Drivers/SocGopDxe/PhyGopDxe.inf
  #Silicon/Phytium/E2000/SocGopDxe/PhyGopDxe.inf
!endif
  #
  #E2k BMC
 #pcie config
  $(PLATFORM_PACKAGE)/setup/PcieConfigDxe/PcieConfigUiDxe.inf
  #
!if $(E2K_BMC_DEBUG) == TRUE
  #Silicon/Phytium/E2000/BmcGopDxe/PhyGopDxe.inf
!endif
  #
  #Gmu Snp
  #
!if $(GMU_ENABLE) == TRUE
  $(SILICON_PACKAGE)/Drivers/GmuDxe/GmuDxe.inf
!endif
  #
  #eMMC
  #
!if $(EMMC_ENABLE) == TRUE
  $(SILICON_PACKAGE)/Drivers/EmmcHostDxe/EmmcHostDxe.inf
  $(SILICON_PACKAGE)/Drivers/MmcDxe/MmcDxe.inf
!endif
  #
  #Bios Update Spi NorFlash
  #
  #$(SILICON_PACKAGE)/Drivers/UpdateBiosSpiNorflashApp/UpdateBiosApp.inf
  #
  #Device Tree
  #
!if $(DT_OR_ACPI) == "DT"
  EmbeddedPkg/Drivers/DtPlatformDxe/DtPlatformDxe.inf {
    <LibraryClasses>
      FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
      DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf
  }
!elseif $(DT_OR_ACPI) == "ACPI"
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  $(PLATFORM_PACKAGE)/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf
  $(PLATFORM_PACKAGE)/AcpiTables/AcpiTables.inf
!else
  EmbeddedPkg/Drivers/DtPlatformDxe/DtPlatformDxe.inf {
    <LibraryClasses>
      FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
      DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf
  }
!endif


  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
  MdeModulePkg/Application/BootManagerMenuApp/BootManagerMenuApp.inf
  $(GENERAL_PACKAGE)/Setup/DisplayEngineDxe/DisplayEngineDxe.inf
  $(GENERAL_PACKAGE)/Setup/SetupBrowserDxe/SetupBrowserDxe.inf
  $(GENERAL_PACKAGE)/Setup/DriverSampleDxe/DriverSampleDxe.inf
  $(GENERAL_PACKAGE)/Setup/PasswordConfigDxe/PasswordConfigUiDxe.inf
  $(GENERAL_PACKAGE)/Setup/UiApp/UiApp.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
      NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
      NULL|$(GENERAL_PACKAGE)/Setup/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
      NULL|$(GENERAL_PACKAGE)/Setup/Library/AdvancedConfigUiLib/AdvancedConfigUiLib.inf
  }


[BuildOptions]
  #*_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
  *_*_*_CC_FLAGS = -DENABLE_MD5_DEPRECATED_INTERFACES
