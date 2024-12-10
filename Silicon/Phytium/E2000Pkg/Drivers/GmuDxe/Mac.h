/** @file
** Mac header file.
** Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef __GMU_MAC_H__
#define __GMU_MAC_H__

#include "Type.h"

#include <Library/IoLib.h>

#include <Protocol/SimpleNetwork.h>
//
//MAC register offsets
//
#define MAC_NCR        0x0000
#define MAC_NCFGR      0x0004
#define MAC_NSR        0x0008
#define GEM_UR          0x000c
#define MAC_TSR        0x0014
#define MAC_RBQP       0x0018
#define MAC_TBQP       0x001c
#define MAC_RSR        0x0020
#define MAC_ISR        0x0024
#define MAC_IER        0x0028
#define MAC_IDR        0x002c
#define MAC_IMR        0x0030
#define MAC_MAN        0x0034
#define MAC_PTR        0x0038
#define MAC_PFR        0x003c
#define MAC_FTO        0x0040
#define MAC_SCF        0x0044
#define MAC_MCF        0x0048
#define MAC_FRO        0x004c
#define MAC_FCSE       0x0050
#define MAC_ALE        0x0054
#define MAC_DTF        0x0058
#define MAC_LCOL       0x005c
#define MAC_EXCOL      0x0060
#define MAC_TUND       0x0064
#define MAC_CSE        0x0068
#define MAC_RRE        0x006c
#define MAC_ROVR       0x0070
#define MAC_RSE        0x0074
#define MAC_ELE        0x0078
#define MAC_RJA        0x007c
#define MAC_USF        0x0080
#define MAC_STE        0x0084
#define MAC_RLE        0x0088
#define MAC_TPF        0x008c
#define MAC_HRB        0x0090
#define MAC_HRT        0x0094
#define MAC_SA1B       0x0098
#define MAC_SA1T       0x009c
#define MAC_SA2B       0x00a0
#define MAC_SA2T       0x00a4
#define MAC_SA3B       0x00a8
#define MAC_SA3T       0x00ac
#define MAC_SA4B       0x00b0
#define MAC_SA4T       0x00b4
#define MAC_TID        0x00b8
#define MAC_TPQ        0x00bc
#define MAC_USRIO      0x00c0
#define MAC_WOL        0x00c4
#define MAC_MID        0x00fc

//
//GEM register offsets.
//
#define GMU_NCFGR         0x0004 /* Network Config */
#define GMU_USRIO         0x000c /* User IO */
#define GMU_DMACFG        0x0010 /* DMA Configuration */
#define GMU_JML           0x0048 /* Jumbo Max Length */
#define GMU_HSMAC         0x0050 /* Hs mac config register*/
#define GMU_HRB           0x0080 /* Hash Bottom */
#define GMU_HRT           0x0084 /* Hash Top */
#define GMU_SA1B          0x0088 /* Specific1 Bottom */
#define GMU_SA1T          0x008C /* Specific1 Top */
#define GMU_SA2B          0x0090 /* Specific2 Bottom */
#define GMU_SA2T          0x0094 /* Specific2 Top */
#define GMU_SA3B          0x0098 /* Specific3 Bottom */
#define GMU_SA3T          0x009C /* Specific3 Top */
#define GMU_SA4B          0x00A0 /* Specific4 Bottom */
#define GMU_SA4T          0x00A4 /* Specific4 Top */
#define GMU_EFTSH         0x00e8 /* PTP Event Frame Transmitted Seconds Register 47:32 */
#define GMU_EFRSH         0x00ec /* PTP Event Frame Received Seconds Register 47:32 */
#define GMU_PEFTSH        0x00f0 /* PTP Peer Event Frame Transmitted Seconds Register 47:32 */
#define GMU_PEFRSH        0x00f4 /* PTP Peer Event Frame Received Seconds Register 47:32 */
#define GMU_OTX           0x0100 /* Octets transmitted */
#define GMU_OCTTXL        0x0100 /* Octets transmitted [31:0] */
#define GMU_OCTTXH        0x0104 /* Octets transmitted [47:32] */
#define GMU_TXCNT         0x0108 /* Frames Transmitted counter */
#define GMU_TXBCCNT       0x010c /* Broadcast Frames counter */
#define GMU_TXMCCNT       0x0110 /* Multicast Frames counter */
#define GMU_TXPAUSECNT    0x0114 /* Pause Frames Transmitted Counter */
#define GMU_TX64CNT       0x0118 /* 64 byte Frames TX counter */
#define GMU_TX65CNT       0x011c /* 65-127 byte Frames TX counter */
#define GMU_TX128CNT      0x0120 /* 128-255 byte Frames TX counter */
#define GMU_TX256CNT      0x0124 /* 256-511 byte Frames TX counter */
#define GMU_TX512CNT      0x0128 /* 512-1023 byte Frames TX counter */
#define GMU_TX1024CNT     0x012c /* 1024-1518 byte Frames TX counter */
#define GMU_TX1519CNT     0x0130 /* 1519+ byte Frames TX counter */
#define GMU_TXURUNCNT     0x0134 /* TX under run error counter */
#define GMU_SNGLCOLLCNT   0x0138 /* Single Collision Frame Counter */
#define GMU_MULTICOLLCNT  0x013c /* Multiple Collision Frame Counter */
#define GMU_EXCESSCOLLCNT 0x0140 /* Excessive Collision Frame Counter */
#define GMU_LATECOLLCNT   0x0144 /* Late Collision Frame Counter */
#define GMU_TXDEFERCNT    0x0148 /* Deferred Transmission Frame Counter */
#define GMU_TXCSENSECNT   0x014c /* Carrier Sense Error Counter */
#define GMU_ORX           0x0150 /* Octets received */
#define GMU_OCTRXL        0x0150 /* Octets received [31:0] */
#define GMU_OCTRXH        0x0154 /* Octets received [47:32] */
#define GMU_RXCNT         0x0158 /* Frames Received Counter */
#define GMU_RXBROADCNT    0x015c /* Broadcast Frames Received Counter */
#define GMU_RXMULTICNT    0x0160 /* Multicast Frames Received Counter */
#define GMU_RXPAUSECNT    0x0164 /* Pause Frames Received Counter */
#define GMU_RX64CNT       0x0168 /* 64 byte Frames RX Counter */
#define GMU_RX65CNT       0x016c /* 65-127 byte Frames RX Counter */
#define GMU_RX128CNT      0x0170 /* 128-255 byte Frames RX Counter */
#define GMU_RX256CNT      0x0174 /* 256-511 byte Frames RX Counter */
#define GMU_RX512CNT      0x0178 /* 512-1023 byte Frames RX Counter */
#define GMU_RX1024CNT     0x017c /* 1024-1518 byte Frames RX Counter */
#define GMU_RX1519CNT     0x0180 /* 1519+ byte Frames RX Counter */
#define GMU_RXUNDRCNT     0x0184 /* Undersize Frames Received Counter */
#define GMU_RXOVRCNT      0x0188 /* Oversize Frames Received Counter */
#define GMU_RXJABCNT      0x018c /* Jabbers Received Counter */
#define GMU_RXFCSCNT      0x0190 /* Frame Check Sequence Error Counter */
#define GMU_RXLENGTHCNT   0x0194 /* Length Field Error Counter */
#define GMU_RXSYMBCNT     0x0198 /* Symbol Error Counter */
#define GMU_RXALIGNCNT    0x019c /* Alignment Error Counter */
#define GMU_RXRESERRCNT   0x01a0 /* Receive Resource Error Counter */
#define GMU_RXORCNT       0x01a4 /* Receive Overrun Counter */
#define GMU_RXIPCCNT      0x01a8 /* IP header Checksum Error Counter */
#define GMU_RXTCPCCNT     0x01ac /* TCP Checksum Error Counter */
#define GMU_RXUDPCCNT     0x01b0 /* UDP Checksum Error Counter */
#define GMU_TISUBN        0x01bc /* 1588 Timer Increment Sub-ns */
#define GMU_TSH           0x01c0 /* 1588 Timer Seconds High */
#define GMU_TSL           0x01d0 /* 1588 Timer Seconds Low */
#define GMU_TN            0x01d4 /* 1588 Timer Nanoseconds */
#define GMU_TA            0x01d8 /* 1588 Timer Adjust */
#define GMU_TI            0x01dc /* 1588 Timer Increment */
#define GMU_EFTSL         0x01e0 /* PTP Event Frame Tx Seconds Low */
#define GMU_EFTN          0x01e4 /* PTP Event Frame Tx Nanoseconds */
#define GMU_EFRSL         0x01e8 /* PTP Event Frame Rx Seconds Low */
#define GMU_EFRN          0x01ec /* PTP Event Frame Rx Nanoseconds */
#define GMU_PEFTSL        0x01f0 /* PTP Peer Event Frame Tx Secs Low */
#define GMU_PEFTN         0x01f4 /* PTP Peer Event Frame Tx Ns */
#define GMU_PEFRSL        0x01f8 /* PTP Peer Event Frame Rx Sec Low */
#define GMU_PEFRN         0x01fc /* PTP Peer Event Frame Rx Ns */
#define GMU_PCSCTRL       0x0200 /* PCS control register*/
#define GMU_PCSSTATUS     0x0204 /* PCS Status*/
#define GMU_PCSANLPBASE   0x0214 /* PCS an lp base */
#define GMU_PFCSTATUS     0x026c /* PFC status*/
#define GMU_DCFG1         0x0280 /* Design Config 1 */
#define GMU_DCFG2         0x0284 /* Design Config 2 */
#define GMU_DCFG3         0x0288 /* Design Config 3 */
#define GMU_DCFG4         0x028c /* Design Config 4 */
#define GMU_DCFG5         0x0290 /* Design Config 5 */
#define GMU_DCFG6         0x0294 /* Design Config 6 */
#define GMU_DCFG7         0x0298 /* Design Config 7 */
#define GMU_DCFG8         0x029C /* Design Config 8 */
#define GMU_DCFG10        0x02A4 /* Design Config 10 */

#define MAC_MAX_QUEUES       8

//
//GMU PCS status register bitfields
//
#define GMU_LINKSTATUS_OFFSET    2
#define GMU_LINKSTATUS_SIZE      1

//
//GMU hs mac config register bitfields
//
#define GMU_HSMACSPEED_OFFSET    0
#define GMU_HSMACSPEED_SIZE      3
//
//GMU pcs_an_lp_base register bitfields
//
#define GMU_SGMIISPEED_OFFSET    10
#define GMU_SGMIISPEED_SIZE      2
#define GMU_SGMIIDUPLEX_OFFSET    12
#define GMU_SGMIIDUPLEX_SIZE      1

//
//GMU specific multi queues register offset
//hw_q can be 0~7
//
#define GMU_TBQP(HwQ)       (0x0440 + ((HwQ) << 2))

//
//phytium custom
//
#define GMU_USX_CONTROL         0x0A80  /* High speed PCS control register */
#define GMU_USX_STATUS          0x0A88 /* High speed PCS status register */
#define GMU_USX_FECERRCNT       0x0AD0 /* usx fec error counter */

#define GMU_SRC_SEL_LN            0x1C04
#define GMU_DIV_SEL0_LN           0x1C08
#define GMU_DIV_SEL1_LN           0x1C0C
#define GMU_PMA_XCVR_POWER_STATE  0x1C10
#define GMU_SPEED_MODE            0x1C14
#define GMU_MII_SELECT            0x1C18
#define GMU_SEL_MII_ON_RGMII      0x1C1C
#define GMU_TX_CLK_SEL0           0x1C20
#define GMU_TX_CLK_SEL1           0x1C24
#define GMU_TX_CLK_SEL2           0x1C28
#define GMU_TX_CLK_SEL3           0x1C2C
#define GMU_RX_CLK_SEL0           0x1C30
#define GMU_RX_CLK_SEL1           0x1C34
#define GMU_CLK_250M_DIV10_DIV100_SEL 0x1C38
#define GMU_TX_CLK_SEL5           0x1C3C
#define GMU_TX_CLK_SEL6           0x1C40
#define GMU_RX_CLK_SEL4           0x1C44
#define GMU_RX_CLK_SEL5           0x1C48
#define GMU_TX_CLK_SEL3_0         0x1C70
#define GMU_TX_CLK_SEL4_0         0x1C74
#define GMU_RX_CLK_SEL3_0         0x1C78
#define GMU_RX_CLK_SEL4_0         0x1C7C
#define GMU_RGMII_TX_CLK_SEL0     0x1C80
#define GMU_RGMII_TX_CLK_SEL1     0x1C84

#define GMU_PHY_INT_ENABLE        0x1C88
#define GMU_PHY_INT_CLEAR         0x1C8C
#define GMU_PHY_INT_STATE         0x1C90

//
//Bitfields in NCR
//
#define MAC_LB_OFFSET        0
#define MAC_LB_SIZE          1
#define MAC_LLB_OFFSET       1
#define MAC_LLB_SIZE         1
#define MAC_RE_OFFSET        2
#define MAC_RE_SIZE          1
#define MAC_TE_OFFSET        3
#define MAC_TE_SIZE          1
#define MAC_MPE_OFFSET       4
#define MAC_MPE_SIZE         1
#define MAC_CLRSTAT_OFFSET   5
#define MAC_CLRSTAT_SIZE     1
#define MAC_INCSTAT_OFFSET   6
#define MAC_INCSTAT_SIZE     1
#define MAC_WESTAT_OFFSET    7
#define MAC_WESTAT_SIZE      1
#define MAC_BP_OFFSET        8
#define MAC_BP_SIZE          1
#define MAC_TSTART_OFFSET    9
#define MAC_TSTART_SIZE      1
#define MAC_THALT_OFFSET     10
#define MAC_THALT_SIZE       1
#define MAC_NCR_TPF_OFFSET   11
#define MAC_NCR_TPF_SIZE     1
#define MAC_TZQ_OFFSET       12
#define MAC_TZQ_SIZE         1
#define MAC_SRTSM_OFFSET     15
#define MAC_OSSMODE_OFFSET   24 /* Enable One Step Synchro Mode */
#define MAC_OSSMODE_SIZE     1
#define MAC_PFC_OFFSET       25 /* Enable PFC */
#define MAC_PFC_SIZE         1
#define MAC_RGMII_OFFSET     28
#define MAC_RGMII_SIZE       1
#define MAC_2PT5G_OFFSET     29
#define MAC_2PT5G_SIZE       1
#define MAC_HSMAC_OFFSET     31 /* Use high speed MAC */
#define MAC_HSMAC_SIZE       1

//
//Bitfields in NCFGR
//
#define MAC_SPD_OFFSET       0
#define MAC_SPD_SIZE         1
#define MAC_FD_OFFSET        1
#define MAC_FD_SIZE          1
#define MAC_BIT_RATE_OFFSET  2
#define MAC_BIT_RATE_SIZE    1
#define MAC_JFRAME_OFFSET    3
#define MAC_JFRAME_SIZE      1
#define MAC_CAF_OFFSET       4
#define MAC_CAF_SIZE         1
#define MAC_NBC_OFFSET       5
#define MAC_NBC_SIZE         1
#define MAC_NCFGR_MTI_OFFSET 6
#define MAC_NCFGR_MTI_SIZE   1
#define MAC_UNI_OFFSET       7
#define MAC_UNI_SIZE         1
#define MAC_BIG_OFFSET       8
#define MAC_BIG_SIZE         1
#define MAC_EAE_OFFSET       9
#define MAC_EAE_SIZE         1
#define MAC_CLK_OFFSET       10
#define MAC_CLK_SIZE         2
#define MAC_RTY_OFFSET       12
#define MAC_RTY_SIZE         1
#define MAC_PAE_OFFSET       13
#define MAC_PAE_SIZE         1
#define MAC_RBOF_OFFSET      14
#define MAC_RBOF_SIZE        2
#define MAC_RLCE_OFFSET      16
#define MAC_RLCE_SIZE        1
#define MAC_DRFCS_OFFSET     17
#define MAC_DRFCS_SIZE       1
#define MAC_EFRHD_OFFSET     18
#define MAC_EFRHD_SIZE       1
#define MAC_IRXFCS_OFFSET    19
#define MAC_IRXFCS_SIZE      1

#define GMU_GBE_OFFSET        10
#define GMU_GBE_SIZE          1
#define GMU_CLK_OFFSET        18
#define GMU_CLK_SIZE          3
#define GMU_DBW_OFFSET        21
#define GMU_DBW_SIZE          2

//
//Bitfields in NSR
//
#define MAC_NSR_LINK_OFFSET  0
#define MAC_NSR_LINK_SIZE    1
#define MAC_MDIO_OFFSET      1
#define MAC_MDIO_SIZE        1
#define MAC_IDLE_OFFSET      2
#define MAC_IDLE_SIZE        1

//
//Bitfields in UR
//
#define GMU_RGMII_OFFSET      0
#define GMU_RGMII_SIZE        1

//
//Bitfields in TSR
//
#define MAC_UBR_OFFSET       0
#define MAC_UBR_SIZE         1
#define MAC_COL_OFFSET       1
#define MAC_COL_SIZE         1
#define MAC_TSR_RLE_OFFSET   2
#define MAC_TSR_RLE_SIZE     1
#define MAC_TGO_OFFSET       3
#define MAC_TGO_SIZE         1
#define MAC_BEX_OFFSET       4
#define MAC_BEX_SIZE         1
#define MAC_COMP_OFFSET      5
#define MAC_COMP_SIZE        1
#define MAC_UND_OFFSET       6
#define MAC_UND_SIZE         1

//
//Bitfields in RSR
//
#define MAC_BNA_OFFSET       0
#define MAC_BNA_SIZE         1
#define MAC_REC_OFFSET       1
#define MAC_REC_SIZE         1
#define MAC_OVR_OFFSET       2
#define MAC_OVR_SIZE         1

//
//Bitfields in ISR/IER/IDR/IMR
//
#define MAC_MFD_OFFSET       0
#define MAC_MFD_SIZE         1
#define MAC_RCOMP_OFFSET     1
#define MAC_RCOMP_SIZE       1
#define MAC_RXUBR_OFFSET     2
#define MAC_RXUBR_SIZE       1
#define MAC_TXUBR_OFFSET     3
#define MAC_TXUBR_SIZE       1
#define MAC_ISR_TUND_OFFSET  4
#define MAC_ISR_TUND_SIZE    1
#define MAC_ISR_RLE_OFFSET   5
#define MAC_ISR_RLE_SIZE     1
#define MAC_TXERR_OFFSET     6
#define MAC_TXERR_SIZE       1
#define MAC_TCOMP_OFFSET     7
#define MAC_TCOMP_SIZE       1
#define MAC_ISR_LINK_OFFSET  9
#define MAC_ISR_LINK_SIZE    1
#define MAC_ISR_ROVR_OFFSET  10
#define MAC_ISR_ROVR_SIZE    1
#define MAC_HRESP_OFFSET     11
#define MAC_HRESP_SIZE       1
#define MAC_PFR_OFFSET       12
#define MAC_PFR_SIZE         1
#define MAC_PTZ_OFFSET       13
#define MAC_PTZ_SIZE         1

//
//Bitfields in MAN
//
#define MAC_DATA_OFFSET      0
#define MAC_DATA_SIZE        16
#define MAC_CODE_OFFSET      16
#define MAC_CODE_SIZE        2
#define MAC_REGA_OFFSET      18
#define MAC_REGA_SIZE        5
#define MAC_PHYA_OFFSET      23
#define MAC_PHYA_SIZE        5
#define MAC_RW_OFFSET        28
#define MAC_RW_SIZE          2
#define MAC_SOF_OFFSET       30
#define MAC_SOF_SIZE         2

//
//Bitfields in USRIO
//
#define MAC_MII_OFFSET             0
#define MAC_MII_SIZE               1
#define MAC_EAM_OFFSET             1
#define MAC_EAM_SIZE               1
#define MAC_TX_PAUSE_OFFSET        2
#define MAC_TX_PAUSE_SIZE          1
#define MAC_TX_PAUSE_ZERO_OFFSET   3
#define MAC_TX_PAUSE_ZERO_SIZE     1

//
//Bitfields in USRIO (AT91)
//
#define MAC_RMII_OFFSET      0
#define MAC_RMII_SIZE        1
#define MAC_CLKEN_OFFSET     1
#define MAC_CLKEN_SIZE       1

//
//Bitfields in WOL
//
#define MAC_IP_OFFSET        0
#define MAC_IP_SIZE          16
#define MAC_MAG_OFFSET       16
#define MAC_MAG_SIZE         1
#define MAC_ARP_OFFSET       17
#define MAC_ARP_SIZE         1
#define MAC_SA1_OFFSET       18
#define MAC_SA1_SIZE         1
#define MAC_WOL_MTI_OFFSET   19
#define MAC_WOL_MTI_SIZE     1

//
//Bitfields in MID
//
#define MAC_IDNUM_OFFSET     16
#define MAC_IDNUM_SIZE       16

//
//Bitfields in DCFG1
//
#define GMU_DBWDEF_OFFSET     25
#define GMU_DBWDEF_SIZE       3

//
//constants for data bus width
//
#define GMU_DBW32       0
#define GMU_DBW64       1
#define GMU_DBW128      2

//
//Constants for CLK
//
#define MAC_CLK_DIV8       0
#define MAC_CLK_DIV16      1
#define MAC_CLK_DIV32      2
#define MAC_CLK_DIV64      3

//
//GMU specific constants for CLK
//
#define GMU_CLK_DIV8        0
#define GMU_CLK_DIV16       1
#define GMU_CLK_DIV32       2
#define GMU_CLK_DIV48       3
#define GMU_CLK_DIV64       4
#define GMU_CLK_DIV96       5
#define GMU_CLK_DIV128      6
#define GMU_CLK_DIV224      7
//
//Constants for MAN register
//
#define MAC_MAN_SOF        1
#define MAC_MAN_WRITE      1
#define MAC_MAN_READ       2
#define MAC_MAN_CODE       2

#define MAC_RX_BUFFER_SIZE     4096
#define GMU_RX_BUFFER_SIZE     4096
#define GMU_TX_BUFFER_SIZE     4096
#define MAC_RX_RING_SIZE       32
#define MAC_TX_RING_SIZE       16
#define MAC_TX_TIMEOUT         10000
#define MAC_AUTONEG_TIMEOUT    5000000
#define RX_BUFFER_MULTIPLE     64
//
//DMA descriptor bitfields
//
#define MAC_RX_USED_OFFSET     0
#define MAC_RX_USED_SIZE       1
#define MAC_RX_WRAP_OFFSET     1
#define MAC_RX_WRAP_SIZE       1
#define MAC_RX_WADDR_OFFSET    2
#define MAC_RX_WADDR_SIZE      30

#define MAC_RX_FRMLEN_OFFSET       0
#define MAC_RX_FRMLEN_SIZE         12
#define MAC_RX_OFFSET_OFFSET       12
#define MAC_RX_OFFSET_SIZE         2
#define MAC_RX_SOF_OFFSET          14
#define MAC_RX_SOF_SIZE            1
#define MAC_RX_EOF_OFFSET          15
#define MAC_RX_EOF_SIZE            1
#define MAC_RX_CFI_OFFSET          16
#define MAC_RX_CFI_SIZE            1
#define MAC_RX_VLAN_PRI_OFFSET     17
#define MAC_RX_VLAN_PRI_SIZE       3
#define MAC_RX_PRI_TAG_OFFSET      20
#define MAC_RX_PRI_TAG_SIZE        1
#define MAC_RX_VLAN_TAG_OFFSET     21
#define MAC_RX_VLAN_TAG_SIZE       1
#define MAC_RX_TYPEID_MATCH_OFFSET 22
#define MAC_RX_TYPEID_MATCH_SIZE   1
#define MAC_RX_SA4_MATCH_OFFSET    23
#define MAC_RX_SA4_MATCH_SIZE      1
#define MAC_RX_SA3_MATCH_OFFSET    24
#define MAC_RX_SA3_MATCH_SIZE      1
#define MAC_RX_SA2_MATCH_OFFSET    25
#define MAC_RX_SA2_MATCH_SIZE      1
#define MAC_RX_SA1_MATCH_OFFSET    26
#define MAC_RX_SA1_MATCH_SIZE      1
#define MAC_RX_EXT_MATCH_OFFSET    28
#define MAC_RX_EXT_MATCH_SIZE      1
#define MAC_RX_UHASH_MATCH_OFFSET  29
#define MAC_RX_UHASH_MATCH_SIZE    1
#define MAC_RX_MHASH_MATCH_OFFSET  30
#define MAC_RX_MHASH_MATCH_SIZE    1
#define MAC_RX_BROADCAST_OFFSET    31
#define MAC_RX_BROADCAST_SIZE      1

//
//RX checksum offload disabled: bit 24 clear in NCFGR
//
#define GMU_RX_TYPEID_MATCH_OFFSET    22
#define GMU_RX_TYPEID_MATCH_SIZE      2

//
//RX checksum offload enabled: bit 24 set in NCFGR
//
#define GMU_RX_CSUM_OFFSET      22
#define GMU_RX_CSUM_SIZE        2

#define MAC_TX_FRMLEN_OFFSET         0
#define MAC_TX_FRMLEN_SIZE           11
#define MAC_TX_LAST_OFFSET           15
#define MAC_TX_LAST_SIZE             1
#define MAC_TX_NOCRC_OFFSET          16
#define MAC_TX_NOCRC_SIZE            1
#define MAC_MSS_MFS_OFFSET           16
#define MAC_MSS_MFS_SIZE             14
#define MAC_TX_LSO_OFFSET            17
#define MAC_TX_LSO_SIZE              2
#define MAC_TX_TCP_SEQ_SRC_OFFSET    19
#define MAC_TX_TCP_SEQ_SRC_SIZE      1
#define MAC_TX_BUF_EXHAUSTED_OFFSET  27
#define MAC_TX_BUF_EXHAUSTED_SIZE    1
#define MAC_TX_UNDERRUN_OFFSET       28
#define MAC_TX_UNDERRUN_SIZE         1
#define MAC_TX_ERROR_OFFSET          29
#define MAC_TX_ERROR_SIZE            1
#define MAC_TX_WRAP_OFFSET           30
#define MAC_TX_WRAP_SIZE             1
#define MAC_TX_USED_OFFSET           31
#define MAC_TX_USED_SIZE             1

#define GMU_TX_FRMLEN_OFFSET      0
#define GMU_TX_FRMLEN_SIZE        14

//
//Buffer descriptor constants
//
#define GMU_RX_CSUM_NONE        0
#define GMU_RX_CSUM_IP_ONLY     1
#define GMU_RX_CSUM_IP_TCP      2
#define GMU_RX_CSUM_IP_UDP      3
#define MAC_RX_FRMLEN_MASK     0xFFF
#define MAC_RX_JFRMLEN_MASK    0x3FFF
//
//Bit manipulation macros
//
#define MAC_BIT(Name)          (1 << MAC_##Name##_OFFSET)
#define MAC_BF(Name, Value)        \
  (((Value) & ((1 << MAC_##Name##_SIZE) - 1)) << MAC_##Name##_OFFSET)
#define MAC_BFEXT(Name, Value)     \
  (((Value) >> MAC_##Name##_OFFSET) & ((1 << MAC_##Name##_SIZE) - 1))
#define MAC_BFINS(Name, Value, Old)      \
  (((Old) & ~(((1 << MAC_##Name##_SIZE) - 1) << MAC_##Name##_OFFSET)) | MAC_BF(Name, Value))

#define GMU_BIT(Name)         (1 << GMU_##Name##_OFFSET)
#define GMU_BF(Name, Value)       \
  (((Value) & ((1 << GMU_##Name##_SIZE) - 1)) << GMU_##Name##_OFFSET)
#define GMU_BFEXT(Name, Value)    \
  (((Value) >> GMU_##Name##_OFFSET) & ((1 << GMU_##Name##_SIZE) - 1))
#define GMU_BFINS(Name, Value, Old)     \
  (((Old) & ~(((1 << GMU_##Name##_SIZE) - 1) << GMU_##Name##_OFFSET)) | GMU_BF(Name, Value))

//
//Register access macros
//
#define MacRead(Port, Reg)      MmioRead32((Port)->Base + MAC_##Reg)
#define MacWrite(Port, Reg, Value)   MmioWrite32((Port)->Base + MAC_##Reg, (Value))
#define GmuRead(Port, Reg)       MmioRead32((Port)->Base + GMU_##Reg)
#define GmuWrite(Port, Reg, Value)    MmioWrite32((Port)->Base + GMU_##Reg, (Value))
#define GmuWriteQueueTBQP(Port, Value, QueueNum)  \
  MmioWrite32((Port)->Base + GMU_TBQP(QueueNum), (Value))

#define DMA_DESC_BYTES(n) (n * sizeof(MAC_DMA_DESC))
#define MAC_TX_DMA_DESC_SIZE (DMA_DESC_BYTES(MAC_TX_RING_SIZE))
#define MAC_RX_DMA_DESC_SIZE (DMA_DESC_BYTES(MAC_RX_RING_SIZE))
#define MAC_TX_DUMMY_DMA_DESC_SIZE (DMA_DESC_BYTES(1))
#define TX_TOTAL_BUFFERSIZE     (MAC_TX_RING_SIZE * GMU_TX_BUFFER_SIZE)
#define RX_TOTAL_BUFFERSIZE     (MAC_RX_RING_SIZE * GMU_RX_BUFFER_SIZE)

#define RXADDR_USED         0x00000001
#define RXADDR_WRAP         0x00000002

#define RXBUF_FRMLEN_MASK   0x00000fff
#define RXBUF_FRAME_START   0x00004000
#define RXBUF_FRAME_END     0x00008000
#define RXBUF_TYPEID_MATCH  0x00400000
#define RXBUF_ADDR4_MATCH   0x00800000
#define RXBUF_ADDR3_MATCH   0x01000000
#define RXBUF_ADDR2_MATCH   0x02000000
#define RXBUF_ADDR1_MATCH   0x04000000
#define RXBUF_BROADCAST     0x80000000

#define TXBUF_FRMLEN_MASK   0x000007ff
#define TXBUF_FRAME_END     0x00008000
#define TXBUF_NOCRC         0x00010000
#define TXBUF_EXHAUSTED     0x08000000
#define TXBUF_UNDERRUN      0x10000000
#define TXBUF_MAXRETRY      0x20000000
#define TXBUF_WRAP          0x40000000
#define TXBUF_USED          0x80000000

//
//Bitfields in DMACFG.
//
#define GMU_FBLDO_OFFSET      0 /* fixed burst length for DMA */
#define GMU_FBLDO_SIZE        5
#define GMU_ENDIA_DESC_OFFSET 6 /* endian swap mode for management descriptor access */
#define GMU_ENDIA_DESC_SIZE   1
#define GMU_ENDIA_PKT_OFFSET  7 /* endian swap mode for packet data access */
#define GMU_ENDIA_PKT_SIZE    1
#define GMU_RXBMS_OFFSET      8 /* RX packet buffer memory size select */
#define GMU_RXBMS_SIZE        2
#define GMU_TXPBMS_OFFSET     10 /* TX packet buffer memory size select */
#define GMU_TXPBMS_SIZE       1
#define GMU_TXCOEN_OFFSET     11 /* TX IP/TCP/UDP checksum gen offload */
#define GMU_TXCOEN_SIZE       1
#define GMU_RXBS_OFFSET       16 /* DMA receive buffer size */
#define GMU_RXBS_SIZE         8
#define GMU_DDRP_OFFSET       24 /* disc_when_no_ahb */
#define GMU_DDRP_SIZE         1
#define GMU_RXEXT_OFFSET      28 /* RX extended Buffer Descriptor mode */
#define GMU_RXEXT_SIZE        1
#define GMU_TXEXT_OFFSET      29 /* TX extended Buffer Descriptor mode */
#define GMU_TXEXT_SIZE        1
#define GMU_ADDR64_OFFSET     30 /* Address bus width - 64b or 32b */
#define GMU_ADDR64_SIZE       1

//
//GMU specific NCFGR bitfields.
//
#define GMU_GBE_OFFSET      10 /* Gigabit mode enable */
#define GMU_GBE_SIZE        1
#define GMU_PCSSEL_OFFSET   11
#define GMU_PCSSEL_SIZE     1
#define GMU_CLK_OFFSET      18 /* MDC clock division */
#define GMU_CLK_SIZE        3
#define GMU_DBW_OFFSET      21 /* Data bus width */
#define GMU_DBW_SIZE        2
#define GMU_RXCOEN_OFFSET   24
#define GMU_RXCOEN_SIZE     1
#define GMU_SGMIIEN_OFFSET  27
#define GMU_SGMIIEN_SIZE    1

//
//GMU pcs control register bitfields
//
#define GMU_AUTONEG_OFFSET    12
#define GMU_AUTONEG_SIZE      1

//
//pcs_an_lp_base register bitfields
//
#define GMU_SPEEDR_OFFSET     10
#define GMU_SPEEDR_SIZE       2
#define GMU_DUPLEX_OFFSET     12
#define GMU_DUPLEX_SIZE       1

//
//Bitfields in USX_CONTROL.
//
#define GMU_SIGNAL_OK_OFFSET            0
#define GMU_SIGNAL_OK_SIZE              1
#define GMU_TX_EN_OFFSET                1
#define GMU_TX_EN_SIZE                  1
#define GMU_RX_SYNC_RESET_OFFSET        2
#define GMU_RX_SYNC_RESET_SIZE          1
#define GMU_FEC_ENABLE_OFFSET           4
#define GMU_FEC_ENABLE_SIZE             1
#define GMU_FEC_ENA_ERR_IND_OFFSET      5
#define GMU_FEC_ENA_ERR_IND_SIZE        1
#define GMU_TX_SCR_BYPASS_OFFSET        8
#define GMU_TX_SCR_BYPASS_SIZE          1
#define GMU_RX_SCR_BYPASS_OFFSET        9
#define GMU_RX_SCR_BYPASS_SIZE          1
#define GMU_SERDES_RATE_OFFSET          12
#define GMU_SERDES_RATE_SIZE            2
#define GMU_USX_CTRL_SPEED_OFFSET       14
#define GMU_USX_CTRL_SPEED_SIZE         3

//
//USXGMII/SGMII/RGMII speed
//
#define GMU_SPEED_100       0
#define GMU_SPEED_1000      1
#define GMU_SPEED_2500      2
#define GMU_SPEED_5000      3
#define GMU_SPEED_10000     4
#define GMU_SPEED_25000     5

/**
  Write phy register through MDIO interface.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyAddr   Phy address.
  @param[in] Reg       Phy register.
  @param[in] Value     Value to write
**/
VOID
MacMdioWrite (
  IN MAC_DEVICE  *mac,
  IN UINT16      PhyAddr,
  IN UINT8       Reg,
  IN UINT16      Value
  );

/**
  Read phy register through MDIO interface.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] PhyAddr   Phy address.
  @param[in] Reg       Phy register.

  @retval    Register value.
**/
UINT16
MacMdioRead (
  IN MAC_DEVICE   *Mac,
  IN UINT16       PhyAddr,
  IN UINT8        Reg
  );

/**
  Mac initialization.
  1.Initialize receive and transmit descriptor.
  2.Initialize descriptor queue using dummy descriptor.
  3.Phy interface mode config.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
EFI_STATUS
MacInit (
  IN MAC_DEVICE *Mac
  );

/**
  Halt the mac controller.
  Disable tx and rx, clear statistics.

  @param[in] Mac       A point to MAC_DEVICE structure.

  @retval    EFI_SUCCESS  Success.
**/
VOID
MacHalt (
  IN MAC_DEVICE *Mac
  );

/**
  Set mac address.

  @param[in] Mac       A point to MAC_DEVICE structure.
  @param[in] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
MacWriteHwaddr (
  IN MAC_DEVICE       *Mac,
  IN EFI_MAC_ADDRESS  *MacAddr
  );

/**
  Get mac address.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[out] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
MacReadHwaddr (
  IN  MAC_DEVICE      *Mac,
  OUT EFI_MAC_ADDRESS  *MacAddr
  );

/**
  Enable mac tx and rx.

  @param[in] Mac      A point to MAC_DEVICE structure.

  @revtal    NULL.
 **/
VOID
MacStartTxRx (
  IN MAC_DEVICE  *Mac
  );

/**
  Configura mac according to the current phy interface type ,speed and duplex.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[in]  Speed     Phy speed, SPEED_10000, SPEED_1000, SPEED_100 or SPEED_10.
  @param[in]  Duplex    Duplex mode, full or half.

  @retval    NULL
**/
VOID
MacConfigAdjust (
  IN MAC_DEVICE  *Mac,
  IN UINT32      Speed,
  IN UINT32      Duplex
  );

/**
  Rec mac filter function config.

  @param[in]  Mac                   A pointer to MAC_DEVICE.
  @param[in]  ReceiveFilterSetting  Filter mask bits.
  @param[in]  Reset                 Set to TRUE to reset the contents of the multicast
                                    receive filters on the network interface to their
                                    default values.
  @param[in]  NumMfilter            Number of filter mac.
  @param[in]  Mfilter               Filter mac list.

  @retval     EFI_SUCCESS           Success.
**/
EFI_STATUS
EFIAPI
MacRxFilters (
  IN  MAC_DEVICE       *Mac,
  IN  UINT32           ReceiveFilterSetting,
  IN  BOOLEAN          Reset,
  IN  UINTN            NumMfilter             OPTIONAL,
  IN  EFI_MAC_ADDRESS  *Mfilter               OPTIONAL
  );

/**
  Get mac statistic.

  @param[in]  Mac     A point to MAC_DEVICE
  @param[out] Stats   A point to EFI_NETWORK_STATISTICS

  @retval    NULL
**/
VOID
MacGetStatistic (
  IN  MAC_DEVICE              *Mac,
  OUT EFI_NETWORK_STATISTICS  *Stats
  );

/**
  Configure phytium gmu module clock parameters according to interface speed.

  @param[in]  Mac       A point to MAC_DEVICE structure.
  @param[in]  Speed     Phy interface speed.10000, 2500, 1000, 100.

  @retval     EFI_SUCCESS    Success.
**/
EFI_STATUS
PhytiumGmuSelClk (
  IN MAC_DEVICE  *Mac,
  IN UINT32      Speed
  );

/**
  Print mac address.

  @param[in] MAcAddr   A point to MAC Address.

  @retval    NULL
**/
VOID
DisplayMacAddr (
  IN EFI_MAC_ADDRESS  *MacAddr
  );

VOID
ParseMacShareMem (
  IN  EFI_PHYSICAL_ADDRESS  Address,
  OUT MAC_DEVICE           *Mac
  );
#endif

