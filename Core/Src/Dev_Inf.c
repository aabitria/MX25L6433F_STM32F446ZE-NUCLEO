/**
  ******************************************************************************
  * @file    Dev_Inf.c
  * @author  MCD Application Team
  * @brief   This file defines the structure containing informations about the 
  *          external flash memory N25Q128A used by STM32CubeProgramer in 
  *          programming/erasing the device.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#include "Dev_Inf.h"
#include "spi_nor.h"

/* This structure containes information used by ST-LINK Utility to program and erase the device */
#if defined (__ICCARM__)
__root struct StorageInfo const StorageInfo  =  {
#else
struct StorageInfo const StorageInfo =  {
#endif
   "MX25L6433F_STM32F446ZE-XNUCLEO-GFX01M1",                // Device Name
   SPI_FLASH,                   					        // Device Type
   0x90000000,                						        // Device Start Address
   SPI_NOR_MAX_SIZE,              						    // Device Size in 16 MBytes
   SPI_NOR_PAGE_SIZE,                                       // Programming Page Size 4096 Bytes
   0xFF,                       						        // Initial Content of Erased Memory
// Specify Size and Address of Sectors (view example below)
   SPI_NOR_SECTOR_NUM, SPI_NOR_SECTOR_SIZE,                 // Sector Num : 256 ,Sector Size: 64 KBytes
   0x00000000, 0x00000000,      
}; 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
