/**
 * @file shell_ext.h
 * @author Letter (NevermindZZT@gmail.com)
 * @brief shell extensions
 * @version 3.0.0
 * @date 2019-12-31
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#ifndef __SHELL_EXT_H__
#define __SHELL_EXT_H__

#include "shell.h"
#include <stddef.h>

/**
 * @brief 数字类型
 * 
 */
typedef enum
{
    NUM_TYPE_DEC,                                           /**< 十进制整型 */
    NUM_TYPE_BIN,                                           /**< 二进制整型 */
    NUM_TYPE_OCT,                                           /**< 八进制整型 */
    NUM_TYPE_HEX,                                           /**< 十六进制整型 */
    NUM_TYPE_FLOAT                                          /**< 浮点型 */
} SHELL_NUM_TYPE_E;

int shell_ext_run(shell_t *shell, shell_cmd_t *command, int argc, char *argv[]);

#endif
