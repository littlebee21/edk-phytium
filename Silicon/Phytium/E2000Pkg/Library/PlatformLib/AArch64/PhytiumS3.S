#include "../S3Resume.h"

.text
.align 2

GCC_ASM_EXPORT(AsmS3SystemOffEntry)
GCC_ASM_EXPORT(AsmS3SystemResetEntry)
GCC_ASM_EXPORT(AsmS3SuspendStartEntry)
GCC_ASM_EXPORT(AsmS3SuspendFinishEntry)
GCC_ASM_EXPORT(AsmS3SuspendEndEntry)

ASM_PFX(AsmS3SystemOffEntry):
  ldr x1, =(0x30C30000)
  mov sp, x1
  b S3SystemOffEntry

ASM_PFX(AsmS3SystemResetEntry):
  ldr x1, =(0x30C30000)
  mov sp, x1
  b S3SystemResetEntry

ASM_PFX(AsmS3SuspendStartEntry):
  ldr x1, =(0x30C30000)
  mov sp, x1
  b S3SuspendStartEntry

ASM_PFX(AsmS3SuspendFinishEntry):
  ldr x1, =(0x30C30000)
  mov sp, x1
  b S3SuspendFinishEntry

ASM_PFX(AsmS3SuspendEndEntry):
  ldr x1, =(0x30C30000)
  mov sp, x1
  b S3SuspendEndEntry
