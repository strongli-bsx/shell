/**
 * ********************************************************
 * @file      shell_port.c
 * @brief     shell example realize
 * @version   1.0
 * @author    awesome
 * @copyright (c) 2025, AWESOME
 * ************************************************************
 * @note      revision note
 * |   Date    |  version  |  author   | Description        |
 * |2025-10-28 |    1.1    |  awesome  | init version       |
 * |2025-11-03 |    5.1    |  awesome  | add weak attr      |
 * |2026-09-09 |    5.2    |  awesome  | rename shell_create|
 * ************************************************************
 */
#include "zshell_port.h"
#include <stdio.h>
#include <string.h>

/*-------------------------------------------*/
/* uart driver */
#include "xuartps_hw.h"
#include "xparameters.h"
#include <sleep.h>
/*-----------------------------------------*/
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

int test_add_func(int b, int a)
{
    // printf("%d parameter(s)\r\n", argc - 1);
    // for (char i = 1; i < argc; i++)
    // {
    //     printf("%s\r\n", argv[i]);
    // }
    printf("%d = %d + %d\r\n", b + a, b, a);
    return 0;
}
CMD_REGISTER(SHELL_CMD_ENABLE_RETURN, func_add, test_add_func, test demo add);
/*
 * @param 1 attr
 * @param 2 name
 * @param 3 func
 * @param 4 desc
 * @param ...
 */
int process_bar(void) {
  int width = 50;  // 
  char bar[51] = { 0 };

  for (int i = 0; i <= 100; i++) {
    shell_abort_check();
    if (shell_is_aborted()) {
      printf("\n\r");
      return -1;
    }
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
    usleep(10000);
  }
  printf("\n\r");
  return 10;
}
CMD_REGISTER(SHELL_CMD_ENABLE_RETURN, pro, process_bar, process bar);
