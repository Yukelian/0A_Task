#ifndef _BSP_W25XX_H
#define _BSP_W25XX_H

#include <stdint.h>

#define SF_MAX_PAGE_SIZE (4 * 1024)

/* ???с─??Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID = 0xC22015,
	W25Q16BV_ID = 0xEF4015, /* BV, JV, FV */
	W25Q32BV_ID = 0xEF4016,
	W25Q64BV_ID = 0xEF4017,
	W25Q128_ID = 0xEF4018,
	W25Q256_ID = 0xEF4019,
	W25Q512_ID = 0xEF4020,
	W25Q01_ID = 0xEF4021,
};

typedef struct
{
	uint32_t ChipID;	 /* зр?ID */
	char ChipName[16];	 /* зр????????????????????? */
	uint32_t TotalSize;	 /* ?????? */
	uint16_t SectorSize; /* ??????з│ */
} SFLASH_T;

void bsp_InitSFlash(void);
uint32_t W25xx_ReadID(void);
void W25xx_EraseChip(void);
void W25xx_EraseSector(uint32_t _uiSectorAddr);
void W25xx_PageWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t W25xx_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize);
void W25xx_ReadBuffer(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void W25xx_ReadInfo(void);

extern SFLASH_T g_tSF;

#endif
