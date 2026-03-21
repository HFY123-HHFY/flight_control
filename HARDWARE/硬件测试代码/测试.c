//测试代码：
if(KEY0 == 1)
{
    LED0 = 1;
}
if (KEY1 == 1)
{
    LED1 = 0;
    Buzzer = 0;
}

if (KEY2 == 1)
{
    LED2 = 0;
}
if (KEY3 == 1)
{
    LED0 = 0;
    LED1 = 1;
    LED2 = 1;
    Buzzer = 1;
}

OLED_Clear();
OLED_Printf(0,0,OLED_8X16, "HELLO WORLD");
OLED_Update();

usart_printf(USART1, "Hello USART1!\r\n");
usart_printf(USART2, "Hello USART2!\r\n");
usart_printf(USART3, "Hello USART3!\r\n");
