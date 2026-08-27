#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

#include "main.h"
#include "fatfs.h"

typedef struct
{
    FATFS fs;
    FIL file;
    char path[64];
    uint8_t mounted;
    uint8_t file_open;
} SDCard_Handle_t;

FRESULT SDCard_Init(SDCard_Handle_t *hsd, const char *file_path);
FRESULT SDCard_Mount(SDCard_Handle_t *hsd);
FRESULT SDCard_OpenAppend(SDCard_Handle_t *hsd, const char *file_path);
FRESULT SDCard_AppendLine(SDCard_Handle_t *hsd, const char *line);
FRESULT SDCard_Close(SDCard_Handle_t *hsd);
FRESULT SDCard_Unmount(SDCard_Handle_t *hsd);

#endif
