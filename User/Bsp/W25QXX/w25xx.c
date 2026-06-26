#include "bsp_w25xx.h"

#include "stm32h7xx_hal.h"

#include "spi.h"

#include <string.h>



/* 定义Flash片选GPIO引脚 PA4 */

#define SF_CS_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define SF_CS_GPIO FLASH_CS_GPIO_Port

#define SF_CS_PIN FLASH_CS_Pin



#define SF_CS_0() SF_CS_GPIO->BSRR = ((uint32_t)SF_CS_PIN << 16U)

#define SF_CS_1() SF_CS_GPIO->BSRR = SF_CS_PIN



#define hspi hspi3



#define CMD_AAI 0xAD	/* AAI 字编程指令(FOR SST25VF016B) */

#define CMD_DISWR 0x04	/* 禁能写, 禁能AAI模式 */

#define CMD_EWRSR 0x50	/* 允许写状态寄存器 */

#define CMD_WRSR 0x01	/* 写状态寄存器 */

#define CMD_WREN 0x06	/* 写使能 */

#define CMD_READ 0x03	/* 读数据字节 */

#define CMD_RDSR 0x05	/* 读状态寄存器 */

#define CMD_RDID 0x9F	/* 读芯片ID命令 */

#define CMD_SE 0x20		/* 扇区擦除命令 */

#define CMD_BE 0xC7		/* 整片擦除命令 */

#define DUMMY_BYTE 0xA5 /* 虚拟字节（SPI通信填充数据） */

#define WIP_FLAG 0x01	/* 忙标志位（写操作进行中 WIP） */



SFLASH_T g_tSF;



uint32_t g_spiLen;



#define SPI_BUFFER_SIZE (4 * 1024)



uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];

uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];



static uint8_t s_spiBuf[4 * 1024]; /* 内部读写缓冲区（用于扇区写操作的数据缓存） */

static void W25xx_WriteEnable(void);

static void W25xx_WaitForWriteEnd(void);

static uint8_t W25xx_NeedErase(uint8_t *_ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _uiLen);

static uint8_t W25xx_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize);

static uint8_t W25xx_AutoWriteSector(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);



/*

*********************************************************************************************************

*	函 数 名: bsp_spiTransfer

*	功能说明: SPI数据传输

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

void bsp_spiTransfer(void)

{

	HAL_StatusTypeDef result;

	result = HAL_SPI_TransmitReceive(&hspi, (uint8_t *)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen, 1000);

	if (result != HAL_OK)

	{

		// Error_Handler(__FILE__, __LINE__);

	}

}



/*

*********************************************************************************************************

*	函 数 名: bsp_InitSFlash

*	功能说明: 初始化Flash外设：配置CS GPIO，读取ID信息

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

void bsp_InitSFlash(void)

{

	/* 读取芯片ID, 识别芯片型号 */

	W25xx_ReadInfo();

}



/*

*********************************************************************************************************

*	函 数 名: sf_SetCS

*	功能说明: 设置FLASH片选信号

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

void sf_SetCS(uint8_t _Level)

{

	if (_Level == 0)

	{

		SF_CS_0();

	}

	else

	{

		SF_CS_1();

	}

}



/*

*********************************************************************************************************

*	函 数 名: sf_EraseSector

*	功能说明: 擦除指定扇区

*	形    参: _uiSectorAddr : 扇区地址

*	返 回 值: 无

*********************************************************************************************************

*/

void W25xx_EraseSector(uint32_t _uiSectorAddr)

{

	W25xx_WriteEnable(); /* 发送写使能命令 */



	/* 发送扇区擦除命令 */

	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[g_spiLen++] = CMD_SE;							 /* 扇区擦除命令 */

	g_spiTxBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF0000) >> 16); /* 扇区地址最高8bit */

	g_spiTxBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF00) >> 8);	 /* 扇区地址中间8bit */

	g_spiTxBuf[g_spiLen++] = (_uiSectorAddr & 0xFF);			 /* 扇区地址最低8bit */

	bsp_spiTransfer();

	sf_SetCS(1); /* 片选拉高 */



	W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */

}



/*

*********************************************************************************************************

*	函 数 名: sf_EraseChip

*	功能说明: 整片擦除芯片

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

void W25xx_EraseChip(void)

{

	W25xx_WriteEnable(); /* 发送写使能命令 */



	/* 发送扇区擦除命令 */

	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[g_spiLen++] = CMD_BE; /* 发送整片擦除命令 */

	bsp_spiTransfer();

	sf_SetCS(1); /* 片选拉高 */



	W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */

}



/*

*********************************************************************************************************

*	函 数 名: sf_PageWrite

*	功能说明: 页写入

*	形    参: _pBuf : 源数据缓冲区

*			  _uiWriteAddr : 写入起始地址?

*			  _usSize : 写入数据大小（每页最大256字节）

*	返 回 值: 无

*********************************************************************************************************

*/

void W25xx_PageWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)

{

	uint32_t i, j;



	if (g_tSF.ChipID == SST25VF016B_ID)

	{

		/* AAI模式每2个字节一组写入 */

		if ((_usSize < 2) && (_usSize % 2))

		{

			return;

		}



		W25xx_WriteEnable(); /* 发送写使能命令 */



		sf_SetCS(0); /* 片选拉低 */

		g_spiLen = 0;

		g_spiTxBuf[g_spiLen++] = CMD_AAI;							/* 发送AAI命令(地址自增编程) */

		g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF0000) >> 16); /* 写入地址最高8bit */

		g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF00) >> 8);	/* 写入地址中间8bit */

		g_spiTxBuf[g_spiLen++] = (_uiWriteAddr & 0xFF);				/* 写入地址最低8bit */

		g_spiTxBuf[g_spiLen++] = (*_pBuf++);						/* 写入第1个数据 */

		g_spiTxBuf[g_spiLen++] = (*_pBuf++);						/* 写入第2个数据 */

		bsp_spiTransfer();

		sf_SetCS(1); /* 片选拉高 */



		W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */



		_usSize -= 2; /* 发送扇区擦除命令 */



		for (i = 0; i < _usSize / 2; i++)

		{

			sf_SetCS(0); /* 片选拉低 */

			g_spiLen = 0;

			g_spiTxBuf[g_spiLen++] = (CMD_AAI);	 /* 发送AAI命令(地址自增编程) */

			g_spiTxBuf[g_spiLen++] = (*_pBuf++); /* 写入数据 */

			g_spiTxBuf[g_spiLen++] = (*_pBuf++); /* 写入数据 */

			bsp_spiTransfer();

			sf_SetCS(1);			 /* 剩余字节 */

			W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */

		}



		/* 发送写使能命令 */

		sf_SetCS(0);

		g_spiLen = 0;

		g_spiTxBuf[g_spiLen++] = (CMD_DISWR);

		bsp_spiTransfer();

		sf_SetCS(1);



		W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */

	}

	else /* for MX25L1606E 或 W25Q64BV */

	{

		for (j = 0; j < _usSize / 256; j++)

		{

			W25xx_WriteEnable(); /* 发送写使能命令 */



			sf_SetCS(0); /* 片选拉低 */

			g_spiLen = 0;

			g_spiTxBuf[g_spiLen++] = (0x02);							/* 页编程命令 */

			g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF0000) >> 16); /* 写入地址最高8bit */

			g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF00) >> 8);	/* 写入地址中间8bit */

			g_spiTxBuf[g_spiLen++] = (_uiWriteAddr & 0xFF);				/* 写入地址最低8bit */

			for (i = 0; i < 256; i++)

			{

				g_spiTxBuf[g_spiLen++] = (*_pBuf++); /* 写入数据 */

			}

			bsp_spiTransfer();

			sf_SetCS(1); /* 片选拉高 */



			W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */



			_uiWriteAddr += 256;

		}



		/* 发送写使能命令 */

		sf_SetCS(0);

		g_spiLen = 0;

		g_spiTxBuf[g_spiLen++] = (CMD_DISWR);

		bsp_spiTransfer();

		sf_SetCS(1);



		W25xx_WaitForWriteEnd(); /* 等待Flash内部写操作完成 */

	}

}



/*

*********************************************************************************************************

*	函 数 名: sf_ReadBuffer

*	功能说明: 连续读取指定长度的数据到缓冲区，不能超过芯片容量

*	形    参: 	_pBuf : 目标缓冲区

*				_uiReadAddr : 起始地址

*				_usSize : 读取长度，不能超过芯片容量

*	返 回 值: 无

*********************************************************************************************************

*/

void W25xx_ReadBuffer(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)

{

	uint16_t rem;

	uint16_t i;



	/* 如果读取长度为0或超出Flash总容量则返回 */

	if ((_uiSize == 0) || (_uiReadAddr + _uiSize) > g_tSF.TotalSize)

	{

		return;

	}



	/* 发送扇区擦除命令 */

	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[g_spiLen++] = (CMD_READ);					   /* 发送读命令 */

	g_spiTxBuf[g_spiLen++] = ((_uiReadAddr & 0xFF0000) >> 16); /* 读取地址最高8bit */

	g_spiTxBuf[g_spiLen++] = ((_uiReadAddr & 0xFF00) >> 8);	   /* 读取地址中间8bit */

	g_spiTxBuf[g_spiLen++] = (_uiReadAddr & 0xFF);			   /* 读取地址最低8bit */

	bsp_spiTransfer();



	/* 分段读取数据（通过DMA接收，单次最多4KB） */

	for (i = 0; i < _uiSize / SPI_BUFFER_SIZE; i++)

	{

		g_spiLen = SPI_BUFFER_SIZE;

		bsp_spiTransfer();



		memcpy(_pBuf, g_spiRxBuf, SPI_BUFFER_SIZE);

		_pBuf += SPI_BUFFER_SIZE;

	}



	rem = _uiSize % SPI_BUFFER_SIZE; /* 剩余字节 */

	if (rem > 0)

	{

		g_spiLen = rem;

		bsp_spiTransfer();



		memcpy(_pBuf, g_spiRxBuf, rem);

	}



	sf_SetCS(1); /* 片选拉高 */

}



/*

*********************************************************************************************************

*	函 数 名: sf_CmpData

*	功能说明: 比较Flash数据

*	形    参: 	_ucpTar : 目标比较数据?

*				_uiSrcAddr : Flash源地址

*				_uiSize : 比较长度，不能超过芯片总容量

*	返 回 值: 0 = 相等, 1 = 不相等

*********************************************************************************************************

*/

static uint8_t W25xx_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)

{

	uint16_t i, j;

	uint16_t rem;



	/* 如果读取长度为0或超出Flash总容量则返回 */

	if ((_uiSrcAddr + _uiSize) > g_tSF.TotalSize)

	{

		return 1;

	}



	if (_uiSize == 0)

	{

		return 0;

	}



	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[g_spiLen++] = (CMD_READ);					  /* 发送读命令 */

	g_spiTxBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF0000) >> 16); /* 源地址最高8bit */

	g_spiTxBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF00) >> 8);	  /* 源地址中间8bit */

	g_spiTxBuf[g_spiLen++] = (_uiSrcAddr & 0xFF);			  /* 源地址最低8bit */

	bsp_spiTransfer();



	/* 分段读取数据（通过DMA接收，单次最多4KB） */

	for (i = 0; i < _uiSize / SPI_BUFFER_SIZE; i++)

	{

		g_spiLen = SPI_BUFFER_SIZE;

		bsp_spiTransfer();



		for (j = 0; j < SPI_BUFFER_SIZE; j++)

		{

			if (g_spiRxBuf[j] != *_ucpTar++)

			{

				goto NOTEQ; /* 不相等 */

			}

		}

	}



	rem = _uiSize % SPI_BUFFER_SIZE; /* 剩余字节 */

	if (rem > 0)

	{

		g_spiLen = rem;

		bsp_spiTransfer();



		for (j = 0; j < rem; j++)

		{

			if (g_spiRxBuf[j] != *_ucpTar++)

			{

				goto NOTEQ; /* 不相等 */

			}

		}

	}

	sf_SetCS(1);

	return 0; /* 相等 */



NOTEQ:

	sf_SetCS(1); /* 片选拉高 */

	return 1;

}



/*

*********************************************************************************************************

*	函 数 名: W25xx_NeedErase

*	功能说明: 判断写PAGE前是否需要擦除

*	形    参:   _ucpOldBuf : 旧数据

*			   _ucpNewBuf : 新数据

*			   _usLen : 比较长度

*	返 回 值: 0 : 无需擦除 1 : 需要擦除

*********************************************************************************************************

*/

static uint8_t W25xx_NeedErase(uint8_t *_ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)

{

	uint16_t i;

	uint8_t ucOld;



	/*

	判断是否需要擦除的逻辑：

	第1步：取 old 反, new 不变

		  old    new

		  1101   0101

	~     1111

		= 0010   0101



	第2步：old 反码与 new 按位与

		  0010   old

	&	  0101   new

		 =0000



	第3步：结果为0，不需要擦除。否则需要擦除

	*/



	for (i = 0; i < _usLen; i++)

	{

		ucOld = *_ucpOldBuf++;

		ucOld = ~ucOld;



		/* 等价于写成: if (ucOld & (*_ucpNewBuf++) != 0) */

		if ((ucOld & (*_ucpNewBuf++)) != 0)

		{

			return 1;

		}

	}

	return 0;

}



/*

*********************************************************************************************************

*	函 数 名: sf_AutoWriteSector

*	功能说明: 写1个扇区，支持自动擦除，如果失败则重试3次，并自动校验数据

*	形    参: 	_pBuf : 目标缓冲区

*				_uiWriteAddr : 写入起始地址

*				_usSize : 写入长度（不能超过扇区大小）

*	返 回 值: 1 : 成功 0 : 失败

*********************************************************************************************************

*/

static uint8_t W25xx_AutoWriteSector(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)

{

	uint16_t i;

	uint16_t j;			  /* 重试计数器 */

	uint32_t uiFirstAddr; /* 扇区首地址 */

	uint8_t ucNeedErase;  /* 1表示需要擦除 */

	uint8_t cRet;



	/* 如果长度为0则无需操作，返回成功 */

	if (_usWrLen == 0)

	{

		return 1;

	}



	/* 如果地址超出芯片总容量则返回失败 */

	if (_uiWrAddr >= g_tSF.TotalSize)

	{

		return 0;

	}



	/* 如果写入长度超过扇区大小则返回失败 */

	if (_usWrLen > g_tSF.SectorSize)

	{

		return 0;

	}



	/* 比较FLASH中的数据是否有变化，无变化则不需写FLASH */

	W25xx_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);

	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)

	{

		return 1;

	}



	/* 判断是否需要擦除扇区 */

	/* Flash编程只能将位从 1->0 或者 0->0，不能从0->1，所以0->1时需先擦除Flash */

	ucNeedErase = 0;

	if (W25xx_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))

	{

		ucNeedErase = 1;

	}



	uiFirstAddr = _uiWrAddr & (~(g_tSF.SectorSize - 1));



	if (_usWrLen == g_tSF.SectorSize) /* 正好是整个扇区则直接写 */

	{

		for (i = 0; i < g_tSF.SectorSize; i++)

		{

			s_spiBuf[i] = _ucpSrc[i];

		}

	}

	else /* 只写扇区的一部分 */

	{

		/* 先读出整个扇区的数据 */

		W25xx_ReadBuffer(s_spiBuf, uiFirstAddr, g_tSF.SectorSize);



		/* 合并新数据到缓冲区 */

		i = _uiWrAddr & (g_tSF.SectorSize - 1);

		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);

	}



	/* 写扇区并校验，如果失败则重试，最多重试3次 */

	cRet = 0;

	for (i = 0; i < 3; i++)

	{



		/* Flash编程只能将位从 1->0 或者 0->0，不能从0->1，所以0->1时需先擦除Flash */

		if (ucNeedErase == 1)

		{

			W25xx_EraseSector(uiFirstAddr); /* 先擦除1个扇区 */

		}



		/* 编程整个扇区 */

		W25xx_PageWrite(s_spiBuf, uiFirstAddr, g_tSF.SectorSize);



		if (W25xx_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)

		{

			cRet = 1;

			break;

		}

		else

		{

			if (W25xx_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)

			{

				cRet = 1;

				break;

			}



			/* 失败后延时等待再重试 */

			for (j = 0; j < 10000; j++)

				;

		}

	}



	return cRet;

}



/*

*********************************************************************************************************

*	函 数 名: sf_WriteBuffer

*	功能说明: 写任意长度数据，支持自动擦除，如果失败则重试3次，并自动校验数据

*	形    参:  _pBuf : 源数据缓冲区

*			   _uiWriteAddr : 写入起始地址

*			   _usSize : 写入长度（不能超过芯片总容量）

*	返 回 值: 1 : 成功 0 : 失败

*********************************************************************************************************

*/

uint8_t W25xx_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize)

{

	uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;



	Addr = _uiWriteAddr % g_tSF.SectorSize;

	count = g_tSF.SectorSize - Addr; /*当前扇区剩余空间*/

	NumOfPage = _usWriteSize / g_tSF.SectorSize;

	NumOfSingle = _usWriteSize % g_tSF.SectorSize; /*不满一个扇区的剩余字节数*/



	if (Addr == 0) /* 写入地址是扇区起始地址  */

	{

		if (NumOfPage == 0) /* 写入长度小于扇区大小 */

		{

			if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)

			{

				return 0;

			}

		}

		else /* 写入长度大于等于扇区大小 */

		{

			while (NumOfPage--)

			{

				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, g_tSF.SectorSize) == 0)

				{

					return 0;

				}

				_uiWriteAddr += g_tSF.SectorSize;

				_pBuf += g_tSF.SectorSize;

			}

			if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)

			{

				return 0;

			}

		}

	}

	else /* 写入地址不是扇区起始地址  */

	{

		if (NumOfPage == 0) /* 写入长度小于扇区大小 */

		{

			if (NumOfSingle > count) /* 写入会跨扇区边界 */

			{

				temp = NumOfSingle - count;



				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, count) == 0) /*写满当前扇区剩余位置*/

				{

					return 0;

				}



				_uiWriteAddr += count;

				_pBuf += count;



				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, temp) == 0) /*在新扇区写剩余数据*/

				{

					return 0;

				}

			}

			else

			{

				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)

				{

					return 0;

				}

			}

		}

		else /* 写入长度大于等于扇区大小 */

		{

			_usWriteSize -= count;

			NumOfPage = _usWriteSize / g_tSF.SectorSize;

			NumOfSingle = _usWriteSize % g_tSF.SectorSize;

			if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, count) == 0)

			{

				return 0;

			}



			_uiWriteAddr += count;

			_pBuf += count;



			while (NumOfPage--)

			{

				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, g_tSF.SectorSize) == 0)

				{

					return 0;

				}

				_uiWriteAddr += g_tSF.SectorSize;

				_pBuf += g_tSF.SectorSize;

			}



			if (NumOfSingle != 0)

			{

				if (W25xx_AutoWriteSector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)

				{

					return 0;

				}

			}

		}

	}

	return 1; /* 成功 */

}



/*

*********************************************************************************************************

*	函 数 名: sf_ReadID

*	功能说明: 读取芯片ID

*	形    参: 无

*	返 回 值: 32bit芯片ID (高8bit为0，有效ID位为低24bit)

*********************************************************************************************************

*/

uint32_t W25xx_ReadID(void)

{

	uint32_t uiID;

	uint8_t id1, id2, id3;



	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[0] = (CMD_RDID); /* 发送读ID命令 */

	g_spiLen = 4;

	bsp_spiTransfer();



	id1 = g_spiRxBuf[1]; /* 读ID第1个字节 */

	id2 = g_spiRxBuf[2]; /* 读ID第2个字节 */

	id3 = g_spiRxBuf[3]; /* 读ID第3个字节 */

	sf_SetCS(1);		 /* 片选拉高 */



	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;



	return uiID;

}



/*

*********************************************************************************************************

*	函 数 名: sf_ReadInfo

*	功能说明: 读取芯片ID，识别芯片型号和参数

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

void W25xx_ReadInfo(void)

{

	/* 读取并识别Flash型号 */

	{

		g_tSF.ChipID = W25xx_ReadID(); /* 芯片ID */



		switch (g_tSF.ChipID)

		{

		case SST25VF016B_ID:

			strcpy(g_tSF.ChipName, "SST25VF016B");

			g_tSF.TotalSize = 2 * 1024 * 1024; /* 总容量 = 2M */

			g_tSF.SectorSize = 4 * 1024;	   /* 扇区大小 = 4K */

			break;



		case MX25L1606E_ID:

			strcpy(g_tSF.ChipName, "MX25L1606E");

			g_tSF.TotalSize = 2 * 1024 * 1024; /* 总容量 = 2M */

			g_tSF.SectorSize = 4 * 1024;	   /* 扇区大小 = 4K */

			break;



		case W25Q16BV_ID:

			strcpy(g_tSF.ChipName, "W25Q16");

			g_tSF.TotalSize = 2 * 1024 * 1024; /* 总容量 = 2M */

			g_tSF.SectorSize = 4 * 1024;	   /* 扇区大小 = 4K */



			break;

		case W25Q32BV_ID:

			strcpy(g_tSF.ChipName, "W25Q32");

			g_tSF.TotalSize = 4 * 1024 * 1024; /* 总容量 = 4M */

			g_tSF.SectorSize = 4 * 1024;	   /* 扇区大小 = 4K */

			break;



		case W25Q64BV_ID:

			strcpy(g_tSF.ChipName, "W25Q64");

			g_tSF.TotalSize = 8 * 1024 * 1024; /* 总容量 = 8M */

			g_tSF.SectorSize = 4 * 1024;	   /* 扇区大小 = 4K */

			break;



		case W25Q128_ID:

			strcpy(g_tSF.ChipName, "W25Q128");

			g_tSF.TotalSize = 16 * 1024 * 1024; /* 总容量 = 16M */

			g_tSF.SectorSize = 4 * 1024;		/* 扇区大小 = 4K */

			break;

		case W25Q256_ID:

			strcpy(g_tSF.ChipName, "W25Q256");

			g_tSF.TotalSize = 32 * 1024 * 1024; /* 总容量 = 32M */

			g_tSF.SectorSize = 4 * 1024;		/* 扇区大小 = 4K */

			break;

		case W25Q512_ID:

			strcpy(g_tSF.ChipName, "W25Q512");

			g_tSF.TotalSize = 64 * 1024 * 1024; /* 总容量 = 64M */

			g_tSF.SectorSize = 4 * 1024;		/* 扇区大小 = 4K */

			break;

		default:

			strcpy(g_tSF.ChipName, "Unknow Flash");

			g_tSF.TotalSize = 2 * 1024 * 1024;

			g_tSF.SectorSize = 4 * 1024;

			break;

		}

	}

}



/*

*********************************************************************************************************

*	函 数 名: sf_WriteEnable

*	功能说明: 向FLASH发送写使能命令

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

static void W25xx_WriteEnable(void)

{

	sf_SetCS(0); /* 片选拉低 */

	g_spiLen = 0;

	g_spiTxBuf[g_spiLen++] = (CMD_WREN); /* 发送写使能 */

	bsp_spiTransfer();

	sf_SetCS(1); /* 片选拉高 */

}



/*

*********************************************************************************************************

*	函 数 名: sf_WaitForWriteEnd

*	功能说明: 等待FLASH内部操作完成（轮询忙标志位）

*	形    参: 无

*	返 回 值: 无

*********************************************************************************************************

*/

static void W25xx_WaitForWriteEnd(void)

{

	sf_SetCS(0);				/* 片选拉低 */

	g_spiTxBuf[0] = (CMD_RDSR); /* 发送读状态寄存器命令 */

	g_spiLen = 2;

	bsp_spiTransfer();

	sf_SetCS(1); /* 片选拉高 */



	while (1)

	{

		sf_SetCS(0);				/* 片选拉低 */

		g_spiTxBuf[0] = (CMD_RDSR); /* 发送读状态寄存器命令 */

		g_spiTxBuf[1] = 0;			/* 填充字节 */

		g_spiLen = 2;

		bsp_spiTransfer();

		sf_SetCS(1); /* 片选拉高 */



		if ((g_spiRxBuf[1] & WIP_FLAG) != SET) /* 判断忙标志位是否清零 */

		{

			break;

		}

	}

}

