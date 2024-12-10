/**


**/
#include <Base.h>

#include <Uefi/UefiBaseType.h>

#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PadLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/ParameterTable.h>
#include <Library/PhytiumGpioLib.h>
#include <Library/BaseLib.h>
#include <Library/ArmLib.h>
//
//LSD DMA Config
//
#define LSD_BASE_ADDR         0x2807E000
#define LSD_NAND_MMCSD_HDDR   (LSD_BASE_ADDR + 0xC0)
#define LSD_CFG_LPI0          (LSD_BASE_ADDR + 0x1A0)
#define LSD_LMC_LPI0          (LSD_BASE_ADDR + 0x1B0)
#define LSD_LSD_LPI0          (LSD_BASE_ADDR + 0x1C0)
#define LSD_NDF_LPI0          (LSD_BASE_ADDR + 0x1D0)
#define LSD_SIO_LPI0          (LSD_BASE_ADDR + 0x1E0)

/**

**/


/*ETH*/
#define GMAC0_BASE_ADDR      0x3200c000
#define GMAC1_BASE_ADDR      0x3200e000
#define GMAC2_BASE_ADDR      0x32010000
#define GMAC3_BASE_ADDR      0x32012000
//phy*
#define GSD_PHY0_BASE        0x32100000
#define GSD_PHY1_BASE        0x32200000
#define GSD_PHY2_BASE        0x32300000
#define GSD_PHY3_BASE        0x32400000

#define TRAINING_RX_DESC     0x80000000
#define TRAINING_TX_DESC     0x80100000
#define TRAINING_TX_BUFFER   0x80200000
#define TRAINING_RX_BUFFER   0x80300000

/* ETH register offsets */
#define NCR    0x0000 /* Network Control */
#define NSR    0x0008 /* Network Status */
#define MAN    0x0034 /* PHY Maintenance */

/* Bit manipulation macros */
#define BIT_MANIPULATION(Name)          \
  (1 << Name##_OFFSET)
#define BF(Name,Value)        \
  (((Value) & ((1 << Name##_SIZE) - 1))  \
   << Name##_OFFSET)
#define BFEXT(Name,Value)\
  (((Value) >> Name##_OFFSET)    \
   & ((1 << Name##_SIZE) - 1))

/* Bitfields in NCR */
#define MPE_OFFSET    4 /* Management port enable */
#define MPE_SIZE      1

/* Bitfields in MAN */
#define DATA_OFFSET    0 /* data */
#define DATA_SIZE      16
#define CODE_OFFSET    16 /* Must be written to 10 */
#define CODE_SIZE      2
#define REGA_OFFSET    18 /* Register address */
#define REGA_SIZE      5
#define PHYA_OFFSET    23 /* PHY address */
#define PHYA_SIZE      5
#define RW_OFFSET      28 /* Operation. 10 is read. 01 is write. */
#define RW_SIZE        2
#define SOF_OFFSET     30 /* Must be written to 1 for Clause 22 */
#define SOF_SIZE       2

/* Bitfields in NSR */
#define IDLE_OFFSET    2 /* The PHY management logic is idle */
#define IDLE_SIZE      1

/* Register access macros */
#define JUST_READ(Port, Reg)        \
  MmioRead32 (Port + Reg)
#define JUST_WRITE(Port, Reg, Value)      \
  MmioWrite32 ((Port) + Reg, (Value))

/**
  Config Phy by mdio write.

  @param[in]   BaseReg  Gmac base register.
  @param[in]   PhyAdr   Phy address.
  @param[in]   Reg      Register.
  @param[in]   Value    Value to write.
**/
STATIC
VOID
JustMdioWrite (
  IN UINT64 BaseReg,
  IN UINT8  PhyAdr,
  IN UINT8  Reg,
  IN UINT16 Value
  )
{
  UINT64 Netctl;
  UINT64 Netstat;
  UINT64 Frame;

  Netctl = JUST_READ(BaseReg, NCR);
  Netctl |= BIT_MANIPULATION(MPE);
  JUST_WRITE(BaseReg, NCR, Netctl);

  Frame = (BF(SOF, 1)
     | BF(RW, 1)
     | BF(PHYA, PhyAdr)
     | BF(REGA, Reg)
     | BF(CODE, 2)
     | BF(DATA, Value));
  JUST_WRITE(BaseReg, MAN, Frame);

  do {
    Netstat = JUST_READ(BaseReg, NSR);
  } while (!(Netstat & BIT_MANIPULATION(IDLE)));

  Netctl = JUST_READ(BaseReg, NCR);
  Netctl &= ~BIT_MANIPULATION(MPE);
  JUST_WRITE(BaseReg, NCR, Netctl);
}


/**
  Mdio Read.

  @param[in]   BaseReg  Gmac base register.
  @param[in]   PhyAdr   Phy address.
  @param[in]   Reg      Register.

  @retval      Read value.
**/
STATIC
UINT16
JustMdioRead (
  UINT64 BaseReg,
  UINT8  PhyAdr,
  UINT8  Reg
  )
{
  UINT64 Netctl;
  UINT64 Netstat;
  UINT64 Frame;

  Netctl = JUST_READ(BaseReg, NCR);
  Netctl |= BIT_MANIPULATION(MPE);
  JUST_WRITE(BaseReg, NCR, Netctl);

  Frame = (BF(SOF, 1)
     | BF(RW, 2)
     | BF(PHYA, PhyAdr)
     | BF(REGA, Reg)
     | BF(CODE, 2));
  JUST_WRITE(BaseReg, MAN, Frame);

  do {
    Netstat = JUST_READ(BaseReg, NSR);
  } while (!(Netstat & BIT_MANIPULATION(IDLE)));

  Frame = JUST_READ(BaseReg, MAN);

  Netctl = JUST_READ(BaseReg, NCR);
  Netctl &= ~BIT_MANIPULATION(MPE);
  JUST_WRITE(BaseReg, NCR, Netctl);

  return BFEXT(DATA, Frame);
}

/**
  Sgmii 1G Phy training.

  @param[in]   MacBase  Gmac base address.
  @param[in]   PhyBase  Phy base address.

  @retval      TRUE     Sucess.
               FALSE    ERROR.
**/
STATIC
BOOLEAN
EthSgmiiTraining (
  IN UINT32 MacBase,
  IN UINT32 PhyBase
  )
{
  UINTN  Index1;
  UINTN  Index2;
  UINTN  Result1;
  UINTN  Result2;
  UINTN  Count;
  UINT16 RecData;
  UINT32 PhyRstOffset;
  UINT32 TxPkts;
  UINT32 RxPkts;
  UINT32 Symbol;
  UINT32 Fcs;

  Index1 = 0;
  Index2 = 0;
  Result1 = 0;
  Result2 = 0;
  Count = 0;
  PhyRstOffset = 0;
  DEBUG ((DEBUG_INFO, "SGMII_TEST_START\n"));
  DEBUG ((DEBUG_INFO, "eth ctrl = 0x%x, phy = 0x%x\n", MacBase, PhyBase));
  RecData = JustMdioRead (MacBase, 0, 0x0);
  //enter sgmii_digital_loopack,4140 1000M//6100 100M//4100 10M
  JustMdioWrite (MacBase, 0, 0x0, 0x4140);

  for (Index1 = 0; Index1 < 10; Index1++) {
    Result1 = 0;
    Result2 = 0;
    DEBUG ((DEBUG_INFO, "send recv test %d\n",Index1));

    DEBUG ((DEBUG_INFO, "108 0x%x\n", MmioRead32 ((UINT64)(MacBase + 0x108))));
    DEBUG ((DEBUG_INFO, "158 0x%x\n", MmioRead32 ((UINT64)(MacBase + 0x158))));
    DEBUG ((DEBUG_INFO, "190 0x%x\n", MmioRead32 ((UINT64)(MacBase + 0x190))));

    //MAC配置
    MmioWrite32 ((UINT64)(MacBase + 0x4d4), 0x0);
    MmioWrite32 ((UINT64)(MacBase + 0x000), 0x00000030);
    MmioWrite32 ((UINT64)(MacBase + 0x018),
                  TRAINING_RX_DESC + 0x00);
    MmioWrite32 ((UINT64)(MacBase + 0x4c8), 0x0);
    MmioWrite32 ((UINT64)(MacBase + 0x01c),
                  TRAINING_TX_DESC + 0x00);
    MmioWrite32 ((UINT64)(MacBase + 0x004), 0x0c54ac1a);
    MmioWrite32 ((UINT64)(MacBase + 0x010), 0x40180f10);
    MmioWrite32 ((UINT64)(MacBase + 0x200), 0x1140);
    MmioWrite32 ((UINT64)(MacBase + 0x028), 0xffffffff);
    MmioWrite32 ((UINT64)(MacBase + 0x000), 0x0000001c);
    ArmInstructionSynchronizationBarrier ();

    DEBUG ((DEBUG_INFO, "008 0x%x\n",MmioRead32 ((UINT64)(MacBase + 0x008))));
    Index2 = 0;
    //rgmii
    while (!(0x1 & MmioRead32 ((UINT64)(MacBase + 0x008)))) {
      MicroSecondDelay (5);
      Index2++;
      if (Index2 > 10) {
        Result1 = 0xff;
        goto TrainingExit;
      }
    }

    DEBUG((DEBUG_INFO, "008 0x%x, Index2:%d\n",
            MmioRead32 ((UINT64)(MacBase + 0x008)), Index2));

    /*rx_buf*/
    SetMem ((VOID *)(TRAINING_RX_BUFFER + 0x000), 0x500, 0);
    /*tx_buf*/
    SetMem ((VOID *)(TRAINING_TX_BUFFER + 0x00), 0x60, 0);
    /*fill tx buf*/
    MmioWrite32 (TRAINING_TX_BUFFER + 0x00, 0xFFFFFFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x04, 0x0000FFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x08, 0x10301200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x0c, 0x00450008);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x10, 0x5e003600);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x14, 0xfd800000);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x18, 0xa8c010b8);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x1c, 0xa8c00200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x20, 0xaa000a00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x24, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x28, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x2c, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x30, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x34, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x38, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x3c, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x40, 0xaa55aa55);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x44, 0x019b51d6);

    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x00, 0xFFFFFFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x04, 0x0000FFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x08, 0x10301200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x0c, 0x00450008);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x10, 0x74003600);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x14, 0xfd800000);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x18, 0xa8c0fab7);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x1c, 0xa8c00200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x20, 0xff000a00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x24, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x28, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x2c, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x30, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x34, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x38, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x3c, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x40, 0xff00ff00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x80 + 0x44, 0xaab94c62);

    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x00, 0xFFFFFFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x04, 0x0000FFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x08, 0x10301200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x0c, 0x00450008);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x10, 0xbe013600);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x14, 0xfd800000);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x18, 0xa8c0b0b6);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x1c, 0xa8c00200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x20, 0xff0f0a00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x24, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x28, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x2c, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x30, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x34, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x38, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x3c, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x40, 0xffffffff);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x100 + 0x44, 0x4615e68b);

    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x00, 0xFFFFFFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x04, 0x0000FFFF);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x08, 0x10301200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x0c, 0x00450008);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x10, 0x2c003600);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x14, 0xfd800000);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x18, 0xa8c042b8);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x1c, 0xa8c00200);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x20, 0x5d9e0a00);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x24, 0xc12460d9);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x28, 0x47529605);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x2c, 0xbe0ef580);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x30, 0xd7849ef6);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x34, 0xd4c4d744);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x38, 0x55c7e8c4);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x3c, 0xb148f5fb);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x40, 0x5d937a1a);
    MmioWrite32 (TRAINING_TX_BUFFER + 0x180 + 0x44, 0xc9868dc4);
    /*rx desc_fill*/
    Index2 = 0;
    for (Index2 = 0; Index2 < 4000; Index2++) {
      MmioWrite32 ((UINT64)TRAINING_RX_DESC + (0x10 * Index2),
                   TRAINING_RX_BUFFER + (0x80 * Index2));
      MmioWrite32 ((UINT64)TRAINING_RX_DESC + 0x04 + (0x10 * Index2), 0x0);
      MmioWrite32 ((UINT64)TRAINING_RX_DESC + 0x08 + (0x10 * Index2), 0x0);
    }
    ArmInstructionSynchronizationBarrier ();
    /*tx desc_fill*/

    Index2 = 0;
    for (Index2 = 0; Index2 < 4000; Index2++) {
      MmioWrite32 ((UINT64)(TRAINING_TX_DESC + 0x08 + (0x10 * Index2)),
                   0x0);
      MmioWrite32 ((UINT64)(TRAINING_TX_DESC + 0x00 + (0x10 * Index2)),
                   (TRAINING_TX_BUFFER + 0x80 * (Index2 / 1000)));
      MmioWrite32 ((UINT64)(TRAINING_TX_DESC + 0x04 + (0x10 * Index2)),
                  0x18048);
    }
    MmioWrite32 ((UINT64)TRAINING_TX_DESC + 0x04 + 0x10 * Index2, 0x80000000);
    ArmInstructionSynchronizationBarrier ();

    //send
    DEBUG ((DEBUG_INFO, "send start\n"));
    MmioWrite32 ((UINT64)(MacBase + 0x000), 0x21c);
    Index2 = 0;
    ArmInstructionSynchronizationBarrier ();

    while (0x1 != (MmioRead32 ((UINT64)(TRAINING_RX_DESC + 0x10 * 3999)) & 0xf)) {
      MicroSecondDelay (50);
      Index2++;
      if (Index2 > 200000) {
        DEBUG ((DEBUG_ERROR, "reg1 = %x,%d\n", MmioRead32 ((UINT64)(TRAINING_RX_DESC + 0x10 * 3999)), __LINE__));
        Result2 |= 1;
        break;
      }
    }

    while (0x80000000 != (MmioRead32 ((UINT64)(TRAINING_TX_DESC + 0x10 * 3999 + 0x4)) & 0xf0000000)) {
      MicroSecondDelay (50);
      Index2++;
      if (Index2 > 300000) {
        DEBUG ((DEBUG_ERROR, "reg2 = %x,%d\n", MmioRead32 ((UINT64)(TRAINING_TX_DESC + 0x10 * 3999 + 0x4)), __LINE__));
        Result2 |= 2;
        break;
      }
    }

    TxPkts = MmioRead32 ((UINT64)(MacBase + 0x108));
    RxPkts = MmioRead32 ((UINT64)(MacBase + 0x158));
    Symbol   = MmioRead32 ((UINT64)(MacBase + 0x198));
    Fcs = MmioRead32 ((UINT64)(MacBase + 0x190));
    ArmInstructionSynchronizationBarrier ();
    if (TxPkts == RxPkts) {
      if (Symbol | Fcs) {
        DEBUG((DEBUG_INFO, "error pkts Fcs:0x%x Symbol:0x%x\n", Fcs,
                Symbol));
        Result2 |= 4;
      }
    } else {
      Result2 |= 8;
    }
    DEBUG((DEBUG_INFO, "TxPkts 0x%x, RxPkts 0x%x\n", TxPkts, RxPkts));

    Result1 = Result2;
    /*close*/
    MmioWrite32 ((UINT64)(MacBase + 0x000), 0x00000000);
    ArmInstructionSynchronizationBarrier ();
TrainingExit:
    if (Result1) {
      DEBUG ((DEBUG_INFO, "RST:SEL_TEST_FAIL %d\n", Result1));
      Count++;//5 times mabe risk
      if (Count == 6) {
        DEBUG ((DEBUG_INFO,("RST:SEL_TEST_ERROR\n")));
        //set_sgmii_digital_loopack_exit(MacBase);
        JustMdioWrite(MacBase, 0, 0x0, RecData);//reset
        return FALSE;
      }

      if (MacBase == GMAC1_BASE_ADDR) {
        PhyRstOffset = 0x4008c;
      } else {
        PhyRstOffset = 0x40254;
      }
      MmioWrite32 ((UINT64)(PhyBase + PhyRstOffset), 0x00000000);
      MicroSecondDelay (20 * 1000);
      MmioWrite32 ((UINT64)(PhyBase + PhyRstOffset), 0x00000001);
      MicroSecondDelay (10 * 1000);
    } else {
      /*rx_buf*/
      SetMem ((VOID *)(TRAINING_RX_BUFFER + 0x000), 0x80*1000, 0);
      /*tx_buf*/
      SetMem ((VOID *)(TRAINING_TX_BUFFER + 0x00), 0x80*4, 0);
      DEBUG ((DEBUG_INFO,("SEL_SGMII_SUCCESS\n")));
      JustMdioWrite (MacBase, 0, 0x0, RecData);//reset
      break;
    }
  }

  DEBUG ((DEBUG_INFO,("RST:SEL_TEST_PASS\n")));

  return TRUE;
}


VOID
LsdDmaChannelConfig (
  VOID
  )
{
  MmioWrite32 (LSD_NAND_MMCSD_HDDR, 0);
  MmioWrite32 (LSD_CFG_LPI0, 0);
  MmioWrite32 (LSD_LMC_LPI0, 0);
  MmioWrite32 (LSD_LSD_LPI0, 0);
  MmioWrite32 (LSD_NDF_LPI0, 0);
  MmioWrite32 (LSD_SIO_LPI0, 0);
}

/**


**/
VOID
UsbSataConfig (
  VOID
  )
{
  //
  //some config
  //
  //
  //gsd sata
  //
  MmioWrite32 (0x32014000 + 0x0, 0x08000000);
  MmioWrite32 (0x32014000 + 0xC, 0x1);
  MmioWrite32 (0x32014000 + 0x118, 0x04000000);
  //
  //psu sata
  //
  MmioWrite32 (0x31a40000 + 0x0, 0x08000000);
  MmioWrite32 (0x31a40000 + 0xC, 0x1);
  MmioWrite32 (0x31a40000 + 0x118, 0x04000000);
  //
  //usb
  //
  MmioWrite32 (0x31a00000 + 0xa040, 0x20031e00);
  MmioWrite32 (0x31a20000 + 0xa040, 0x20031e00);
  //mouse and keyboard
  //3.0
  MmioWrite32 (0x31a18004, MmioRead32 (0x31a18004) | 0x80000);
  MmioWrite32 (0x31a38004, MmioRead32 (0x31a18004) | 0x80000);
  //2.0
  MmioWrite32 (0x32881004, MmioRead32 (0x32881004) | 0x80000);
  MmioWrite32 (0x328c1004, MmioRead32 (0x328c1004) | 0x80000);
  //
  //usb3.0 bypass overrun
  //
  if (PcdGetBool (PcdPhytiumUsb30OverrunBypass)) {
    MmioWrite32 (0x31A10038, 3);
    DEBUG ((DEBUG_INFO, "Usb3.0 - 0 overrun bypass!\n"));
  }
  if (PcdGetBool (PcdPhytiumUsb31OverrunBypass)) {
    MmioWrite32 (0x31A30038, 3);
    DEBUG ((DEBUG_INFO, "Usb3.0 - 1 overrun bypass!\n"));
  }
  //
  //usb2.0 bypass overrun
  //
  if (PcdGetBool (PcdPhytiumUsb20OverrunBypass)) {
    MmioWrite32 (0x32880038, 0x3);
  }
  if (PcdGetBool (PcdPhytiumUsb21OverrunBypass)) {
    MmioWrite32 (0x328C0038, 0x3);
  }
  //
  //vhub host
  //
  MmioWrite32 (0x319c0000, 0xd9d9);
  MmioWrite32 (0x31990020, 0xa);
  MmioWrite32 (0x31990000, 0x69);
  //
  //usb2.0 25M switch
  //
  MmioWrite32 (0x3288002c, 0x0);
  MmioWrite32 (0x328c002c, 0x0);
  MmioWrite32 (0x319c0000, 0xd909);
  MmioWrite32 (0x31a10020, 0x0);
  MmioWrite32 (0x31a30020, 0x0);
  //vhub
  MmioWrite32 (0x31981004, MmioRead32 (0x31981004) | 0x80000);
}

/**
  Config pad information to regitsters according parameter table.

  @retval    EFI_SUCCESS
             EFI_NOT_FOUND
**/
EFI_STATUS
ConfigPad (
  VOID
  )
{
  UINT32 Groups;
  UINT32 Size;
  UINT32 PadBase;
  UINT32 Buffer[2048];    //max 2k
  UINT32 Index;

  ZeroMem (Buffer, sizeof (Buffer));

  PadBase = PLAT_INFO_BASE;
  DEBUG ((DEBUG_INFO, "pad_base = 0x%x\n", PadBase));
  if (MmioRead32 ((UINT64) PadBase) != PARAMETER_PAD_MAGIC) {
    DEBUG ((DEBUG_ERROR, "ERROR: invalid gpio pad data!\n"));
    return EFI_NOT_FOUND;
  }

  Groups = MmioRead32 ((UINT64) (PadBase + 0x4));
  Size = Groups * 2 * (sizeof (UINT32));
  CopyMem ((VOID *)Buffer, (VOID *)(UINT64)(PadBase + 0x8), Size);
  for (Index = 0; Index < Groups; Index++) {
    DEBUG ((DEBUG_INFO, "value = 0x%x, addr = 0x%x\n", Buffer[Index * 2 + 1], Buffer[Index * 2]));
    MmioWrite32 ((UINT64) Buffer[Index * 2], Buffer[Index * 2 + 1]);
  }
  //
  //some config
  //
  UsbSataConfig ();
  return EFI_SUCCESS;
}


/**
  Print board type.

  @param[in]  BoardType    Board type.

  @retval     Null.
**/
VOID
PrintBoardType (
  IN UINT32  BoardType
  )
{
  switch (BoardType) {
  case 0:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium Q/A\n", BoardType));
    break;
  case 1:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium Q/B\n", BoardType));
    break;
  case 2:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium Q/C1\n", BoardType));
    break;
  case 3:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium Q/C2\n", BoardType));
    break;
  case 4:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium D/S/A\n", BoardType));
    break;
  case 5:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium D/S/B\n", BoardType));
    break;
  case 6:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium D/S/C1\n", BoardType));
    break;
  case 7:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium D/S/C2\n", BoardType));
    break;
  case 8:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium G\n", BoardType));
    break;
  case 10:
    DEBUG ((DEBUG_INFO, "Board Type : %d, Phytium D\n", BoardType));
    break;
  default:
    DEBUG ((DEBUG_INFO, "Error Board Type!"));
    break;
  }
}

/**
  Set pad configuration with board type.

  @param[in]  BoardType    Boart type.
                           4 : Phytium D/S A.
                           5 : Phytium D/S B.
                           8 : Great Wall F080-E.

  @retval     Null.
**/
VOID
ConfigPadWithBoardType (
  IN UINT32  BoardType
  )
{
  UINT32  Value;

  //
  //some config
  //
  UsbSataConfig ();
  PrintBoardType (BoardType);
  switch (BoardType) {
    case 4:
      //
      //gmu mdio and mdc
      //
      //MmioWrite32 (0x32B300f0, 0x4 | (MmioRead32(0x32B300f0)));
      //MmioWrite32 (0x32B300f4, 0x4 | (MmioRead32(0x32B300f4)));
      MmioWrite32 (0x32B300f8, 0x73);
      MmioWrite32 (0x32B300fc, 0x73);
      MmioWrite32 (0x32B30100, 0x73);
      MmioWrite32 (0x32B30104, 0x73);
      //Mio3
      MmioWrite32 (0x32b30184, 0x71);
      MmioWrite32 (0x32b30188, 0x71);
      //Mio6 ddr spd
      MmioWrite32 (0x32b300a8, 0x74);
      MmioWrite32 (0x32b300ac, 0x74);
      //
      //bypass usu3-1 overrun
      //
      //MmioWrite32 (0x31a10038, 0x3);
      //dp
      //MmioWrite32 (0x32b300c0, 0x71);
      //MmioWrite32 (0x32b300c4, 0x71);
      ////
      ////Spim0
      ////SPIM0_SCLK = PAD_GPIO79 fun:2
      ////SPIM0_TXD = PAD_GPIO80 fun:2
      ////PAD_GPIO81 = SPIM0_RXD fun:2
      ////SPIM0_CS0 = PAD_GPIO82 fun:2
      ////SPIM0_CS1 = PAD_GPIO83 fun:2
      ////SPIM0_CS2 = PAD_GPIO84 fun:2
      ////SPIM0_CS3 = PAD_GPIO85 fun:2
      ////
      //MmioWrite32 (0x32B30134, 0x72);
      //MmioWrite32 (0x32B30138, 0x72);
      //MmioWrite32 (0x32B3013c, 0x72);
      //MmioWrite32 (0x32B30140, 0x72);
      //MmioWrite32 (0x32B30144, 0x72);
      //MmioWrite32 (0x32B30148, 0x72);
      //MmioWrite32 (0x32B3014c, 0x72);
      //
      //Spim2
      //SPI2_SCLK = PAD_GPIO61;
      //SPI2_TXD = PAD_GPIO62;
      //PAD_GPIO63 = SPI2_RXD;
      //SPI2_CSN0 = PAD_GPIO64;
      //
      break;
    case 5:
    // dp0_hpd, gpio51 00c0  fun0
      Value =  MmioRead32 (0x32B300c0);
      Value &= ~(0x7);
      MmioWrite32 (0x32B300c0, Value);

      MmioWrite32 (0x32b30168, 0x73);
      MmioWrite32 (0x32b3016c, 0x73);
      MmioWrite32 (0x32b30170, 0x73);
      MmioWrite32 (0x32b30174, 0x73);
      MmioWrite32 (0x32b30178, 0x73);
      MmioWrite32 (0x32b3017c, 0x73);
      MmioWrite32 (0x32b30180, 0x73);
      MmioWrite32 (0x32b30184, 0x73);
      MmioWrite32 (0x32b30188, 0x73);
    //
    //RGMII1/Mac3
    //
     // MmioWrite32 (0x32b301bc, 0x71);
     // MmioWrite32 (0x32b301c0, 0x71);
     // MmioWrite32 (0x32b30210, 0x75);
     // MmioWrite32 (0x32b30214, 0x75);
     // MmioWrite32 (0x32b30218, 0x75);
     // MmioWrite32 (0x32b3021c, 0x75);
    //rgmii
      MmioWrite32 (0x32b301cc, 0x41);
      MmioWrite32 (0x32b301d0, 0x41);
      MmioWrite32 (0x32b301d4, 0x41);
      MmioWrite32 (0x32b301d8, 0x41);
      MmioWrite32 (0x32b301dc, 0x41);
      MmioWrite32 (0x32b301e0, 0x41);
      MmioWrite32 (0x32b301e4, 0x41);
      MmioWrite32 (0x32b301e8, 0x41);
      MmioWrite32 (0x32b301ec, 0x41);
      //
      //rgmii tx clock
      //
      MmioWrite32 (0x32b301f0, 0x41);
      MmioWrite32 (0x32b311f0, 0x5f00);
      MmioWrite32 (0x32b301f4, 0x41);
      MmioWrite32 (0x32b301f8, 0x41);
    //mdio/mdc
      Value = MmioRead32 (0x32b301fc);
      Value &= ~(0x7);
      Value |= 0x1;
      MmioWrite32 (0x32b301fc, Value);
      Value = MmioRead32 (0x32b30200);
      Value &= ~(0x7);
      Value |= 0x1;
      MmioWrite32 (0x32b30200, Value);
      //Mio6 ddr spd
      MmioWrite32 (0x32b300a8, 0x74);
      MmioWrite32 (0x32b300ac, 0x74);
      //
      //pad_gpio148,[74:69] fun:0 pad_gpio[78:75] fun:1 EMMC0
      //
      MmioWrite32 (0x32b300f8, 0x70);
      MmioWrite32 (0x32b300fc, 0x70);
      MmioWrite32 (0x32b30100, 0x70);
      MmioWrite32 (0x32b30104, 0x70);
      MmioWrite32 (0x32b30108, 0x70);
      MmioWrite32 (0x32b3010c, 0x70);
      MmioWrite32 (0x32b30110, 0x70);
      MmioWrite32 (0x32b30114, 0x70);
      MmioWrite32 (0x32b30118, 0x70);
      MmioWrite32 (0x32b3011c, 0x70);
      MmioWrite32 (0x32b30120, 0x70);
      MmioWrite32 (0x32b30124, 0x71);
      MmioWrite32 (0x32b30128, 0x71);
      MmioWrite32 (0x32b3012c, 0x71);
      MmioWrite32 (0x32b30130, 0x71);
      MmioWrite32 (0x32b30134, 0x71);
      MmioWrite32 (0x32b30138, 0x71);
      MmioWrite32 (0x32b3013c, 0x71);
      //
      //eMMC1
      //
      //MmioWrite32 (0x32b31240, 0x70);
      //MmioWrite32 (0x32b30164, 0x70);
      //MmioWrite32 (0x32b30168, 0x70);
      //MmioWrite32 (0x32b3016c, 0x70);
      //MmioWrite32 (0x32b30170, 0x70);
      //MmioWrite32 (0x32b30174, 0x70);
      //MmioWrite32 (0x32b30178, 0x70);
      //MmioWrite32 (0x32b3017c, 0x70);
      //MmioWrite32 (0x32b30180, 0x70);
      //MmioWrite32 (0x32b30184, 0x70);
      //MmioWrite32 (0x32b30188, 0x70);
      //MmioWrite32 (0x32b3018c, 0x75);
      //MmioWrite32 (0x32b30190, 0x75);
      //MmioWrite32 (0x32b30194, 0x75);
      //MmioWrite32 (0x32b30198, 0x75);
      //MmioWrite32 (0x32b3019c, 0x75);
      //MmioWrite32 (0x32b301a0, 0x75);
      //MmioWrite32 (0x32b301a4, 0x75);
      //MmioWrite32 (0x32b301a8, 0x75);
      //
      //I2c rtc Mio8
      //
      MmioWrite32 (0x32b301ac, 0x73);
      MmioWrite32 (0x32b301b0, 0x73);
      break;
    case 8:
      //
      //pad_gpio[19:15] fun:5 gpio
      //
      MmioWrite32 (0x32b30038, 0x75);
      MmioWrite32 (0x32b3003c, 0x75);
      MmioWrite32 (0x32b30040, 0x75);
      MmioWrite32 (0x32b30044, 0x75);
      MmioWrite32 (0x32b30048, 0x75);
      //
      //pad_gpio[28:24] fun:5 gpio
      //
      MmioWrite32 (0x32b3005c, 0x75);
      MmioWrite32 (0x32b30060, 0x75);
      MmioWrite32 (0x32b30064, 0x75);
      MmioWrite32 (0x32b30068, 0x75);
      MmioWrite32 (0x32b3006c, 0x70);
      //
      //pad_gpio[33:32] fun:4 Mio3-I2c
      //
      MmioWrite32 (0x32b30074, 0x74);
      MmioWrite32 (0x32b30078, 0x74);
      //
      //pad_gpio[36:34] fun:1 SMBUS
      //
      MmioWrite32 (0x32b3007c, 0x71);
      MmioWrite32 (0x32b30080, 0x71);
      MmioWrite32 (0x32b30084, 0x71);
	  //
      //pad_gpio[89:90] fun:4 Mio9-I2c RTC
      //
      MmioWrite32 (0x32b3015c, 0x74);
      MmioWrite32 (0x32b30160, 0x74);
      //
      //pad_gpio[60:59] fun:5 Mio10-I2c
      //
      MmioWrite32 (0x32b300e0, 0x75);
      MmioWrite32 (0x32b300e4, 0x75);
      //
      //pad_gpio148,[74:69] fun:0 pad_gpio[78:75] fun:1 EMMC0
      //
      MmioWrite32 (0x32b30108, 0x70);
      MmioWrite32 (0x32b3010c, 0x70);
      MmioWrite32 (0x32b30110, 0x70);
      MmioWrite32 (0x32b30114, 0x70);
      MmioWrite32 (0x32b30118, 0x70);
      MmioWrite32 (0x32b3011c, 0x70);
      MmioWrite32 (0x32b30120, 0x70);
      MmioWrite32 (0x32b30124, 0x71);
      MmioWrite32 (0x32b30128, 0x71);
      MmioWrite32 (0x32b3012c, 0x71);
      MmioWrite32 (0x32b30130, 0x71);
      //
      //MIPI pad_gpio[82:79] fun:2 spim0, pad_gpio[84:83] fun:6 gpio
      //
      MmioWrite32 (0x32b30134, 0x72);
      MmioWrite32 (0x32b30138, 0x72);
      MmioWrite32 (0x32b3013c, 0x72);
      MmioWrite32 (0x32b30140, 0x72);
      MmioWrite32 (0x32b30144, 0x76);
      MmioWrite32 (0x32b30148, 0x76);
      //
      //pad_gpio[88:86] fun:6 gpio
      //
      MmioWrite32 (0x32b30150, 0x76);
      MmioWrite32 (0x32b30154, 0x76);
      MmioWrite32 (0x32b30158, 0x76);
      //
      //pad_gpio[98:92] pad_gpio[30:29] fun:3 lpcm
      //
      MmioWrite32 (0x32b30168, 0x73);
      MmioWrite32 (0x32b3016c, 0x73);
      MmioWrite32 (0x32b30170, 0x73);
      MmioWrite32 (0x32b30174, 0x73);
      MmioWrite32 (0x32b30178, 0x73);
      MmioWrite32 (0x32b3017c, 0x73);
      MmioWrite32 (0x32b30180, 0x73);
      MmioWrite32 (0x32b30184, 0x73);
      MmioWrite32 (0x32b30188, 0x73);
      //
      //pad_gpio[100:99] fun:3 mio4-I2c
      //
      MmioWrite32 (0x32b3018c, 0x73);
      MmioWrite32 (0x32b30190, 0x73);
      //
      //pad_gpio[105:101] fun:6 gpio
      //
      MmioWrite32 (0x32b30194, 0x76);
      MmioWrite32 (0x32b30198, 0x76);
      MmioWrite32 (0x32b3019c, 0x76);
      MmioWrite32 (0x32b301a0, 0x76);
      MmioWrite32 (0x32b301a4, 0x76);
      //
      //pad_gpio[110:106] fun:4 spim1
      //
      MmioWrite32 (0x32b301a8, 0x74);
      MmioWrite32 (0x32b301ac, 0x74);
      MmioWrite32 (0x32b301b0, 0x74);
      MmioWrite32 (0x32b301b4, 0x74);
      MmioWrite32 (0x32b301b8, 0x74);
      //
      //pad_gpio[122:121] fun:3 mio12-I2c
      //
      MmioWrite32 (0x32b301e4, 0x73);
      MmioWrite32 (0x32b301e8, 0x73);
      //
      //pad_gpio[128:126] fun:6 gpio
      //
      MmioWrite32 (0x32b301f8, 0x76);
      MmioWrite32 (0x32b301fc, 0x76);
      MmioWrite32 (0x32b30200, 0x76);
      //
      //pad_gpio[136:132] fun:2 HDA
      //
      MmioWrite32 (0x32b30210, 0x72);
      MmioWrite32 (0x32b30214, 0x72);
      MmioWrite32 (0x32b30218, 0x72);
      MmioWrite32 (0x32b3021c, 0x72);
      MmioWrite32 (0x32b30220, 0x72);
      //if (PcdGet8 (PcdPhytiumUsb30OverrunBypass)) {
      //  MmioWrite32 (0x31A10038, 3);
      //}
      //if (PcdGet8 (PcdPhytiumUsb31OverrunBypass)) {
      //  MmioWrite32 (0x31A30038, 3);
      //}
      MmioWrite32 (0x31A10038, 3);
      MmioWrite32 (0x31A30038, 3);
      //
      //pad_gpio[147:146] fun:6 Mio15-I2c
      //
      MmioWrite32 (0x32b30248, 0x276);
      MmioWrite32 (0x32b3024c, 0x276);
      break;
    case 0x9:
      MmioWrite32 (0x32b300c0, 0x45);
      MmioWrite32 (0x32b301c4, 0x41);
      break;
    //lhl add
    case 10:
      //
      //pad_gpio[19:15] fun:5 gpio
      //
      MmioWrite32 (0x32b30038, 0x75);
      MmioWrite32 (0x32b3003c, 0x75);
      MmioWrite32 (0x32b30040, 0x75);
      MmioWrite32 (0x32b30044, 0x75);
      MmioWrite32 (0x32b30048, 0x75);
      //
      //pad_gpio[28:24] fun:5 gpio
      //
      MmioWrite32 (0x32b3005c, 0x75);
      MmioWrite32 (0x32b30060, 0x75);
      MmioWrite32 (0x32b30064, 0x75);
      MmioWrite32 (0x32b30068, 0x75);
      MmioWrite32 (0x32b3006c, 0x70);  //lhl add for usb wake
      //
      //pad_gpio[33:32] fun:4 Mio3-I2c
      //
      MmioWrite32 (0x32b30074, 0x74);
      MmioWrite32 (0x32b30078, 0x74);
      //
      //pad_gpio[36:34] fun:1 SMBUS
      //
      MmioWrite32 (0x32b3007c, 0x71);
      MmioWrite32 (0x32b30080, 0x71);
      MmioWrite32 (0x32b30084, 0x71);
      //
      //pad_gpio[60:59] fun:5 Mio10-I2c
      //
      MmioWrite32 (0x32b300e0, 0x76);  //ÒªÅä³ÉGPIO¹¦ÄÜ for usb en
      MmioWrite32 (0x32b300e4, 0x76);
      //
      //pad_gpio148,[74:69] fun:0 pad_gpio[78:75] fun:1 EMMC0
      //
      MmioWrite32 (0x32b30108, 0x70);
      MmioWrite32 (0x32b3010c, 0x70);
      MmioWrite32 (0x32b30110, 0x70);
      MmioWrite32 (0x32b30114, 0x70);
      MmioWrite32 (0x32b30118, 0x70);
      MmioWrite32 (0x32b3011c, 0x70);
      MmioWrite32 (0x32b30120, 0x70);
      MmioWrite32 (0x32b30124, 0x71);
      MmioWrite32 (0x32b30128, 0x71);
      MmioWrite32 (0x32b3012c, 0x71);
      MmioWrite32 (0x32b30130, 0x71);
      MmioWrite32 (0x32b31120, 0x1f00);  //delay 1ns
      //
      //MIPI pad_gpio[82:79] fun:2 spim0, pad_gpio[84:83] fun:6 gpio
      //
      MmioWrite32 (0x32b30134, 0x72);
      MmioWrite32 (0x32b30138, 0x72);
      MmioWrite32 (0x32b3013c, 0x72);
      MmioWrite32 (0x32b30140, 0x72);
      MmioWrite32 (0x32b30144, 0x76);
      MmioWrite32 (0x32b30148, 0x76);
      //
      //pad_gpio[88:86] fun:6 gpio
      //
      MmioWrite32 (0x32b30150, 0x76);
      MmioWrite32 (0x32b30154, 0x76);
      MmioWrite32 (0x32b30158, 0x76);
      //
      //pad_gpio[98:92] pad_gpio[30:29] fun:3 lpcm
      //
      MmioWrite32 (0x32b30168, 0x23);
      MmioWrite32 (0x32b3016c, 0x23);
      MmioWrite32 (0x32b30170, 0x23);
      MmioWrite32 (0x32b30174, 0x23);
      MmioWrite32 (0x32b30178, 0x23);
      MmioWrite32 (0x32b3017c, 0x23);
      MmioWrite32 (0x32b30180, 0x23);
      MmioWrite32 (0x32b30184, 0x23);
      MmioWrite32 (0x32b30188, 0x23);

      MmioWrite32 (0x32b31174, 0x5100);
      MmioWrite32 (0x32b31178, 0x5100);
      MmioWrite32 (0x32b3117c, 0x5100);
      MmioWrite32 (0x32b31180, 0x5100);
      //
      //pad_gpio[100:99] fun:3 mio4-I2c
      //
      //RGMII1
      MmioWrite32 (0x32b301cc, 0x41);
      MmioWrite32 (0x32b301d0, 0x41);
      MmioWrite32 (0x32b301d4, 0x41);
      MmioWrite32 (0x32b301d8, 0x41);
      MmioWrite32 (0x32b301dc, 0x41);
      MmioWrite32 (0x32b301e0, 0x41);
      MmioWrite32 (0x32b301e4, 0x41);
      MmioWrite32 (0x32b301e8, 0x41);
      MmioWrite32 (0x32b301ec, 0x41);
      //
      //rgmii tx clock
      //
      MmioWrite32 (0x32b301f0, 0x41);
      MmioWrite32 (0x32b311f0, 0x5f00);
      MmioWrite32 (0x32b301f4, 0x41);
      MmioWrite32 (0x32b301f8, 0x41);
    //mdio/mdc
      Value = MmioRead32 (0x32b301fc);
      Value &= ~(0x7);
      Value |= 0x1;
      MmioWrite32 (0x32b301fc, Value);
      Value = MmioRead32 (0x32b30200);
      Value &= ~(0x7);
      Value |= 0x1;
      MmioWrite32 (0x32b30200, Value);
      //lhl modify modify for rgmii0
     MmioWrite32 (0x32b30054, 0x75);//auto deteced sata pcie
  
  #if 0
     MmioWrite32 (0x32b3018c, 0x71);
     MmioWrite32 (0x32b30190, 0x71);
     //  lhl modify
      
      //pad_gpio[105:101] fun:6 gpio  
      // 
      MmioWrite32 (0x32b30194, 0x71);
      MmioWrite32 (0x32b30198, 0x71);
      MmioWrite32 (0x32b3019c, 0x71);
      MmioWrite32 (0x32b301a0, 0x71);
      MmioWrite32 (0x32b301a4, 0x71);

      MmioWrite32 (0x32b311b0, 0x5f00);
      //
      //pad_gpio[110:105] fun:4
      //
      MmioWrite32 (0x32b301a8, 0x71);
      MmioWrite32 (0x32b301ac, 0x71);
      MmioWrite32 (0x32b301b0, 0x71);
      MmioWrite32 (0x32b301b4, 0x71);
      MmioWrite32 (0x32b301b8, 0x71);

      //pad_gpio[112:111] //mdc 
      MmioWrite32 (0x32b301bc, 0x71);
      MmioWrite32 (0x32b301c0, 0x71);
      
      ////lhl modify for rgmii0
#endif
      
      //pad_gpio[122:121] fun:3 mio12-I2c
      //lhl modify  no use
     //mioWrite32 (0x32b301e4, 0x73);
   // MmioWrite32 (0x32b301e8, 0x73);
      // //lhl modify 
      
      //pad_gpio[128:126] fun:6 gpio
      //
      /*mioWrite32 (0x32b301f8, 0x76);  no use
      MmioWrite32 (0x32b301fc, 0x76);
      MmioWrite32 (0x32b30200, 0x76);*///lhl modfiy
      
      //
      //pad_gpio[136:132] fun:2 HDA
      //
      MmioWrite32 (0x32b30210, 0x72);
      MmioWrite32 (0x32b30214, 0x72);
      MmioWrite32 (0x32b30218, 0x72);
      MmioWrite32 (0x32b3021c, 0x72);
      MmioWrite32 (0x32b30220, 0x72);
      //if (PcdGet8 (PcdPhytiumUsb30OverrunBypass)) {
      //  MmioWrite32 (0x31A10038, 3);
      //}
      //if (PcdGet8 (PcdPhytiumUsb31OverrunBypass)) {
      //  MmioWrite32 (0x31A30038, 3);
      //}
      MmioWrite32 (0x31A10038, 3);
      MmioWrite32 (0x31A30038, 3);
      //
      //pad_gpio[64:61] fun:6 SPIM2 default  lhl add
      //
     /* MmioWrite32 (0x32b300e8, 0x276);
      MmioWrite32 (0x32b300ec, 0x276);
      MmioWrite32 (0x32b300f0, 0x276);
      MmioWrite32 (0x32b300f4, 0x276);*/
      //pad_gpio[52:51]:  //hdp 
      MmioWrite32 (0x32b300c0, 0x70);
      MmioWrite32 (0x32b300c4, 0x70);
      
      //pad_gpio[54:53]:  mio0a mio0b  lhl add RTC I2C
      MmioWrite32 (0x32b300c8, 0x75);
      MmioWrite32 (0x32b300cc, 0x75);
      
      //pad_gpio[55:56]:  mio1a mio1b  lhl add
      MmioWrite32 (0x32b300d0, 0x75);
      MmioWrite32 (0x32b300d4, 0x75);
         
      //lhl add 
      break;
    default:
      DEBUG ((DEBUG_INFO, "Not Supported!"));
      break;
  }
}

/**
  Config phy through SMC message according to the board type.

  @param[in]  BoardType    Boart type.
                           4 : Phytium D/S A.
                           5 : Phytium D/S B.
                           8 : Great Wall F080-E.

  @retval     Null.
**/
VOID
PhyConfigWithBoardType (
  IN UINT32  BoardType
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  DEBUG ((DEBUG_INFO, "Phy Config!\n"));
  PrintBoardType (BoardType);
  SetMem ((VOID*)&ArmSmcArgs, sizeof (ARM_SMC_ARGS), 0);
  switch (BoardType) {
  case 4:
    ArmSmcArgs.Arg0 = 0xc2000015;
    ArmSmcArgs.Arg1 = 0x3f;
    ArmSmcArgs.Arg2 = 0x508;
    ArmSmcArgs.Arg3 = 0x3300;
    ArmSmcArgs.Arg4 = 0x3300;
    break;
  case 5:
    ArmSmcArgs.Arg0 = 0xc2000015;
    ArmSmcArgs.Arg1 = 0x3f;
    ArmSmcArgs.Arg2 = 0x100;
    ArmSmcArgs.Arg3 = 0x300000;
    ArmSmcArgs.Arg4 = 0x300000;
    break;
  case 8:
    //
    //psu_phy0 : usb3.0-0
    //psu_phy1 : usb3.0-1
    //gsd_phy0 : none
    //gsd_phy1 : SATA1
    //gsd_phy2 : DP0
    //gsd_phy3 : none
    //
    ArmSmcArgs.Arg0 = 0xc2000015;
    ArmSmcArgs.Arg1 = 0x1B;
    ArmSmcArgs.Arg2 = 0x184;
    ArmSmcArgs.Arg3 = 0x0;
    ArmSmcArgs.Arg4 = 0x0;
    break;
  default:
    DEBUG ((DEBUG_INFO, "Not Supported!"));
    break;
  }
  ArmCallSmc (&ArmSmcArgs);
}

/**
  Parse the single PHY configuration obtained from the parameter table.

  @param[in]   BoardConfig    A pointer to BOARD_CONFIG.
  @param[in]   Num            Index of phy.
  @param[out]  PhyConfig      A pointer to PHY_CONFIG.

  @retval      Null.
**/
STATIC
VOID
ParsePhyConfig (
  IN  BOARD_CONFIG  *BoardConfig,
  IN  UINT8         Num,
  OUT PHY_CONFIG    *PhyConfig
  )
{
  PhyConfig->PhySel = ((BoardConfig->PhySelMode >> (Num * 2)) & 0x3);
  switch (Num) {
  case 0:
  case 1:
    PhyConfig->MacMode = 0;
    break;
  case 2:
  case 3:
  case 4:
  case 5:
    PhyConfig->MacMode = (BoardConfig->GmacSpeedMode >> ((Num - 2) * 4)) & 0xF;
    PhyConfig->PhySpeed = (BoardConfig->GmacSpeedMode >> ((Num - 2) * 4 + 16)) & 0xF;
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Error Phy Index!\n"));
    break;
  }
  DEBUG ((DEBUG_INFO, "Phy Config[%d] : \n", Num));
  DEBUG ((DEBUG_INFO, "Phy Sel : %d\n", PhyConfig->PhySel));
  DEBUG ((DEBUG_INFO, "Mac Mode : %d\n", PhyConfig->MacMode));
  DEBUG ((DEBUG_INFO, "Mac Speed : %d\n", PhyConfig->PhySpeed));
}

/**
  Configure phy according to the parameter table.

  @retval    EFI_SUCCESS    Success.
  @retval    EFI_NOT_FOUND  Not fount phy information.
**/
EFI_STATUS
PhyConfigWithParTable (
  VOID
  )
{
  EFI_STATUS    Status;
  PHY_CONFIG    PhyConfig[6];
  UINT8         Buffer[256];
  BOARD_CONFIG  *BoardConfig;
  UINT32        Index;
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT8         GpioIndex;
  UINT8         GpioPort;
  UINT8         Detect;

  Status = GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Get phy config from parameter table failed!\n"));
    return EFI_NOT_FOUND;
  }
  BoardConfig = (BOARD_CONFIG *)Buffer;
  for (Index = 0; Index < 6; Index++) {
    ZeroMem (&PhyConfig[Index], sizeof (PHY_CONFIG));
    ParsePhyConfig (BoardConfig, Index, &PhyConfig[Index]);
  }

  SetMem ((VOID*)&ArmSmcArgs, sizeof (ARM_SMC_ARGS), 0);
  //
  //Sata0 and PCIe auto detect
  //
  if (PcdGetBool (PcdSata0PcieAutoDetectEnableEnable)) {
    GpioIndex = (PcdGet8 (PcdSata0PcieAutoDetectPort) >> 4) & 0xF;
    GpioPort = (PcdGet8 (PcdSata0PcieAutoDetectPort) >> 0) & 0xF;
    Detect = 0;
    Status = DirectGetGpioInputLevel (GpioIndex, GpioPort, &Detect);
    DEBUG ((DEBUG_INFO, "Sata0 and PCIe audo detect:\n"));
    DEBUG ((DEBUG_INFO,
            "Inde : %d, Port : %d, Detect : %d, Status : %r",
            GpioIndex, GpioPort, Detect, Status
            ));
    if (!EFI_ERROR (Status)) {
      if (Detect == AUTO_DETECT_SATA) {
        PhyConfig[1].PhySel = 0;
      } else if (Detect == AUTO_DETECT_PCIE) {
        PhyConfig[1].PhySel = 2;
      }
    }
  }
  ArmSmcArgs.Arg0 = 0xc2000015;
  ArmSmcArgs.Arg1 = BoardConfig->PhySelMode >> 24;
  //
  //Phy Sel
  //
  ArmSmcArgs.Arg2 = ((PhyConfig[0].PhySel & 0x3) << 0) | \
                    ((PhyConfig[1].PhySel & 0x3) << 2) | \
                    ((PhyConfig[2].PhySel & 0x3) << 4) | \
                    ((PhyConfig[3].PhySel & 0x3) << 6) | \
                    ((PhyConfig[4].PhySel & 0x3) << 8) | \
                    ((PhyConfig[5].PhySel & 0x3) << 10);
  //
  //Mac Mode
  //
  ArmSmcArgs.Arg3 = ((PhyConfig[0].MacMode & 0xf) << 0) | \
                    ((PhyConfig[1].MacMode & 0xf) << 4) | \
                    ((PhyConfig[2].MacMode & 0xf) << 8) | \
                    ((PhyConfig[3].MacMode & 0xf) << 12) | \
                    ((PhyConfig[4].MacMode & 0xf) << 16) | \
                    ((PhyConfig[5].MacMode & 0xf) << 20);
  //
  //Phy Speed
  //
  ArmSmcArgs.Arg4 = ((PhyConfig[0].PhySpeed & 0xf) << 0) | \
                    ((PhyConfig[1].PhySpeed & 0xf) << 4) | \
                    ((PhyConfig[2].PhySpeed & 0xf) << 8) | \
                    ((PhyConfig[3].PhySpeed & 0xf) << 12) | \
                    ((PhyConfig[4].PhySpeed & 0xf) << 16) | \
                    ((PhyConfig[5].PhySpeed & 0xf) << 20);

  DEBUG ((DEBUG_INFO, "Smc phy config : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
                         ArmSmcArgs.Arg0,
                         ArmSmcArgs.Arg1,
                         ArmSmcArgs.Arg2,
                         ArmSmcArgs.Arg3,
                         ArmSmcArgs.Arg4
                         ));

  ArmCallSmc (&ArmSmcArgs);

  if (PcdGetBool (PcdSgmiiTraining)) {
    for (Index = 2; Index < 6; Index++) {
      if (((PhyConfig[Index].PhySel & 0x3) & 0xf) == 0 &&
          (PhyConfig[Index].MacMode == 3) &&
          (PhyConfig[Index].PhySpeed & 0xf) == 3) {
            if (0 == EthSgmiiTraining (GMAC0_BASE_ADDR + ((Index - 2) * 0x2000), GSD_PHY0_BASE + ((Index - 2) * 0x100000))) {
              DEBUG ((DEBUG_ERROR, "Sgmii Training Error!\n"));
            }
          }
    }
  }


  return EFI_SUCCESS;
}

/**
  Configure mio selection according to the parameter table.

  @retval    EFI_SUCCESS    Success.
  @retval    EFI_NOT_FOUND  Not fount mio information.
**/
EFI_STATUS
MioConfigWithParTable (
  VOID
  )
{
  EFI_STATUS    Status;
  UINT8         Buffer[256];
  BOARD_CONFIG  *BoardConfig;
  UINT32        Index;
  UINT32        MioSel;
  UINT8         Sel;

  Status = GetParameterInfo (PM_BOARD, Buffer, sizeof(Buffer));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Get mio config from parameter table failed!\n"));
    return EFI_NOT_FOUND;
  }
  BoardConfig = (BOARD_CONFIG *)Buffer;
  MioSel = BoardConfig->MioSel;
  DEBUG ((DEBUG_INFO, "MIO Select : 0x%x\n", MioSel));
  for (Index = 0; Index < MIO_MAX_COUNT; Index++) {
    Sel = (MioSel >> Index) & 0x1;
    if (Sel == MIO_I2C_SELECT) {
      MmioWrite32 (MIO_REGISTER_BASE + Index * MIO_BASE_OFFSET +
                     MIO_CONFIG_BASE  + MIO_SELECT_REG, 0);
      DEBUG ((DEBUG_INFO, "MIO[%d] : I2c\n", Index));
    } else if (Sel == MIO_UART_SELECT) {
      MmioWrite32 (MIO_REGISTER_BASE + Index * MIO_BASE_OFFSET +
                     MIO_CONFIG_BASE  + MIO_SELECT_REG, 1);
      DEBUG ((DEBUG_INFO, "MIO[%d] : Uart\n", Index));
    }
  }

  return EFI_SUCCESS;
}
