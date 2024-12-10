/** @file
Setup Information Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  SETUPDATA_H_
#define  SETUPDATA_H_

typedef  struct {
  UINT8    WatchdogPost;
  UINT16   WatchdogTimeoutPost;
  UINT8    WatchdogTimeoutActionPost;
  UINT8    WatchdogOS;
  UINT16   WatchdogTimeoutOS;
  UINT8    WatchdogTimeoutActionOS;
} WATCHDOG_CONFIG;

#endif
