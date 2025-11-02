/**
 * @file shell_ext.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief shell extensions
 * @version 3.0.0
 * @date 2019-12-31
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#include "shell_cfg.h"
#include "shell.h"
#include "shell_ext.h"
#include "string.h"

/**
 * @brief 判断数字进制
 * 
 * @param string 参数字符串
 * @return SHELL_NUM_TYPE_E 进制
 */
static SHELL_NUM_TYPE_E shell_ext_num_type(char *string)
{
    char *p = string;
    SHELL_NUM_TYPE_E type = NUM_TYPE_DEC;

    if ((*p == '0') && ((*(p + 1) == 'x') || (*(p + 1) == 'X')))
    {
        type = NUM_TYPE_HEX;
    }
    else if ((*p == '0') && ((*(p + 1) == 'b') || (*(p + 1) == 'B')))
    {
        type = NUM_TYPE_BIN;
    }
    else if (*p == '0')
    {
        type = NUM_TYPE_OCT;
    }
    
    while (*p++)
    {
        if (*p == '.' && *(p + 1) != 0)
        {
            type = NUM_TYPE_FLOAT;
            break;
        }
    }

    return type;
}


/**
 * @brief 字符转数字
 * 
 * @param code 字符
 * @return char 数字
 */
static char shell_ext_char_to_num(char code)
{
    if ((code >= '0') && (code <= '9'))
    {
        return code -'0';
    }
    else if ((code >= 'a') && (code <= 'f'))
    {
        return code - 'a' + 10;
    }
    else if ((code >= 'A') && (code <= 'F'))
    {
        return code - 'A' + 10;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief 解析字符参数
 * 
 * @param string 字符串参数
 * @return char 解析出的字符
 */
static char shell_ext_parse_char(char *string)
{
    char *p = (*string == '\'') ? (string + 1) : string;
    char value = 0;

    if (*p == '\\')
    {
        switch (*(p + 1))
        {
        case 'b':
            value = '\b';
            break;
        case 'r':
            value = '\r';
            break;
        case 'n':
            value = '\n';
            break;
        case 't':
            value = '\t';
            break;
        case '0':
            value = 0;
            break;
        default:
            value = *(p + 1);
            break;
        }
    }
    else
    {
        value = *p;
    }
    return value;
}


/**
 * @brief 解析字符串参数
 * 
 * @param string 字符串参数
 * @return char* 解析出的字符串
 */
static char* shell_ext_parse_string(char *string)
{
    char *p = string;
    unsigned short index = 0;

    if (*string == '\"')
    {
        p = ++string;
    }

    while (*p)
    {
        if (*p == '\\')
        {
            *(string + index) = shell_ext_parse_char(p);
            p++;
        }
        else if (*p == '\"')
        {
            *(string + index) = 0;
        }
        else
        {
            *(string + index) = *p;
        }
        p++;
        index ++;
    }
    *(string + index) = 0;
    return string;
}


/**
 * @brief 解析数字参数
 * 
 * @param string 字符串参数
 * @return size_t 解析出的数字
 */
static size_t shell_ext_parse_number(char *string)
{
    SHELL_NUM_TYPE_E type = NUM_TYPE_DEC;
    char radix = 10;
    char *p = string;
    char offset = 0;
    signed char sign = 1;
    size_t value_int = 0;
    float value_float = 0.0;
    size_t devide = 0;

    if (*string == '-')
    {
        sign = -1;
    }

    type = shell_ext_num_type(string + ((sign == -1) ? 1 : 0));

    switch ((char)type)
    {
    case NUM_TYPE_HEX:
        radix = 16;
        offset = 2;
        break;
    
    case NUM_TYPE_OCT:
        radix = 8;
        offset = 1;
        break;

    case NUM_TYPE_BIN:
        radix = 2;
        offset = 2;
        break;
    
    default:
        break;
    }

    p = string + offset + ((sign == -1) ? 1 : 0);

    while (*p)
    {
        if (*p == '.')
        {
            devide = 1;
            p++;
            continue;
        }
        value_int = value_int * radix + shell_ext_char_to_num(*p);
        devide *= 10;
        p++;
    }
    if (type == NUM_TYPE_FLOAT && devide != 0)
    {
        value_float = (float)value_int / devide * sign;
        return *(size_t *)(&value_float);
    }
    else
    {
        return value_int * sign;
    }
}


/**
 * @brief 解析变量参数
 * 
 * @param shell shell对象
 * @param var 变量
 * @param result 解析结果
 *
 * @return int 0 解析成功 --1 解析失败
 */
static int shell_ext_parse_var(shell_t *shell, char *var, size_t *result)
{
    shell_cmd_t *command = shell_seek_cmd(shell,
                                             var + 1,
                                             shell->command_list.base,
                                             0);
    if (command)
    {
        *result = shell_get_var_value(shell, command);
        return 0;
    }
    else
    {
        return -1;
    }
}


/**
 * @brief 解析参数
 * 
 * @param shell shell对象
 * @param string 参数
 * @param type 参数类型
 * @param result 解析结果
 * 
 * @return int 0 解析成功 --1 解析失败
 */
static int shell_ext_parse_para(shell_t *shell, char *string, char *type, size_t *result)
{
    if (type == NULL || (*string == '$' && *(string + 1)))
    {
        if (*string == '\'' && *(string + 1))
        {
            *result = (size_t)shell_ext_parse_char(string);
            return 0;
        }
        else if (*string == '-' || (*string >= '0' && *string <= '9'))
        {
            *result = shell_ext_parse_number(string);
            return 0;
        }
        else if (*string == '$' && *(string + 1))
        {
            return shell_ext_parse_var(shell, string, result);
        }
        else if (*string)
        {
            *result = (size_t)shell_ext_parse_string(string);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief 执行命令
 * 
 * @param shell shell对象
 * @param command 命令
 * @param argc 参数个数
 * @param argv 参数
 * @return int 返回值
 */
int shell_ext_run(shell_t *shell, shell_cmd_t *command, int argc, char *argv[])
{
    int ret = 0;
    size_t params[SHELL_PARAMETER_MAX_NUMBER] = {0};
    int param_number = command->attr.para.param_num > (argc - 1) ? 
        command->attr.para.param_num : (argc - 1);
    int i;
    for ( i = 0; i < argc - 1; i++)
    {
        {
            if (shell_ext_parse_para(shell, argv[i + 1], NULL, &params[i]) != 0)
            {
                return -1;
            }
        }
    }
    switch (param_number)
    {
#if SHELL_PARAMETER_MAX_NUMBER >= 1
    case 0:
        ret = command->data.cmd.function();
        break;
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 1 */
#if SHELL_PARAMETER_MAX_NUMBER >= 2
    case 1:
    {
        int (*func)(size_t) = command->data.cmd.function;
        ret = func(params[0]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 2 */
#if SHELL_PARAMETER_MAX_NUMBER >= 3
    case 2:
    {
        int (*func)(size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 3 */
#if SHELL_PARAMETER_MAX_NUMBER >= 4
    case 3:
    {
        int (*func)(size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 4 */
#if SHELL_PARAMETER_MAX_NUMBER >= 5
    case 4:
    {
        int (*func)(size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 5 */
#if SHELL_PARAMETER_MAX_NUMBER >= 6
    case 5:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 6 */
#if SHELL_PARAMETER_MAX_NUMBER >= 7
    case 6:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 7 */
#if SHELL_PARAMETER_MAX_NUMBER >= 8
    case 7:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 8 */
#if SHELL_PARAMETER_MAX_NUMBER >= 9
    case 8:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t)
            = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 9 */
#if SHELL_PARAMETER_MAX_NUMBER >= 10
    case 9:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 10 */
#if SHELL_PARAMETER_MAX_NUMBER >= 11
    case 10:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 11 */
#if SHELL_PARAMETER_MAX_NUMBER >= 12
    case 11:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 12 */
#if SHELL_PARAMETER_MAX_NUMBER >= 13
    case 12:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 13 */
#if SHELL_PARAMETER_MAX_NUMBER >= 14
    case 13:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 14 */
#if SHELL_PARAMETER_MAX_NUMBER >= 15
    case 14:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t, size_t)
            = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12], params[13]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 15 */
#if SHELL_PARAMETER_MAX_NUMBER >= 16
    case 15:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t, size_t, size_t)
            = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12], params[13], params[14]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 16 */
    default:
        ret = -1;
        break;
    }
    
    return ret;
}

