#include "sd_spi.h"
#include <string.h>

/* Makra muszą pasować do nazw z CubeMX */
#define SD_CS_LOW()    HAL_GPIO_WritePin(SDCard_CS_GPIO_Port, SDCard_CS_Pin, GPIO_PIN_RESET)
#define SD_CS_HIGH()   HAL_GPIO_WritePin(SDCard_CS_GPIO_Port, SDCard_CS_Pin, GPIO_PIN_SET)

#define SD_SPI_HANDLE  (&hspi1)

#define SD_DUMMY_BYTE  0xFFU
#define SD_BLOCK_SIZE  512U

/* Komendy karty SD w trybie SPI */
#define CMD0    0U    /* GO_IDLE_STATE */
#define CMD8    8U    /* SEND_IF_COND */
#define CMD16   16U   /* SET_BLOCKLEN */
#define CMD17   17U   /* READ_SINGLE_BLOCK */
#define CMD24   24U   /* WRITE_BLOCK */
#define CMD55   55U   /* APP_CMD */
#define CMD58   58U   /* READ_OCR */
#define ACMD41  41U   /* SD_SEND_OP_COND */

#define SD_INIT_TIMEOUT_MS 1000U
#define SD_READY_TIMEOUT_MS 500U

/* Typ wykrytej karty */
#define CT_SD1    0x01U
#define CT_SD2    0x02U
#define CT_SDHC   0x04U
#define CT_BLOCK  0x08U

static volatile DSTATUS sd_status = STA_NOINIT;
static uint8_t sd_card_type = 0U;

/* ------------------------------------------------------------
 * Niskopoziomowa komunikacja SPI
 * ------------------------------------------------------------ */

static uint8_t SD_SPI_Transfer(uint8_t tx)
{
  uint8_t rx = 0U;

  HAL_SPI_TransmitReceive(SD_SPI_HANDLE, &tx, &rx, 1U, HAL_MAX_DELAY);

  return rx;
}

static void SD_SPI_Release(void)
{
  /* Karta wymaga zegara nawet wtedy, gdy nic nie wysyłamy */
  SD_SPI_Transfer(SD_DUMMY_BYTE);
}

static void SD_Select(void)
{
  SD_CS_LOW();
  SD_SPI_Release();
}

static void SD_Deselect(void)
{
  SD_CS_HIGH();

  /* Dodatkowe bajty po zwolnieniu CS pomagają kartzie zakończyć operację */
  SD_SPI_Release();
  SD_SPI_Release();
}

/* ------------------------------------------------------------
 * Pomocnicze funkcje protokołu SD
 * ------------------------------------------------------------ */

static uint8_t SD_WaitReady(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint8_t response;

  do {
    response = SD_SPI_Transfer(SD_DUMMY_BYTE);

    if ((HAL_GetTick() - start) > timeout_ms) {
      return 0xFFU;
    }
  } while (response != SD_DUMMY_BYTE);

  return 0U;
}

static uint8_t SD_SendCommand(uint8_t command, uint32_t argument)
{
  uint8_t packet[6];
  uint8_t response = 0xFFU;
  uint8_t crc = 0xFFU;
  uint8_t retry;

  /* Przed większością komend karta musi być gotowa */
  if ((command != CMD0) && (command != CMD12)) {
    if (SD_WaitReady(SD_READY_TIMEOUT_MS) != 0U) {
      return 0xFFU;
    }
  }

  packet[0] = 0x40U | command;
  packet[1] = (uint8_t)(argument >> 24U);
  packet[2] = (uint8_t)(argument >> 16U);
  packet[3] = (uint8_t)(argument >> 8U);
  packet[4] = (uint8_t)(argument >> 0U);

  /* Poprawne CRC tylko dla CMD0 i CMD8 */
  if (command == CMD0) {
    crc = 0x95U;
  } else if (command == CMD8) {
    crc = 0x87U;
  }

  packet[5] = crc;

  HAL_SPI_Transmit(SD_SPI_HANDLE, packet, 6U, HAL_MAX_DELAY);

  /* Karta może wysłać kilka bajtów 0xFF przed odpowiedzią */
  for (retry = 0U; retry < 10U; retry++) {
    response = SD_SPI_Transfer(SD_DUMMY_BYTE);

    if ((response & 0x80U) == 0U) {
      break;
    }
  }

  return response;
}

static uint8_t SD_SendApplicationCommand(uint8_t command, uint32_t argument)
{
  SD_SendCommand(CMD55, 0U);
  return SD_SendCommand(command, argument);
}

/* ------------------------------------------------------------
 * Inicjalizacja karty SD
 * ------------------------------------------------------------ */

static DSTATUS SD_CardInit(void)
{
  uint8_t response;
  uint8_t ocr[4];
  uint32_t start;

  sd_card_type = 0U;

  SD_Deselect();

  /* Minimum 74 cykli zegara przy CS = HIGH */
  for (uint8_t i = 0U; i < 10U; i++) {
    SD_SPI_Transfer(SD_DUMMY_BYTE);
  }

  SD_Select();

  /* CMD0: wejście karty w tryb SPI */
  response = SD_SendCommand(CMD0, 0U);

  if (response != 0x01U) {
    SD_Deselect();
    return STA_NOINIT;
  }

  /* CMD8: sprawdzenie, czy to karta SD w wersji 2.0 */
  response = SD_SendCommand(CMD8, 0x000001AAU);

  if (response == 0x01U) {
    /* RCA, unused, voltage range, check pattern */
    for (uint8_t i = 0U; i < 4U; i++) {
      ocr[i] = SD_SPI_Transfer(SD_DUMMY_BYTE);
    }

    if ((ocr[2] == 0x01U) && (ocr[3] == 0xAAU)) {
      start = HAL_GetTick();

      do {
        response = SD_SendApplicationCommand(ACMD41, 1UL << 30U);

        if ((HAL_GetTick() - start) > SD_INIT_TIMEOUT_MS) {
          SD_Deselect();
          return STA_NOINIT;
        }
      } while (response != 0U);

      /* CMD58: sprawdzenie CCS w OCR; 1 oznacza SDHC/SDXC */
      if (SD_SendCommand(CMD58, 0U) == 0U) {
        for (uint8_t i = 0U; i < 4U; i++) {
          ocr[i] = SD_SPI_Transfer(SD_DUMMY_BYTE);
        }

        if ((ocr[0] & 0x40U) != 0U) {
          sd_card_type = CT_SD2 | CT_SDHC | CT_BLOCK;
        } else {
          sd_card_type = CT_SD2;
        }
      }
    }
  } else {
    /* Starsze karty SD v1 lub MMC */
    response = SD_SendCommand(CMD58, 0U);

    if ((response & 0xFDU) == 0U) {
      sd_card_type = CT_SD1;
    }

    start = HAL_GetTick();

    do {
      response = SD_SendApplicationCommand(ACMD41, 0U);

      if ((HAL_GetTick() - start) > SD_INIT_TIMEOUT_MS) {
        SD_Deselect();
        return STA_NOINIT;
      }
    } while (response != 0U);

    /* Ustaw długość bloku na 512 B */
    if (SD_SendCommand(CMD16, SD_BLOCK_SIZE) != 0U) {
      sd_card_type = 0U;
    }
  }

  SD_Deselect();

  if (sd_card_type == 0U) {
    return STA_NOINIT;
  }

  return 0U;
}

/* ------------------------------------------------------------
 * Odczyt i zapis sektorów
 * ------------------------------------------------------------ */

static DRESULT SD_ReadSingleBlock(BYTE *buffer, DWORD sector)
{
  uint8_t response;
  uint32_t start;
  uint32_t address = sector;

  /* SDHC/SDXC używa numerów bloków, starsze karty adresu bajtowego */
  if ((sd_card_type & CT_BLOCK) == 0U) {
    address *= SD_BLOCK_SIZE;
  }

  if (SD_SendCommand(CMD17, address) != 0U) {
    return RES_ERROR;
  }

  /* Czekaj na token początku danych: 0xFE */
  start = HAL_GetTick();

  do {
    response = SD_SPI_Transfer(SD_DUMMY_BYTE);

    if ((HAL_GetTick() - start) > SD_READY_TIMEOUT_MS) {
      return RES_ERROR;
    }
  } while (response != 0xFEU);

  for (uint16_t i = 0U; i < SD_BLOCK_SIZE; i++) {
    buffer[i] = SD_SPI_Transfer(SD_DUMMY_BYTE);
  }

  /* CRC można pominąć w tym prostym driverze */
  SD_SPI_Transfer(SD_DUMMY_BYTE);
  SD_SPI_Transfer(SD_DUMMY_BYTE);

  return RES_OK;
}

static DRESULT SD_WriteSingleBlock(const BYTE *buffer, DWORD sector)
{
  uint8_t response;
  uint32_t address = sector;

  if ((sd_card_type & CT_BLOCK) == 0U) {
    address *= SD_BLOCK_SIZE;
  }

  if (SD_SendCommand(CMD24, address) != 0U) {
    return RES_ERROR;
  }

  /* Token początku zapisu pojedynczego bloku */
  SD_SPI_Transfer(0xFEU);

  for (uint16_t i = 0U; i < SD_BLOCK_SIZE; i++) {
    SD_SPI_Transfer(buffer[i]);
  }

  /* Dummy CRC */
  SD_SPI_Transfer(SD_DUMMY_BYTE);
  SD_SPI_Transfer(SD_DUMMY_BYTE);

  response = SD_SPI_Transfer(SD_DUMMY_BYTE);

  /* Karta powinna potwierdzić przyjęcie danych */
  if ((response & 0x1FU) != 0x05U) {
    return RES_ERROR;
  }

  if (SD_WaitReady(SD_READY_TIMEOUT_MS) != 0U) {
    return RES_ERROR;
  }

  return RES_OK;
}

/* ------------------------------------------------------------
 * Interfejs wymagany przez user_diskio.c
 * ------------------------------------------------------------ */

DSTATUS SD_disk_initialize(BYTE pdrv)
{
  if (pdrv != 0U) {
    return STA_NOINIT;
  }

  sd_status = STA_NOINIT;

  if (SD_CardInit() == 0U) {
    sd_status &= ~STA_NOINIT;
  }

  return sd_status;
}

DSTATUS SD_disk_status(BYTE pdrv)
{
  if (pdrv != 0U) {
    return STA_NOINIT;
  }

  return sd_status;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
  if ((pdrv != 0U) || (count == 0U)) {
    return RES_PARERR;
  }

  if ((sd_status & STA_NOINIT) != 0U) {
    return RES_NOTRDY;
  }

  SD_Select();

  for (UINT i = 0U; i < count; i++) {
    if (SD_ReadSingleBlock(&buff[i * SD_BLOCK_SIZE], sector + i) != RES_OK) {
      SD_Deselect();
      return RES_ERROR;
    }
  }

  SD_Deselect();

  return RES_OK;
}

#if _USE_WRITE == 1
DRESULT SD_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
  if ((pdrv != 0U) || (count == 0U)) {
    return RES_PARERR;
  }

  if ((sd_status & STA_NOINIT) != 0U) {
    return RES_NOTRDY;
  }

  SD_Select();

  for (UINT i = 0U; i < count; i++) {
    if (SD_WriteSingleBlock(&buff[i * SD_BLOCK_SIZE], sector + i) != RES_OK) {
      SD_Deselect();
      return RES_ERROR;
    }
  }

  SD_Deselect();

  return RES_OK;
}
#endif

#if _USE_IOCTL == 1
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  DRESULT result = RES_ERROR;

  if (pdrv != 0U) {
    return RES_PARERR;
  }

  if ((sd_status & STA_NOINIT) != 0U) {
    return RES_NOTRDY;
  }

  switch (cmd) {
    case CTRL_SYNC:
      SD_Select();

      if (SD_WaitReady(SD_READY_TIMEOUT_MS) == 0U) {
        result = RES_OK;
      }

      SD_Deselect();
      break;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = SD_BLOCK_SIZE;
      result = RES_OK;
      break;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1U;
      result = RES_OK;
      break;

    case GET_SECTOR_COUNT:
      /*
       * Bez odczytu i parsowania rejestru CSD nie znamy pojemności karty.
       * Dla prostego loggera można zostawić 0; FatFs i tak odczyta
       * geometrię z tablicy partycji / boot sectora.
       */
      *(DWORD *)buff = 0U;
      result = RES_OK;
      break;

    default:
      result = RES_PARERR;
      break;
  }

  return result;
}
#endif
