#include "ArmPlatform.h"

DefinitionBlock("DsdtTable.aml", "DSDT", 2, "PHYLTD", "PHYTIUM.", EFI_ACPI_ARM_OEM_REVISION) {

  include ("Cpu.asl")
  include ("Uart.asl")
  include ("Dp.asl")
  //include ("Mmc.asl")
  //include ("Spi.asl")
  include ("Hda.asl")
  include ("Usb.asl")
  include ("Sata.asl")
  include ("Gpio.asl")
  include ("I2c.asl")
  //include ("Lpc.asl")
  include ("Vpu.asl")
  //include ("Scp.asl")
  //include ("Scto.asl")
  include ("Mac.asl")
  //include ("Can.asl")
  include ("ThermalZone.asl")
  include ("I2s.asl")
}
