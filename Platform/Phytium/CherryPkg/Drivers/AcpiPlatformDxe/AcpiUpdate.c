/**
  Realize dynamic update of ACPI tables.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiUpdate.h"

/**
  Update _CRS of DP node.
  Update _CRS: Cma base address and size. through pcd PcdDcDpCmaEnable.
  If Cma function is enabled, it is allocated a memory space of CMA_SIZE.
  Fill the base address and size to the second Memory32fixed of _CRS. If
  Cma function is disabled, the base address and size should be 0.

  @param[in]  RootHandle      Pointer to the root of an AML tree.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  EFI_OUT_OF_RESOURCES   Out of resources.
**/
STATIC
EFI_STATUS
UpdateDpNodeCrs (
  AML_ROOT_NODE_HANDLE  RootHandle
  )
{
  EFI_STATUS                   Status;
  AML_OBJECT_NODE_HANDLE       NameOpCrsNode;
  AML_DATA_NODE_HANDLE         DpRegBaseRdNode;
  AML_DATA_NODE_HANDLE         DpCmaBaseRdNode;
  EFI_PHYSICAL_ADDRESS         CmaAddrBase;

  //
  // Get the _CRS object defined by the "Name ()" statement.
  //
  Status = AmlFindNode (
             RootHandle,
             "\\_SB.DCP0._CRS",
             &NameOpCrsNode
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Find DCP0._CRS Failed : %r!\n ", Status));
    return Status;
  }

  //
  //Get first region, dcdp register base addr
  //
  Status = AmlNameOpGetFirstRdNode (NameOpCrsNode, &DpRegBaseRdNode);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Get dp first region failed : %r\n", Status));
    return Status;
  }
  if (DpRegBaseRdNode == NULL) {
    DEBUG ((DEBUG_ERROR, "Get dp first region failed Null\n"));
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }

  //
  //Get second region, dcdp cma base addr
  //
  Status = AmlNameOpGetNextRdNode (
             DpRegBaseRdNode,
             &DpCmaBaseRdNode
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Get dp second region failed : %r\n", Status));
    return Status;
  }

  if (DpCmaBaseRdNode == NULL) {
    DEBUG ((DEBUG_ERROR, "Get dp second region failed Null\n"));
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }

  //
  //Update Cma
  //CMA Enable : Allocate reserved memory, and update the second Memory32Fixed
  //CMA Disable : Update the second Memory32Fixed base and size to 0.
  //
  if (PcdGetBool (PcdDcDpCmaEnable) == TRUE) {
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES (CMA_SIZE),
                    &CmaAddrBase
                    );
    ASSERT (!EFI_ERROR(Status));
    DEBUG ((DEBUG_INFO, "Cma Address Base : %llx\n", CmaAddrBase));
    Status = AmlUpdateMemory32Fixed (
             DpCmaBaseRdNode,
             CmaAddrBase,
             CMA_SIZE
             );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Update CMA Failed : %r\n", Status));
      return Status;
    }
  } else {
    Status = AmlUpdateMemory32Fixed (
             DpCmaBaseRdNode,
             0x0,
             0x0
             );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Update CMA Failed : %r\n", Status));
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Update pipe_mask of DP node.
  Update PIPE: PIPE value defines pipe_mask of DP. Bit 0 defines enable/disable
  of DP0. Bit1 defines enable/disable of DP1. The enable/disable status of DP0
  is obtained from PcdDcDpChanel0Enable. The enable/disable status of DP1
  is obtained from PcdDcDpChanel1Enable.

  @param[in]  RootHandle      Pointer to the root of an AML tree.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  EFI_OUT_OF_RESOURCES   Out of resources.
**/
STATIC
EFI_STATUS
UpdateDpNodePipe (
  AML_ROOT_NODE_HANDLE  RootHandle
  )
{
  EFI_STATUS              Status;
  UINT32                  Pipe;
  AML_OBJECT_NODE_HANDLE  NameOpIdNode;

  Status = EFI_SUCCESS;

  Status = AmlFindNode (
             RootHandle,
             "\\_SB.DCP0.PIPE",
             &NameOpIdNode
             );
  if (EFI_ERROR ( Status )) {
    DEBUG ((DEBUG_ERROR, "Not found _SB.DCP0.PIPE!\n"));
    return Status;
  }

  Pipe= 0;
  if (PcdGetBool (PcdDcDpChanel0Enable)) {
    Pipe |= 0x1;
  } else {
    Pipe &= ~(0x1);
  }
  if (PcdGetBool (PcdDcDpChanel1Enable)) {
    Pipe |= (0x1 << 1);
  } else {
    Pipe &= ~(0x1 << 1);
  }

  DEBUG ((DEBUG_INFO, "Update _SB.DCP0.PIPE to 0x%x\n", Pipe));
  AmlNameOpUpdateInteger (NameOpIdNode, Pipe);

  return EFI_SUCCESS;
}

/**
  Update IIS0 and IIS1 node.
  IIS0 and IIS1 nodes exists by default in Dp.asl. If PcdDcDpChanel0Enable is
  disable, IIS0 node is to be deleted automatically. If PcdDcDpChanel1Enable is
  disable, IIS1 node is to be deleted automatically.

  @param[in]  RootHandle      Pointer to the root of an AML tree.

  @retval  EFI_SUCCESS            The function completed successfully.
**/
STATIC
EFI_STATUS
UpdateDpI2sNode (
  AML_ROOT_NODE_HANDLE  RootHandle
  )
{
  EFI_STATUS                Status;
  AML_OBJECT_NODE_HANDLE    NameOpIdNode;

  //
  //Update IIS0
  //
  Status = AmlFindNode (
             RootHandle,
             "\\_SB.IIS0",
             &NameOpIdNode
             );
  if (EFI_SUCCESS == Status) {
    if (!PcdGetBool (PcdDcDpChanel0Enable)) {
      AmlDetachNode (NameOpIdNode);
      DEBUG ((DEBUG_INFO, "Delete Open _SB.IIS0 Node\n"));
    }
  }
  //
  //Update IIS1
  //
  Status = AmlFindNode (
             RootHandle,
             "\\_SB.IIS1",
             &NameOpIdNode
             );
  if (EFI_SUCCESS == Status) {
    if (!PcdGetBool (PcdDcDpChanel1Enable)) {
      AmlDetachNode (NameOpIdNode);
      DEBUG ((DEBUG_INFO, "Delete Open _SB.IIS1 Node\n"));
    }
  }

  return EFI_SUCCESS;
}

/**
  Update Dp node;
  1.Update Cma base and size of _CRS.
  2.Update pipe_mask through PIPE.

  @param[in]  RootHandle      Pointer to the root of an AML tree.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_INVALID_PARAMETER  Invalid parameter.
  @retval  EFI_OUT_OF_RESOURCES   Out of resources.
**/
STATIC
EFI_STATUS
UpdateDpNode (
  AML_ROOT_NODE_HANDLE  RootHandle
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  Status = UpdateDpNodeCrs (RootHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Update Dp Node Crs Failed : %r!\n", Status));
  }

  Status = UpdateDpNodePipe (RootHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Update Dp Node Pipe Failed : %r!\n", Status));
  }

  return EFI_SUCCESS;
}

/**
  Update PMDK node according to PcdDcDpChanel0Enable and PcdDcDpChanel1Enable.
  1.DPCN node. Total DP lanes, 0/1/2;
  2.PIPE node. Pipe bit mask of DP node, bit0 - DP0, bit1 - DP1.

  @param[in]  RootHandle      Pointer to the root of an AML tree.

  @retval  EFI_SUCCESS            The function completed successfully.
  @retval  EFI_NOT_FOUND          Node not found.
**/
STATIC
EFI_STATUS
UpdatePmdkNode (
  AML_ROOT_NODE_HANDLE  RootHandle
  )
{
  EFI_STATUS              Status;
  UINT32                  Pipe;
  UINT32                  Dpcn;
  AML_OBJECT_NODE_HANDLE  NameOpIdNode;

  Status = EFI_SUCCESS;

  Status = AmlFindNode (
             RootHandle,
             "\\_SB.PMDK.PIPE",
             &NameOpIdNode
             );
  if (EFI_ERROR ( Status )) {
    DEBUG ((DEBUG_ERROR, "Not found _SB.PMDK.PIPE!\n"));
    return Status;
  }

  Dpcn = 0;
  Pipe = 0;
  if (PcdGetBool (PcdDcDpChanel0Enable)) {
    Pipe |= 0x1;
    Dpcn++;
  } else {
    Pipe &= ~(0x1);
  }
  if (PcdGetBool (PcdDcDpChanel1Enable)) {
    Pipe |= (0x1 << 1);
    Dpcn++;
  } else {
    Pipe &= ~(0x1 << 1);
  }

  DEBUG ((DEBUG_INFO, "Update _SB.PMDK.PIPE to 0x%x\n", Pipe));
  AmlNameOpUpdateInteger (NameOpIdNode, Pipe);

  Status = AmlFindNode (
             RootHandle,
             "\\_SB.PMDK.DPCN",
             &NameOpIdNode
             );
  if (EFI_ERROR ( Status )) {
    DEBUG ((DEBUG_ERROR, "Not found _SB.PMDK.DPCN!\n"));
    return Status;
  }
  DEBUG ((DEBUG_INFO, "Update _SB.PMDK.DPCN to 0x%x\n", Dpcn));
  AmlNameOpUpdateInteger (NameOpIdNode, Dpcn);

  return EFI_SUCCESS;
}

/**
  The event of update dsdt.

  @param[in]  Event        Event whose notification function is being invoked.
  @param[in]  Context      Pointer to the notification function's context.

  @retval    Null.
**/
STATIC
VOID
UpdateAcpiDsdtAfterPciEnum (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS                   Status;
  EFI_STATUS                   Status1;
  VOID                         *Interface;
  UINTN                        TableKey;
  UINTN                        Index;
  EFI_ACPI_SDT_HEADER          *Table;
  EFI_ACPI_SDT_PROTOCOL        *AcpiTableProtocol;
  EFI_ACPI_TABLE_PROTOCOL      *AcpiTable;
  EFI_ACPI_TABLE_VERSION       TableVersion;
  EFI_ACPI_DESCRIPTION_HEADER  **NewTable;
  AML_ROOT_NODE_HANDLE         RootNodeHandle;
  UINTN                        TableSize;
  UINTN                        TableHandle;

  Status = gBS->LocateProtocol (
                  &gAcpiUpdateAfterPciEmunGuid,
                  NULL,
                  (VOID**)&Interface
                  );
  if (EFI_ERROR(Status)) {
    return;
  }
  gBS->CloseEvent(Event);

  NewTable = (EFI_ACPI_DESCRIPTION_HEADER**) AllocateZeroPool ((sizeof (EFI_ACPI_DESCRIPTION_HEADER*)));

  if (NewTable == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: DSDT-FIXUP: Failed to allocate memory for Table ."
      " Status = %r\n",
      Status
      ));
    ASSERT_EFI_ERROR(Status);
  }

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid,
                  NULL,
                  (VOID**) &AcpiTableProtocol
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Unable to locate ACPI table protocol\n"));
    ASSERT_EFI_ERROR(Status);
  }

  //
  // Search for DSDT Table
  //
  for (Index = 0; Index < EFI_ACPI_MAX_NUM_TABLES; Index++) {
    Status = AcpiTableProtocol->GetAcpiTable(Index, &Table, &TableVersion, &TableKey);
    if (EFI_ERROR(Status)) {
      break;
    }
    if (Table->Signature != EFI_ACPI_6_4_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      continue;
    }

    Status = AmlParseDefinitionBlock (
             (CONST EFI_ACPI_DESCRIPTION_HEADER *)Table,
             &RootNodeHandle
             );

    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: DSDT-FIXUP:"
        " Failed to parse DSDT Template. Status = %r\n",
        Status
        ));
      ASSERT_EFI_ERROR(Status);
    }
    //
    //Update all dsdt node
    //
    Status = UpdateDpNode (RootNodeHandle);
    Status = UpdateDpI2sNode (RootNodeHandle);
    Status = UpdatePmdkNode (RootNodeHandle);
    //
    // Serialize the tree.
    //
    Status = AmlSerializeDefinitionBlock (
               RootNodeHandle,
               NewTable
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: DSDT-FIXUP: Failed to Serialize DSDT Table Data."
        " Status = %r\n",
        Status
        ));
      ASSERT_EFI_ERROR(Status);
    }

    Status = gBS->LocateProtocol (
                    &gEfiAcpiTableProtocolGuid,
                    NULL,
                    (VOID**)&AcpiTable
                    );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR(Status);
    }

    Status = AcpiTable->UninstallAcpiTable (AcpiTable, TableKey);
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "ERROR: DSDT-FIXUP: Failed to uninstall Dsdt table.\n"));
      ASSERT_EFI_ERROR(Status);
    }

    TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) *NewTable)->Length;
    //
    // Install ACPI table
    //
    Status = AcpiTable->InstallAcpiTable (
                          AcpiTable,
                          *NewTable,
                          TableSize,
                          &TableHandle
                          );

    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "ERROR: DSDT-FIXUP: Failed to install Dsdt table.\n"));
      ASSERT_EFI_ERROR(Status);
    }

    if (RootNodeHandle != NULL) {
      Status1 = AmlDeleteTree (RootNodeHandle);
      if (EFI_ERROR (Status1)) {
        DEBUG ((
          DEBUG_ERROR,
          "ERROR: DSDT-FIXUP: Failed to cleanup AML tree."
          " Status = %r\n",
          Status1
          ));
      ASSERT_EFI_ERROR(Status1);
      }
    }
  }
}

/**
  Create a event to update dsdt.

  @param[in,out]  AcpiHdr      A pointer to EFI_ACPI_COMMON_HEADER.

  @retval    EFI_SUCCESS    Success.
**/
STATIC
EFI_STATUS
UpdateDsdt(
  IN OUT EFI_ACPI_COMMON_HEADER *AcpiHdr
  )
{
  VOID  *EventReg;

  EfiCreateProtocolNotifyEvent (
        &gAcpiUpdateAfterPciEmunGuid,
        TPL_CALLBACK,
        UpdateAcpiDsdtAfterPciEnum,
        NULL,
        &EventReg
        );

  return EFI_SUCCESS;
}

/**
  Update acpi tables.

  @param[in,out]  TableHeader      A pointer to EFI_ACPI_COMMON_HEADER.

  @retval    EFI_SUCCESS    Success.
**/
EFI_STATUS
UpdateAcpiTable (
  IN OUT EFI_ACPI_COMMON_HEADER      *TableHeader
  )
{
  switch (TableHeader->Signature) {
    case EFI_ACPI_6_4_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      DEBUG ((DEBUG_INFO, "%a(), %d, Dsdt.asl update\n",__FUNCTION__,__LINE__));
      UpdateDsdt(TableHeader);
      break;

    default:
      break;

  }
  return EFI_SUCCESS;
}
