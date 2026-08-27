#include "sdcard.h"
#include <string.h>

FRESULT SDCard_Mount(SDCard_Handle_t *hsd)
{
    if (!hsd) return FR_INVALID_OBJECT;

    FRESULT res = f_mount(&hsd->fs, "", 1);
    hsd->mounted = (res == FR_OK);
    return res;
}

FRESULT SDCard_OpenAppend(SDCard_Handle_t *hsd, const char *file_path)
{
    if (!hsd || !file_path) return FR_INVALID_OBJECT;

    strncpy(hsd->path, file_path, sizeof(hsd->path) - 1);
    hsd->path[sizeof(hsd->path) - 1] = '\0';

    if (hsd->file_open)
    {
        f_close(&hsd->file);
        hsd->file_open = 0;
    }

    FRESULT res = f_open(&hsd->file, hsd->path, FA_WRITE | FA_OPEN_APPEND);
    if (res == FR_OK)
        hsd->file_open = 1;

    return res;
}

FRESULT SDCard_AppendLine(SDCard_Handle_t *hsd, const char *line)
{
    if (!hsd || !line || !hsd->file_open) return FR_INVALID_OBJECT;

    UINT bw = 0;
    FRESULT res = f_puts(line, &hsd->file) < 0 ? FR_DISK_ERR : FR_OK;

    if (res == FR_OK)
        res = f_puts("\r\n", &hsd->file) < 0 ? FR_DISK_ERR : FR_OK;

    if (res == FR_OK)
        res = f_sync(&hsd->file);

    return res;
}

FRESULT SDCard_Close(SDCard_Handle_t *hsd)
{
    if (!hsd) return FR_INVALID_OBJECT;

    if (hsd->file_open)
    {
        FRESULT res = f_close(&hsd->file);
        hsd->file_open = 0;
        return res;
    }

    return FR_OK;
}

FRESULT SDCard_Unmount(SDCard_Handle_t *hsd)
{
    if (!hsd) return FR_INVALID_OBJECT;

    SDCard_Close(hsd);
    FRESULT res = f_mount(NULL, "", 1);
    hsd->mounted = 0;
    return res;
}

FRESULT SDCard_Init(SDCard_Handle_t *hsd, const char *file_path)
{
    if (!hsd || !file_path) return FR_INVALID_OBJECT;

    memset(hsd, 0, sizeof(*hsd));

    FRESULT res = SDCard_Mount(hsd);
    if (res != FR_OK)
        return res;

    return SDCard_OpenAppend(hsd, file_path);
}
