#include "config.h"
#include "system.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

BrewConfig_t config_list[CONFIG_MAX_COUNT];
uint8_t config_count = 0U;
BrewConfig_t active_config;

static FATFS config_fs;
static uint8_t config_mounted = 0U;

static uint8_t Config_Mount(void)
{
    FRESULT result;

    if (config_mounted != 0U)
    {
        return 1U;
    }

    result = f_mount(&config_fs, "0:/", 1U);

    if (result == FR_OK)
    {
        config_mounted = 1U;
        return 1U;
    }

    return 0U;
}

static uint8_t Config_IsSdWorking(void)
{
    if (Config_Mount() == 0U)
    {
        return 0U;
    }

    return 1U;
}

static void Config_GetPath(const char *name, char *path, uint16_t size)
{
    snprintf(path, size, "%s/%s", CONFIG_DIR, name);
}

static uint8_t Config_IsConfigFile(const char *name)
{
    uint16_t length = (uint16_t)strlen(name);

    if (length < 5U)
    {
        return 0U;
    }

    if (strcmp(&name[length - 4U], CONFIG_EXTENSION) == 0)
    {
        return 1U;
    }

    return 0U;
}

void Config_Init(void)
{
    config_count = 0U;
    config_mounted = 0U;

    memset(config_list, 0, sizeof(config_list));

    active_config.name[0] = '\0';
    active_config.setpoint = 68.0f;
}

void Config_RefreshList(void)
{
    DIR directory;
    FILINFO file_info;
    FRESULT result;

    config_count = 0U;
    memset(config_list, 0, sizeof(config_list));

    if (Config_Mount() == 0U)
    {
        return;
    }

    result = f_opendir(&directory, CONFIG_DIR);

    if (result == FR_NO_PATH)
    {
        result = f_mkdir(CONFIG_DIR);

        if (result != FR_OK)
        {
            return;
        }

        result = f_opendir(&directory, CONFIG_DIR);
    }

    if (result != FR_OK)
    {
        return;
    }

    while ((config_count < CONFIG_MAX_COUNT) &&
           (f_readdir(&directory, &file_info) == FR_OK) &&
           (file_info.fname[0] != '\0'))
    {
        if (((file_info.fattrib & AM_DIR) == 0U) &&
            (Config_IsConfigFile(file_info.fname) != 0U))
        {
            strncpy(config_list[config_count].name,
                    file_info.fname,
                    CONFIG_NAME_LENGTH - 1U);

            config_list[config_count].name[CONFIG_NAME_LENGTH - 1U] = '\0';
            config_list[config_count].setpoint = 68.0f;

            config_count++;
        }
    }

    f_closedir(&directory);
}

uint8_t Config_Load(uint8_t index)
{
    FIL file;
    FRESULT result;
    char path[96];
    char line[64];

    if (index >= config_count)
    {
        return 0U;
    }

    if (Config_IsSdWorking() == 0U)
    {
        return 2U;   /* kod: karta nie działa */
    }

    Config_GetPath(config_list[index].name, path, sizeof(path));

    result = f_open(&file, path, FA_READ);

    if (result != FR_OK)
    {
        return 0U;
    }

    active_config.setpoint = 68.0f;
    strncpy(active_config.name,
            config_list[index].name,
            CONFIG_NAME_LENGTH - 1U);
    active_config.name[CONFIG_NAME_LENGTH - 1U] = '\0';

    while (f_gets(line, sizeof(line), &file) != NULL)
    {
        if (strncmp(line, "SETPOINT=", 9U) == 0)
        {
            active_config.setpoint = (float)atof(&line[9]);
        }
    }

    f_close(&file);

    setpoint_temp = active_config.setpoint;

    return 1U;
}
uint8_t Config_SaveNew(void)
{
    FIL file;
    FRESULT result;
    char path[96];
    char line[64];
    UINT written = 0U;
    uint8_t index;
    uint8_t number = 1U;

    if (Config_IsSdWorking() == 0U)
    {
        return 2U;
    }

    for (;;)
    {
        snprintf(path, sizeof(path), "%s/NOWA%u%s",
                 CONFIG_DIR,
                 number,
                 CONFIG_EXTENSION);

        result = f_open(&file, path, FA_READ);

        if (result == FR_NO_FILE)
        {
            break;
        }

        if (result == FR_OK)
        {
            f_close(&file);
        }
        else
        {

            return 2U;
        }

        number++;

        if (number > 99U)
        {
            return 0U;
        }
    }

    result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);

    if (result != FR_OK)
    {
        return 0U;
    }

    snprintf(line, sizeof(line), "SETPOINT=%.1f\n", 68.0f);

    result = f_write(&file, line, strlen(line), &written);

    f_close(&file);

    if ((result != FR_OK) || (written == 0U))
    {
        return 0U;
    }

    Config_RefreshList();

    for (index = 0U; index < config_count; index++)
    {
        if (strcmp(config_list[index].name,
                   &path[strlen(CONFIG_DIR) + 1U]) == 0)
        {
            Config_Load(index);
            break;
        }
    }

    return 1U;
}
