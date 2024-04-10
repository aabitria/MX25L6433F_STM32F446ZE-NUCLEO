/*
 * spi_nor.h
 *
 *  Created on: Apr 3, 2024
 *      Author: alvin.abitria
 */

#ifndef INC_SPI_NOR_H_
#define INC_SPI_NOR_H_

#include "stm32f4xx_hal.h"
#include "spi.h"

#define JEDEC_ID                                0xC22017

#define SPI_NOR_PAGE_SIZE                       0x100
#define SPI_NOR_SECTOR_SIZE                     0x1000
#define SPI_NOR_BLOCK_SIZE                      (64 * 1024)
#define SPI_NOR_MAX_SIZE                        (8 * 1024 * 1024)
#define SPI_NOR_SECTOR_NUM                      (SPI_NOR_MAX_SIZE / SPI_NOR_SECTOR_SIZE)

#define RESET_EN                                0x66
#define RESET                                   0x99
#define READ_ID                                 0x9F

#define WRITE_ENABLE                            0x06
#define WRITE_DISABLE                           0x04
#define READ_STATUS_REG                         0x05

#define READ                                    0x03
#define PAGE_PROGRAM                            0x02

#define SECTOR_ERASE_64K                        0x20
#define BLK_ERASE_64K                           0xD8
#define CHIP_ERASE                              0x60



void spi_nor_receive (uint8_t *data, uint16_t size);
void spi_nor_polling_transmit (uint8_t *data, uint16_t size);
void spi_nor_transmit (uint8_t *data, uint16_t size);
void spi_nor_wait_write_complete (void);
void spi_nor_read (uint32_t addr, uint8_t *data, uint16_t size);
void spi_nor_page_write (uint32_t addr, uint8_t *data, uint16_t size);
void spi_nor_write (uint32_t addr, uint8_t *data, uint16_t size);
void spi_nor_sector_erase (uint32_t addr);
void spi_nor_block_erase_64k (uint32_t addr);
void spi_nor_chip_erase (void);

uint32_t spi_nor_read_chip_id (void);
int spi_nor_init (void);
void spi_nor_reset (void);


#endif /* INC_SPI_NOR_H_ */
