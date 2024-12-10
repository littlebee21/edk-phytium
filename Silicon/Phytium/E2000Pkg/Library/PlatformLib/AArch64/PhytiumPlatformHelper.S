#
#  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

#include <AsmMacroIoLibV8.h>
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <AutoGen.h>

.text
.align 2

GCC_ASM_EXPORT(ArmPlatformPeiBootAction)
GCC_ASM_EXPORT(ArmPlatformIsPrimaryCore)
GCC_ASM_EXPORT(ArmPlatformGetPrimaryCoreMpId)
GCC_ASM_EXPORT(ArmPlatformGetCorePosition)

PrimaryCoreMpid:  .word    0x0


ASM_PFX(ArmPlatformPeiBootAction):
  // Save MPIDR_EL1[23:0] in a variable.
  //mov   x20, x30
  //bl    ASM_PFX(ArmReadMpidr)
  //lsl   w0, w0, #8
  //lsr   w0, w0, #8
  //ldr   x1, =PrimaryCoreMpid
  //str   w0, [x1]

  //ret   x20

   //Flash chip set speed
  //wirte cmd_port
#if 0
  ldr x0, =0x28008010
  ldr w1, =(0x6<<24)|(0x1<<22)|(0x1<<21)
  str w1, [x0]
  //wirte ld_port
  ldr x0, =0x2800801c
  ldr w1, =0x1
  str w1, [x0]
  //wirte cmd_port
  ldr x0, =0x28008010
  ldr w1, =(0x1<<24)|(0x1<<22)|(0x1<<21)|(0x1<<13)|(0x1<<3)
  str w1, [x0]
  //wirte ld_port
  ldr x0, =0x2800801c
  ldr w1, =0x200
  str w1, [x0]
  //close write enable
  ldr x0, =0x28008010
  ldr w1, =(0x4<<24)|(0x1<<22)|(0x1<<21)
  str w1, [x0]


  //set qspi sck  37.5M 1-1-4
  ldr x0, =0x28008004
  ldr w1, =0x6b24007B
  str w1, [x0]
#endif
  
  mrs x0, sctlr_el2
  ldr x1, =(0x1 << 12) 
  orr x0, x0, x1
  msr sctlr_el2, x0  
  //Flash chip sck get from parametertable
  ldr x0, =0xF5410
  ldr w1, [x0]
  lsr w2, w1, #16
  and w2, w2, 0xff
  add w2, w2, 3
  //set qspi sck
  ldr x0, =0x28008004
  ldr w1, =0x8
  orr w1, w1, w2
  str w1, [x0]

  ret

//UINTN
//ArmPlatformGetPrimaryCoreMpId (
//  VOID
//  );
ASM_PFX(ArmPlatformGetPrimaryCoreMpId):
  ldr   x0, =PrimaryCoreMpid
  ldr   w0, [x0]
  ret

//UINTN
//ArmPlatformIsPrimaryCore (
//  IN UINTN MpId
//  );
ASM_PFX(ArmPlatformIsPrimaryCore):
  mov    x0, #1
  ret

//UINTN
//ArmPlatformGetCorePosition (
//  IN UINTN MpId
//  );
// With this function: CorePos = (ClusterId * 4) + CoreId
ASM_PFX(ArmPlatformGetCorePosition):
  and   x1, x0, #ARM_CORE_MASK
  and   x0, x0, #ARM_CLUSTER_MASK
  add   x0, x1, x0, LSR #6
  ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED
