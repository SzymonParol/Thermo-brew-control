#ifndef CONFIG_H_
#define CONFIG_H_

#include "main.h"
#include <stdint.h>

#define CONFIG_MAX_COUNT      8U
#define CONFIG_NAME_LENGTH    32U

#define CONFIG_DIR            "0:/CONFIGS"
#define CONFIG_EXTENSION      ".CFG"

typedef struct
{
    char name[CONFIG_NAME_LENGTH];
    float setpoint;
} BrewConfig_t;

extern BrewConfig_t config_list[CONFIG_MAX_COUNT];
extern uint8_t config_count;
extern BrewConfig_t active_config;

void Config_Init(void);
void Config_RefreshList(void);
uint8_t Config_Load(uint8_t index);
uint8_t Config_SaveNew(void);

#endif /* CONFIG_H_ */
