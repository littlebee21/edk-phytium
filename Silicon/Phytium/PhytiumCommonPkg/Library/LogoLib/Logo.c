#include <PiDxe.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BmpSupportLib.h>
#include <IndustryStandard/Bmp.h>
#include <Protocol/BootLogo.h>

EFI_STATUS
EFIAPI
EnableQuietBoot (
  IN  EFI_GUID  *LogoFile
  )
{
  EFI_STATUS                    Status;
  UINT32                        SizeOfX;
  UINT32                        SizeOfY;
  UINTN                         ImageSize;
  UINTN                         BltSize;
  UINTN                         Height;
  UINTN                         Width;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt = NULL;
  UINT8                         *ImageData = NULL;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;


  DEBUG((EFI_D_INFO, "EnableQuietBoot()\n"));

  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
  DEBUG ((EFI_D_INFO, "Status : %r\n", Status));
  if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
  }

  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
  SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;
  DEBUG((EFI_D_INFO, "Gop:%lX %d x %d\n", (UINT64)(UINTN)GraphicsOutput, SizeOfX, SizeOfY));

  Status = GetSectionFromAnyFv (LogoFile, EFI_SECTION_RAW, 0, (VOID **) &ImageData, &ImageSize);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_INFO, "Logo File Not Found!\n"));
    Status = EFI_UNSUPPORTED;
    goto ProcExit;
  }
  Status = TranslateBmpToGopBlt (
             ImageData,
             ImageSize,
             &Blt,
             &BltSize,
             &Height,
             &Width
             );
  if (EFI_ERROR (Status)) {
      Status = EFI_UNSUPPORTED;
      goto ProcExit;

  }
  Status = GraphicsOutput->Blt (
                      GraphicsOutput,
                      Blt,
                      EfiBltBufferToVideo,
                      0,
                      0,
                      (UINTN) (SizeOfX - Width) / 2,
                      (UINTN) (SizeOfY - Height) / 2,
                      Width,
                      Height,
                      Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                      );
  DEBUG((EFI_D_INFO, "Draw %d x %d: %r\n", Width, Height, Status));

ProcExit:
  if(ImageData != NULL){
    FreePool(ImageData);
  }
  if(Blt!=NULL){
    FreePool(Blt);
  }
  return Status;
}



/**
  Use SystemTable Conout to turn on video based Simple Text Out consoles. The
  Simple Text Out screens will now be synced up with all non video output devices

  @retval EFI_SUCCESS     UGA devices are back in text mode and synced up.

**/
EFI_STATUS
EFIAPI
DisableQuietBoot (
  VOID
  )
{
  //
  // Enable Cursor on Screen
  //
  //gST->ConOut->EnableCursor (gST->ConOut, TRUE);
  return EFI_SUCCESS;
}
