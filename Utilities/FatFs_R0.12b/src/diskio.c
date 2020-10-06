/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
char fat[SECTOR_COUNT][SECTOR_SIZE];
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
	)
{
	return 	RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
	)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
	)
{
	memcpy(buff, fat[sector], SECTOR_SIZE);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
	)
{
	memcpy(fat[sector], buff, SECTOR_SIZE);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
	)
{

	DRESULT res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC:		/* Make sure that no pending write process */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:	/* Get number of sectors on the disk (DWORD) */
		*(DWORD*)buff = (DWORD)SECTOR_COUNT;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
		*(WORD*)buff = 512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = 512;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}
	return res;

}

