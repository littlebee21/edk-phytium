#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/HiiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SpiNorFlashProtocol.h>
#include <Guid/FileInfo.h>

extern void cpld_reboot(void);
extern void SystemReboot(void);
CHAR16 *BiosNames[2] = {L"BIOS.ROM" , L"BIOS.FD"};

#define CD_STANDARD_ID                      "CD001"
#define CD_EXPRESS_STANDARD_ID_SIZE         5
#define CD_EXPRESS_VOLUME_TYPE_OFFSET       0
#define CD_EXPRESS_STANDARD_ID_OFFSET       1
#define CD_EXPRESS_VOLUME_SPACE_OFFSET      80
#define CD_EXPRESS_ROOT_DIR_RECORD_OFFSET   156
#define CD_EXPRESS_VOLUME_TYPE_PRIMARY      1
#define CD_EXPRESS_VOLUME_TYPE_TERMINATOR   255
#define CD_EXPRESS_DIR_FILE_REC_FLAG_ISDIR  0x02
#define CD_BLOCK_SIZE        0x800
#define SPI_FLASH_BASE_ADDRESS    0x80000000000
#define PBF_HEADER_SIZE           0x200000 
#define PBF_HEADER1_SIZE          0x30000 
#define BIOS_OFFSET_ADDRESS       PBF_HEADER_SIZE + PBF_HEADER1_SIZE
#define BIOS_START_ADDRESS        SPI_FLASH_BASE_ADDRESS + BIOS_OFFSET_ADDRESS

#define EFI_BIOS_HEAD_SIGNATURE SIGNATURE_32 ('B', 'I', 'O', 'S')
#define EFI_BIOS_VENDOR         SIGNATURE_32 ('P', 'H', 'Y', 'T')

typedef struct {
  UINT8     Year;
  UINT8     Month;
  UINT8     Day;
  UINT8     Hour;
  UINT8     Minute;
  UINT8     Second;
} BIOS_RELEASE_TIME;

typedef struct {
  UINT32               Signature;
  UINT32               BiosVendor;
  UINT16               BiosMajorVersion;
  UINT16               BiosMinorVersion;
  BIOS_RELEASE_TIME    Time;
  UINT8                BiosCheckSum[16];
} EFI_UPDATE_FIRMWARE_HEADER;

#pragma pack(1)
typedef struct {
  UINT8   Length;
  UINT8   ExtendedAttributeRecordLength;
  UINT32  LocationOfExtent[2];
  UINT32  DataLength[2];
  UINT8   DateTime[7];
  UINT8   Flag;
  UINT8   FileUnitSize;
  UINT8   InterleaveGapSize;
  UINT32  VolumeSequenceNumber;
  UINT8   FileIDLength;
  UINT8   FileID[1];
} CD_EXPRESS_DIR_FILE_RECORD;
#pragma pack()

EFI_HII_HANDLE gUpdateBiosHandle;
EFI_NORFLASH_DRV_PROTOCOL *mFlash = NULL;

EFI_STATUS
ShowCopyRightsAndWarning ()
{
  UINTN   Columns, Rows;
  UINTN   StringLen;
  UINTN   Index;
  CHAR16  *String[] = {
    L"************************************************************\n",
    L"*                  Phytium Flash Update                    *\n",
    L"*        Copyright(C) 2006-2017, Phytium Co.,Ltd.          *\n",
    L"*                  All rights reserved                     *\n",
    L"************************************************************\n"
  };

  // gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_BLACK | EFI_YELLOW);
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &Columns, &Rows);
  StringLen = StrLen(String[0]);
  Columns   = (Columns - StringLen) / 2;
  Rows      = 0;
  //
  //
  //
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);
  for (Index = 0; Index < 5; Index ++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows ++);
    Print(String[Index]);
  }
  Print(L"\n");

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
ReadFileInFs (
  EFI_FILE_PROTOCOL  *RootFile,
  CHAR16             *FilePathName,
  UINT8              **OutFileData,
  UINTN              *OutFileSize
  )
{
  EFI_STATUS          Status;
  EFI_FILE_PROTOCOL   *File     = NULL;
  EFI_FILE_INFO       *FileInfo = NULL;
  UINTN               FileInfoSize;
  UINT8               *FileData = NULL;
  UINTN               FileSize = 0;

  Status = RootFile->Open(
                       RootFile,
                       &File,
                       FilePathName,
                       EFI_FILE_MODE_READ,
                       0
                       );
  if(EFI_ERROR(Status)){
    goto ProcExit;
  }

  FileInfo = NULL;
  FileInfoSize = 0;
  Status = File->GetInfo (
                   File,
                   &gEfiFileInfoGuid,
                   &FileInfoSize,
                   FileInfo
                   );
  if(Status == EFI_BUFFER_TOO_SMALL){
    FileInfo = AllocatePool(FileInfoSize);
    if(FileInfo == NULL){
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Status = File->GetInfo (
                       File,
                       &gEfiFileInfoGuid,
                       &FileInfoSize,
                       FileInfo
                       );
    }
  }
  if(EFI_ERROR(Status)){
    goto ProcExit;
  }

	if(FileInfo->Attribute & EFI_FILE_DIRECTORY){
    Status = EFI_INVALID_PARAMETER;
    goto ProcExit;
	}

  FileSize = (UINTN)FileInfo->FileSize;
  FileData = AllocatePages(EFI_SIZE_TO_PAGES(FileSize));
  if(FileData == NULL){
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }

  Status = File->Read(File, &FileSize, FileData);
  if(EFI_ERROR(Status)){
    goto ProcExit;;
  }

  *OutFileData = FileData;
  *OutFileSize = FileSize;

ProcExit:
  if(EFI_ERROR(Status) && FileData != NULL){
    FreePages(FileData, EFI_SIZE_TO_PAGES(FileSize));
  }
  if(FileInfo != NULL){
    FreePool(FileInfo);
  }
  if(File != NULL){
    File->Close(File);
  }
  return Status;
}


EFI_STATUS
ReadBiosFdInFs (
  CHAR16  *BiosFileName,
  UINT8   **BiosData,
  UINTN   *BiosSize
  )
{
  EFI_STATUS                       Status;
  UINTN                            Index;
  UINTN                            HandleCount;
  EFI_HANDLE                       *Handles = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FS;
  EFI_FILE_PROTOCOL                *RootFile = NULL;

  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiSimpleFileSystemProtocolGuid,
                 NULL,
                 &HandleCount,
                 &Handles
                 );
  if(EFI_ERROR(Status) || HandleCount == 0){
    Status = EFI_NOT_FOUND;
    goto ProcExit;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID**)&FS
                    );
    ASSERT(!EFI_ERROR (Status));

    if(RootFile != NULL){
      RootFile->Close(RootFile);
      RootFile = NULL;
    }
  	Status = FS->OpenVolume(FS, &RootFile);
  	if(EFI_ERROR(Status)){
      DEBUG((EFI_D_ERROR, "OpenRootFile Error:%r\n", Status));
      continue;
    }

	  Status = ReadFileInFs(RootFile, BiosFileName, BiosData, BiosSize);
  	if(!EFI_ERROR(Status)){
      break;
    }
  }

  if(Index >= HandleCount){
    Status = EFI_NOT_FOUND;
    goto ProcExit;
  }

ProcExit:
  if(Handles != NULL){
    FreePool(Handles);
  }
  if(RootFile != NULL){
    RootFile->Close(RootFile);
  }
  return Status;
}


BOOLEAN
StringCmp (
  IN UINT8      *Source1,
  IN UINT8      *Source2,
  IN UINTN      Size,
  IN BOOLEAN    CaseSensitive
  )
{
  UINTN Index;
  UINT8 Dif;

  for (Index = 0; Index < Size; Index++) {
    if (Source1[Index] == Source2[Index]) {
      continue;
    }

    if (!CaseSensitive) {
      Dif = (UINT8) ((Source1[Index] > Source2[Index]) ? (Source1[Index] - Source2[Index]) : (Source2[Index] - Source1[Index]));
      if (Dif == ('a' - 'A') || Dif == ('_' - '-')) {
        continue;
      }
    }

    return FALSE;
  }

  return TRUE;
}


EFI_BLOCK_IO_PROTOCOL   *mCdromBlkIo = NULL;
UINT32                  mFolderLba   = 0;

EFI_STATUS
EFIAPI
RetrieveFileFromRoot (
  IN EFI_BLOCK_IO_PROTOCOL     *BlkIo,
  IN UINT32                    Lba,
  IN CHAR8                     *FileName,
  IN UINT8                     *Buffer,
  IN OUT UINT8                 **FileData,
  IN OUT UINTN                 *FileSize
  )
{
  EFI_STATUS                      Status;
  CD_EXPRESS_DIR_FILE_RECORD      *FileRecord;
  UINTN                           FileNameLen;
  UINT8                           *DataBuffer;


  DEBUG ((EFI_D_INFO, "%a(%X)\n", __FUNCTION__, Lba));

  Status = BlkIo->ReadBlocks (
                    BlkIo,
                    BlkIo->Media->MediaId,
                    Lba,
                    BlkIo->Media->BlockSize,
                    Buffer
                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ReadBlocks %X : %r\n", Lba, Status));
    return Status;
  }

  while (1) {
    FileRecord = (CD_EXPRESS_DIR_FILE_RECORD *) Buffer;
    if (FileRecord->Length == 0) {
      break;
    }
    //
    // Not intend to check other flag now
    //
    if ((FileRecord->Flag & CD_EXPRESS_DIR_FILE_REC_FLAG_ISDIR)) {
      Buffer += FileRecord->Length;
      continue;
    }

    FileNameLen = AsciiStrLen (FileName);
    if (!StringCmp (FileRecord->FileID, (UINT8 *) FileName, FileNameLen, FALSE)) {
      DEBUG ((EFI_D_ERROR, "Name not match\n"));
      Buffer += FileRecord->Length;
      continue;
    }

    DEBUG ((EFI_D_INFO, "+%X, L:%X\n", FileRecord->LocationOfExtent[0], FileRecord->DataLength[0]));

    DataBuffer = AllocatePool((UINTN)(FileRecord->DataLength[0] / CD_BLOCK_SIZE + 1) * CD_BLOCK_SIZE);
    if (NULL == Buffer) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      Status = BlkIo->ReadBlocks (
                        BlkIo,
                        BlkIo->Media->MediaId,
                        FileRecord->LocationOfExtent[0],
                        (UINTN)(FileRecord->DataLength[0] / CD_BLOCK_SIZE + 1) * CD_BLOCK_SIZE,
                        DataBuffer
                        );
      if(EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "ReadBlocks:%r\n", Status));
      }
      *FileData = DataBuffer;
      *FileSize = FileRecord->DataLength[0];
    }
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
FindFileLbaInCdrom (
  EFI_BLOCK_IO_PROTOCOL   *BlkIo,
  UINT8                   *Buffer,
  CHAR8                   *FileName,
  UINT8                   **FileData,
  UINTN                   *FileSize
  )
{
  EFI_STATUS                      Status;
  UINTN                           Lba;
  UINT8                           Type;
  UINT8                           *StandardID;
  UINT32                          RootDirLBA;
  CD_EXPRESS_DIR_FILE_RECORD      *RoorDirRecord;
  UINTN                           VolumeSpaceSize;
  BOOLEAN                         StartOfVolume;
  UINTN                           OriginalLBA;


  DEBUG ((EFI_D_INFO, "%a()\n", __FUNCTION__));

  Lba             = 16;
  VolumeSpaceSize = 0;
  StartOfVolume   = TRUE;
  OriginalLBA     = 16;
  while (TRUE) {
    SetMem (Buffer, BlkIo->Media->BlockSize, 0);
    Status = BlkIo->ReadBlocks (
                      BlkIo,
                      BlkIo->Media->MediaId,
                      Lba,
                      BlkIo->Media->BlockSize,
                      Buffer
                      );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "ReadBlocks %X : %r\n", Lba, Status));
      return Status;
    }
    StandardID = (UINT8 *) (Buffer + CD_EXPRESS_STANDARD_ID_OFFSET);
    if (!StringCmp (StandardID, (UINT8 *) CD_STANDARD_ID, CD_EXPRESS_STANDARD_ID_SIZE, TRUE)) {
      DEBUG ((EFI_D_ERROR, "Bad Std Id\n"));
      break;
    }

    if (StartOfVolume) {
      OriginalLBA   = Lba;
      StartOfVolume = FALSE;
    }

    Type = *(UINT8 *) (Buffer + CD_EXPRESS_VOLUME_TYPE_OFFSET);
    if (Type == CD_EXPRESS_VOLUME_TYPE_TERMINATOR) {
      if (VolumeSpaceSize == 0) {
        break;
      } else {
        Lba             = (OriginalLBA + VolumeSpaceSize);
        VolumeSpaceSize = 0;
        StartOfVolume   = TRUE;
        continue;
      }
    }

    if (Type != CD_EXPRESS_VOLUME_TYPE_PRIMARY) {
      Lba ++;
      continue;
    }

    VolumeSpaceSize = *(UINT32 *) (Buffer + CD_EXPRESS_VOLUME_SPACE_OFFSET);
    RoorDirRecord   = (CD_EXPRESS_DIR_FILE_RECORD *) (Buffer + CD_EXPRESS_ROOT_DIR_RECORD_OFFSET);
    RootDirLBA      = RoorDirRecord->LocationOfExtent[0];
    DEBUG ((EFI_D_INFO, "VolumeSpaceSize:%X, RootDirLBA:%X\n", VolumeSpaceSize, RootDirLBA));
    Status          = RetrieveFileFromRoot (BlkIo, RootDirLBA, FileName, Buffer, FileData, FileSize);
    if (!EFI_ERROR (Status)) {
      return Status;
    }
    Lba ++;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
SearchFileInCdrom (
  CHAR8  *FileName,
  UINT8  *Buffer,
  UINT8   **FileData,
  UINTN   *FileSize
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              *HandleBuffer;
  UINTN                   NumberOfHandles;
  UINTN                   Index;
  EFI_BLOCK_IO_PROTOCOL   *BlkIo;
  BOOLEAN                 Find = FALSE;


  DEBUG ((EFI_D_INFO, "%a(%a)\n", __FUNCTION__, FileName));

  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index = 0; Index < NumberOfHandles; Index ++) {
    DEBUG ((EFI_D_INFO, "BlockIo Index:%d\n", Index));
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **)&BlkIo
                    );
    if (!EFI_ERROR (Status)) {
      DEBUG((EFI_D_INFO, "%a() Line=%d\n", __FUNCTION__, __LINE__));
      if(BlkIo->Media->LogicalPartition){
        continue;
      }
      if (BlkIo->Media->BlockSize == CD_BLOCK_SIZE) {
        Status = FindFileLbaInCdrom (BlkIo, Buffer, FileName, FileData, FileSize);
        if (EFI_ERROR (Status)) {
          continue;
        }
        Find = TRUE;
        break;
      } else {
        Status = EFI_NOT_FOUND;
      }
    }
    DEBUG((EFI_D_INFO, "%a() Line=%d\n", __FUNCTION__, __LINE__));
    Status = EFI_NOT_FOUND;
  }
  FreePool (HandleBuffer);
  if(Find) {
    return EFI_SUCCESS;
  } else {
    return EFI_NOT_FOUND;
  }
}


EFI_STATUS
ReadFileFromCdrom (
  CHAR16  *Name,
  UINT8   **FileData,
  UINTN   *FileSize
  )
{
  EFI_STATUS      Status;
  UINT8           *Buffer = NULL;
  CHAR8           *FileName = NULL;
  UINT8           NameLen;

  DEBUG ((EFI_D_ERROR,"%a()\n",__FUNCTION__));
  NameLen = StrSize (Name);
  // Initialize ASCII variables
  NameLen = NameLen / 2;
  FileName = AllocatePool (NameLen);
  if (FileName == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProExit;
  }
  UnicodeStrToAsciiStrS (Name, FileName,NameLen);
  DEBUG ((EFI_D_ERROR,"line:%d,file:%a\n",__LINE__,FileName));
  Buffer = AllocatePool (CD_BLOCK_SIZE);
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ProExit;
  }

  Status = SearchFileInCdrom (FileName, Buffer, FileData, FileSize);
  if (EFI_ERROR(Status)) {
    goto ProExit;
  }

ProExit:
  if (Buffer != NULL) {
    FreePool (Buffer);
  }
  if (FileName != NULL) {
    FreePool (FileName);
  }

return Status;
}

BOOLEAN
InvalidInformation(
  UINT16  StringToken1,
  UINT16  StringToken2
  )
{
  CHAR16                            str1[64];
  CHAR16                            str2[64];
  CHAR16                           *HiiFormatString;
  EFI_INPUT_KEY                     InputKey;
  UINTN                             EventIndex;

  DEBUG((EFI_D_INFO, "%a()\n", __FUNCTION__));

  HiiFormatString = HiiGetString(gUpdateBiosHandle, StringToken1, NULL);
  UnicodeSPrint (str1, sizeof (str1), L"%s", HiiFormatString);
  if(HiiFormatString != NULL) {
    FreePool(HiiFormatString);
  }
  HiiFormatString = HiiGetString(gUpdateBiosHandle, StringToken2, NULL);
  UnicodeSPrint (str2, sizeof (str2), L"%s", HiiFormatString);
  if(HiiFormatString != NULL) {
    FreePool(HiiFormatString);
  }

  CreatePopUp (
    EFI_LIGHTGRAY | EFI_BACKGROUND_RED,
    NULL,
    str1,
    str2,
    NULL
    );
  while(1){
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if(InputKey.ScanCode == SCAN_NULL && InputKey.UnicodeChar == CHAR_CARRIAGE_RETURN){
      break;
    }
  }

  return TRUE;
}


EFI_STATUS
UpdateSpi (
  IN UINTN                     Address,
  IN UINT8                     *Buffer,
  IN UINTN                     Size,
  IN CHAR16                    *String
)
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       Count;
  VOID        *TempBuffer = NULL;

  TempBuffer = AllocatePool(SIZE_64KB);
  Status = EFI_SUCCESS;
  Count  = (Size / SIZE_64KB);
  // make sure Address & Size is 64k align
  if ((Address % SIZE_64KB) || (Size % SIZE_64KB)) {
    Count ++;
    Address = Address & (~0xFFFF);
    Print(L"0x%x",Address);
  }

  Print(L"%s%02d%%%",String, 1);
  for (Index = 0; Index < Count; Index ++) {
    if (TempBuffer != NULL) {
      mFlash->Read (
                Address + Index * SIZE_64KB,
                TempBuffer,
                SIZE_64KB
                );
      if (CompareMem(TempBuffer, Buffer + Index * SIZE_64KB, SIZE_64KB) == 0) {
        Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
        continue;
      }
    }
    Status = mFlash->EraseWrite (
                       Address + Index * SIZE_64KB,
                       Buffer + Index * SIZE_64KB,
                       SIZE_64KB
                       );
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
  }
  Print(L"\r%s Success!\n",String);

ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }
  return Status;
}
#if 0
EFI_STATUS
CheckBiosImage (
  UINT8        *Buffer,
  UINTN        FileSize
)
{
  EFI_STATUS               Status;
  UINT64                   SignOffset;
  UINT32                   Signature;

  // check FvMain.Hdr.Signature.
  // FV header Signature offset is 40
  SignOffset = PcdGet64 (PcdFvBaseAddress) - PcdGet64 (PcdFdBaseAddress) + 40;
  Signature  = *(UINT32*)(Buffer + BIOS_OFFSET_ADDRESS + SignOffset);
  if (Signature != FVH_SIGNATURE) {
    Status = EFI_INVALID_PARAMETER;
    goto ProExit;
  }
  Status = EFI_SUCCESS;

ProExit:

  return Status;
}
#endif

EFI_STATUS
CheckBiosImage (
  UINT8        *Buffer,
  UINTN        FileSize
)
{
  EFI_STATUS               Status;
  UINT64                   SignOffset;
  UINT32                   Signature;
  UINTN                    Md5ContextSize;
  VOID                    *Md5Ctx;
  UINT8                    HashValue[16];
  UINT8                    MD5Value[16];
  // check BIOS.Signature.
  // BIOS header Signature offset is 0
  Status = EFI_SUCCESS;
  SignOffset = 0;
  Signature  = *(UINT32*)(Buffer + SignOffset);
  SignOffset = 0x12;
  CopyMem(MD5Value, (Buffer + SignOffset), 16);
  //DEBUG((EFI_D_INFO,"the MD5Value0 1  is %x%x",MD5Value[0],MD5Value[1]));
  if (Signature != EFI_BIOS_HEAD_SIGNATURE) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_ERROR , "Bios Signature isn't right ,please check the bios if corret or not !\n"));
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_SIGNATURE_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

  Md5ContextSize = Md5GetContextSize();
  Md5Ctx = AllocatePool (Md5ContextSize);
  if (Md5Ctx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  if (!Md5Init(Md5Ctx)) {
    goto ProExit;
  }
  if (!Md5Update(Md5Ctx,(Buffer+0x30), FileSize - 0x30)) {
    goto ProExit;
  }
  if (Md5Final(Md5Ctx, HashValue)) {
    Status = EFI_SUCCESS;
  }
  //DEBUG((EFI_D_INFO,"the HashValue 1  is %x%x",HashValue[0],HashValue[1]));

  if (CompareMem(HashValue, MD5Value, 16) == 0){
    //DEBUG((EFI_D_INFO,"the HashValue is identical\n"));
    Status = EFI_SUCCESS;
  }else{
    DEBUG((EFI_D_ERROR,"the HashValue is not identical\n"));
    Status = EFI_INVALID_PARAMETER;
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_MD5_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

ProExit:

  return Status;
}

EFI_STATUS
CompareBios (
  UINT8  *BiosData
  )
{
  EFI_STATUS               Status = EFI_SUCCESS;
  UINT64                   Offset;
  UINT8                    *Buffer;
  UINT8                    *FileBuf;

  Buffer = AllocatePool(PcdGet32 (PcdFvSize));
  if(Buffer == NULL){
    Status = EFI_OUT_OF_RESOURCES;
    goto ProExit;
  }
  // compare fvmain
  Offset = PcdGet64 (PcdFvBaseAddress) - PcdGet64 (PcdFdBaseAddress);
  CopyMem (Buffer, (void*)(BIOS_START_ADDRESS + Offset), PcdGet32 (PcdFvSize));
  FileBuf = (UINT8*)(BiosData + BIOS_OFFSET_ADDRESS + Offset);
  if (CompareMem(FileBuf, Buffer, PcdGet32 (PcdFvSize))) {
    Status = EFI_CRC_ERROR;
  }

ProExit:

  return Status;
}


EFI_STATUS
EFIAPI
UpdateBiosAppEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
  {
  UINT8                   *BiosData = NULL;
  UINTN                    BiosSize = 0;
  EFI_STATUS               Status;
  UINTN                    Attribute;

  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid, NULL, (VOID*) &mFlash);
  if (EFI_ERROR(Status)){
    Print(L"  Locate SpiFlashProtocol failed!\n");
    goto ProcExit;
  }

  Attribute = EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK;
  gST->ConOut->SetAttribute(gST->ConOut, Attribute);

  gUpdateBiosHandle = HiiAddPackages (
                      &gEfiCallerIdGuid,
                      gImageHandle,
                      UpdateBiosAppStrings,
                      NULL
                      );
  ASSERT (gUpdateBiosHandle != NULL);

  DEBUG((EFI_D_INFO, "UpdateBiosAppEntry\n"));
  Status = ReadBiosFdInFs(BiosNames[1], &BiosData, &BiosSize);
  if (EFI_ERROR(Status)){
    Status = ReadFileFromCdrom (BiosNames[1], &BiosData, &BiosSize);
    if (EFI_ERROR(Status)) {
      InvalidInformation (STRING_TOKEN(STR_UPDATE_BIOS_ERROR_INFO), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
      goto ProcExit;
    }
  }
  ShowCopyRightsAndWarning ();
  Print(L"  Reading file.................%r\n", Status);
  Print(L"  BiosSize:%x\n", BiosSize);
  DEBUG((EFI_D_INFO, "InputFD(%X,%X)\n", BiosData, BiosSize));
  
  Status = UpdateSpi (
               0,
	       BiosData,
	       BiosSize,
	       L"  Updating backup bios space."
	       );
  DEBUG((EFI_D_ERROR, "Backup Status:%r\n",Status));

ProcExit:
  if (BiosData != NULL){
    FreePages(BiosData, EFI_SIZE_TO_PAGES(BiosSize));
  }
  Attribute = EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK;
  gST->ConOut->SetAttribute(gST->ConOut, Attribute);
  if (!EFI_ERROR (Status)) {
    Print(L"\nBIOS has been updated, system will reboot now!\n");
    // stall 2s
    gBS->Stall (2000 * 1000);
    SystemReboot();
    //gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  }
  HiiRemovePackages (gUpdateBiosHandle);
  return Status;
}


