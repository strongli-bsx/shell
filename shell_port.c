/**
 * ********************************************************
 * @file      shell_port.c
 * @brief     shell example realize
 * @version   1.0
 * @author    awesome
 * @copyright (c) 2025, AWESOME
 * ********************************************************
 * @note      revision note
 * |   Date    |  version  |  author   | Description     |
 * |2025-10-28 |    1.1    |  awesome  | init version    |
 * |2025-11-03 |    5.1    |  awesome  | add weak attr   |
 * |2026-01-27 |    1.2    |  Awesome  | add process bar |
 * ********************************************************
 */
#include "shell_port.h"
#include "../src/uart_lite.h"
#include "xuartps.h"

shell_t shell;
uint8_t shell_buffer[512];

__attribute__((weak)) int uart_send_byte(uint8_t *data, uint32_t size)
{
    return 0;
}

__attribute__((weak)) int uart_receive(uint8_t *data, uint32_t size)
{
    return 0;
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

int func(int argc, char *argv[])
{
    printf("%d parameter(s)\r\n", argc - 1);
    for (char i = 1; i < argc; i++)
    {
        printf("%s\r\n", argv[i]);
    }
    return 0;
}

/*
 * @param 1 attr
 * @param 2 name
 * @param 3 func
 * @param 4 desc
 * @param ...
 */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE (SHELL_TYPE_CMD_MAIN),
                 func, func, test);
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
    osal_usleep(100000);
  }
  printf("\n\r");
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
  SHELL_TYPE_CMD_FUNC), pro, process_bar, process bar);
