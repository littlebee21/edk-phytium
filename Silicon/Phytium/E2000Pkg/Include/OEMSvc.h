
#ifndef __OEMSVC_H_
#define __OEMSVC_H_

/* SMC function IDs for OEM Service queries */
#define PHYTIUM_OEM_SVC_PSSI_VERSION   0x8200ff03
#define PHYTIUM_OEM_SVC_PBF_VERSION    0x82000001
#define PHYTIUM_OEM_SVC_CPU_VERSION    0xc2000002
#define PHYTIUM_OEM_SVC_CPU_MAPS       0xc2000003
#define PHYTIUM_OEM_SVC_CPU_CONF       0xc2000004
#define PHYTIUM_OEM_SVC_MEM_REGIONS    0xc2000005
#define PHYTIUM_OEM_SVC_MCU_DIMMS      0xc2000006
#define PHYTIUM_OEM_SVC_PCI_CONTROLLER 0xc2000007
#define PHYTIUM_OEM_SVC_HOST_BRIDGE    0xc2000008
#define PHYTIUM_OEM_SVC_GET_FLASH_CMD  0xC200000C

#define PHYTIUM_IOBASE_MASK           0xfffffff
#define PHYTIUM_MEMIO32_MASK          0xffffffff
#define PHYTIUM_MEMIO64_MASK          0xffffffffff

#pragma pack(1)

typedef struct {
  UINT64     CpuMapCount;
  UINT64     CpuMap[1];
} PHYTIUM_CPU_MAP_INFOR;


typedef struct {
  UINT64     CpuFreq;             // Hz
  UINT64     CpuL3CacheSize;      // Byte
  UINT64     CpuL3CacheLineSize;  // Byte
} PHYTIUM_CPU_COURE_INFOR;

typedef struct {
  UINT64		CupVersion;				//cpu version
  PHYTIUM_CPU_COURE_INFOR	CpuCoreInfo;		//cpu core info
  PHYTIUM_CPU_MAP_INFOR		CpuMapInfo;			//cpu map info
}PHYTIUM_CPU_INFO;

typedef struct {
  UINT64     MemSize;    // MB
  UINT64     MemDramId;
  UINT64     MemModuleId;
  UINT64     MemSerial;
  UINT64     MemSlotNumber;
  UINT64     MemFeatures;
} MCU_DIMM;

#define	MCU_DIMM_MAXCOUNT	2

typedef struct {
  UINT64     MemFreq;    // MHz
  UINT64     MemDimmCount;
  MCU_DIMM   McuDimm[1];
} MCU_DIMMS;

typedef struct {
  UINT64     MemStart;
  UINT64     MemSize;
  UINT64     MemNodeId;
} MEMORY_BLOCK;

typedef struct {
  UINT64       MemBlockCount;
  MEMORY_BLOCK MemBlock[1];
} PHYTIUM_MEMORY_INFOR;

typedef struct {
  UINT8    PciLane;
  UINT8    PciSpeed;
  UINT8    Reserved[6];
} PCI_BLOCK;

typedef struct {
  UINT64       PciCount;
  PCI_BLOCK    PciBlock[1];
} PHYTIUM_PCI_CONTROLLER;

typedef struct {
  UINT8    BusStart;
  UINT8    BusEnd;
  UINT8    Reserved[6];
  UINT64   PciConfigBase;
  UINT64   IoBase;
  UINT64   IoSize;
  UINT64   Mem32Base;
  UINT64   Mem32Size;
  UINT64   Mem64Base;
  UINT64   Mem64Size;
  UINT16   IntA;
  UINT16   IntB;
  UINT16   IntC;
  UINT16   IntD;
} PCI_HOST_BLOCK;

typedef struct {
  UINT64          PciHostCount;
  PCI_HOST_BLOCK  PciHostBlock[1];
} PHYTIUM_PCI_HOST_BRIDGE;

#pragma pack ()


#endif /* __OEMSVC_H_ */
