/**
 * ********************************************************
 * @file      shell_port.c
 * @brief     shell example realize
 * @version   1.0
 * @author    awesome
 * @copyright (c) 2025, AWESOME
 * ********************************************************
 * @note      revision note
 * |   Date    |  version  |  author   | Description  |
 * |2025-10-28 |    1.1    |  awesome  | init version |
 * |2025-11-03 |    5.1    |  awesome  | add weak attr|
 * ********************************************************
 */
#include "zshell_port.h"
#include "../Bsp/usart/bsp_debug_usart.h"

shell_t shell;
uint8_t shell_buffer[512];

__attribute__((weak)) int uart_send_byte(uint8_t *data, uint32_t size)
{
    for(int i = 0; i < size; ++i)
    {
        USART_SendData(USART1, data[i]);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {
        }
    }
    return size;
}

__attribute__((weak)) int uart_receive(uint8_t *data, uint32_t size)
{
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {
        return 0;
    }
    *data = USART_ReceiveData(USART1);
    return 1;
}

int shell_write(uint8_t *data, uint16_t size)
{
    return uart_send_byte(data, size);
}

int shell_read(uint8_t *data, uint16_t size)
{
    return uart_receive(data, size);
}

void init_shell(void)
{
    shell.write = shell_write;
    shell.read = shell_read;
    shell_init(&shell, shell_buffer, 512);
}

int func(int b, int a)
{
    // printf("%d parameter(s)\r\n", argc - 1);
    // for (char i = 1; i < argc; i++)
    // {
    //     printf("%s\r\n", argv[i]);
    // }
    printf("%d = %d + %d\r\n", b + a, b, a);
    return 0;
}

/*
 * @param 1 attr
 * @param 2 name
 * @param 3 func
 * @param 4 desc
 * @param ...
 */
CMD_REGISTER(SHELL_CMD_ENABLE_RETURN, func, func, test);
int process_bar(void) {
  int width = 50;  // 
  char bar[51] = { 0 };

  for (int i = 0; i <= 100; i++) {
    memset(bar, ' ', width);
    bar[width] = '\0';

    int fill_length = (i * width) / 100;

    for (int j = 0; j < fill_length; j++) {
      bar[j] = '#';
    }

    if (fill_length > 0 && (fill_length + 1) < width) {
      bar[fill_length] = '-';
      bar[fill_length + 1] = '>';
    }

    printf("\r[%s] %3d%%", bar, i);
    fflush(stdout);
    osal_usleep(1000);
  }
  printf("\n\r");
  return 10;
}
CMD_REGISTER(SHELL_CMD_ENABLE_RETURN, pro, process_bar, process bar);
