/*
 * spi_nor.c
 *
 *  Created on: Apr 3, 2024
 *      Author: alvin.abitria
 */

#include "spi_nor.h"



static inline void spi_nor_select (void)
{
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
}


static inline void spi_nor_unselect (void)
{
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
}


static void spi_write_enable (void)
{
	uint8_t buffer;

    buffer = WRITE_ENABLE;
    spi_nor_select();
    spi_nor_transmit(&buffer, 1);
    spi_nor_unselect();
}


static void spi_write_disable (void)
{
	uint8_t buffer;

    buffer = WRITE_DISABLE;
    spi_nor_select();
    spi_nor_transmit(&buffer, 1);
    spi_nor_unselect();
}


void spi_nor_receive (uint8_t *data, uint16_t size)
{
	HAL_SPI_Receive(&hspi2, data, size, HAL_MAX_DELAY);
}


void spi_nor_polling_transmit (uint8_t *data, uint16_t size)
{

}


void spi_nor_transmit (uint8_t *data, uint16_t size)
{
	HAL_SPI_Transmit(&hspi2, data, size, HAL_MAX_DELAY);
}


void spi_nor_wait_write_complete (void)
{
	// Send RDSR, then read bit 0 if true
	uint8_t tx_buf = 0, rx_buf = 0;

	tx_buf = READ_STATUS_REG;
	spi_nor_select();
	spi_nor_transmit(&tx_buf, sizeof(tx_buf));

	// After sending, sending receive clks should fetch the data
	do {
		spi_nor_receive(&rx_buf, sizeof(rx_buf));
	} while(rx_buf & 0x01);

	spi_nor_unselect();
}


// All sizes are in # bytes
void spi_nor_read (uint32_t addr, uint8_t *data, uint16_t size)
{
	uint8_t buffer[4] = {0};

	// Send out 0x03 cmd (Read Bytes) along with address
	// then fetch the data with size dictating the length of SPI clk
	// no need to send WREN

	buffer[0] = READ;
	buffer[1] = (addr >> 16) & 0xff;
	buffer[2] = (addr >> 8) & 0xff;
	buffer[3] = (addr) & 0xff;

	spi_nor_select();
	spi_nor_transmit(buffer, sizeof(buffer));

	spi_nor_receive(data, size);

	spi_nor_unselect();
}


/* For all 256 bytes be written, A[7:0] = 0 */
void spi_nor_page_write (uint32_t addr, uint8_t *data, uint16_t size)
{
    uint8_t buffer[4] = {0};
    uint16_t write_size;

	// Send WREN
    spi_write_enable();

	// Send cmd with PAGE_WRITE and address
    if (size > SPI_NOR_PAGE_SIZE)
    	write_size = SPI_NOR_PAGE_SIZE;
    else
    	write_size = size;

    buffer[0] = 0x02;
	buffer[1] = (addr >> 16) & 0xff;
	buffer[2] = (addr >> 8) & 0xff;
	buffer[3] = (addr) & 0xff;
    spi_nor_select();
    spi_nor_transmit(buffer, sizeof(buffer));

    spi_nor_transmit(data, write_size);

    spi_nor_unselect();

	// wait ofr write to complete
    spi_nor_wait_write_complete();

    spi_write_disable();
}


/* Write will be done as a series of page_write's */
void spi_nor_write (uint32_t addr, uint8_t *data, uint16_t size)
{
    uint32_t addr_alignment = 0;
    uint32_t count_unaligned = 0, num_page = 0, num_excess_bytes = 0, temp = 0;

	addr_alignment = addr % SPI_NOR_PAGE_SIZE;
	count_unaligned = SPI_NOR_PAGE_SIZE - addr_alignment;
	num_page = size / SPI_NOR_PAGE_SIZE;
	num_excess_bytes = size % SPI_NOR_PAGE_SIZE;

	if (addr_alignment == 0) {

		if (num_page == 0) {						/* size < one page */
			spi_nor_page_write(addr, data, size);	/* just write and that's it*/
		} else {                                    /* Multiple pages, there might be excess */
			while (num_page--) {
				spi_nor_page_write(addr, data, SPI_NOR_PAGE_SIZE);
				addr += SPI_NOR_PAGE_SIZE;
				data += SPI_NOR_PAGE_SIZE;
			}

			if (num_excess_bytes) {
				spi_nor_page_write(addr, data, num_excess_bytes);
			}
		}
	} else {
		/* Address is not page aligned */
		if (num_page == 0) {                       /* size < one page */
			if (num_excess_bytes > count_unaligned) {
				temp = num_excess_bytes - count_unaligned;

				spi_nor_page_write(addr, data, count_unaligned);
				addr += count_unaligned;
				data += count_unaligned;

				spi_nor_page_write(addr, data, temp);
			} else {
			    spi_nor_page_write(addr, data, size);
			}
		} else {

			/* handle the 1st unaligned partial page */
			spi_nor_page_write(addr, data, count_unaligned);
			addr += count_unaligned;
			data += count_unaligned;

			/* handle the remainder which will be aligned from now on */
			size -= count_unaligned;
			num_page = size / SPI_NOR_PAGE_SIZE;
			num_excess_bytes = size % SPI_NOR_PAGE_SIZE;

			/* write the whole pages */
			while (num_page--) {
				spi_nor_page_write(addr, data, SPI_NOR_PAGE_SIZE);
				addr += SPI_NOR_PAGE_SIZE;
				data += SPI_NOR_PAGE_SIZE;
			}

			/* handle the last excess partial if there's any */
			if (num_excess_bytes) {
				spi_nor_page_write(addr, data, num_excess_bytes);
			}
		}
	}
}


void spi_nor_sector_erase (uint32_t addr)
{
    uint8_t buffer[4] = {0};

    spi_write_enable();

    buffer[0] = SECTOR_ERASE_64K;
    buffer[1] = (addr >> 16) & 0xff;
    buffer[2] = (addr >> 8) & 0xff;
    buffer[3] = (addr) & 0xff;
    spi_nor_select();
    spi_nor_transmit(buffer, sizeof(buffer));
    spi_nor_unselect();

    spi_nor_wait_write_complete();

    spi_write_disable();

}


void spi_nor_block_erase_64k (uint32_t addr)
{
    uint8_t buffer[4] = {0};

    spi_write_enable();

    buffer[0] = BLK_ERASE_64K;
    buffer[1] = (addr >> 16) & 0xff;
    buffer[2] = (addr >> 8) & 0xff;
    buffer[3] = (addr) & 0xff;
    spi_nor_select();
    spi_nor_transmit(buffer, sizeof(buffer));
    spi_nor_unselect();

    spi_nor_wait_write_complete();

    spi_write_disable();
}


void spi_nor_chip_erase (void)
{
    uint8_t buffer = 0;

    spi_write_enable();

    buffer = CHIP_ERASE;
    spi_nor_select();
    spi_nor_transmit(&buffer, 1);
    spi_nor_unselect();

    spi_nor_wait_write_complete();

    spi_write_disable();
}

#if 0
void spi_nor_read_devid (void)
{

}


void spi_nor_read_manufacturer_id (void)
{

}
#endif

uint32_t spi_nor_read_chip_id (void)
{
	// Reads ID - sends 1 byte command, expects 3 bytes reply
	uint8_t cmd[2] = {0};
	uint8_t data[3] = {0};
	uint32_t result;

	cmd[0] = READ_ID;

	spi_nor_select();
	spi_nor_transmit(cmd, 1);
	spi_nor_receive(data, sizeof(data));
	spi_nor_unselect();

    result = (data[0] << 16) | (data[1] << 8) | data[2];
    return result;
}


void spi_nor_reset (void)
{
    uint8_t cmd = RESET_EN;

    spi_nor_select();
	spi_nor_transmit(&cmd, 1);
	spi_nor_unselect();

    cmd = RESET;

    spi_nor_select();
	spi_nor_transmit(&cmd, 1);
	spi_nor_unselect();

	HAL_Delay(1);
}


int spi_nor_init (void)
{
	uint32_t jedec_id = 0;

	HAL_Delay(6);
	spi_nor_reset();
	jedec_id = spi_nor_read_chip_id();

	if (jedec_id != JEDEC_ID)
		return 0;

	return 1;
}

