#include <Library/ParameterTable.h>
#include <Library/PcdLib.h>
/**
  Get partmeter table base address.

  @param[in]  Id parameter tabe identifier

  @retval  pointer to parameter base address
**/
VOID *
GetParameterBase(
  IN UINT32 Id
  )
{
  PARAMETER_HEADER *Head;
  UINT32 Offset, Magic;

  switch (Id) {
  case PM_PLL:
    Offset = PM_PLL_OFFSET;
    Magic = PARAMETER_PLL_MAGIC;
    break;
  case PM_PCIE:
    Offset = PM_PCIE_OFFSET;
    Magic = PARAMETER_PCIE_MAGIC;
    break;
  case PM_COMMON:
    Offset = PM_COMMON_OFFSET;
    Magic = PARAMETER_COMMON_MAGIC;
    break;
  case PM_DDR:
    Offset = PM_DDR_OFFSET;
    Magic = PARAMETER_DDR_MAGIC;
    break;
  case PM_BOARD:
    Offset = PM_BOARD_OFFSET;
    Magic = PARAMETER_BOARD_MAGIC;
    break;
  default:
  DEBUG((DEBUG_ERROR, "Invalid Id\n"));
  while(1);
  }

  Head = (PARAMETER_HEADER *)(UINT64)(O_PARAMETER_BASE + Offset);
  if(Magic != Head->Magic) {
    return NULL;
  } else {
    return (VOID *)(Head);
  }
}

/**
  Get Parameter information.

  @param[in]  Id parameter tabe identifier
  @param[in,out]  Data  pointer to paramter information

  @retval  EFI_SUCCESS    get information suceess
  @retval  EFI_NOT_FOUND  get fail
**/
EFI_STATUS
GetParameterInfo (
  UINT32 Id,
  UINT8 *Data,
  UINT32 DataSize
  )
{
  PARAMETER_HEADER *Head;

  if((Data == NULL) || (DataSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Head = (PARAMETER_HEADER *)GetParameterBase(Id);
  DEBUG ((DEBUG_INFO, "Id : %x, Head : %x\n", Id, Head));
  if(Head == NULL) {
    return EFI_NOT_FOUND;
  }

  if(DataSize < Head->Size){
    DEBUG((DEBUG_ERROR, "DataSize is too small\n", DataSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem(Data, Head, Head->Size);

  return EFI_SUCCESS;
}

/**
  Get Secure Parameter information.

  @param[in]  Id parameter tabe identifier
  @param[in,out]  Data  pointer to paramter information

  @retval  EFI_SUCCESS            Get information suceess
  @retval  EFI_INVALID_PARAMETER  Get fail
**/
EFI_STATUS
GetParameterSecureInfo (
  UINT32 Id,
  UINT8  *Data,
  UINT32 Size
  )
{
  UINT64               SecureAddr;

  if (Id != PM_SECURE) {
    return EFI_INVALID_PARAMETER;
  }

  SecureAddr = O_PARAMETER_BASE + PM_SECURE_OFFSET;

  CopyMem (Data, (VOID *)SecureAddr, sizeof(SECURE_BASE_INFO));

  return EFI_SUCCESS;
}
