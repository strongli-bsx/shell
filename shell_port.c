/**
 * ********************************************************
 * @file      shell_port.c
 * @brief     shell example realize
 * @version   1.0
 * @author    awesome
 * @copyright (c) 2025, AWESOME
 * **************************************************************
 * @note      revision note
 * |   Date    |  version  |  author   | Description         |
 * |2025-10-28 |    1.1    |  awesome  | init version        |
 * |2025-11-03 |    5.1    |  awesome  | add weak attr       | 
 * |2026-01-27 |    1.2    |  Awesome  | add process bar     |
 * |2026-09-09 |    1.3    |  Awesome  | rename shell_create |
 * **************************************************************
 */
#include "shell_port.h"
#include "xuartps_hw.h"
#include "xparameters.h"
// #include <unistd.h>
#include <stdio.h>
#include <string.h>


__attribute__((weak)) int uart_send_byte(char *data, int size)
{
    for (int i = 0; i < size; i++) {
        while (!XUartPs_IsTransmitFifoEmpty(STDOUT_BASEADDRESS)) { }
        XUartPs_SendByte(STDOUT_BASEADDRESS, (unsigned char)data[i]);
    }
    return size;
}

__attribute__((weak)) int uart_receive(char *data, int size)
{
    /* shell_task 每次调用 size == 1，非阻塞读单字节 */
    (void)size;
    if (!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) {
        return 0;
    }
    *data = (char)XUartPs_RecvByte(STDIN_BASEADDRESS);
    return 1;
}

signed short shell_write(char *data, unsigned short size)
{
    return (signed short)uart_send_byte(data, size);
}

signed short shell_read(char *data, unsigned short size)
{
    return (signed short)uart_receive(data, size);
}

void shell_create(shell_t *shell, char *buffer, unsigned short size)
{
    shell->write = shell_write;
    shell->read = shell_read;
    shell_init(shell, buffer, size);
}

int func(int argc, char *argv[])
{
    printf("%d parameter(s)\r\n", argc - 1);
    for (int i = 1; i < argc; i++)
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
    // fflush(stdout);
    // osal_usleep(100000);
    // sleep(1);
    for (volatile int d = 0; d < 2000000; d++) { __asm__ volatile("nop"); }
  }
  printf("\n\r");
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
  SHELL_TYPE_CMD_FUNC), pro, process_bar, process bar);
