#include "ili9341.h"
#include "spi.h"

#define ILI9341_SWRESET  0x01
#define ILI9341_SLPOUT   0x11
#define ILI9341_DISPON   0x29
#define ILI9341_CASET    0x2A
#define ILI9341_PASET    0x2B
#define ILI9341_RAMWR    0x2C
#define ILI9341_MADCTL  0x36
#define ILI9341_COLMOD  0x3A

static void LCD_Select(void)
{
    HAL_GPIO_WritePin(
        LCD_CS_GPIO_Port,
        LCD_CS_Pin,
        GPIO_PIN_RESET
    );
}

static void LCD_Unselect(void)
{
    HAL_GPIO_WritePin(
        LCD_CS_GPIO_Port,
        LCD_CS_Pin,
        GPIO_PIN_SET
    );
}

static void LCD_CommandMode(void)
{
    HAL_GPIO_WritePin(
        LCD_DC_GPIO_Port,
        LCD_DC_Pin,
        GPIO_PIN_RESET
    );
}

static void LCD_DataMode(void)
{
    HAL_GPIO_WritePin(
        LCD_DC_GPIO_Port,
        LCD_DC_Pin,
        GPIO_PIN_SET
    );
}

static void LCD_WriteCommand(
    uint8_t command
)
{
    LCD_Select();
    LCD_CommandMode();

    HAL_SPI_Transmit(
        &hspi3,
        &command,
        1,
        1000
    );

    LCD_Unselect();
}

static void LCD_WriteData(
    uint8_t *data,
    uint16_t length
)
{
    LCD_Select();
    LCD_DataMode();

    HAL_SPI_Transmit(
        &hspi3,
        data,
        length,
        1000
    );

    LCD_Unselect();
}

static void LCD_Reset(void)
{
    HAL_GPIO_WritePin(
        LCD_RST_GPIO_Port,
        LCD_RST_Pin,
        GPIO_PIN_RESET
    );

    HAL_Delay(20);

    HAL_GPIO_WritePin(
        LCD_RST_GPIO_Port,
        LCD_RST_Pin,
        GPIO_PIN_SET
    );

    HAL_Delay(120);
}

static void ILI9341_SetAddressWindow(
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2
)
{
    uint8_t data[4];

    LCD_WriteCommand(ILI9341_CASET);

    data[0] = x1 >> 8;
    data[1] = x1 & 0xFF;
    data[2] = x2 >> 8;
    data[3] = x2 & 0xFF;

    LCD_WriteData(data, 4);

    LCD_WriteCommand(ILI9341_PASET);

    data[0] = y1 >> 8;
    data[1] = y1 & 0xFF;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xFF;

    LCD_WriteData(data, 4);

    LCD_WriteCommand(ILI9341_RAMWR);
}

void ILI9341_Init(void)
{
    HAL_GPIO_WritePin(
        LCD_CS_GPIO_Port,
        LCD_CS_Pin,
        GPIO_PIN_SET
    );

    HAL_GPIO_WritePin(
        LCD_DC_GPIO_Port,
        LCD_DC_Pin,
        GPIO_PIN_SET
    );

    HAL_GPIO_WritePin(
        LCD_RST_GPIO_Port,
        LCD_RST_Pin,
        GPIO_PIN_SET
    );

    LCD_Reset();

    LCD_WriteCommand(ILI9341_SWRESET);
    HAL_Delay(150);

    LCD_WriteCommand(ILI9341_SLPOUT);
    HAL_Delay(120);

    uint8_t color_mode = 0x55;

    LCD_WriteCommand(ILI9341_COLMOD);
    LCD_WriteData(&color_mode, 1);
    HAL_Delay(10);

    uint8_t memory_access = 0x48;

    LCD_WriteCommand(ILI9341_MADCTL);
    LCD_WriteData(&memory_access, 1);

    LCD_WriteCommand(ILI9341_DISPON);
    HAL_Delay(100);

    ILI9341_FillScreen(ILI9341_BLACK);
}

void ILI9341_SetRotation(
    uint8_t rotation
)
{
    uint8_t memory_access;

    switch (rotation % 4)
    {
        case 0:
            memory_access = 0x48;
            break;

        case 1:
            memory_access = 0x28;
            break;

        case 2:
            memory_access = 0x88;
            break;

        default:
            memory_access = 0xE8;
            break;
    }

    LCD_WriteCommand(ILI9341_MADCTL);
    LCD_WriteData(&memory_access, 1);
}

void ILI9341_DrawPixel(
    uint16_t x,
    uint16_t y,
    uint16_t color
)
{
    uint8_t data[2];

    if (x >= ILI9341_WIDTH ||
        y >= ILI9341_HEIGHT)
    {
        return;
    }

    ILI9341_SetAddressWindow(
        x,
        y,
        x,
        y
    );

    data[0] = color >> 8;
    data[1] = color & 0xFF;

    LCD_WriteData(data, 2);
}

void ILI9341_FillScreen(
    uint16_t color
)
{
    uint8_t data[2];

    data[0] = color >> 8;
    data[1] = color & 0xFF;

    ILI9341_SetAddressWindow(
        0,
        0,
        ILI9341_WIDTH - 1,
        ILI9341_HEIGHT - 1
    );

    LCD_Select();
    LCD_DataMode();

    for (uint32_t i = 0;
         i < (uint32_t)ILI9341_WIDTH *
             ILI9341_HEIGHT;
         i++)
    {
        HAL_SPI_Transmit(
            &hspi3,
            data,
            2,
            1000
        );
    }

    LCD_Unselect();
}
