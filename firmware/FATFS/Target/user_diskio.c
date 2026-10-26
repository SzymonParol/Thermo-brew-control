/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   FatFs disk I/O driver for SD card connected through SPI.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

#include <string.h>
#include "ff_gen_drv.h"
#include "main.h"
#include "sd_spi.h"

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif
#if _USE_IOCTL == 1
DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif

Diskio_drvTypeDef USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if _USE_WRITE
  USER_write,
#endif
#if _USE_IOCTL
  USER_ioctl,
#endif
};

DSTATUS USER_initialize (
  BYTE pdrv)
{
  /* USER CODE BEGIN INIT */
  return SD_disk_initialize(pdrv);
  /* USER CODE END INIT */
}

DSTATUS USER_status (
  BYTE pdrv)
{
  /* USER CODE BEGIN STATUS */
  return SD_disk_status(pdrv);
  /* USER CODE END STATUS */
}

DRESULT USER_read (
  BYTE pdrv,
  BYTE *buff,
  DWORD sector,
  UINT count)
{
  /* USER CODE BEGIN READ */
  return SD_disk_read(pdrv, buff, sector, count);
  /* USER CODE END READ */
}

#if _USE_WRITE == 1
DRESULT USER_write (
  BYTE pdrv,
  const BYTE *buff,
  DWORD sector,
  UINT count)
{
  /* USER CODE BEGIN WRITE */
  return SD_disk_write(pdrv, buff, sector, count);
  /* USER CODE END WRITE */
}
#endif

#if _USE_IOCTL == 1
DRESULT USER_ioctl (
  BYTE pdrv,
  BYTE cmd,
  void *buff)
{
  /* USER CODE BEGIN IOCTL */
  return SD_disk_ioctl(pdrv, cmd, buff);
  /* USER CODE END IOCTL */
}
#endif
