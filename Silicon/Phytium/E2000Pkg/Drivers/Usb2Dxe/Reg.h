/** @file
Phytium usb2 register description.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PHYTIUM_USB2_REG_H_
#define  PHYTIUM_USB2_REG_H_

#define BIT(Index)                    (0x1 << Index)

#define USBIEN                        0x198
#define USBIEN_SUDAVIE                BIT0
#define USBIEN_SOFIE                  BIT1
#define USBIEN_SUTOKIE                BIT2
#define USBIEN_SUSPIE                 BIT3
#define USBIEN_URESIE                 BIT4
#define USBIEN_HSPEEDIE               BIT5
#define USBIEN_LPMIE                  BIT7

#define USBCS                         0x1a3
#define USBCS_LSMODE                  BIT0
#define USBCS_LPMNYET                 BIT1
#define USBCS_SIGSUME                 BIT5
#define USBCS_DISCON                  BIT6
#define USBCS_WAKESRC                 BIT7

#define USBIR_SOF                     BIT1
#define USBIR_SUSP                    BIT3
#define USBIR_URES                    BIT4
#define USBIR_HSPEED                  BIT5

#define USBIR_SUDAV                   BIT0
#define USBIR_SUTOK                   BIT2
#define USBIR_LPMIR                   BIT7

#define OTGIRQ_IDLEIRQ                BIT0
#define OTGIRQ_SRPDETIRQ              BIT1
#define OTGIRQ_CONIRQ                 BIT2
#define OTGIRQ_LOCSOFIRQ              BIT2
#define OTGIRQ_VBUSERRIRQ             BIT3
#define OTGIRQ_PERIPHIRQ              BIT4
#define OTGIRQ_IDCHANGEIRQ            BIT5
#define OTGIRQ_HOSTDISCON             BIT6
#define OTGIRQ_BSE0SRPIRQ             BIT7

#define OTGCTRL_BUSREQ                BIT0
#define OTGCTRL_ABUSDROP              BIT1
#define OTGCTRL_ASETBHNPEN            BIT2
#define OTGCTRL_BHNPEN                BIT3
#define OTGCTRL_SRPVBUSDETEN          BIT4
#define OTGCTRL_SRPDATDETEN           BIT5
#define OTGCTRL_FORCEBCONN            BIT7

#define OTGSTATUS_ID                  BIT6

#define ENDPRST_EP                    0x0f
#define ENDPRST_IO_TX                 BIT4
#define ENDPRST_TOGRST                BIT5
#define ENDPRST_FIFORST               BIT6
#define ENDPRST_TOGSETQ               BIT7

#define FIFOCTRL_EP                   0x0f
#define FIFOCTRL_IO_TX                BIT4
#define FIFOCTRL_FIFOAUTO             BIT5

#define SPEEDCTRL_LS                  BIT0
#define SPEEDCTRL_FS                  BIT1
#define SPEEDCTRL_HS                  BIT2
#define SPEEDCTRL_HSDISABLE           BIT7

#define CON_TYPE_CONTROL              0x00
#define CON_TYPE_ISOC                 0x04
#define CON_TYPE_BULK                 0x08
#define CON_TYPE_INT                  0x0C
#define CON_TYPE_ISOC_1_ISOD          0x00
#define CON_TYPE_ISOC_2_ISOD          0x10
#define CON_TYPE_ISOC_3_ISOD          0x20
#define CON_STALL                     0x40
#define CON_VAL                       0x80

#define ERR_TYPE                      0x1c
#define ERR_COUNT                     0x03
#define ERR_RESEND                    BIT6
#define ERR_UNDERRIEN                 BIT7

#define ERR_NONE                      0
#define ERR_CRC                       1
#define ERR_DATA_TOGGLE_MISMATCH      2
#define ERR_STALL                     3
#define ERR_TIMEOUT                   4
#define ERR_PID                       5
#define ERR_TOO_LONG_PACKET           6
#define ERR_DATA_UNDERRUN             7

#define EP0CS_HCSETTOGGLE             BIT6
#define EP0CS_HCSET                   BIT4
#define EP0CS_RXBUSY_MASK             BIT3
#define EP0CS_TXBUSY_MASK             BIT2
#define EP0CS_STALL                   BIT0
#define EP0CS_HSNAK                   BIT1
#define EP0CS_DSTALL                  BIT4
#define EP0CS_CHGSET                  BIT7

#define CS_ERR                       0x01
#define CS_BUSY                      0x02
#define CS_NPAK                      0x0c
#define CS_NPAK_OFFSET               0x02
#define CS_AUTO                      0x10

#define CON_BUF_SINGLE               0x00
#define CON_BUF_DOUBLE               0x01
#define CON_BUF_TRIPLE               0x02
#define CON_BUF_QUAD                 0x03
#define CON_BUF                      0x03

//
//address 0x08
//
typedef struct _EP_REG {
  UINT16 RxBc;            //outbc (hcinbc)
  UINT8  RxCon;           //outcon (hcincon)
  UINT8  RxCs;            //outcs (hcincs)
  UINT16 TxBc;            //inbc  (hcoutbc)
  UINT8  TxCon;           //incon (hcoutcon)
  UINT8  TxCs;            //incs (hcoutcs)
} EP_REG;

//
//address 0xC4
//
typedef struct _EP_EXT_REG {
  UINT8 TxCtrl;           //hcoutctrl
  UINT8 TxErr;            //hcouterr
  UINT8 RxCtrl;           //hcinctrl
  UINT8 RxErr;            //hcinerr
} EP_EXT_REG;

//
//address 0x200 to  0x23F
//
typedef struct _RX_SOFTIMER_REG {
  UINT16 Timer;           //hcintmr
  UINT8  Reserved;
  UINT8  Ctrl;            //hcintmrctrl
} RX_SOFTIMER_REG;

//
//address 0x240 to  0x27F
//
typedef struct _TX_SOFTIMER_REG {
  UINT16 Timer;           //hcouttmr
  UINT8  Reserved;
  UINT8  Ctrl;            //hcouttmrctrl
} TX_SOFTIMER_REG;

//
//address 0x304
//
typedef struct _RX_STADDR_REG {
  UINT16 Addr;
  UINT16 Reserved;
} RX_STADDR_REG;

//
//address 0x344
//
typedef struct _TX_STADDR_REG {
  UINT16 Addr;
  UINT16 Reserved;
} TX_STADDR_REG;

//
//address 0x384
//
typedef struct _IRQ_MODE_REG {
  UINT8 Outirqmode;
  UINT8 Reserved21;
  UINT8 Inirqmode;
  UINT8 Reserved22;
} IRQ_MODE_REG;

typedef struct _HW_REGS {
  UINT8            Ep0Rxbc;              /*address 0x00*/
  UINT8            Ep0Txbc;              /*address 0x01*/
  UINT8            Ep0cs;                /*address 0x02*/
  UINT8            Reserved0;
  UINT8            Lpmctrll;             /*address 0x04*/
  UINT8            Lpmctrlh;             /*address 0x05*/
  UINT8            Lpmclock;             /*address 0x06*/
  UINT8            Ep0fifoctrl;          /*address 0x07*/
  EP_REG           Ep[15];               /*address 0x08*/
  UINT8            Reserved1[4];
  UINT32           Fifodat[15];          /*address 0x84*/
  UINT8            Ep0ctrl;              /*address 0xC0*/
  UINT8            Tx0err;               /*address 0xC1*/
  UINT8            Reserved2;
  UINT8            Rx0err;               /*address 0xC3*/
  EP_EXT_REG       EpExt[15];
  UINT8            Ep0datatx[64];        /*address 0x100*/
  UINT8            Ep0datarx[64];        /*address 0x140*/
  UINT8            Setupdat[8];          /*address 0x180*/
  UINT16           Txirq;                /*address 0x188*/
  UINT16           Rxirq;                /*address 0x18A*/
  UINT8            Usbirq;               /*address 0x18C*/
  UINT8            Reserved4;
  UINT16           Rxpngirq;             /*address 0x18E*/
  UINT16           Txfullirq;            /*address 0x190*/
  UINT16           Rxemptirq;            /*address 0x192*/
  UINT16           Txien;                /*address 0x194*/
  UINT16           Rxien;                /*address 0x196*/
  UINT8            Usbien;               /*address 0x198*/
  UINT8            Reserved6;
  UINT16           Rxpngien;             /*address 0x19A*/
  UINT16           Txfullien;            /*address 0x19C*/
  UINT16           Rxemptien;            /*address 0x19E*/
  UINT8            Usbivect;             /*address 0x1A0*/
  UINT8            Foivect;            /*address 0x1A1*/
  UINT8            Endprst;              /*address 0x1A2*/
  UINT8            Usbcs;                /*address 0x1A3*/
  UINT16           Frmnr;                /*address 0x1A4*/
  UINT8            Fnaddr;               /*address 0x1A6*/
  UINT8            Clkgate;              /*address 0x1A7*/
  UINT8            Fifoctrl;             /*address 0x1A8*/
  UINT8            Speedctrl;            /*address 0x1A9*/
  UINT8            Reserved8[1];         /*address 0x1AA*/
  UINT8            Portctrl;             /*address 0x1AB*/
  UINT16           Hcfrmnr;              /*address 0x1AC*/
  UINT16           Hcfrmremain;          /*address 0x1AE*/
  UINT8            Reserved9[4];         /*address 0x1B0*/
  UINT16           Rxerrirq;             /*address 0x1B4*/
  UINT16           Txerrirq;             /*address 0x1B6*/
  UINT16           Rxerrien;             /*address 0x1B8*/
  UINT16           Txerrien;             /*address 0x1BA*/
  /*OTG extension*/
  UINT8            Otgirq;               /*address 0x1BC*/
  UINT8            Otgstate;             /*address 0x1BD*/
  UINT8            Otgctrl;              /*address 0x1BE*/
  UINT8            Otgstatus;            /*address 0x1BF*/
  UINT8            Otgien;               /*address 0x1C0*/
  UINT8            Taaidlbdis;           /*address 0x1C1*/
  UINT8            Tawaitbcon;           /*address 0x1C2*/
  UINT8            Tbvbuspls;            /*address 0x1C3*/
  UINT8            Otg2ctrl;             /*address 0x1C4*/
  UINT8            Reserved10[2];        /*address 0x1C5*/
  UINT8            Tbvbusdispls;         /*address 0x1C7*/
  UINT8            Traddr;               /*address 0x1C8*/
  UINT8            Trwdata;              /*address 0x1C9*/
  UINT8            Trrdata;              /*address 0x1CA*/
  UINT8            Trctrl;               /*address 0x1CB*/
  UINT16           Isoautoarm;           /*address 0x1CC*/
  UINT8            Adpbc1ien;            /*address 0x1CE*/
  UINT8            Adpbc2ien;            /*address 0x1CF*/
  UINT8            Adpbcctr0;            /*address 0x1D0*/
  UINT8            Adpbcctr1;            /*address 0x1D1*/
  UINT8            Adpbcctr2;            /*address 0x1D2*/
  UINT8            Adpbc1irq;            /*address 0x1D3*/
  UINT8            Adpbc0status;         /*address 0x1D4*/
  UINT8            Adpbc1status;         /*address 0x1D5*/
  UINT8            Adpbc2status;         /*address 0x1D6*/
  UINT8            Adpbc2irq;            /*address 0x1D7*/
  UINT16           Isodctrl;             /*address 0x1D8*/
  UINT8            Reserved11[2];
  UINT16           Isoautodump;          /*address 0x1DC*/
  UINT8            Reserved12[2];
  UINT8            Ep0maxpack;           /*address 0x1E0*/
  UINT8            Reserved13;
  UINT16           Rxmaxpack[15];        /*address 0x1E2*/
  RX_SOFTIMER_REG  Rxsofttimer[16];      /*address 0x200 to  0x23F*/
  TX_SOFTIMER_REG  Txsofttimer[16];      /*address 0x240 to  0x27F*/
  UINT8            Reserved14[132];
  RX_STADDR_REG    RxStaddr[15];         /*address 0x304*/
  UINT8            Reserved15[4];
  TX_STADDR_REG    TxStaddr[15];         /*address 0x344*/
  UINT8            Reserved16[4];        /*address 0x380*/
  IRQ_MODE_REG     Irqmode[15];          /*address 0x384*/
  /*The Microprocessor control*/
  UINT8            Cpuctrl;              /*address 0x3C0*/
  UINT8            Reserved17[15];
  /*The debug counters and workarounds*/
  UINT8            DebugRxBcl;           /*address 0x3D0*/
  UINT8            DebugRxBch;           /*address 0x3D1*/
  UINT8            DebugRxStatus;        /*address 0x3D2*/
  UINT8            DebugIrq;             /*address 0x3D3*/
  UINT8            DebugTxBcl;           /*address 0x3D4*/
  UINT8            DebugTxBch;           /*address 0x3D5*/
  UINT8            DebugTxStatus;        /*address 0x3D6*/
  UINT8            DebugIen;             /*address 0x3D7*/
  UINT8            PhywaEn;              /*address 0x3D8*/
  /*endian*/
  UINT8            Wa1Cnt;               /*address 0x3D9*/
  UINT8            Reserved18[2];        /*address 0x3DA*/
  UINT8            EndianSfrCs;          /*address 0x3DC*/
  UINT8            Reserved19[2];        /*address 0x3DD*/
  UINT8            EndianSfrS;           /*address 0x3DF*/
  UINT8            Reserved20[2];        /*address 0x3E0*/
  UINT16           Txmaxpack[15];        /*address 0x3E2*/
} HW_REGS;

typedef struct _CUSTOM_REG {
  UINT32 SecureCtrl;                     /*address 0x80000*/
  UINT32 SecsidAtst;                     /*address 0x80004*/
  UINT32 NsaidSmmuid;                    /*address 0x80008*/
  UINT32 Ace;                            /*address 0x8000c*/
  UINT32 Wakeup;                         /*address 0x80010*/
  UINT32 Debug;                          /*address 0x80014*/
} CUSTOM_REG;

typedef struct _VHUB_REG {
  UINT32 GenCfg;                         /*address 0x00*/
  UINT32 GenSt;                          /*address 0x04*/
  UINT32 BcCfg;                          /*address 0x08*/
  UINT32 BcSt;                           /*address 0x0c*/
  UINT32 AdpCfg;                         /*address 0x10*/
  UINT32 AdpSt;                          /*address 0x14*/
  UINT32 DbgCfg;                         /*address 0x18*/
  UINT32 DbgSt;                          /*address 0x1c*/
  UINT32 UtmipCfg;                       /*address 0x20*/
  UINT32 UtmipSt;                        /*address 0x24*/
} VHUB_REG;

typedef struct _DMA_REG {
  UINT32 Conf;         /*address 0x400*/
  UINT32 Sts;          /*address 0x404*/
  UINT32 Reserved5[5];
  UINT32 EpSel;        /*address 0x41C*/
  UINT32 Traddr;       /*address 0x420*/
  UINT32 EpCfg;        /*address 0x424*/
  UINT32 EpCmd;        /*address 0x428*/
  UINT32 EpSts;        /*address 0x42c*/
  UINT32 EpStsSid;     /*address 0x430*/
  UINT32 EpStsEn;      /*address 0x434*/
  UINT32 Drbl;         /*address 0x438*/
  UINT32 EpIen;        /*address 0x43C*/
  UINT32 EpIsts;       /*address 0x440*/
} DMA_REG;

#endif
