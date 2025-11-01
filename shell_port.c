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
 * ********************************************************
 */
#include "shell_port.h"
#include "../src/uart_lite.h"
#include "xuartps.h"

shell_t shell;
uint8_t shell_buffer[512];
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
