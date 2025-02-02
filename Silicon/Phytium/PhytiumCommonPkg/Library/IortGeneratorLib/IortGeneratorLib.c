/** @file
Iort table common libary.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Acpi62.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <PhytiumAcpiHelp.h>
#include <AcpiPlatform.h>
#include "IortGeneratorLib.h"


UINT32                   gIortNodeCount;
IORT_NODE_INDEXER        *gNodeIndexer;

/** Returns the size of the ITS Group node.

    @param [in]  Node    Pointer to ITS Group node.

    @retval Size of the ITS Group Node.
**/
STATIC
UINT32
GetItsGroupNodeSize (
  IN  CONST CM_ARM_ITS_GROUP_NODE * Node
  )
{
  ASSERT (Node != NULL);

  /* Size of ITS Group Node +
     Size of ITS Identifier array
  */
  return sizeof (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE) +
           (Node->ItsIdCount * sizeof (UINT32));
}


/** Returns the total size required for the ITS Group nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to ITS Group node list.
    @param [in]       NodeCount       Count of the ITS Group nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the ITS Group Nodes.
**/
STATIC
UINT32
GetSizeofItsGroupNodes (
  IN      CONST UINT32                         NodeStartOffset,
  IN      CONST CM_ARM_ITS_GROUP_NODE  *       NodeList,
  IN            UINT32                         NodeCount,
  IN OUT        IORT_NODE_INDEXER     ** CONST NodeIndexer
  )
{
  UINT32  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token = NodeList->Token;
    (*NodeIndexer)->Object = (VOID*)NodeList;
    (*NodeIndexer)->Offset = Size + NodeStartOffset;
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetItsGroupNodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }
  return Size;
}

/** Returns the size of the Named Component node.

    @param [in]  Node    Pointer to Named Component node.

    @retval Size of the Named Component node.
**/
STATIC
UINT32
GetNamedComponentNodeSize (
  IN  CONST CM_ARM_NAMED_COMPONENT_NODE  *Node
  )
{
  ASSERT (Node != NULL);

  /* Size of Named Component node +
     Size of ID mapping array +
     Size of ASCII string + 'padding to 32-bit word aligned'.
  */
  return (UINT32)(sizeof (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE) +
                  (Node->IdMappingCount *
                   sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE)) +
                  ALIGN_VALUE (AsciiStrSize (Node->ObjectName), 4));
}

/** Returns the total size required for the Named Component nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to Named Component node list.
    @param [in]       NodeCount       Count of the Named Component nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the Named Component nodes.
**/
STATIC
UINT64
GetSizeofNamedComponentNodes (
  IN      CONST UINT32                              NodeStartOffset,
  IN      CONST CM_ARM_NAMED_COMPONENT_NODE         *NodeList,
  IN            UINT32                              NodeCount,
  IN OUT        IORT_NODE_INDEXER          **CONST  NodeIndexer
  )
{
  UINT64  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token  = NodeList->Token;
    (*NodeIndexer)->Object = (VOID *)NodeList;
    (*NodeIndexer)->Offset = (UINT32)(Size + NodeStartOffset);
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetNamedComponentNodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }

  return Size;
}


/** Returns the size of the Root Complex node.

    @param [in]  Node    Pointer to Root Complex node.

    @retval Size of the Root Complex node.
**/
STATIC
UINT32
GetRootComplexNodeSize (
  IN  CONST CM_ARM_ROOT_COMPLEX_NODE  * Node
  )
{
  ASSERT (Node != NULL);

  /* Size of Root Complex node +
     Size of ID mapping array
  */
  return sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE) +
           (Node->IdMappingCount *
            sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE));
}

/** Returns the total size required for the Root Complex nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to Root Complex node list.
    @param [in]       NodeCount       Count of the Root Complex nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the Root Complex nodes.
**/
STATIC
UINT32
GetSizeofRootComplexNodes (
  IN      CONST UINT32                              NodeStartOffset,
  IN      CONST CM_ARM_ROOT_COMPLEX_NODE    *       NodeList,
  IN            UINT32                              NodeCount,
  IN OUT        IORT_NODE_INDEXER          ** CONST NodeIndexer
  )
{
  UINT32  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token = NodeList->Token;
    (*NodeIndexer)->Object = (VOID*)NodeList;
    (*NodeIndexer)->Offset = Size + NodeStartOffset;
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetRootComplexNodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }

  return Size;
}

/** Returns the size of the SMMUv1/SMMUv2 node.

    @param [in]  Node    Pointer to SMMUv1/SMMUv2 node list.

    @retval Size of the SMMUv1/SMMUv2 node.
**/
STATIC
UINT32
GetSmmuV1V2NodeSize (
  IN  CONST CM_ARM_SMMUV1_SMMUV2_NODE  * Node
  )
{
  ASSERT (Node != NULL);

  /* Size of SMMU v1/SMMU v2 node +
     Size of ID mapping array +
     Size of context interrupt array +
     Size of PMU interrupt array
  */
  return sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE) +
           (Node->IdMappingCount *
            sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE)) +
           (Node->ContextInterruptCount *
            sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT)) +
           (Node->PmuInterruptCount *
            sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT));
}

/** Returns the total size required for the SMMUv1/SMMUv2 nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to SMMUv1/SMMUv2 node list.
    @param [in]       NodeCount       Count of the SMMUv1/SMMUv2 nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the SMMUv1/SMMUv2 nodes.
**/
STATIC
UINT32
GetSizeofSmmuV1V2Nodes (
  IN      CONST UINT32                              NodeStartOffset,
  IN      CONST CM_ARM_SMMUV1_SMMUV2_NODE   *       NodeList,
  IN            UINT32                              NodeCount,
  IN OUT        IORT_NODE_INDEXER          ** CONST NodeIndexer
  )
{
  UINT32  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token = NodeList->Token;
    (*NodeIndexer)->Object = (VOID*)NodeList;
    (*NodeIndexer)->Offset = Size + NodeStartOffset;
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetSmmuV1V2NodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }
  return Size;
}

/** Returns the size of the SMMUv3 node.

    @param [in]  Node    Pointer to SMMUv3 node list.

    @retval Total size of the SMMUv3 nodes.
**/
STATIC
UINT32
GetSmmuV3NodeSize (
  IN  CONST CM_ARM_SMMUV3_NODE  *Node
  )
{
  ASSERT (Node != NULL);

  /* Size of SMMU v1/SMMU v2 node +
     Size of ID mapping array
  */
  return (UINT32)(sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU3_NODE) +
                  (Node->IdMappingCount *
                   sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE)));
}

/** Returns the total size required for the SMMUv3 nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to SMMUv3 node list.
    @param [in]       NodeCount       Count of the SMMUv3 nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the SMMUv3 nodes.
**/
STATIC
UINT64
GetSizeofSmmuV3Nodes (
  IN      CONST UINT32                       NodeStartOffset,
  IN      CONST CM_ARM_SMMUV3_NODE           *NodeList,
  IN            UINT32                       NodeCount,
  IN OUT        IORT_NODE_INDEXER   **CONST  NodeIndexer
  )
{
  UINT64  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token  = NodeList->Token;
    (*NodeIndexer)->Object = (VOID *)NodeList;
    (*NodeIndexer)->Offset = (UINT32)(Size + NodeStartOffset);
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetSmmuV3NodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }

  return Size;
}

/** Returns the size of the PMCG node.

    @param [in]  Node    Pointer to PMCG node.

    @retval Size of the PMCG node.
**/
STATIC
UINT32
GetPmcgNodeSize (
  IN  CONST CM_ARM_PMCG_NODE  *Node
  )
{
  ASSERT (Node != NULL);

  /* Size of PMCG node +
     Size of ID mapping array
  */
  return (UINT32)(sizeof (EFI_ACPI_6_0_IO_REMAPPING_PMCG_NODE) +
                  (Node->IdMappingCount *
                   sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE)));
}

/** Returns the total size required for the PMCG nodes and
    updates the Node Indexer.

    This function calculates the size required for the node group
    and also populates the Node Indexer array with offsets for the
    individual nodes.

    @param [in]       NodeStartOffset Offset from the start of the
                                      IORT where this node group starts.
    @param [in]       NodeList        Pointer to PMCG node list.
    @param [in]       NodeCount       Count of the PMCG nodes.
    @param [in, out]  NodeIndexer     Pointer to the next Node Indexer.

    @retval Total size of the PMCG nodes.
**/
STATIC
UINT64
GetSizeofPmcgNodes (
  IN      CONST UINT32                     NodeStartOffset,
  IN      CONST CM_ARM_PMCG_NODE           *NodeList,
  IN            UINT32                     NodeCount,
  IN OUT        IORT_NODE_INDEXER **CONST  NodeIndexer
  )
{
  UINT64  Size;

  ASSERT (NodeList != NULL);

  Size = 0;
  while (NodeCount-- != 0) {
    (*NodeIndexer)->Token  = NodeList->Token;
    (*NodeIndexer)->Object = (VOID *)NodeList;
    (*NodeIndexer)->Offset = (UINT32)(Size + NodeStartOffset);
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer = %p, Token = %p, Object = %p, Offset = 0x%x\n",
      *NodeIndexer,
      (*NodeIndexer)->Token,
      (*NodeIndexer)->Object,
      (*NodeIndexer)->Offset
      ));

    Size += GetPmcgNodeSize (NodeList);
    (*NodeIndexer)++;
    NodeList++;
  }

  return Size;
}


/** Returns the offset of the Node referenced by the Token.

    @param [in]  NodeIndexer  Pointer to node indexer array.
    @param [in]  NodeCount    Count of the nodes.
    @param [in]  Token        Reference token for the node.
    @param [out] NodeOffset   Offset of the node from the
                              start of the IORT table.

    @retval EFI_SUCCESS       Success.
    @retval EFI_NOT_FOUND     No matching token reference
                              found in node indexer array.
**/
STATIC
EFI_STATUS
GetNodeOffsetReferencedByToken (
  IN  IORT_NODE_INDEXER * NodeIndexer,
  IN  UINT32              NodeCount,
  IN  CM_OBJECT_TOKEN     Token,
  OUT UINT32            * NodeOffset
  )
{
  DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer: Search Token = %p\n",
      Token
      ));
  while (NodeCount-- != 0) {
    DEBUG ((
      DEBUG_INFO,
      "IORT: Node Indexer: NodeIndexer->Token = %p, Offset = %d\n",
      NodeIndexer->Token,
      NodeIndexer->Offset
      ));
    if (NodeIndexer->Token == Token) {
      *NodeOffset = NodeIndexer->Offset;
      DEBUG ((
        DEBUG_INFO,
        "IORT: Node Indexer: Token = %p, Found\n",
        Token
        ));
      return EFI_SUCCESS;
    }
    NodeIndexer++;
  }
  DEBUG ((
    DEBUG_INFO,
    "IORT: Node Indexer: Token = %p, Not Found\n",
    Token
    ));
  return EFI_NOT_FOUND;
}

/** Update the Id Mapping Array.

    This function retrieves the Id Mapping Array object referenced by the
    IdMappingToken and updates the IdMapArray.

    @param [in]     IdMapArray       Pointer to an array of Id Mappings.
    @param [in]     IdCount          Number of Id Mappings.
    @param [in]     IdMappingToken   Reference Token for retrieving the
                                     Id Mapping Array object.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddIdMappingArray (
  IN  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE  *IdMapArray,
  IN  UINT32                              IdCount,
  IN  CONST CM_OBJECT_TOKEN               IdMappingToken
  )
{
  EFI_STATUS            Status;
  CM_ARM_ID_MAPPING     *IdMappings;
  UINT32                IdMappingCount;

  ASSERT (IdMapArray != NULL);

  // Get the Id Mapping Array
  Status = GetEArmObjIdMappingArray (
             IdMappingToken,
             &IdMappings,
             &IdMappingCount
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get Id Mapping array. Status = %r\n",
      Status
      ));
    return Status;
  }

  if (IdMappingCount < IdCount) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get the required number of Id Mappings.\n"
      ));
    return EFI_NOT_FOUND;
  }

  // Populate the Id Mapping array
  while (IdCount-- != 0) {
    Status = GetNodeOffsetReferencedByToken (
              gNodeIndexer,
              gIortNodeCount,
              IdMappings->OutputReferenceToken,
              &IdMapArray->OutputReference
              );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to get Output Reference for ITS Identifier array."
        "Reference Token = %p"
        " Status = %r\n",
        IdMappings->OutputReferenceToken,
        Status
        ));
      return Status;
    }

    IdMapArray->InputBase = IdMappings->InputBase;
    IdMapArray->NumIds = IdMappings->NumIds;
    IdMapArray->OutputBase = IdMappings->OutputBase;
    IdMapArray->Flags = IdMappings->Flags;

    IdMapArray++;
    IdMappings++;
  } // Id Mapping array

  return EFI_SUCCESS;
}

/** Update the ITS Group Node Information.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the ITS Group
                                     Nodes.
    @param [in]     NodeList         Pointer to an array of ITS Group Node
                                     Objects.
    @param [in]     NodeCount        Number of ITS Group Node Objects.

    @retval EFI_SUCCESS              Table generated successfully.
    @retval EFI_INVALID_PARAMETER    A parameter is invalid.
    @retval EFI_NOT_FOUND            The required object was not found.
**/
STATIC EFI_STATUS
AddItsGroupNodes (
  IN  CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE       *Iort,
  IN  CONST UINT32                                 NodesStartOffset,
  IN  CONST CM_ARM_ITS_GROUP_NODE                 *NodeList,
  IN        UINT32                                 NodeCount
  )
{
  EFI_STATUS                            Status;
  EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE    *ItsGroupNode;
  UINT32                                *ItsIds;
  CM_ARM_ITS_IDENTIFIER                 *ItsIdentifier;
  UINT32                                ItsIdentifierCount;
  UINT32                                IdIndex;

  UINT64                              NodeLength;
  ASSERT (Iort != NULL);

  ItsGroupNode = (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE*)((UINT8*)Iort +
                  NodesStartOffset);

  while (NodeCount-- != 0) {
    NodeLength = GetItsGroupNodeSize (NodeList);
    if (NodeLength > MAX_UINT16) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: ITS Id Array Node length 0x%lx > MAX_UINT16."
        " Status = %r\n",
        NodeLength,
        Status
        ));
      return Status;
    }

    // Populate the node header
    ItsGroupNode->Node.Type          = EFI_ACPI_IORT_TYPE_ITS_GROUP;
    ItsGroupNode->Node.Length        = (UINT16)NodeLength;
    ItsGroupNode->Node.Revision      = 0;
    ItsGroupNode->Node.Reserved      = EFI_ACPI_RESERVED_DWORD;
    ItsGroupNode->Node.NumIdMappings = 0;
    ItsGroupNode->Node.IdReference   = 0;

    // IORT specific data
    ItsGroupNode->NumItsIdentifiers = NodeList->ItsIdCount;
    ItsIds                          = (UINT32 *)((UINT8 *)ItsGroupNode +
                                                 sizeof (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE));

    Status = GetEArmObjGicItsIdentifierArray (
               NodeList->ItsIdToken,
               &ItsIdentifier,
               &ItsIdentifierCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to get ITS Identifier array. Status = %r\n",
        Status
        ));
      return Status;
    }

    if (ItsIdentifierCount < ItsGroupNode->NumItsIdentifiers) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to get the required number of ITS Identifiers.\n"
        ));
      return EFI_NOT_FOUND;
    }

    // Populate the ITS identifier array
    for (IdIndex = 0; IdIndex < ItsGroupNode->NumItsIdentifiers; IdIndex++) {
      ItsIds[IdIndex] = ItsIdentifier[IdIndex].ItsId;
    } // ITS identifier array

    // Next IORT Group Node
    ItsGroupNode = (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE*)((UINT8*)ItsGroupNode +
                    ItsGroupNode->Node.Length);
    NodeList++;
  } // IORT Group Node

  return EFI_SUCCESS;
}

/** Update the Named Component Node Information.

    This function updates the Named Component node information in the IORT
    table.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the Named
                                     Component Nodes.
    @param [in]     NodeList         Pointer to an array of Named Component
                                     Node Objects.
    @param [in]     NodeCount        Number of Named Component Node Objects.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddNamedComponentNodes (
  IN      CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE       *Iort,
  IN      CONST UINT32                                NodesStartOffset,
  IN      CONST CM_ARM_NAMED_COMPONENT_NODE           *NodeList,
  IN            UINT32                                NodeCount
  )
{
  EFI_STATUS                                 Status;
  EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE  *NcNode;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE         *IdMapArray;
  CHAR8                                      *ObjectName;
  UINTN                                      ObjectNameLength;
  UINT64                                     NodeLength;

  ASSERT (Iort != NULL);

  NcNode = (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE *)((UINT8 *)Iort +
                                                         NodesStartOffset);

  while (NodeCount-- != 0) {
    NodeLength = GetNamedComponentNodeSize (NodeList);
    if (NodeLength > MAX_UINT16) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Named Component Node length 0x%lx > MAX_UINT16."
        " Status = %r\n",
        NodeLength,
        Status
        ));
      return Status;
    }

    // Populate the node header
    NcNode->Node.Type          = EFI_ACPI_IORT_TYPE_NAMED_COMP;
    NcNode->Node.Length        = (UINT16)NodeLength;
    NcNode->Node.Revision      = 2;
    NcNode->Node.Reserved      = EFI_ACPI_RESERVED_DWORD;
    NcNode->Node.NumIdMappings = NodeList->IdMappingCount;

    ObjectNameLength         = AsciiStrLen (NodeList->ObjectName) + 1;
    NcNode->Node.IdReference =
      (UINT32)(sizeof (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE) +
               (ALIGN_VALUE (ObjectNameLength, 4)));

    // Named Component specific data
    NcNode->Flags             = NodeList->Flags;
    NcNode->CacheCoherent     = NodeList->CacheCoherent;
    NcNode->AllocationHints   = NodeList->AllocationHints;
    NcNode->Reserved          = EFI_ACPI_RESERVED_WORD;
    NcNode->MemoryAccessFlags = NodeList->MemoryAccessFlags;
    NcNode->AddressSizeLimit  = NodeList->AddressSizeLimit;

    // Copy the object name
    ObjectName = (CHAR8 *)((UINT8 *)NcNode +
                           sizeof (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE));
    Status = AsciiStrCpyS (
               ObjectName,
               ObjectNameLength,
               NodeList->ObjectName
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to copy Object Name. Status = %r\n",
        Status
        ));
      return Status;
    }

    if ((NodeList->IdMappingCount > 0) &&
        (NodeList->IdMappingToken != CM_NULL_TOKEN))
    {
      // Ids for Named Component
      IdMapArray = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE *)((UINT8 *)NcNode +
                                                          NcNode->Node.IdReference);

      Status = AddIdMappingArray (
                 IdMapArray,
                 NodeList->IdMappingCount,
                 NodeList->IdMappingToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to add Id Mapping Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }

    // Next Named Component Node
    NcNode = (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE *)((UINT8 *)NcNode +
                                                           NcNode->Node.Length);
    NodeList++;
  } // Named Component Node

  return EFI_SUCCESS;
}

/** Update the Root Complex Node Information.

    This function updates the Root Complex node information in the IORT table.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the Root Complex
                                     Nodes.
    @param [in]     NodeList         Pointer to an array of Root Complex Node
                                     Objects.
    @param [in]     NodeCount        Number of Root Complex Node Objects.

    @retval EFI_SUCCESS              Table generated successfully.
    @retval EFI_INVALID_PARAMETER    A parameter is invalid.
    @retval EFI_NOT_FOUND            The required object was not found.
**/
STATIC
EFI_STATUS
AddRootComplexNodes (
  IN      CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE        *       Iort,
  IN      CONST UINT32                                         NodesStartOffset,
  IN      CONST CM_ARM_ROOT_COMPLEX_NODE               *       NodeList,
  IN            UINT32                                         NodeCount
  )
{
  EFI_STATUS                           Status;
  EFI_ACPI_6_0_IO_REMAPPING_RC_NODE  * RcNode;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE * IdMapArray;

  ASSERT (Iort != NULL);

  RcNode = (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE*)((UINT8*)Iort +
            NodesStartOffset);

  while (NodeCount-- != 0) {
    // Populate the node header
    RcNode->Node.Type = EFI_ACPI_IORT_TYPE_ROOT_COMPLEX;
    RcNode->Node.Length = GetRootComplexNodeSize (NodeList);
    RcNode->Node.Revision = 1;
    RcNode->Node.Reserved = EFI_ACPI_RESERVED_DWORD;
    RcNode->Node.NumIdMappings = NodeList->IdMappingCount;
    RcNode->Node.IdReference = sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE);

    // Root Complex specific data
    RcNode->CacheCoherent = NodeList->CacheCoherent;
    RcNode->AllocationHints = NodeList->AllocationHints;
    RcNode->Reserved = EFI_ACPI_RESERVED_WORD;
    RcNode->MemoryAccessFlags = NodeList->MemoryAccessFlags;
    RcNode->AtsAttribute = NodeList->AtsAttribute;
    RcNode->PciSegmentNumber = NodeList->PciSegmentNumber;
    RcNode->MemoryAddressSize = NodeList->MemoryAddressSize;
    RcNode->Reserved1[0] = EFI_ACPI_RESERVED_BYTE;
    RcNode->Reserved1[1] = EFI_ACPI_RESERVED_BYTE;
    RcNode->Reserved1[2] = EFI_ACPI_RESERVED_BYTE;

    if ((NodeList->IdMappingCount > 0) &&
        (NodeList->IdMappingToken != CM_NULL_TOKEN)) {
      // Ids for Root Complex
      IdMapArray = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE*)((UINT8*)RcNode +
                    RcNode->Node.IdReference);
      Status = AddIdMappingArray (
                 IdMapArray,
                 NodeList->IdMappingCount,
                 NodeList->IdMappingToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to add Id Mapping Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }

    // Next Root Complex Node
    RcNode = (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE*)((UINT8*)RcNode +
              RcNode->Node.Length);
    NodeList++;
  } // Root Complex Node

  return EFI_SUCCESS;
}


/** Update the SMMU Interrupt Array.

    This function retrieves the InterruptArray object referenced by the
    InterruptToken and updates the SMMU InterruptArray.

    @param [in, out] InterruptArray   Pointer to an array of Interrupts.
    @param [in]      InterruptCount   Number of entries in the InterruptArray.
    @param [in]      InterruptToken   Reference Token for retrieving the SMMU
                                      InterruptArray object.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddSmmuInterrruptArray (
  IN OUT        EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT    *InterruptArray,
  IN            UINT32                                 InterruptCount,
  IN      CONST CM_OBJECT_TOKEN                        InterruptToken
  )
{
  EFI_STATUS              Status;
  CM_ARM_SMMU_INTERRUPT   *SmmuInterrupt;
  UINT32                  SmmuInterruptCount;

  ASSERT (InterruptArray != NULL);


  // Get the SMMU Interrupt Array
  Status = GetEArmObjSmmuInterruptArray (
             InterruptToken,
             &SmmuInterrupt,
             &SmmuInterruptCount
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get SMMU Interrupt array. Status = %r\n",
      Status
      ));
    return Status;
  }

  if (SmmuInterruptCount < InterruptCount) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get the required number of SMMU Interrupts.\n"
      ));
    return EFI_NOT_FOUND;
  }

  // Populate the Id Mapping array
  while (InterruptCount-- != 0) {
    InterruptArray->Interrupt = SmmuInterrupt->Interrupt;
    InterruptArray->InterruptFlags = SmmuInterrupt->Flags;
    InterruptArray++;
    SmmuInterrupt++;
  } // Id Mapping array

  return EFI_SUCCESS;
}

/** Update the SMMU v1/v2 Node Information.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the SMMU v1/v2
                                     Nodes.
    @param [in]     NodeList         Pointer to an array of SMMU v1/v2 Node
                                     Objects.
    @param [in]     NodeCount        Number of SMMU v1/v2 Node Objects.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddSmmuV1V2Nodes (
  IN      CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE       *       Iort,
  IN      CONST UINT32                                        NodesStartOffset,
  IN      CONST CM_ARM_SMMUV1_SMMUV2_NODE             *       NodeList,
  IN            UINT32                                        NodeCount
  )
{
  EFI_STATUS                            Status;
  EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE * SmmuNode;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE  * IdMapArray;

  EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT  * ContextInterruptArray;
  EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT  * PmuInterruptArray;

  ASSERT (Iort != NULL);

  SmmuNode = (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE*)((UINT8*)Iort +
              NodesStartOffset);

  while (NodeCount-- != 0) {
    // Populate the node header
    SmmuNode->Node.Type = EFI_ACPI_IORT_TYPE_SMMUv1v2;
    SmmuNode->Node.Length = GetSmmuV1V2NodeSize (NodeList);
    SmmuNode->Node.Revision = 0;
    SmmuNode->Node.Reserved = EFI_ACPI_RESERVED_DWORD;
    SmmuNode->Node.NumIdMappings = NodeList->IdMappingCount;
    SmmuNode->Node.IdReference = sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE) +
      (NodeList->ContextInterruptCount *
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT)) +
      (NodeList->PmuInterruptCount *
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT));

    // SMMU v1/v2 specific data
    SmmuNode->Base = NodeList->BaseAddress;
    SmmuNode->Span = NodeList->Span;
    SmmuNode->Model = NodeList->Model;
    SmmuNode->Flags = NodeList->Flags;

    // Reference to Global Interrupt Array
    SmmuNode->GlobalInterruptArrayRef =
      OFFSET_OF (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE, SMMU_NSgIrpt);

    // Context Interrupt
    SmmuNode->NumContextInterrupts = NodeList->ContextInterruptCount;
    SmmuNode->ContextInterruptArrayRef =
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE);
    ContextInterruptArray =
      (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT*)((UINT8*)SmmuNode +
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE));

    // PMU Interrupt
    SmmuNode->NumPmuInterrupts = NodeList->PmuInterruptCount;
    SmmuNode->PmuInterruptArrayRef = SmmuNode->ContextInterruptArrayRef +
      (NodeList->ContextInterruptCount *
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT));
    PmuInterruptArray =
      (EFI_ACPI_6_0_IO_REMAPPING_SMMU_INT*)((UINT8*)SmmuNode +
      SmmuNode->PmuInterruptArrayRef);

    SmmuNode->SMMU_NSgIrpt = NodeList->SMMU_NSgIrpt;
    SmmuNode->SMMU_NSgIrptFlags = NodeList->SMMU_NSgIrptFlags;
    SmmuNode->SMMU_NSgCfgIrpt = NodeList->SMMU_NSgCfgIrpt;
    SmmuNode->SMMU_NSgCfgIrptFlags = NodeList->SMMU_NSgCfgIrptFlags;

    // Add Context Interrupt Array
    Status = AddSmmuInterrruptArray (
               ContextInterruptArray,
               SmmuNode->NumContextInterrupts,
               NodeList->ContextInterruptToken
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to Context Interrupt Array. Status = %r\n",
        Status
        ));
      return Status;
    }

    // Add PMU Interrupt Array
    if ((SmmuNode->NumPmuInterrupts > 0) &&
        (NodeList->PmuInterruptToken != CM_NULL_TOKEN)) {
      Status = AddSmmuInterrruptArray (
                 PmuInterruptArray,
                 SmmuNode->NumPmuInterrupts,
                 NodeList->PmuInterruptToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to PMU Interrupt Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }

    if ((NodeList->IdMappingCount > 0) &&
        (NodeList->IdMappingToken != CM_NULL_TOKEN)) {
      // Ids for SMMU v1/v2 Node
      IdMapArray = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE*)((UINT8*)SmmuNode +
                    SmmuNode->Node.IdReference);
      Status = AddIdMappingArray (
                 IdMapArray,
                 NodeList->IdMappingCount,
                 NodeList->IdMappingToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to add Id Mapping Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }
    // Next SMMU v1/v2 Node
    SmmuNode = (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE*)((UINT8*)SmmuNode +
                SmmuNode->Node.Length);
    NodeList++;
  } // SMMU v1/v2 Node

  return EFI_SUCCESS;
}

/** Update the SMMUv3 Node Information.

    This function updates the SMMUv3 node information in the IORT table.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the SMMUv3 Nodes.
    @param [in]     NodeList         Pointer to an array of SMMUv3 Node Objects.
    @param [in]     NodeCount        Number of SMMUv3 Node Objects.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddSmmuV3Nodes (
  IN      CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE               *Iort,
  IN      CONST UINT32                                        NodesStartOffset,
  IN      CONST CM_ARM_SMMUV3_NODE                            *NodeList,
  IN            UINT32                                        NodeCount
  )
{
  EFI_STATUS                            Status;
  EFI_ACPI_6_0_IO_REMAPPING_SMMU3_NODE  *SmmuV3Node;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE    *IdMapArray;
  UINT64                                NodeLength;

  ASSERT (Iort != NULL);

  SmmuV3Node = (EFI_ACPI_6_0_IO_REMAPPING_SMMU3_NODE *)((UINT8 *)Iort +
                                                        NodesStartOffset);

  while (NodeCount-- != 0) {
    NodeLength = GetSmmuV3NodeSize (NodeList);
    if (NodeLength > MAX_UINT16) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: SMMU V3 Node length 0x%lx > MAX_UINT16. Status = %r\n",
        NodeLength,
        Status
        ));
      return Status;
    }

    // Populate the node header
    SmmuV3Node->Node.Type          = EFI_ACPI_IORT_TYPE_SMMUv3;
    SmmuV3Node->Node.Length        = (UINT16)NodeLength;
    SmmuV3Node->Node.Revision      = 2;
    SmmuV3Node->Node.Reserved      = EFI_ACPI_RESERVED_DWORD;
    SmmuV3Node->Node.NumIdMappings = NodeList->IdMappingCount;
    SmmuV3Node->Node.IdReference   =
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_SMMU3_NODE);

    // SMMUv3 specific data
    SmmuV3Node->Base         = NodeList->BaseAddress;
    SmmuV3Node->Flags        = NodeList->Flags;
    SmmuV3Node->Reserved     = EFI_ACPI_RESERVED_WORD;
    SmmuV3Node->VatosAddress = NodeList->VatosAddress;
    SmmuV3Node->Model        = NodeList->Model;
    SmmuV3Node->Event        = NodeList->EventInterrupt;
    SmmuV3Node->Pri          = NodeList->PriInterrupt;
    SmmuV3Node->Gerr         = NodeList->GerrInterrupt;
    SmmuV3Node->Sync         = NodeList->SyncInterrupt;

    if ((SmmuV3Node->Flags & EFI_ACPI_IORT_SMMUv3_FLAG_PROXIMITY_DOMAIN) != 0) {
      // The Proximity Domain Valid flag is set to 1
      SmmuV3Node->ProximityDomain = NodeList->ProximityDomain;
    } else {
      SmmuV3Node->ProximityDomain = 0;
    }

    if ((SmmuV3Node->Event != 0) && (SmmuV3Node->Pri != 0) &&
        (SmmuV3Node->Gerr != 0) && (SmmuV3Node->Sync != 0))
    {
      // If all the SMMU control interrupts are GSIV based,
      // the DeviceID mapping index field is ignored.
      SmmuV3Node->DeviceIdMappingIndex = 0;
    } else {
      SmmuV3Node->DeviceIdMappingIndex = NodeList->DeviceIdMappingIndex;
    }

    if ((NodeList->IdMappingCount > 0) &&
        (NodeList->IdMappingToken != CM_NULL_TOKEN))
    {
      // Ids for SMMUv3 node
      IdMapArray = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE *)((UINT8 *)SmmuV3Node +
                                                          SmmuV3Node->Node.IdReference);
      Status = AddIdMappingArray (
                 IdMapArray,
                 NodeList->IdMappingCount,
                 NodeList->IdMappingToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to add Id Mapping Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }

    // Next SMMUv3 Node
    SmmuV3Node = (EFI_ACPI_6_0_IO_REMAPPING_SMMU3_NODE *)((UINT8 *)SmmuV3Node +
                                                          SmmuV3Node->Node.Length);
    NodeList++;
  } // SMMUv3 Node

  return EFI_SUCCESS;
}

/** Update the PMCG Node Information.

    This function updates the PMCG node information in the IORT table.

    @param [in]     Iort             Pointer to IORT table structure.
    @param [in]     NodesStartOffset Offset for the start of the PMCG Nodes.
    @param [in]     NodeList         Pointer to an array of PMCG Node Objects.
    @param [in]     NodeCount        Number of PMCG Node Objects.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
AddPmcgNodes (
  IN      CONST EFI_ACPI_6_0_IO_REMAPPING_TABLE               *Iort,
  IN      CONST UINT32                                        NodesStartOffset,
  IN      CONST CM_ARM_PMCG_NODE                              *NodeList,
  IN            UINT32                                        NodeCount
  )
{
  EFI_STATUS                           Status;
  EFI_ACPI_6_0_IO_REMAPPING_PMCG_NODE  *PmcgNode;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE   *IdMapArray;
  /*ACPI_IORT_GENERATOR                  *Generator;*/
  UINT64                               NodeLength;

  ASSERT (Iort != NULL);

  /*Generator = (ACPI_IORT_GENERATOR *)This;*/
  PmcgNode  = (EFI_ACPI_6_0_IO_REMAPPING_PMCG_NODE *)((UINT8 *)Iort +
                                                      NodesStartOffset);

  while (NodeCount-- != 0) {
    NodeLength = GetPmcgNodeSize (NodeList);
    if (NodeLength > MAX_UINT16) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: PMCG Node length 0x%lx > MAX_UINT16. Status = %r\n",
        NodeLength,
        Status
        ));
      return Status;
    }

    // Populate the node header
    PmcgNode->Node.Type          = EFI_ACPI_IORT_TYPE_PMCG;
    PmcgNode->Node.Length        = (UINT16)NodeLength;
    PmcgNode->Node.Revision      = 1;
    PmcgNode->Node.Reserved      = EFI_ACPI_RESERVED_DWORD;
    PmcgNode->Node.NumIdMappings = NodeList->IdMappingCount;
    PmcgNode->Node.IdReference   = sizeof (EFI_ACPI_6_0_IO_REMAPPING_PMCG_NODE);

    // PMCG specific data
    PmcgNode->Base                  = NodeList->BaseAddress;
    PmcgNode->OverflowInterruptGsiv = NodeList->OverflowInterrupt;
    PmcgNode->Page1Base             = NodeList->Page1BaseAddress;

    Status = GetNodeOffsetReferencedByToken (
               gNodeIndexer,
               gIortNodeCount,
               NodeList->ReferenceToken,
               &PmcgNode->NodeReference
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to get Output Reference for PMCG Node."
        "Reference Token = %p"
        " Status = %r\n",
        NodeList->ReferenceToken,
        Status
        ));
      return Status;
    }

    if ((NodeList->IdMappingCount > 0) &&
        (NodeList->IdMappingToken != CM_NULL_TOKEN))
    {
      // Ids for PMCG node
      IdMapArray = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE *)((UINT8 *)PmcgNode +
                                                          PmcgNode->Node.IdReference);

      Status = AddIdMappingArray (
                 IdMapArray,
                 NodeList->IdMappingCount,
                 NodeList->IdMappingToken
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: IORT: Failed to add Id Mapping Array. Status = %r\n",
          Status
          ));
        return Status;
      }
    }

    // Next PMCG Node
    PmcgNode = (EFI_ACPI_6_0_IO_REMAPPING_PMCG_NODE *)((UINT8 *)PmcgNode +
                                                       PmcgNode->Node.Length);
    NodeList++;
  } // PMCG Node

  return EFI_SUCCESS;
}

/**
  The AddAcpiHeader function updates the ACPI header structure pointed by
  the AcpiHeader.

  @param [in,out] AcpiHeader     Pointer to the ACPI table header to be
                                 updated.
  @param [in]     Length         Length of the ACPI table.

  @retval EFI_SUCCESS           The ACPI table is updated successfully.
**/
EFI_STATUS
EFIAPI
AddAcpiHeader (
  IN OUT  EFI_ACPI_DESCRIPTION_HEADER  *CONST AcpiHeader,
  IN      CONST UINT32                 Length
  )
{
  UINT8 OemId[6];

  OemId[0] = 'P';
  OemId[1] = 'H';
  OemId[2] = 'Y';
  OemId[3] = 'L';
  OemId[4] = 'T';
  OemId[5] = 'D';

  AcpiHeader->Signature = EFI_ACPI_6_0_IO_REMAPPING_TABLE_SIGNATURE;
  AcpiHeader->Length = Length;
  AcpiHeader->Revision = EFI_ACPI_IO_REMAPPING_TABLE_REVISION;
  AcpiHeader->Checksum = 0;
  CopyMem(AcpiHeader->OemId, OemId, sizeof(AcpiHeader->OemId));
  AcpiHeader->OemTableId = EFI_ACPI_IORT_OEM_TABLE_ID;
  AcpiHeader->OemRevision = EFI_ACPI_PHYTIUM_OEM_REVISION;
  AcpiHeader->CreatorId = EFI_ACPI_PHYTIUM_CREATOR_ID;
  AcpiHeader->CreatorRevision = EFI_ACPI_PHYTIUM_CREATOR_REVISION;

  return EFI_SUCCESS;
}


/** Construct the IORT ACPI table.

    @param [in, out] Table        Pointer to the constructed ACPI Table.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
**/
STATIC
EFI_STATUS
EFIAPI
BuildIortTable (
  IN OUT       EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  EFI_STATUS                             Status;

  UINT32                                 TableSize;
  UINT64                                 NodeSize;

  UINT32                                 IortNodeCount;
  UINT32                                 ItsGroupNodeCount;
  UINT32                                 NamedComponentNodeCount;
  UINT32                                 RootComplexNodeCount;
  UINT32                                 SmmuV1V2NodeCount;
  UINT32                                 SmmuV3NodeCount;
  UINT32                                 PmcgNodeCount;

  UINT32                                 ItsGroupOffset;
  UINT32                                 NamedComponentOffset;
  UINT32                                 RootComplexOffset;
  UINT32                                 SmmuV1V2Offset;
  UINT32                                 SmmuV3Offset;
  UINT32                                 PmcgOffset;

  CM_ARM_ITS_GROUP_NODE                  *ItsGroupNodeList;
  CM_ARM_NAMED_COMPONENT_NODE            *NamedComponentNodeList;
  CM_ARM_ROOT_COMPLEX_NODE               *RootComplexNodeList;
  CM_ARM_SMMUV1_SMMUV2_NODE              *SmmuV1V2NodeList;
  CM_ARM_SMMUV3_NODE                     *SmmuV3NodeList;
  CM_ARM_PMCG_NODE                       *PmcgNodeList;

  EFI_ACPI_6_0_IO_REMAPPING_TABLE        *Iort;
  IORT_NODE_INDEXER                      *NodeIndexer;

    *Table = NULL;

  // Get the ITS group node info
  Status = GetEArmObjItsGroup (
             &ItsGroupNodeList,
             &ItsGroupNodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get ITS Group Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the ITS group node count
  IortNodeCount = ItsGroupNodeCount;

  // Get the Named component node info
  Status = GetEArmObjNamedComponent (
             &NamedComponentNodeList,
             &NamedComponentNodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get Named Component Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the Named Component group count
  IortNodeCount += NamedComponentNodeCount;


  // Get the Root complex node info
  Status = GetEArmObjRootComplex (
             &RootComplexNodeList,
             &RootComplexNodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get Root Complex Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the Root Complex node count
  IortNodeCount += RootComplexNodeCount;

  // Get the SMMU v1/v2 node info
  Status = GetEArmObjSmmuV1SmmuV2 (
             &SmmuV1V2NodeList,
             &SmmuV1V2NodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get SMMUv1/SMMUv2 Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the SMMU v1/v2 node count
  IortNodeCount += SmmuV1V2NodeCount;

  // Get the SMMUv3 node info
  Status = GetEArmObjSmmuV3 (
             &SmmuV3NodeList,
             &SmmuV3NodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get SMMUv3 Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the SMMUv3 node count
  IortNodeCount += SmmuV3NodeCount;

  // Get the PMCG node info
  Status = GetEArmObjPmcg (
             &PmcgNodeList,
             &PmcgNodeCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to get PMCG Node Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Add the PMCG node count
  IortNodeCount += PmcgNodeCount;


  // Allocate Node Indexer array
  NodeIndexer = (IORT_NODE_INDEXER*)AllocateZeroPool (
                                      (sizeof (IORT_NODE_INDEXER) *
                                       IortNodeCount)
                                      );
  if (NodeIndexer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to allocate memory for Node Indexer" \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  DEBUG ((DEBUG_INFO, "INFO: NodeIndexer = %p\n", NodeIndexer));
  gIortNodeCount = IortNodeCount;
  gNodeIndexer = NodeIndexer;
  // Calculate the size of the IORT table
  TableSize = sizeof (EFI_ACPI_6_0_IO_REMAPPING_TABLE);

  // ITS Group Nodes
  if (ItsGroupNodeCount > 0) {
    ItsGroupOffset = TableSize;
    // Size of ITS Group node list.
    NodeSize = GetSizeofItsGroupNodes (
                 ItsGroupOffset,
                 ItsGroupNodeList,
                 ItsGroupNodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of Group Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " ItsGroupNodeCount = %d\n" \
      " ItsGroupOffset = %d\n",
      ItsGroupNodeCount,
      ItsGroupOffset
      ));
  }

  // Named Component Nodes
  if (NamedComponentNodeCount > 0) {
    NamedComponentOffset = (UINT32)TableSize;
    // Size of Named Component node list.
    NodeSize = GetSizeofNamedComponentNodes (
                 NamedComponentOffset,
                 NamedComponentNodeList,
                 NamedComponentNodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of Named Component Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " NamedComponentNodeCount = %d\n" \
      " NamedComponentOffset = %d\n",
      NamedComponentNodeCount,
      NamedComponentOffset
      ));
  }

  // Root Complex Nodes
  if (RootComplexNodeCount > 0) {
    RootComplexOffset = TableSize;
    // Size of Root Complex node list.
    NodeSize = GetSizeofRootComplexNodes (
                 RootComplexOffset,
                 RootComplexNodeList,
                 RootComplexNodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of Root Complex Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " RootComplexNodeCount = %d\n" \
      " RootComplexOffset = %d\n",
      RootComplexNodeCount,
      RootComplexOffset
      ));
  }

  // SMMUv1/SMMUv2 Nodes
  if (SmmuV1V2NodeCount > 0) {
    SmmuV1V2Offset = TableSize;
    // Size of SMMUv1/SMMUv2 node list.
    NodeSize = GetSizeofSmmuV1V2Nodes (
                 SmmuV1V2Offset,
                 SmmuV1V2NodeList,
                 SmmuV1V2NodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of SMMUv1/v2 Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " SmmuV1V2NodeCount = %d\n" \
      " SmmuV1V2Offset = %d\n",
      SmmuV1V2NodeCount,
      SmmuV1V2Offset
      ));
  }

  // SMMUv3 Nodes
  if (SmmuV3NodeCount > 0) {
    SmmuV3Offset = (UINT32)TableSize;
    // Size of SMMUv3 node list.
    NodeSize = GetSizeofSmmuV3Nodes (
                 SmmuV3Offset,
                 SmmuV3NodeList,
                 SmmuV3NodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of SMMUv3 Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " SmmuV3NodeCount = %d\n" \
      " SmmuV3Offset = %d\n",
      SmmuV3NodeCount,
      SmmuV3Offset
      ));
  }

  // PMCG Nodes
  if (PmcgNodeCount > 0) {
    PmcgOffset = (UINT32)TableSize;
    // Size of PMCG node list.
    NodeSize = GetSizeofPmcgNodes (
                 PmcgOffset,
                 PmcgNodeList,
                 PmcgNodeCount,
                 &NodeIndexer
                 );
    if (NodeSize > MAX_UINT32) {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Invalid Size of PMCG Nodes. Status = %r\n",
        Status
        ));
      goto error_handler;
    }

    TableSize += NodeSize;

    DEBUG ((
      DEBUG_INFO,
      " PmcgNodeCount = %d\n" \
      " PmcgOffset = %d\n",
      PmcgNodeCount,
      PmcgOffset
      ));
  }

  DEBUG ((
    DEBUG_INFO,
    "INFO: IORT:\n" \
    " IortNodeCount = %d\n" \
    " TableSize = %d\n",
    IortNodeCount,
    TableSize
    ));

  if (TableSize > MAX_UINT32) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: IORT Table Size 0x%lx > MAX_UINT32," \
      " Status = %r\n",
      TableSize,
      Status
      ));
    goto error_handler;
  }

  // Allocate the Buffer for IORT table
  *Table = (EFI_ACPI_DESCRIPTION_HEADER*)AllocateZeroPool (TableSize);
  if (*Table == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to allocate memory for IORT Table, Size = %d," \
      " Status = %r\n",
      TableSize,
      Status
      ));
    goto error_handler;
  }

  Iort = (EFI_ACPI_6_0_IO_REMAPPING_TABLE*)*Table;

  DEBUG ((
    DEBUG_INFO,
    "IORT: Iort = 0x%p TableSize = 0x%x\n",
    Iort,
    TableSize
    ));

  Status = AddAcpiHeader (
             &Iort->Header,
             TableSize
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: IORT: Failed to add ACPI header. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  // Update IORT table
  Iort->NumNodes = IortNodeCount;
  Iort->NodeOffset = sizeof (EFI_ACPI_6_0_IO_REMAPPING_TABLE);
  Iort->Reserved = EFI_ACPI_RESERVED_DWORD;

  if (ItsGroupNodeCount > 0) {
    Status = AddItsGroupNodes (
               Iort,
               ItsGroupOffset,
               ItsGroupNodeList,
               ItsGroupNodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add ITS Group Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (NamedComponentNodeCount > 0) {
    Status = AddNamedComponentNodes (
               Iort,
               NamedComponentOffset,
               NamedComponentNodeList,
               NamedComponentNodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add Named Component Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (RootComplexNodeCount > 0) {
    Status = AddRootComplexNodes (
               Iort,
               RootComplexOffset,
               RootComplexNodeList,
               RootComplexNodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add Root Complex Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (SmmuV1V2NodeCount > 0) {
    Status = AddSmmuV1V2Nodes (
               Iort,
               SmmuV1V2Offset,
               SmmuV1V2NodeList,
               SmmuV1V2NodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add SMMU v1/v2 Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (SmmuV3NodeCount > 0) {
    Status = AddSmmuV3Nodes (
               Iort,
               SmmuV3Offset,
               SmmuV3NodeList,
               SmmuV3NodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add SMMUv3 Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (PmcgNodeCount > 0) {
    Status = AddPmcgNodes (
               Iort,
               PmcgOffset,
               PmcgNodeList,
               PmcgNodeCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: IORT: Failed to add SMMUv3 Node. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  return EFI_SUCCESS;

error_handler:
  if (*Table != NULL) {
    FreePool (*Table);
    *Table = NULL;
  }
  return Status;
}

/**
  Buid and install IORT table.

  @retval EFI_SUCCESS           The IORT table is install successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object was not found.
**/
EFI_STATUS
EFIAPI
BulidAndInstallIortTable(
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_ACPI_DESCRIPTION_HEADER       *Table;
  EFI_ACPI_TABLE_PROTOCOL           *AcpiTableProtocol;
  UINTN                             AcpiTableHandle;

  Status = EFI_SUCCESS;

  AcpiTableProtocol = NULL;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID**)&AcpiTableProtocol);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  BuildIortTable (&Table);

  Status = AcpiTableProtocol->InstallAcpiTable (
                                 AcpiTableProtocol,
                                 Table,
                                 Table->Length,
                                 &AcpiTableHandle);

  return Status;
}

/**
  Install IORT table.

  @retval EFI_SUCCESS           The IORT table is install successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object was not found.
**/
EFI_STATUS
InstallIortTable (
  VOID
  )
{
  EFI_STATUS   Status;

  DEBUG ((DEBUG_INFO, "Install Iort Table\n"));
  InitPlatformInfo ();
  Status = BulidAndInstallIortTable();

  return Status;
}


