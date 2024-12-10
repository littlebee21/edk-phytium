#include "McuInfo.h"

#if 0
//#define MCU_USE_MANUAL
typedef struct WarnAlarmFun {
    VOID (*Alarm)(UINTN ErrorType);
} WarnAlarmFunT;

VOID
SendCpldCtr (
  UINT32 cmd
  )
{
#if 0
  UINT32 i;
  DEBUG((DEBUG_INFO, "send cmd to cpld : %d \n", cmd));
  //start
  MmioWrite32(PWR_CTR0_REG, 0x1);
  MicroSecondDelay(1000);
  for(i = 0; i < cmd; i++){
  MmioWrite32(PWR_CTR1_REG, 0x1);
  MicroSecondDelay(2000);
  MmioWrite32(PWR_CTR1_REG, 0x0);
  MicroSecondDelay(1000);
  }
  //end
  MicroSecondDelay(2000);
  MmioWrite32(PWR_CTR0_REG, 0x0);
#endif
}

VOID
Pulldown (
  VOID
  )
{
#if 0
//Output low level 100ms
#if 0
  UINT32  PullDown;

  PullDown = MmioRead32(GPIO_0_BASE + GPIO_SWPORT_DR);
  CLEARBIT(PullDown,2);
  MmioWrite32(GPIO_0_BASE + GPIO_SWPORT_DR, PullDown);
#endif

#if 1
  MmioWrite32(PWR_CTR1_REG, 0x0);
#endif

  MicroSecondDelay(100000);
#endif
}

VOID
AlarmCpld (
  UINTN  ErrorType
  )
{
#if 0
  MmioWrite32(PWR_CTR0_REG, 0x0);

  switch(ErrorType) {
  case ERROR_TYPE0:  //3 short 1 long
    SendCpldCtr(16);
    break;
  case ERROR_TYPE1:  //1 short 1 long
    SendCpldCtr(17);
    break;
  default:
    Pulldown();
    }
#endif
}

STATIC
WarnAlarmFunT WarnAlarmFunCpld = {
  .Alarm = AlarmCpld
};

VOID
AlarmSe (
  UINTN  ErrorType
  )
{
#if 0
  switch(ErrorType) {
  case ERROR_TYPE0:  //3 short 1 long
    send_se_ctr(0x20);
    break;

  case ERROR_TYPE1:  //1 short 1 long
    send_se_ctr(0x21);
    break;
  default:
    Pulldown();
  }
#endif
}

STATIC
WarnAlarmFunT WarnAlarmFunSe = {
  .Alarm = AlarmSe
};

void *
GetFlag (
  VOID
  )
{
#if 0
  switch (pm_get_s3_flag_source()) {
  case 0:
    return &WarnAlarmFunCpld;
  case 2:
    return &WarnAlarmFunSe;
  default:
    return &WarnAlarmFunCpld;
  }
#endif
}

VOID
Alarm (
  UINTN ErrorType
  )
{
#if 0
  //pwr_ctr_fun_t *pwr_flag_p = get_flag_fun();
  WarnAlarmFunT *WarnAlarmFunP;
  WarnAlarmFunP = GetFlag ();

  if (NULL == WarnAlarmFunP || NULL == WarnAlarmFunP->Alarm) {
    while (1);
  }
  WarnAlarmFunP->Alarm (ErrorType);
#endif
}
#endif
/**
  Print Ddr information

  @param[in]  DdrConfigData  DDR configuration

  @retval  None
**/
VOID
PrintDdrInfo (
  IN DDR_CONFIG *DdrConfigData
  )
{
  UINT8 Temp;
  UINT16 SdramDensity;
  UINT64 DimmCapacity;

  if (DdrConfigData->DdrSpdInfo.DramType == DDR4_TYPE){
    DEBUG ((DEBUG_INFO, "  tAAmim    = %dps\n", DdrConfigData->DdrSpdInfo.TAAmin));
    DEBUG ((DEBUG_INFO, "  tRCDmin   = %dps\n", DdrConfigData->DdrSpdInfo.TRCDmin));
    DEBUG ((DEBUG_INFO, "  tRPmin    = %dps\n", DdrConfigData->DdrSpdInfo.TRPmin));
    DEBUG ((DEBUG_INFO, "  tRASmin   = %dps\n", DdrConfigData->DdrSpdInfo.TRASmin));
    DEBUG ((DEBUG_INFO, "  tRCmin    = %dps\n", DdrConfigData->DdrSpdInfo.TRCmin));
    DEBUG ((DEBUG_INFO, "  tFAWmin   = %dps\n", DdrConfigData->DdrSpdInfo.TFAWmin));
    DEBUG ((DEBUG_INFO, "  tRRD_Smin = %dps\n", DdrConfigData->DdrSpdInfo.TRRDSmin));
    DEBUG ((DEBUG_INFO, "  tRRD_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TRRDLmin));
    DEBUG ((DEBUG_INFO, "  tCCD_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TCCDLmin));
    DEBUG ((DEBUG_INFO, "  tWRmin    = %dps\n", DdrConfigData->DdrSpdInfo.TWRmin));
    DEBUG ((DEBUG_INFO, "  tWTR_Smin = %dps\n", DdrConfigData->DdrSpdInfo.TWTRSmin));
    DEBUG ((DEBUG_INFO, "  tWTR_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TWTRLmin));
    DEBUG ((DEBUG_INFO, "  tRFC1min  = %dps\n", DdrConfigData->DdrSpdInfo.TRFC1min));
  } else if (DdrConfigData->DdrSpdInfo.DramType == DDR5_TYPE) {
    DEBUG ((DEBUG_INFO, "  tAAmim    = %dps\n", DdrConfigData->DdrSpdInfo.TAAmin));
    DEBUG ((DEBUG_INFO, "  tRCDmin   = %dps\n", DdrConfigData->DdrSpdInfo.TRCDmin));
    DEBUG ((DEBUG_INFO, "  tRPmin    = %dps\n", DdrConfigData->DdrSpdInfo.TRPmin));
    DEBUG ((DEBUG_INFO, "  tRASmin   = %dps\n", DdrConfigData->DdrSpdInfo.TRASmin));
    DEBUG ((DEBUG_INFO, "  tRCmin    = %dps\n", DdrConfigData->DdrSpdInfo.TRCmin));
    DEBUG ((DEBUG_INFO, "  tFAWmin   = %dps\n", DdrConfigData->DdrSpdInfo.TFAWmin));
    DEBUG ((DEBUG_INFO, "  tRRD_Smin = %dps\n", DdrConfigData->DdrSpdInfo.TRRDSmin));
    DEBUG ((DEBUG_INFO, "  tRRD_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TRRDLmin));
    DEBUG ((DEBUG_INFO, "  tCCD_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TCCDLmin));
    DEBUG ((DEBUG_INFO, "  tWRmin    = %dps\n", DdrConfigData->DdrSpdInfo.TWRmin));
    DEBUG ((DEBUG_INFO, "  tWTR_Smin = %dps\n", DdrConfigData->DdrSpdInfo.TWTRSmin));
    DEBUG ((DEBUG_INFO, "  tWTR_Lmin = %dps\n", DdrConfigData->DdrSpdInfo.TWTRLmin));
    DEBUG ((DEBUG_INFO, "  tRFC1min  = %dps\n", DdrConfigData->DdrSpdInfo.TRFC1min));
    DEBUG ((DEBUG_INFO, "  tRFC2min  = %dps\n", DdrConfigData->DdrSpdInfo.TRFC2min));
    DEBUG ((DEBUG_INFO, "  tRFCsbmin = %dps\n", DdrConfigData->DdrSpdInfo.TRFC4RFCsbmin));
  } else if (DdrConfigData->DdrSpdInfo.DramType == LPDDR4_TYPE) {
  }

  Temp = 4 << DdrConfigData->DdrSpdInfo.DataWidth;
  if (DdrConfigData->DdrSpdInfo.DramType == DDR3_TYPE) {
    SdramDensity = (1UL << (DdrConfigData->DdrSpdInfo.RowNum + DdrConfigData->DdrSpdInfo.ColNum)) \
                   * Temp * DdrConfigData->DdrSpdInfo.BankNum >> 20; //units: Mb
  } else {
    if (DdrConfigData->DdrSpdInfo.DramType == LPDDR4_TYPE) {
      SdramDensity = (1UL << (DdrConfigData->DdrSpdInfo.RowNum + DdrConfigData->DdrSpdInfo.ColNum)) \
                     * Temp * DdrConfigData->DdrSpdInfo.BankNum >> 20; //units: Mb
    } else {
      SdramDensity = (1UL << (DdrConfigData->DdrSpdInfo.RowNum + DdrConfigData->DdrSpdInfo.ColNum)) \
                     * Temp * DdrConfigData->DdrSpdInfo.BgNum * DdrConfigData->DdrSpdInfo.BankNum >> 20; //units: Mb
    }
  }

  if ((DdrConfigData->MiscEnable >> 13) & 0x1) {
    DimmCapacity = SdramDensity * DdrConfigData->DdrSpdInfo.RankNum * 32 / Temp >> 13;   //units: GB
  } else {
    DimmCapacity = SdramDensity * DdrConfigData->DdrSpdInfo.RankNum * 64 / Temp >> 13;   //units: GB
  }
  DEBUG ((DEBUG_INFO, "\tDimm_Capacity = %lldGB\n", DimmCapacity));
  switch (DdrConfigData->DdrSpdInfo.DramType) {
  case DDR3_TYPE:
    DEBUG ((DEBUG_INFO, "\tDDR3"));
    break;
  case DDR4_TYPE:
    DEBUG ((DEBUG_INFO, "\tDDR4"));
    break;
  case LPDDR4_TYPE:
    DEBUG ((DEBUG_INFO, "\tLPDDR4"));
    break;
  case DDR5_TYPE:
    DEBUG ((DEBUG_INFO, "\tDDR5"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "\tdram_type=0x%x", DdrConfigData->DdrSpdInfo.DramType));
    break;
  }
  switch (DdrConfigData->DdrSpdInfo.DimmType) {
  case 1:
    DEBUG ((DEBUG_INFO, "\tRDIMM"));
    break;
  case 2:
    DEBUG ((DEBUG_INFO, "\tUDIMM"));
    break;
  case 3:
  case 9:
    DEBUG ((DEBUG_INFO, "\tSODIMM"));
    break;
  case 4:
    DEBUG ((DEBUG_INFO, "\tLRDIMM"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "\tdimm_type=0x%x", DdrConfigData->DdrSpdInfo.DimmType));
    break;
  }
  if (DdrConfigData->DdrSpdInfo.DramType == DDR4_TYPE) {
    DEBUG ((DEBUG_INFO, "/%d Bank Groups", DdrConfigData->DdrSpdInfo.BgNum));
    DEBUG ((DEBUG_INFO, "/%d Banks", DdrConfigData->DdrSpdInfo.BankNum));
  } else {
    DEBUG ((DEBUG_INFO, "/%d Banks", DdrConfigData->DdrSpdInfo.BankNum));
    DEBUG ((DEBUG_INFO, "/sdram_density=%dGb", SdramDensity >> 10));
  }
  DEBUG ((DEBUG_INFO, "/Column %d", DdrConfigData->DdrSpdInfo.ColNum));
  DEBUG ((DEBUG_INFO, "/Row %d", DdrConfigData->DdrSpdInfo.RowNum));
  switch (DdrConfigData->DdrSpdInfo.DataWidth) {
  case DIMM_x4:
    DEBUG ((DEBUG_INFO, "/X4"));
    break;
  case DIMM_x8:
    DEBUG ((DEBUG_INFO, "/X8"));
    break;
  case DIMM_x16:
    DEBUG ((DEBUG_INFO, "/X16"));
    break;
  case DIMM_x32:
    DEBUG ((DEBUG_INFO, "/X32"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "/data_width=0x%x", DdrConfigData->DdrSpdInfo.DataWidth));
    break;
  }
  DEBUG ((DEBUG_INFO, "/%d Rank", DdrConfigData->DdrSpdInfo.RankNum));
  switch (DdrConfigData->DdrSpdInfo.EccType) {
  case 0:
    DEBUG ((DEBUG_INFO, "/NO ECC"));
    break;
  case 1:
    DEBUG ((DEBUG_INFO, "/ECC"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "/ecc_type=0x%x", DdrConfigData->DdrSpdInfo.EccType));
    break;
  }
  if(DdrConfigData->DdrSpdInfo.MirrorType == 0) {
    DEBUG ((DEBUG_INFO, "/Standard\n"));
  } else {
    DEBUG ((DEBUG_INFO, "/Mirror\n"));
  }
  DEBUG ((DEBUG_INFO, "\tModual:"));
  switch (DdrConfigData->DdrSpdInfo.ModuleManufacturerId) {
  case SAMSUNG_VENDOR:
    DEBUG ((DEBUG_INFO, "Samsung"));
    break;
  case MICRON_VENDOR:
    DEBUG ((DEBUG_INFO, "Micron"));
    break;
  case HYNIX_VENDOR:
    DEBUG ((DEBUG_INFO, "Hynix"));
    break;
  case KINGSTON_VENDOR:
    DEBUG ((DEBUG_INFO, "KingSton"));
    break;
  case RAMAXEL_VENDOR:
    DEBUG ((DEBUG_INFO, "Ramaxel"));
    break;
  case LANQI_VENDOR:
    DEBUG ((DEBUG_INFO, "Lanqi"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "Unknown=0x%x", DdrConfigData->DdrSpdInfo.ModuleManufacturerId));
  }
}

/**
  Parse DDR3 Spd information

  @param[in]     Channel        DDR Channel
  @param[in]     Buffer         Spd information
  @param[in,out] DdrConfigData  Ddr Config data struct.

  @retval  None
**/
VOID
ParseDDR3Spd (
  IN      UINT8       Channel,
  IN      UINT8       *Buffer,
  IN OUT  DDR_CONFIG  *DdrConfigData
  )
{
  UINT16 Temp;

  DEBUG ((DEBUG_INFO, "Parse DDR3 SPD\n"));

  Temp = Buffer[3] & 0xf;
  DdrConfigData->DdrSpdInfo.DimmType = Temp;

  DdrConfigData->DdrSpdInfo.BankNum = 8;

  Temp = Buffer[5] & 0x7;
  DdrConfigData->DdrSpdInfo.ColNum = Temp + 9;

  Temp = Buffer[5] >> 3 & 0x7;
  DdrConfigData->DdrSpdInfo.RowNum = Temp + 12;

  //Module Organization
  Temp = Buffer[7] & 0x7;
  DdrConfigData->DdrSpdInfo.DataWidth = Temp;

  Temp = Buffer[7] >> 3 & 0x7;
  DdrConfigData->DdrSpdInfo.RankNum= Temp + 1;

  //Module Organization
  Temp = Buffer[8] >> 3 & 0x7;
  DdrConfigData->DdrSpdInfo.EccType = Temp;

  if (DdrConfigData->DdrSpdInfo.DimmType == UDIMM_TYPE) {
    Temp = Buffer[63] & 0x1;
    DdrConfigData->DdrSpdInfo.MirrorType = Temp;
  }

  DdrConfigData->DdrSpdInfo.ModuleManufacturerId = ((UINT16)Buffer[117] << 8) + Buffer[118];

  //tRCDmin
  DdrConfigData->DdrSpdInfo.TRCDmin = Buffer[18] * SPD_MTB;
  //tRPmin
  DdrConfigData->DdrSpdInfo.TRPmin = Buffer[20] * SPD_MTB;
  //tRASmin
  DdrConfigData->DdrSpdInfo.TRASmin = (((((UINT16)Buffer[21]) & 0xf) << 8) + Buffer[23]) * SPD_MTB;
  //tRCmin
  DdrConfigData->DdrSpdInfo.TRCmin = (((((UINT16)Buffer[21]>>4) & 0xf) << 8) + Buffer[23]) * SPD_MTB;
  //tFAWmin
  DdrConfigData->DdrSpdInfo.TFAWmin = ((((UINT16)Buffer[28] & 0xf) << 8) + Buffer[29]) * SPD_MTB;
  //tRRDmin
  //DdrConfigData->DdrSpdInfo.TRRDmin = Buffer[19] * SPD_MTB;
  //tWRmin
  DdrConfigData->DdrSpdInfo.TWRmin = 15000;
  //tWTRmin, 7.5ns
  //DdrConfigData->DdrSpdInfo.TWTRmin = 7500;

  PrintDdrInfo (DdrConfigData);
  Temp = ((UINT16)Buffer[147] << 8) + Buffer[149];
  DEBUG ((DEBUG_INFO, "/Dram:"));
  switch (Temp) {
  case SAMSUNG_VENDOR:
    DEBUG ((DEBUG_INFO, "Samsung"));
    break;
  case MICRON_VENDOR:
    DEBUG ((DEBUG_INFO, "Micron"));
    break;
  case HYNIX_VENDOR:
    DEBUG ((DEBUG_INFO, "Hynix"));
    break;
  case KINGSTON_VENDOR:
    DEBUG ((DEBUG_INFO, "KingSton"));
    break;
  case RAMAXEL_VENDOR:
    DEBUG ((DEBUG_INFO, "Ramaxel"));
    break;
  case LANQI_VENDOR:
    DEBUG ((DEBUG_INFO, "Lanqi"));
    break;
  case CXMT_VENDOR:
    DEBUG ((DEBUG_INFO, "CXMT"));
    break;
  case UNILC_VENDOR:
    DEBUG ((DEBUG_INFO, "Unilc"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "Unknown=0x%x", Temp));
  }

  Temp = ((UINT32)Buffer[122] << 24) + ((UINT32)Buffer[123] << 16)
           + ((UINT32)Buffer[124] << 8) + Buffer[125];
  DEBUG ((DEBUG_INFO, "/Serial:0x%x\n", Temp));
}

/**
  Parse DDR4 Spd information

  @param[in]      Channel        DDR Channel
  @param[in]      Buffer         Spd information
  @param[in,out]  DdrConfigData  Ddr config data struct

  @retval  None
**/
VOID
ParseDDR4Spd (
  UINT8      Channel,
  UINT8      *Buffer,
  DDR_CONFIG *DdrConfigData
  )
{
  UINT16 Temp;

  DEBUG ((DEBUG_INFO, "Parse DDR4 Spd\n"));

  Temp = Buffer[3] & 0xf;
  DdrConfigData->DdrSpdInfo.DimmType = Temp;

  //
  //SDRAM Density and Banks
  //
  Temp = Buffer[4] >> 6 & 0x3;
  if (!Temp) {
    DdrConfigData->DdrSpdInfo.BgNum = 0x0;
  } else {
    DdrConfigData->DdrSpdInfo.BgNum = 0x1 << Temp;
  }

  Temp = Buffer[4] >> 4 & 0x3;
  if (Temp == 1) {
    DdrConfigData->DdrSpdInfo.BankNum = 8;
  } else {
    DdrConfigData->DdrSpdInfo.BankNum = 4;
  }

  //
  //SDRAM Addressing
  //
  Temp = Buffer[5] & 0x7;
  DdrConfigData->DdrSpdInfo.ColNum = Temp + 9;

  Temp = Buffer[5]>>3 & 0x7;
  DdrConfigData->DdrSpdInfo.RowNum = Temp + 12;

  //Module Organization
  Temp = Buffer[12] & 0x7;
  DdrConfigData->DdrSpdInfo.DataWidth = Temp;

  Temp = Buffer[12]>>3 & 0x7;
  DdrConfigData->DdrSpdInfo.RankNum= Temp + 1;

  //
  //Module Organization
  //
  Temp = Buffer[13]>>3 & 0x7;
  DdrConfigData->DdrSpdInfo.EccType = Temp;

  //
  //(Registered): Address Mapping from Register to DRAM
  //
  if ((DdrConfigData->DdrSpdInfo.DimmType == RDIMM_TYPE)
        || (DdrConfigData->DdrSpdInfo.DimmType == LRDIMM_TYPE)) {
    Temp = Buffer[136] & 0x1;
  } else {
    Temp = Buffer[131] & 0x1;
  }

  DdrConfigData->DdrSpdInfo.MirrorType = Temp;
  DdrConfigData->DdrSpdInfo.F0RC03 = (Buffer[137]>>4) & MASK_4BIT;
  DdrConfigData->DdrSpdInfo.F0RC04 = Buffer[137] & MASK_4BIT;
  DdrConfigData->DdrSpdInfo.F0RC05 = Buffer[138] & MASK_4BIT;

  DdrConfigData->DdrSpdInfo.RcdNum = (UINT16)Buffer[256];
  DdrConfigData->DdrSpdInfo.ModuleManufacturerId = ((UINT16)Buffer[320] << 8) + Buffer[321];

  //tAAmin , Buffer[123] may be positive/negative
  DdrConfigData->DdrSpdInfo.TAAmin = Buffer[24] * SPD_MTB + (INT8)Buffer[123] * SPD_FTB;
  //tRCDmin , Buffer[122] may be positive/negative
  DdrConfigData->DdrSpdInfo.TRCDmin = Buffer[25] * SPD_MTB + (INT8)Buffer[122] * SPD_FTB;
  //tRPmin
  DdrConfigData->DdrSpdInfo.TRPmin = Buffer[26] * SPD_MTB + (INT8)Buffer[121] * SPD_FTB;
  //tRASmin
  DdrConfigData->DdrSpdInfo.TRASmin = (((((UINT16)Buffer[27]) & 0xf)<<8) + Buffer[28]) * SPD_MTB;
  //tRCmin
  DdrConfigData->DdrSpdInfo.TRCmin = (((((UINT16)Buffer[27]>>4) & 0xf)<<8) + Buffer[29]) * SPD_MTB + (INT8)Buffer[120] * SPD_FTB;
  //tFAWmin
  DdrConfigData->DdrSpdInfo.TFAWmin = ((((UINT16)Buffer[36] & 0xf) <<8) + Buffer[37]) * SPD_MTB;
  //tRDD_Smin
  DdrConfigData->DdrSpdInfo.TRRDSmin = Buffer[38] * SPD_MTB + (INT8)Buffer[119] * SPD_FTB;
  //tRDD_Lmin
  DdrConfigData->DdrSpdInfo.TRRDLmin = Buffer[39] * SPD_MTB + (INT8)Buffer[118] * SPD_FTB;
  //tCCD_Lmin
  DdrConfigData->DdrSpdInfo.TCCDLmin = Buffer[40] * SPD_MTB + (INT8)Buffer[117] * SPD_FTB;
  //tWRmin
  if ((Buffer[42] == 0x0) && (Buffer[41] == 0x0) ) {
    DEBUG ((DEBUG_INFO, "Error! spd byte42 = 0\n"));
    DdrConfigData->DdrSpdInfo.TWRmin = 15000;
  } else {
    DdrConfigData->DdrSpdInfo.TWRmin = Buffer[41] * SPD_MTB * 256 + Buffer[42] * SPD_MTB;
  }

  if ((Buffer[43] == 0) && (Buffer[44] == 0) ) {
    DEBUG ((DEBUG_INFO, "Error! spd byte43 = 0\n"));
    DdrConfigData->DdrSpdInfo.TWTRSmin = 2500;
  } else {
    DdrConfigData->DdrSpdInfo.TWTRSmin = ((((UINT16)Buffer[43] & 0xf) << 8) + Buffer[44]) * SPD_MTB;
  }

  if((Buffer[43] == 0) && (Buffer[45] == 0)) {
    DdrConfigData->DdrSpdInfo.TWTRLmin = 7500;
  } else {
    DdrConfigData->DdrSpdInfo.TWTRLmin = ((((UINT16)Buffer[43] >> 4) & 0xf) + Buffer[45]) * SPD_MTB;
  }

  //tRFC1min
  DdrConfigData->DdrSpdInfo.TRFC1min = (((UINT16)Buffer[31]<<8) + Buffer[30]) * SPD_MTB;
  //tRFC2min
  DdrConfigData->DdrSpdInfo.TRFC2min = (((UINT16)Buffer[33]<<8) + Buffer[32]) * SPD_MTB;
  //tRFC4min
  DdrConfigData->DdrSpdInfo.TRFC4RFCsbmin = (((UINT16)Buffer[35]<<8) + Buffer[34]) * SPD_MTB;

  PrintDdrInfo (DdrConfigData);
  Temp = ((UINT16)Buffer[350] << 8) + Buffer[351];
  DEBUG ((DEBUG_INFO, "/Dram:"));

  switch (Temp) {
  case SAMSUNG_VENDOR:
    DEBUG ((DEBUG_INFO, "Samsung"));
    break;
  case MICRON_VENDOR:
    DEBUG ((DEBUG_INFO, "Micron"));
    break;
  case HYNIX_VENDOR:
    DEBUG ((DEBUG_INFO, "Hynix"));
    break;
  case KINGSTON_VENDOR:
    DEBUG ((DEBUG_INFO, "KingSton"));
    break;
  case RAMAXEL_VENDOR:
    DEBUG ((DEBUG_INFO, "Ramaxel"));
    break;
  case LANQI_VENDOR:
    DEBUG ((DEBUG_INFO, "Lanqi"));
    break;
  case CXMT_VENDOR:
    DEBUG ((DEBUG_INFO, "CXMT"));
    break;
  case UNILC_VENDOR:
    DEBUG ((DEBUG_INFO, "Unilc"));
    break;
  default:
    DEBUG ((DEBUG_INFO, "Unknown=0x%x", Temp));
  }
  Temp = ((UINT32)Buffer[325] << 24) + ((UINT32)Buffer[326] <<16)
          + ((UINT32)Buffer[327] << 8) + Buffer[328];
  DEBUG ((DEBUG_INFO, "/Serial:0x%x\n", Temp));
}

/**
  Probe memory module

  @param[in]      Channel        Memory module channel, only 0
  @param[in,out]  DDR            Configuration information
  @param[in]      SpdIndex       Index of dimm spd, 0
  @param[in]      Dimm           Dimm Index, 0

  @retval         EFI_SUCCESS    Memory module exist.
                  EFI_TIMEOUT    Memory module don't exist.
                  EFI_NOT_FOUND  I2c Slave Device not found.
**/
EFI_STATUS
DimmProbe (
  IN UINT8           Channel,
  IN OUT DDR_CONFIG  *DdrConfigData,
  IN UINT8           SpdIndex,
  IN UINT8           Dimm
  )
{
  UINT64    I2CBaseAddress;
  UINT32    I2CSpeed;
  UINT32    I2CSlaveAddress;
  UINT8     Buffer[SPD_NUM *2];
  UINT8     Temp;
  UINT8     DramType;
  UINT8     *SlaveAddress;
  I2C_INFO  I2cInfo;

  if ((Channel != 0) || (Dimm != 0)) {
    DEBUG ((DEBUG_INFO, "Channel and Dimm is not 0, not supported!\n"));
    return EFI_UNSUPPORTED;
  }
  ZeroMem (Buffer, sizeof (Buffer));
  SlaveAddress   = PcdGetPtr(PcdDdrI2cAddress);
  I2CBaseAddress = PcdGet64 (PcdSpdI2cControllerBaseAddress);
  I2CSpeed = PcdGet32 (PcdSpdI2cControllerSpeed);
  I2CSlaveAddress = SlaveAddress[SpdIndex];

  ZeroMem (&I2cInfo, sizeof (I2C_INFO));
  I2cInfo.BusAddress = PcdGet64 (PcdSpdI2cControllerBaseAddress);
  I2cInfo.Speed = PcdGet32 (PcdSpdI2cControllerSpeed);
  I2cInfo.SlaveAddress = I2CSlaveAddress;
  DEBUG ((DEBUG_INFO, "I2CBase:%x, I2CSpeed:%x, I2CSlaveAddress:%x\n",
            I2cInfo.BusAddress, I2cInfo.Speed, I2cInfo.SlaveAddress));
  //
  // Initialize I2C Bus which contain SPD
  //
  I2cInit(&I2cInfo);
  Temp = I2cRead (&I2cInfo, 0, 1, Buffer, 256);
  if (Temp != 0) {
    DEBUG((DEBUG_ERROR, "Read I2C Failed\n"));
    return EFI_NOT_FOUND;
  }
  if (Dimm == 0) {
    //Get Dram Type
    DramType = Buffer[2] & 0xf;
    DdrConfigData->DdrSpdInfo.DramType = DramType;
    if (DramType == DDR3_TYPE) {
      ParseDDR3Spd (Channel, Buffer, DdrConfigData);
    } else {
      SpdSetpage (&I2cInfo, 1);
      I2cRead (&I2cInfo, 0, 1, &Buffer[256], 256);
      SpdSetpage (&I2cInfo, 0);
      ParseDDR4Spd (Channel, Buffer, DdrConfigData);
    }
  } else {
    DEBUG ((DEBUG_INFO, "2 Dimm in 1 Channel not supported!\n"));
  }

  return EFI_SUCCESS;
}

/**
  Get DDR spd parameter

  @param[in]      Channel        Channel memory module channel
  @param[in,out]  DdrConfigData  DDR configuration information
  @param[in]      SpdIndex       Index of spd
  @param[in]      Dimm           Index of Dimm

  @retval         EFI_SUCCESS    Get spd info success.
                  EFI_NOT_FOUND  Memory module don't exist.
**/
EFI_STATUS
GetDdrSpdParameter (
  IN     UINT8       Channel,
  IN OUT DDR_CONFIG  *DdrConfigData,
  IN     UINT8       SpdIndex,
  IN     UINT8       Dimm
  )
{
  EFI_STATUS Status;

  DEBUG ((DEBUG_INFO, "Read Parameter form SPD\n"));
  Status = DimmProbe (Channel, DdrConfigData, SpdIndex, Dimm);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  return EFI_SUCCESS;
}

#if 0
/**
  Initialize Ddr config data.

  param[in,out]    DdrConfigData  Ddr config data struct.
**/
VOID
DdrConfigInit (
  IN OUT DDR_CONFIG *DdrConfigData
  )
{
  DdrConfigData->Head.Magic = PARAMETER_MCU_MAGIC;
  DdrConfigData->Head.Version = PARAM_MCU_VERSION;
  DdrConfigData->Head.Size = PARAM_MCU_SIZE;
  DdrConfigData->ChannelEnable = PARAM_CH_ENABLE;
  DdrConfigData->EccEnable = PARAM_ECC_ENABLE;
  DdrConfigData->DmEnable = PARAM_DM_ENABLE;
  DdrConfigData->ForceSpdEnable = PARAM_FORCE_SPD_DISABLE;
  DdrConfigData->MiscEnable = PARAM_MCU_MISC_ENABLE;
  DdrConfigData->TrainDebug = PARAM_TRAIN_DEBUF;
  DdrConfigData->TrainRecover = 0;
}
#endif

/**
  Get DDR config  parameter

  @param[in,out]  DDR    configuration information
  @param[in]      S3Flag S3 status

  @retval  None
**/
VOID
GetDdrConfigParameter (
  IN OUT DDR_CONFIG  *DdrConfigData,
  IN     UINT8       S3Flag
  )
{
  UINT8        Channel;
  UINT8        Dimm;
  UINT8        SpdIndex;
  UINT8        ChannelCount;
  EFI_STATUS   Status;
  UINT8        DevicesNumber;
  UINTN        DimmCount;
  UINT8        Flag;
  UINT8        Flag1;
  UINT8        Flag2;
  UINT8        ForceSpd;

  DimmCount = PcdGetSize (PcdDdrI2cAddress);
  ChannelCount = PcdGet8 (PcdDdrChannelCount);
  DevicesNumber = DimmCount;
  SpdIndex = 0;
  Flag     = 0;
  Flag1    = 0;
  Flag2    = 0;
  ForceSpd = (UINT8)(DdrConfigData->MiscEnable & 0x1);

  DEBUG((DEBUG_INFO, "ChannelCount %d\n", ChannelCount));
  DEBUG((DEBUG_INFO, "DimmCount %d\n", DimmCount));
  if (ForceSpd == 1) {
    DEBUG((DEBUG_INFO, "Read Parameter form Parameter Table\n"));
    PrintDdrInfo (DdrConfigData);
    DEBUG((DEBUG_INFO, "\n"));
  } else {
    for(Channel = 0; Channel < ChannelCount; Channel++) {
      DEBUG ((DEBUG_INFO, "Channel Enable : %x\n", DdrConfigData->ChEnable));
      if (!((DdrConfigData->ChEnable >> Channel) & 0x1)) {
        continue;
      }
      DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
      for (Dimm = 0; Dimm < 1; Dimm ++) {
        DEBUG((DEBUG_INFO, "Channel %d Dimm %d\n", Channel,Dimm));
        DEBUG((DEBUG_INFO, "SpdIndex %d\n", SpdIndex));
        Status = GetDdrSpdParameter (Channel, DdrConfigData,SpdIndex,Dimm);//if read spd == there is a memdev
        if(EFI_ERROR(Status)) {
          DevicesNumber--;
          DEBUG ((DEBUG_INFO, "Channel %d Dimm %d Don't Probe\n\n", Channel, Dimm));
          DdrConfigData->ChEnable &= ~(0x1 << Channel);
          DEBUG((DEBUG_INFO,"Channel %d Not Initialized~~~~~~\n",Channel));
        } else {
          if (!((DdrConfigData->MiscEnable >> 1) & 0x1)) {
            DEBUG((DEBUG_INFO, "Ecc Disable\n"));
          }
        }
        SpdIndex++;
      }
    }
  }
  if (DevicesNumber == 0) {
   // Alarm(ERROR_TYPE0);
    DEBUG((DEBUG_ERROR, " Don't Find Memory Devices-------\n"));
  }
  if (S3Flag) {
    DdrConfigData->TrainRecover = 0x1;
  }
}
