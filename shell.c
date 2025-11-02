/**
 * ********************************************************
 * \file      shell.c
 * \brief     shell func realize
 * \version   1.1
 * \author    Letter(NevermindZZT@gmail.com), awesome
 * \copyright (c) Letter, 2020
 * \copyright (c) 2025, AWESOME
 * ********************************************************
 * \note      revision note
 * |   Date    |  version  |  author   | Description
 * |2019-12-30 |    1.0    |  Letter   | init version
 * |2025-10-31 |    1.1    |  awesome  | modify to c style
 * ********************************************************
 */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "shell.h"
#include "shell_ext.h"
#include "shell_cfg.h"

/*-----------------------------------------------------------------------------*/
/*! shell command start address */
extern const unsigned int _shell_command_start;
/*! shell command end address */
extern const unsigned int _shell_command_end;
/*-----------------------------------------------------------------------------*/
/**
 * -----------------------------------------
 *  default user config
 * -----------------------------------------
 * */
const char shell_default_user[] = SHELL_DEFAULT_USER;
const char shell_default_user_password[] = SHELL_DEFAULT_USER_PASSWORD;
const char shell_default_user_desc[] = "default user";
const shell_cmd_t shell_default_user_cmd SHELL_USED SHELL_SECTION("shell_cmd") =
{
    .attr.value = SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_USER),
    .data.user.name = shell_default_user,
    .data.user.pasd = shell_default_user_password,
    .data.user.desc = shell_default_user_desc
};
/*-----------------------------------------------------------------------------*/

/**
 * ------------------
 *  shell text
 * ------------------
 * */
static const char *shell_text[] =
{
    [SHELL_TEXT_INFO] =
        "\r\n"
        " _         _   _                  _          _ _ \r\n"
        "| |    ___| |_| |_ ___ _ __   ___| |__   ___| | |\r\n"
        "| |   / _ \\ __| __/ _ \\ '__| / __| '_ \\ / _ \\ | |\r\n"
        "| |__|  __/ |_| ||  __/ |    \\__ \\ | | |  __/ | |\r\n"
        "|_____\\___|\\__|\\__\\___|_|    |___/_| |_|\\___|_|_|\r\n"
        "\r\n"
        "Build:       " __DATE__ " " __TIME__ "\r\n"
        "Version:     " SHELL_VERSION "\r\n"
        "Copyright:   (c) 2025 Awesome \r\n",
    [SHELL_TEXT_CMD_TOO_LONG] =
        "\r\nWarning: Command is too long\r\n",
    [SHELL_TEXT_CMD_LIST] =
        "\r\nCommand List:\r\n",
    [SHELL_TEXT_VAR_LIST] =
        "\r\nVar List:\r\n",
    [SHELL_TEXT_USER_LIST] =
        "\r\nUser List:\r\n",
    [SHELL_TEXT_KEY_LIST] =
        "\r\nKey List:\r\n",
    [SHELL_TEXT_CMD_NOT_FOUND] =
        "\r\nCommand not Found\r\n",
    [SHELL_TEXT_POINT_CANNOT_MODIFY] =
        "can't set pointer\r\n",
    [SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY] =
        "can't set read only var\r\n",
    [SHELL_TEXT_NOT_VAR] =
        " is not a var\r\n",
    [SHELL_TEXT_VAR_NOT_FOUND] =
        "Var not Fount\r\n",
    [SHELL_TEXT_HELP_HEADER] =
        "command help of ",
    [SHELL_TEXT_PASSWORD_HINT] =
        "Please input password:",
    [SHELL_TEXT_PASSWORD_ERROR] =
        "\r\npassword error\r\n",
    [SHELL_TEXT_CLEAR_CONSOLE] =
        "\033[2J\033[1H",
    [SHELL_TEXT_CLEAR_LINE] =
        "\033[2K\r",
    [SHELL_TEXT_TYPE_CMD] =
        "CMD ",
    [SHELL_TEXT_TYPE_VAR] =
        "VAR ",
    [SHELL_TEXT_TYPE_USER] =
        "USER",
    [SHELL_TEXT_TYPE_KEY] =
        "KEY ",
    [SHELL_TEXT_TYPE_NONE] =
        "NONE",
};
/*-----------------------------------------------------------------------------*/
uint8_t pairedChars[][2] = {
    { '\"', '\"' },
#if SHELL_SUPPORT_ARRAY_PARAM == 1
    { '[', ']' },
#endif /** SHELL_SUPPORT_ARRAY_PARAM == 1 */
    /* {'(', ')'}, */
    /* {'{', '}'}, */
    /* {'<', '>'}, */
    /* {'\'', '\''}, */
    /* {'`', '`'}, */
};
/*-----------------------------------------------------------------------------*/
/*! shell list */
static shell_t *shell_list[SHELL_MAX_NUMBER] = {
    NULL
};
/*-----------------------------------------------------------------------------*/
/*! shell func declaraiton */
static void shell_add(shell_t *shell);
static void shell_write_prompt(shell_t *shell, uint8_t newline);
static void shell_write_return_value(shell_t *shell, int value);
static int shell_show_var(shell_t *shell, shell_cmd_t *command);
static void shell_set_user(shell_t *shell, const shell_cmd_t *user);
static void shell_write_cmd_help(shell_t *shell, char *cmd);
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/*-------------               shell basic function         --------------------*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/**
 * -----------------------------------------------
 * @brief      add shell to shell_list
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_add(shell_t *shell)
{
    for(short i = 0; i < SHELL_MAX_NUMBER; i++) {
        if(shell_list[i] == NULL) {
            shell_list[i] = shell;
            return;
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      remove shell cmd in shell_list
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_remove(shell_t *shell)
{
    for(short i = 0; i < SHELL_MAX_NUMBER; i++) {
        if(shell_list[i] == shell) {
            shell_list[i] = NULL;
            return;
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      get current active shell struct
 * -----------------------------------------------
 * @return     current active shell struct
 * -----------------------------------------------
 */
shell_t *shell_get_current(void)
{
    for(short i = 0; i < SHELL_MAX_NUMBER; i++) {
        if(shell_list[i] && shell_list[i]->status.is_active) {
            return shell_list[i];
        }
    }
    return NULL;
}

/**
 * -----------------------------------------------
 * @brief      write byte to shell
 * -----------------------------------------------
 * @param[in]  shell: shell obj
 * @param[in]  data : char to write
 * -----------------------------------------------
 */
static void shell_write_byte(shell_t *shell, char data)
{
    shell->write(&data, 1);
}

/**
 * -----------------------------------------------
 * @brief      write string to shell
 * -----------------------------------------------
 * @param[in]  shell: shell obj
 * @param[in]  string: string to write
 * @return     num of bytes written
 * -----------------------------------------------
 */
uint16_t shell_write_string(shell_t *shell, const char *string)
{
    uint16_t count = 0;
    const char *p = string;
    
    SHELL_ASSERT(shell->write);

    while(*p++) {
        count++;
    }
    return shell->write((char *)string, count);
}

/**
 * -----------------------------------------------
 * @brief      write cmd desc to shell
 * -----------------------------------------------
 * @param[in]  shell : shell obj
 * @param[in]  string: cmd desc to write
 * @return     num of bytes desc written
 * -----------------------------------------------
 */
static uint16_t shell_write_cmd_desc(shell_t *shell, const char *string)
{
    uint16_t count = 0;
    const char *p = string;
    
    SHELL_ASSERT(shell->write);

    while(*p && *p != '\r' && *p != '\n') {
        p++;
        count++;
    }

    if(count > 36) {
        shell->write((char *)string, 36);
        shell->write("...", 3);
    } else {
        shell->write((char *)string, count);
    }
    return count > 36 ? 36 : 39;
}
/*-----------------------------------------------------------------------------*/
/*----------------            shell export function        --------------------*/
/*-----------------------------------------------------------------------------*/

/**
 * -----------------------------------------------
 * @brief      write prompt to shell, 
 *             include user name, path and "$ "
 * -----------------------------------------------
 * @param[in]  shell : shell obj
 * @param[in]  newline: whether to write a newline
 * -----------------------------------------------
 */
static void shell_write_prompt(shell_t *shell, uint8_t newline)
{
    if(shell->status.is_checked) {
        if(newline) {
            shell_write_string(shell, "\r\n");
        }
        shell_write_string(shell, shell->info.sh_cmd->data.user.name);
        shell_write_string(shell, ":");
        shell_write_string(shell, shell->info.path ? shell->info.path : "~");
        shell_write_string(shell, "$ ");
    } else {
        shell_write_string(shell, shell_text[SHELL_TEXT_PASSWORD_HINT]);
    }
}

#if SHELL_PRINT_BUFFER > 0
/**
 * -----------------------------------------------
 * @brief      shell printf
 * -----------------------------------------------
 * @param[in]  shell: shell obj
 * @param[in]  fmt  : format string
 * -----------------------------------------------
 */
void shell_print(shell_t *shell, const char *fmt, ...)
{
    char buffer[SHELL_PRINT_BUFFER];
    va_list vargs;
    int len;

    SHELL_ASSERT(shell);

    va_start(vargs, fmt);
    len = vsnprintf(buffer, SHELL_PRINT_BUFFER, fmt, vargs);
    va_end(vargs);
    if(len > SHELL_PRINT_BUFFER) {
        len = SHELL_PRINT_BUFFER;
    }
    shell->write(buffer, len);
}

#endif

#if SHELL_SCAN_BUFFER > 0
/**
 * @brief shell格式化输入
 *
 * @param shell shell对象
 * @param fmt 格式化字符串
 * @param ... 参数
 */
void shell_scan(shell_t *shell, char *fmt, ...)
{
    char buffer[SHELL_SCAN_BUFFER];
    va_list vargs;
    short index = 0;

    SHELL_ASSERT(shell);

    if(shell->read) {
        do{
            if(shell->read(&buffer[index], 1) == 1) {
                shell->write(&buffer[index], 1);
                index++;
            }
        } while(buffer[index - 1] != '\r' && buffer[index - 1] != '\n' &&
                index < SHELL_SCAN_BUFFER);
        shell_write_string(shell, "\r\n");
        buffer[index] = '\0';
    }

    va_start(vargs, fmt);
    vsscanf(buffer, fmt, vargs);
    va_end(vargs);
}

#endif

/**
 * -----------------------------------------------
 * @brief      shell check permission
 * -----------------------------------------------
 * @param[in]  shell: shell obj
 * @param[in]  command: shell_cmd_t
 * @return     0: has permission
 * @return     -1: no permission
 * -----------------------------------------------
 */
signed char shell_check_permission(shell_t *shell, shell_cmd_t *command)
{
    return ((!command->attr.para.permission ||
             command->attr.para.type == SHELL_TYPE_USER ||
             (shell->info.sh_cmd &&
              (command->attr.para.permission &
               shell->info.sh_cmd->attr.para.permission))) &&
            (shell->status.is_checked || command->attr.para.enable_unchecked))
               ? 0
               : -1;
}

/**
 * -----------------------------------------------
 * @brief      shell int covert hex
 * @details    convert int to hex string
 * -----------------------------------------------
 * @param[in]  value : int value to convert
 * @param[out] buffer: hex string buffer
 * @return     hex string length
 * -----------------------------------------------
 */
signed char shell_to_hex(unsigned int value, char *buffer)
{
    char byte;
    uint8_t i = 8;
    buffer[8] = 0;
    while(value) {
        byte = value & 0x0000000F;
        buffer[--i] = (byte > 9) ? (byte + 87) : (byte + 48);
        value >>= 4;
    }
    return 8 - i;
}

/**
 * -----------------------------------------------
 * @brief      shell int covert dec
 * @details    convert int to dec string
 * -----------------------------------------------
 * @param[in]  value : int value to convert
 * @param[out] buffer: dec string buffer
 * @return     dec string length
 * -----------------------------------------------
 */
signed char shell_to_dec(int value, char *buffer)
{
    uint8_t i = 11;
    int v = value;
    if(value < 0) {
        v = -value;
    }
    buffer[11] = 0;
    while(v) {
        buffer[--i] = v % 10 + 48;
        v /= 10;
    }
    if(value < 0) {
        buffer[--i] = '-';
    }
    if(value == 0) {
        buffer[--i] = '0';
    }
    return 11 - i;
}

/**
 * -----------------------------------------------
 * @brief      shell string copy
 * @details    copy src string to dest string
 * -----------------------------------------------
 * @param[in]  dest : dest string
 * @param[in]  src  : src string
 * @return     string length
 * -----------------------------------------------
 */
static uint16_t shell_string_copy(char *dest, char *src)
{
    uint16_t count = 0;
    while(*(src + count)) {
        *(dest + count) = *(src + count);
        count++;
    }
    *(dest + count) = 0;
    return count;
}

/**
 * -----------------------------------------------
 * @brief      shell string compare
 * @details    compare dest string with src string
 * -----------------------------------------------
 * @param[in]  dest : dest string
 * @param[in]  src  : src string
 * @return     match length
 * -----------------------------------------------
 */
static uint16_t shell_string_compare(char *dest, char *src)
{
    uint16_t match = 0;
    uint16_t i = 0;

    while(*(dest + i) && *(src + i)) {
        if(*(dest + i) != *(src + i)) {
            break;
        }
        match++;
        i++;
    }
    return match;
}

/**
 * -----------------------------------------------
 * @brief      shell get command name
 * @details    get command name from command struct
 *             if command type is SHELL_TYPE_CMD_FUNC,
 *             return command name,
 *             else if command type is SHELL_TYPE_VAR_NODE,
 *             return variable name,
 *             else if command type is SHELL_TYPE_USER,
 *             return user name,
 *             else return key name
 * -----------------------------------------------
 * @param[in]  command: shell_cmd_t
 * @return     command name
 * -----------------------------------------------
 */
static const char *shell_get_command_name(shell_cmd_t *command)
{
    static char buffer[9];
    for(uint8_t i = 0; i < 9; i++) {
        buffer[i] = '0';
    }
    if(command->attr.para.type <= SHELL_TYPE_CMD_FUNC) {
        return command->data.cmd.name;
    } else if(command->attr.para.type <= SHELL_TYPE_VAR_NODE) {
        return command->data.var.name;
    } else if(command->attr.para.type <= SHELL_TYPE_USER) {
        return command->data.user.name;
    } else {
        shell_to_hex(command->data.key.value, buffer);
        return buffer;
    }
}

/**
 * -----------------------------------------------
 * @brief      shell get command desc
 * @details    get command desc from command struct
 * -----------------------------------------------
 * @param[in]  command: shell_cmd_t
 * @return     command desc
 * -----------------------------------------------
 */
static const char *shell_get_command_desc(shell_cmd_t *command)
{
    if(command->attr.para.type <= SHELL_TYPE_CMD_FUNC) {
        return command->data.cmd.desc;
    } else if(command->attr.para.type <= SHELL_TYPE_VAR_NODE) {
        return command->data.var.desc;
    } else if(command->attr.para.type <= SHELL_TYPE_USER) {
        return command->data.user.desc;
    } else {
        return command->data.key.desc;
    }
}


/**
 * ----------------------------------------------
 * @brief      Initialize shell obj,
 *             and config shell cmd buf and size
 * ----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  buffer: shell cmd buf
 * @param[in]  size  : shell buf size
 * ----------------------------------------------
 */
void shell_init(shell_t *shell, char *buffer, uint16_t size)
{
    /*! shell info init */
    shell->info.sh_cmd = NULL;

    /*! shell parser init */
    shell->parser.length = 0;
    shell->parser.cursor = 0;
    shell->parser.buffer = buffer;
    shell->parser.buffer_size = size / (SHELL_HISTORY_MAX_NUMBER + 1);
    
    /*! shell status init */
    shell->status.is_checked = 1;
    
    /*! shell history init */
    shell->history.offset = 0;
    shell->history.number = 0;
    shell->history.record = 0;
    for(short i = 0; i < SHELL_HISTORY_MAX_NUMBER; i++) {
        shell->history.item[i] = buffer + shell->parser.buffer_size * (i + 1);
    }

    /*! shell command list init */
    shell->command_list.base = (shell_cmd_t *)(&_shell_command_start);
    shell->command_list.count =
        ((size_t)(&_shell_command_end) - (size_t)(&_shell_command_start)) /
        sizeof(shell_cmd_t);

    shell_add(shell);

    shell_set_user(shell, shell_seek_cmd(shell,
                                         SHELL_DEFAULT_USER,
                                         shell->command_list.base,
                                         0));
    shell_write_prompt(shell, 1);
}


/**
 * -----------------------------------------------
 * @brief      shell list item
 * @details    list shell command item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  item  : shell command item
 * -----------------------------------------------
 */
void shell_list_item(shell_t *shell, shell_cmd_t *item)
{
    short spaceLength;

    spaceLength = 22 - shell_write_string(shell, shell_get_command_name(item));
    spaceLength = (spaceLength > 0) ? spaceLength : 4;
    do{
        shell_write_byte(shell, ' ');
    } while(--spaceLength);
    if(item->attr.para.type <= SHELL_TYPE_CMD_FUNC) {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_CMD]);
    } else if(item->attr.para.type <= SHELL_TYPE_VAR_NODE) {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_VAR]);
    } else if(item->attr.para.type <= SHELL_TYPE_USER) {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_USER]);
    } else if(item->attr.para.type <= SHELL_TYPE_KEY) {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_KEY]);
    } else {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_NONE]);
    }
#if SHELL_HELP_SHOW_PERMISSION == 1
    shell_write_string(shell, "  ");
    for(signed char i = 7; i >= 0; i--) {
        shell_write_byte(shell,
                         item->attr.para.permission & (1 << i) ? 'x' : '-');
    }
#endif
    shell_write_string(shell, "  ");
    shell_write_string(shell, shell_get_command_desc(item));
    shell_write_string(shell, "\r\n");
}


/**
 * -----------------------------------------------
 * @brief      shell list command
 * @details    list shell command item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_list_command(shell_t *shell)
{
    shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
    shell_write_string(shell, shell_text[SHELL_TEXT_CMD_LIST]);
    for(short i = 0; i < shell->command_list.count; i++) {
        if(base[i].attr.para.type <= SHELL_TYPE_CMD_FUNC &&
           shell_check_permission(shell, &base[i]) == 0)
        {
            shell_list_item(shell, &base[i]);
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell list var
 * @details    list shell var item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_list_var(shell_t *shell)
{
    shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
    shell_write_string(shell, shell_text[SHELL_TEXT_VAR_LIST]);
    for(short i = 0; i < shell->command_list.count; i++) {
        if(base[i].attr.para.type > SHELL_TYPE_CMD_FUNC &&
           base[i].attr.para.type <= SHELL_TYPE_VAR_NODE &&
           shell_check_permission(shell, &base[i]) == 0)
        {
            shell_list_item(shell, &base[i]);
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell list user
 * @details    list shell user item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_list_user(shell_t *shell)
{
    shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
    shell_write_string(shell, shell_text[SHELL_TEXT_USER_LIST]);
    for(short i = 0; i < shell->command_list.count; i++) {
        if(base[i].attr.para.type > SHELL_TYPE_VAR_NODE &&
           base[i].attr.para.type <= SHELL_TYPE_USER &&
           shell_check_permission(shell, &base[i]) == 0)
        {
            shell_list_item(shell, &base[i]);
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell list key
 * @details    list shell key item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_list_key(shell_t *shell)
{
    shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
    shell_write_string(shell, shell_text[SHELL_TEXT_KEY_LIST]);
    for(short i = 0; i < shell->command_list.count; i++) {
        if(base[i].attr.para.type > SHELL_TYPE_USER &&
           base[i].attr.para.type <= SHELL_TYPE_KEY &&
           shell_check_permission(shell, &base[i]) == 0)
        {
            shell_list_item(shell, &base[i]);
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell list all
 * @details    list shell all item
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_list_all(shell_t *shell)
{
#if SHELL_HELP_LIST_USER == 1
    shell_list_user(shell);
#endif
    shell_list_command(shell);
#if SHELL_HELP_LIST_VAR == 1
    shell_list_var(shell);
#endif
#if SHELL_HELP_LIST_KEY == 1
    shell_list_key(shell);
#endif
}

/**
 * -----------------------------------------------
 * @brief      shell delete command line
 * @details    delete shell command line
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  length : delete length
 * -----------------------------------------------
 */
void shell_delete_command_line(shell_t *shell, uint8_t length)
{
    while(length--) {
        shell_write_string(shell, "\b \b");
    }
}

/**
 * -----------------------------------------------
 * @brief      shell clear command line
 * @details    clear shell command line
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_clear_command_line(shell_t *shell)
{
    for(short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
        shell_write_byte(shell, ' ');
    }
    shell_delete_command_line(shell, shell->parser.length);
}

/**
 * -----------------------------------------------
 * @brief      shell insert byte
 * @details    insert shell byte
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  data : insert byte
 * -----------------------------------------------
 */
void shell_insert_byte(shell_t *shell, char data)
{
    /* 判断输入数据是否过长 */
    if(shell->parser.length >= shell->parser.buffer_size - 1) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_TOO_LONG]);
        shell_write_prompt(shell, 1);
        shell_write_string(shell, shell->parser.buffer);
        return;
    }

    /* 插入数据 */
    if(shell->parser.cursor == shell->parser.length) {
        shell->parser.buffer[shell->parser.length++] = data;
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor++;
        shell_write_byte(shell, shell->status.is_checked ? data : '*');
    } else if(shell->parser.cursor < shell->parser.length) {
        for(short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
            shell->parser.buffer[shell->parser.cursor + i] =
                shell->parser.buffer[shell->parser.cursor + i - 1];
        }
        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.buffer[++shell->parser.length] = 0;
        for(short i = shell->parser.cursor - 1; i < shell->parser.length; i++) {
            shell_write_byte(shell,
                             shell->status.is_checked ? shell->parser.buffer[i] : '*');
        }
        for(short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
            shell_write_byte(shell, '\b');
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell delete byte
 * @details    delete shell byte
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  direction : delete direction {@code 1}delete cursor front byte {@code -1}delete cursor byte
 * -----------------------------------------------
 */
void shell_delete_byte(shell_t *shell, signed char direction)
{
    char offset = (direction == -1) ? 1 : 0;

    if((shell->parser.cursor == 0 && direction == 1) ||
       (shell->parser.cursor == shell->parser.length && direction == -1))
    {
        return;
    }
    if(shell->parser.cursor == shell->parser.length && direction == 1) {
        shell->parser.cursor--;
        shell->parser.length--;
        shell->parser.buffer[shell->parser.length] = 0;
        shell_delete_command_line(shell, 1);
    } else {
        for(short i = offset;
            i < shell->parser.length - shell->parser.cursor;
            i++)
        {
            shell->parser.buffer[shell->parser.cursor + i - 1] =
                shell->parser.buffer[shell->parser.cursor + i];
        }
        shell->parser.length--;
        if(!offset) {
            shell->parser.cursor--;
            shell_write_byte(shell, '\b');
        }
        shell->parser.buffer[shell->parser.length] = 0;
        for(short i = shell->parser.cursor; i < shell->parser.length; i++) {
            shell_write_byte(shell, shell->parser.buffer[i]);
        }
        shell_write_byte(shell, ' ');
        for(short i = shell->parser.length - shell->parser.cursor + 1;
            i > 0;
            i--)
        {
            shell_write_byte(shell, '\b');
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell split string
 * @details    split shell string
 * -----------------------------------------------
 * @param[in]  string : shell string
 * @param[in]  strLen : shell string length
 * @param[out] array : split string array
 * @param[in]  splitKey : split key
 * @param[in]  maxNum : max split number
 * -----------------------------------------------
 * @return     int : split string number
 * -----------------------------------------------
 */
int shell_split(char *string, uint16_t strLen, char *array[], char splitKey,
                short maxNum)
{
    uint8_t record = 1;
    uint8_t pairedLeft[16] = {
        0
    };
    uint8_t pariedCount = 0;
    int count = 0;

    for(short i = 0; i < maxNum; i++) {
        array[i] = NULL;
    }

    for(uint16_t i = 0; i < strLen; i++) {
        if(pariedCount == 0) {
            if(string[i] != splitKey && record == 1 && count < maxNum) {
                array[count++] = &(string[i]);
                record = 0;
            } else if((string[i] == splitKey || string[i] == ' ') &&
                      record == 0)
            {
                string[i] = 0;
                if(string[i + 1] != ' ') {
                    record = 1;
                }
                continue;
            }
        }

        for(uint8_t j = 0; j < sizeof(pairedChars) / 2; j++) {
            if(pariedCount > 0 && string[i] == pairedChars[j][1] &&
               pairedLeft[pariedCount - 1] == pairedChars[j][0])
            {
                --pariedCount;
                break;
            } else if(string[i] == pairedChars[j][0]) {
                pairedLeft[pariedCount++] = pairedChars[j][0];
                pariedCount &= 0x0F;
                break;
            }
        }

        if(string[i] == '\\' && string[i + 1] != 0) {
            i++;
        }
    }
    return count;
}

/**
 * -----------------------------------------------
 * @brief      shell parser param
 * @details    parser shell param
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_parser_param(shell_t *shell)
{
    shell->parser.param_count =
        shell_split(shell->parser.buffer, shell->parser.length,
                    shell->parser.param, ' ', SHELL_PARAMETER_MAX_NUMBER);
}

/**
 * -----------------------------------------------
 * @brief      shell remove param quotes
 * @details    remove shell param quotes
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_remove_param_quotes(shell_t *shell)
{
    uint16_t paramLength;
    for(uint16_t i = 0; i < shell->parser.param_count; i++) {
        if(shell->parser.param[i][0] == '\"') {
            shell->parser.param[i][0] = 0;
            shell->parser.param[i] = &shell->parser.param[i][1];
        }
        paramLength = strlen(shell->parser.param[i]);
        if(shell->parser.param[i][paramLength - 1] == '\"') {
            shell->parser.param[i][paramLength - 1] = 0;
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell seek cmd
 * @details    seek shell cmd
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  cmd : shell cmd
 * @param[in]  base : shell cmd base
 * @param[in]  compareLength : compare length
 * -----------------------------------------------
 * @return     shell_cmd_t* : shell cmd
 * -----------------------------------------------
 */
shell_cmd_t *shell_seek_cmd(shell_t *shell,
                            const char *cmd,
                            shell_cmd_t *base,
                            uint16_t compare_length)
{
    const char *name;
    uint16_t count = shell->command_list.count -
        ((size_t)base - (size_t)shell->command_list.base) / sizeof(shell_cmd_t);
    for(uint16_t i = 0; i < count; i++) {
        if(base[i].attr.para.type == SHELL_TYPE_KEY ||
           shell_check_permission(shell, &base[i]) != 0)
        {
            continue;
        }
        name = shell_get_command_name(&base[i]);
        if(!compare_length) {
            if(strcmp(cmd, name) == 0) {
                return &base[i];
            }
        } else {
            if(strncmp(cmd, name, compare_length) == 0) {
                return &base[i];
            }
        }
    }
    return NULL;
}

/**
 * -----------------------------------------------
 * @brief      shell get var value
 * @details    get shell var value
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  command : shell cmd
 * -----------------------------------------------
 * @return     int : shell var value
 * -----------------------------------------------
 */
int shell_get_var_value(shell_t *shell, shell_cmd_t *command)
{
    int value = 0;
    switch(command->attr.para.type) {
    case SHELL_TYPE_VAR_INT:
        value = *((int *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_SHORT:
        value = *((short *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_CHAR:
        value = *((char *)(command->data.var.value));
        break;
    case SHELL_TYPE_VAR_STRING:
    case SHELL_TYPE_VAR_POINT:
        value = (size_t)(command->data.var.value);
        break;
    case SHELL_TYPE_VAR_NODE:
    {
        int (*func)(void *) =
            ((shell_node_var_attr_t *)command->data.var.value)->get;
        value = func ? func(
            ((shell_node_var_attr_t *)command->data.var.value)->var) : 0;
        break;
    }
    default:
        break;
    }
    return value;
}

/**
 * -----------------------------------------------
 * @brief      shell set var value
 * @details    set shell var value
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  command : shell cmd
 * @param[in]  value : shell var value
 * -----------------------------------------------
 * @return     int : shell var value
 * -----------------------------------------------
 */
int shell_set_var_value(shell_t *shell, shell_cmd_t *command, int value)
{
    if(command->attr.para.read_only) {
        shell_write_string(shell,
                           shell_text[SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY]);
    } else {
        switch(command->attr.para.type) {
        case SHELL_TYPE_VAR_INT:
            *((int *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_SHORT:
            *((short *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_CHAR:
            *((char *)(command->data.var.value)) = value;
            break;
        case SHELL_TYPE_VAR_STRING:
            shell_string_copy(((char *)(command->data.var.value)),
                              (char *)(size_t)value);
            break;
        case SHELL_TYPE_VAR_POINT:
            shell_write_string(shell,
                               shell_text[SHELL_TEXT_POINT_CANNOT_MODIFY]);
            break;
        case SHELL_TYPE_VAR_NODE:
            if(((shell_node_var_attr_t *)command->data.var.value)->set) {
                if(((shell_node_var_attr_t *)command->data.var.value)->var) {
                    int (*func)(void *,
                                int) =
                        ((shell_node_var_attr_t *)command->data.var.value)->set;
                    func(
                        ((shell_node_var_attr_t *)command->data.var.value)->var,
                        value);
                } else {
                    int (*func)(int) =
                        ((shell_node_var_attr_t *)command->data.var.value)->set;
                    func(value);
                }
            }
            break;
        default:
            break;
        }
    }
    return shell_show_var(shell, command);
}

/**
 * -----------------------------------------------
 * @brief      shell show var
 * @details    show shell var
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  command : shell cmd
 * -----------------------------------------------
 * @return     int : shell var value
 * -----------------------------------------------
 */
static int shell_show_var(shell_t *shell, shell_cmd_t *command)
{
    char buffer[12] = "00000000000";
    int value = shell_get_var_value(shell, command);

    shell_write_string(shell, command->data.var.name);
    shell_write_string(shell, " = ");

    switch(command->attr.para.type) {
    case SHELL_TYPE_VAR_STRING:
        shell_write_string(shell, "\"");
        shell_write_string(shell, (char *)(size_t)value);
        shell_write_string(shell, "\"");
        break;
    /* case SHELL_TYPE_VAR_INT: */
    /* case SHELL_TYPE_VAR_SHORT: */
    /* case SHELL_TYPE_VAR_CHAR: */
    /* case SHELL_TYPE_VAR_POINT: */
    default:
        shell_write_string(shell, &buffer[11 - shell_to_dec(value, buffer)]);
        shell_write_string(shell, ", 0x");
        for(short i = 0; i < 11; i++) {
            buffer[i] = '0';
        }
        shell_to_hex(value, buffer);
        shell_write_string(shell, buffer);
        break;
    }

    shell_write_string(shell, "\r\n");
    return value;
}

/**
 * -----------------------------------------------
 * @brief      shell set var
 * @details    set shell var
 * -----------------------------------------------
 * @param[in]  name : shell var name
 * @param[in]  value : shell var value
 * -----------------------------------------------
 * @return     int : shell var value
 * -----------------------------------------------
 */
int shell_set_var(char *name, int value)
{
    shell_t *shell = shell_get_current();
    if(shell == NULL) {
        return 0;
    }
    shell_cmd_t *command = shell_seek_cmd(shell,
                                          name,
                                          shell->command_list.base,
                                          0);
    if(!command) {
        shell_write_string(shell, shell_text[SHELL_TEXT_VAR_NOT_FOUND]);
        return 0;
    }
    if(command->attr.para.type < SHELL_TYPE_VAR_INT ||
       command->attr.para.type > SHELL_TYPE_VAR_NODE)
    {
        shell_write_string(shell, name);
        shell_write_string(shell, shell_text[SHELL_TEXT_NOT_VAR]);
        return 0;
    }
    return shell_set_var_value(shell, command, value);
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    setVar, shell_set_var, set var);

/**
 * -----------------------------------------------
 * @brief      shell run command
 * @details    run shell command
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  command : shell cmd
 * -----------------------------------------------
 * @return     int : shell cmd return value
 * -----------------------------------------------
 */
unsigned int shell_run_command(shell_t *shell, shell_cmd_t *command)
{
    int returnValue = 0;
    shell->status.is_active = 1;
    if(command->attr.para.type == SHELL_TYPE_CMD_MAIN) {
        shell_remove_param_quotes(shell);
        int (*func)(int, char **) = command->data.cmd.function;
        returnValue = func(shell->parser.param_count, shell->parser.param);
        if(!command->attr.para.disable_return) {
            shell_write_return_value(shell, returnValue);
        }
    }
    else if (command->attr.para.type == SHELL_TYPE_CMD_FUNC)
    {
        returnValue = shell_ext_run(shell,
                                  command,
                                  shell->parser.param_count,
                                  shell->parser.param);
        if (!command->attr.para.disable_return)
        {
            shell_write_return_value(shell, returnValue);
        }
    }
    else if(command->attr.para.type >= SHELL_TYPE_VAR_INT &&
            command->attr.para.type <= SHELL_TYPE_VAR_NODE)
    {
        shell_show_var(shell, command);
    } else if(command->attr.para.type == SHELL_TYPE_USER) {
        shell_set_user(shell, command);
    }
    shell->status.is_active = 0;

    return returnValue;
}

/**
 * -----------------------------------------------
 * @brief      shell check password
 * @details    check shell password
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_check_password(shell_t *shell)
{
    if(strcmp(shell->parser.buffer, shell->info.sh_cmd->data.user.pasd) == 0) {
        shell->status.is_checked = 1;
        shell_write_string(shell, shell_text[SHELL_TEXT_INFO]);
    } else {
        shell_write_string(shell, shell_text[SHELL_TEXT_PASSWORD_ERROR]);
    }
    shell->parser.length = 0;
    shell->parser.cursor = 0;
}

/**
 * -----------------------------------------------
 * @brief      set user info
 * -----------------------------------------------
 * @param[in]  shell: shell ptr obj
 * @param[in]  user :  
 * -----------------------------------------------
 */
static void shell_set_user(shell_t *shell, const shell_cmd_t *user)
{
    shell->info.sh_cmd = user;
    shell->status.is_checked =
        ((user->data.user.pasd && strlen(user->data.user.pasd) != 0) &&
         (shell->parser.param_count < 2 ||
          strcmp(user->data.user.pasd, shell->parser.param[1]) != 0))
            ? 0
            : 1;

    /*! clear console */
    shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_CONSOLE]);
 
    /*! show basic info */
    if(shell->status.is_checked) {
        shell_write_string(shell, shell_text[SHELL_TEXT_INFO]);
    }
}

/**
 * -----------------------------------------------
 * @brief      shell write return value
 * @details    write shell return value
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  value : shell return value
 * -----------------------------------------------
 */
static void shell_write_return_value(shell_t *shell, int value)
{
    char buffer[12] = "00000000000";
    shell_write_string(shell, "Return: ");
    shell_write_string(shell, &buffer[11 - shell_to_dec(value, buffer)]);
    shell_write_string(shell, ", 0x");
    for(short i = 0; i < 11; i++) {
        buffer[i] = '0';
    }
    shell_to_hex(value, buffer);
    shell_write_string(shell, buffer);
    shell_write_string(shell, "\r\n");
#if SHELL_KEEP_RETURN_VALUE == 1
    shell->info.retVal = value;
#endif
}

#if SHELL_HISTORY_MAX_NUMBER > 0
/**
 * -----------------------------------------------
 * @brief      shell history add
 * @details    add shell history
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_history_add(shell_t *shell)
{
    shell->history.offset = 0;
    if(shell->history.number > 0 &&
       strcmp(shell->history.item[(shell->history.record ==
                                   0 ? SHELL_HISTORY_MAX_NUMBER : shell->history
                                   .record) - 1],
              shell->parser.buffer) == 0)
    {
        return;
    }
    if(shell_string_copy(shell->history.item[shell->history.record],
                         shell->parser.buffer) != 0)
    {
        shell->history.record++;
    }
    if(++shell->history.number > SHELL_HISTORY_MAX_NUMBER) {
        shell->history.number = SHELL_HISTORY_MAX_NUMBER;
    }
    if(shell->history.record >= SHELL_HISTORY_MAX_NUMBER) {
        shell->history.record = 0;
    }
}

/**
 * -----------------------------------------------
 * @brief      shell history
 * @details    show shell history
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  dir   : direction {@code <0} up {@code >0} down
 * -----------------------------------------------
 */
static void shell_history(shell_t *shell, signed char dir)
{
    if(dir > 0) {
        if(shell->history.offset-- <=
           -((shell->history.number >
              shell->history.record) ? shell->history.number : shell->history.
             record))
        {
            shell->history.offset =
                -((shell->history.number > shell->history.record)
                                          ? shell->history.number
                                          : shell->history.record);
        }
    } else if(dir < 0) {
        if(++shell->history.offset > 0) {
            shell->history.offset = 0;
            return;
        }
    } else {
        return;
    }
    shell_clear_command_line(shell);
    if(shell->history.offset == 0) {
        shell->parser.cursor = shell->parser.length = 0;
    } else {
        if((shell->parser.length = shell_string_copy(shell->parser.buffer,
                                                     shell->history.item[(shell
                                                                          ->
                                                                          history
                                                                          .
                                                                          record
                                                                          +
                                                                          SHELL_HISTORY_MAX_NUMBER
                                                                          +
                                                                          shell
                                                                          ->
                                                                          history
                                                                          .
                                                                          offset)
                                                                         %
                                                                         SHELL_HISTORY_MAX_NUMBER
                                                     ])) == 0)
        {
            return;
        }
        shell->parser.cursor = shell->parser.length;
        shell_write_string(shell, shell->parser.buffer);
    }
}

#endif /** SHELL_HISTORY_MAX_NUMBER > 0 */

/**
 * -----------------------------------------------
 * @brief      shell normal input
 * @details    normal input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  data  : input byte
 * -----------------------------------------------
 */
void shell_normal_input(shell_t *shell, char data)
{
    shell->status.tab_flag = 0;
    shell_insert_byte(shell, data);
}

/**
 * -----------------------------------------------
 * @brief      shell exec command
 * @details    exec shell command
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_exec(shell_t *shell)
{

    if(shell->parser.length == 0) {
        return;
    }

    shell->parser.buffer[shell->parser.length] = 0;

    if(shell->status.is_checked) {
#if SHELL_HISTORY_MAX_NUMBER > 0
        shell_history_add(shell);
#endif /** SHELL_HISTORY_MAX_NUMBER > 0 */
        shell_parser_param(shell);
        shell->parser.length = shell->parser.cursor = 0;
        if(shell->parser.param_count == 0) {
            return;
        }
        shell_write_string(shell, "\r\n");

        shell_cmd_t *command = shell_seek_cmd(shell,
                                              shell->parser.param[0],
                                              shell->command_list.base,
                                              0);
        if(command != NULL) {
            shell_run_command(shell, command);
        } else {
            shell_write_string(shell, shell_text[SHELL_TEXT_CMD_NOT_FOUND]);
        }
    } else {
        shell_check_password(shell);
    }
}

#if SHELL_HISTORY_MAX_NUMBER > 0
/**
 * -----------------------------------------------
 * @brief      shell up direction key input
 * @details    up direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_up(shell_t *shell)
{
    shell_history(shell, 1);
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x1B5B4100, shell_up, up);

/**
 * -----------------------------------------------
 * @brief      shell down direction key input
 * @details    down direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_down(shell_t *shell)
{
    shell_history(shell, -1);
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x1B5B4200, shell_down, down);
#endif /** SHELL_HISTORY_MAX_NUMBER > 0 */

/**
 * -----------------------------------------------
 * @brief      shell right direction key input
 * @details    right direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_right(shell_t *shell)
{
    if(shell->parser.cursor < shell->parser.length) {
        shell_write_byte(shell, shell->parser.buffer[shell->parser.cursor++]);
    }
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x1B5B4300, shell_right, right);

/**
 * -----------------------------------------------
 * @brief      shell left direction key input
 * @details    left direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_left(shell_t *shell)
{
    if(shell->parser.cursor > 0) {
        shell_write_byte(shell, '\b');
        shell->parser.cursor--;
    }
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x1B5B4400, shell_left, left);

/**
 * -----------------------------------------------
 * @brief      shell tab key input
 * @details    tab key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_tab(shell_t *shell)
{
    uint16_t maxMatch = shell->parser.buffer_size;
    uint16_t lastMatchIndex = 0;
    uint16_t matchNum = 0;
    uint16_t length;

    if(shell->parser.length == 0) {
        shell_list_all(shell);
        shell_write_prompt(shell, 1);
    } else if(shell->parser.length > 0) {
        shell->parser.buffer[shell->parser.length] = 0;
        shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
        for(short i = 0; i < shell->command_list.count; i++) {
            if(shell_check_permission(shell,
                                      &base[i]) == 0 &&
               shell_string_compare(shell->parser.buffer,
                                    (
                                        char
                                        *)shell_get_command_name(&base[i])) ==
               shell->parser.length)
            {
                if(matchNum != 0) {
                    if(matchNum == 1) {
                        shell_write_string(shell, "\r\n");
                    }
                    shell_list_item(shell, &base[lastMatchIndex]);
                    length =
                        shell_string_compare((char *)shell_get_command_name(&
                                                                            base
                                                                            [
                                                                                lastMatchIndex
                                                                            ]),
                                             (char *)shell_get_command_name(&
                                                                            base
                                                                            [i]));
                    maxMatch = (maxMatch > length) ? length : maxMatch;
                }
                lastMatchIndex = i;
                matchNum++;
            }
        }
        if(matchNum == 0) {
            return;
        }
        if(matchNum == 1) {
            shell_clear_command_line(shell);
        }
        if(matchNum != 0) {
            shell->parser.length =
                shell_string_copy(shell->parser.buffer,
                                  (char *)shell_get_command_name(&base[
                                                                     lastMatchIndex
                                                                 ]));
        }
        if(matchNum > 1) {
            shell_list_item(shell, &base[lastMatchIndex]);
            shell_write_prompt(shell, 1);
            shell->parser.length = maxMatch;
        }
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor = shell->parser.length;
        shell_write_string(shell, shell->parser.buffer);
    }

    if(SHELL_GET_TICK()) {
        if(matchNum == 1 && shell->status.tab_flag &&
           SHELL_GET_TICK() - shell->info.active_time < SHELL_DOUBLE_CLICK_TIME)
        {
#if SHELL_QUICK_HELP == 1
            shell_write_string(shell, "\r\n");
            shell_write_cmd_help(shell, shell->parser.buffer);
            shell_write_prompt(shell, 1);
            shell_write_string(shell, shell->parser.buffer);
#else
            shell_clear_command_line(shell);
            for(short i = shell->parser.length; i >= 0; i--) {
                shell->parser.buffer[i + 5] = shell->parser.buffer[i];
            }
            shell_string_copy(shell->parser.buffer, "help");
            shell->parser.buffer[4] = ' ';
            shell->parser.length += 5;
            shell->parser.cursor = shell->parser.length;
            shell_write_string(shell, shell->parser.buffer);
#endif
        } else {
            shell->status.tab_flag = 1;
        }
    }
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), 0x09000000, shell_tab, tab);

/**
 * -----------------------------------------------
 * @brief      shell backspace key input
 * @details    backspace key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_backspace(shell_t *shell)
{
    shell_delete_byte(shell, 1);
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x08000000, shell_backspace, backspace);
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x7F000000, shell_backspace, backspace);

/**
 * -----------------------------------------------
 * @brief      shell delete key input
 * @details    delete key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_delete(shell_t *shell)
{
    shell_delete_byte(shell, -1);
}

SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x1B5B337E, shell_delete, delete);

/**
 * -----------------------------------------------
 * @brief      shell enter key input
 * @details    enter key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_enter(shell_t *shell)
{
    shell_exec(shell);
    shell_write_prompt(shell, 1);
}

#if SHELL_ENTER_LF == 1
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x0A000000, shell_enter, enter);
#endif
#if SHELL_ENTER_CR == 1
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x0D000000, shell_enter, enter);
#endif
#if SHELL_ENTER_CRLF == 1
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0) | SHELL_CMD_ENABLE_UNCHECKED,
                 0x0D0A0000, shell_enter, enter);
#endif

/**
 * -----------------------------------------------
 * @brief      shell write command help
 * @details    write command help shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  cmd   : command string
 * -----------------------------------------------
 */
static void shell_write_cmd_help(shell_t *shell, char *cmd)
{
    shell_cmd_t *command = shell_seek_cmd(shell,
                                          cmd,
                                          shell->command_list.base,
                                          0);
    if(command) {
        shell_write_string(shell, shell_text[SHELL_TEXT_HELP_HEADER]);
        shell_write_string(shell, shell_get_command_name(command));
        shell_write_string(shell, "\r\n");
        shell_write_string(shell, shell_get_command_desc(command));
        shell_write_string(shell, "\r\n");
    } else {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_NOT_FOUND]);
    }
}

/**
 * -----------------------------------------------
 * @brief      shell help command
 * @details    show command help shell
 * -----------------------------------------------
 * @param[in]  argc : argument count
 * @param[in]  argv : argument vector
 * -----------------------------------------------
 */
void shell_help(int argc, char *argv[])
{
    shell_t *shell = shell_get_current();
    SHELL_ASSERT(shell);
    if(argc == 1) {
        shell_list_all(shell);
    } else if(argc > 1) {
        shell_write_cmd_help(shell, argv[1]);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN,
    help, shell_help, show command info);


#if SHELL_SUPPORT_END_LINE == 1
void shellWriteEndLine(shell_t *shell, char *buffer, int len)
{
    SHELL_LOCK(shell);
    if (!shell->status.is_active)
    {
        shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_LINE]);
    }
    shell->write(buffer, len);

    if (!shell->status.is_active)
    {
        shell_write_prompt(shell, 0);
        if (shell->parser.length > 0)
        {
            shell_write_string(shell, shell->parser.buffer);
            for (short i = 0; i < shell->parser.length - shell->parser.cursor; i++)
            {
                shell_write_byte(shell, '\b');
            }
        }
    }
    SHELL_UNLOCK(shell);
}
#endif /** SHELL_SUPPORT_END_LINE == 1 */

/**
 * -----------------------------------------------
 * @brief      shell input handler
 * @details    handle shell input
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  data  : input data
 * -----------------------------------------------
 */
void shell_handler(shell_t *shell, char data)
{
    SHELL_ASSERT(data);
    SHELL_LOCK(shell);

#if SHELL_LOCK_TIMEOUT > 0
    if(shell->info.sh_cmd->data.user.pasd &&
       strlen(shell->info.sh_cmd->data.user.pasd) != 0 && SHELL_GET_TICK())
    {
        if(SHELL_GET_TICK() - shell->info.active_time > SHELL_LOCK_TIMEOUT) {
            shell->status.is_checked = 0;
        }
    }
#endif

    /* 根据记录的按键键值计算当前字节在按键键值中的偏移 */
    char keyByteOffset = 24;
    int keyFilter = 0x00000000;
    if((shell->parser.key_value & 0x0000FF00) != 0x00000000) {
        keyByteOffset = 0;
        keyFilter = 0xFFFFFF00;
    } else if((shell->parser.key_value & 0x00FF0000) != 0x00000000) {
        keyByteOffset = 8;
        keyFilter = 0xFFFF0000;
    } else if((shell->parser.key_value & 0xFF000000) != 0x00000000) {
        keyByteOffset = 16;
        keyFilter = 0xFF000000;
    }

    /* 遍历ShellCommand列表，尝试进行按键键值匹配 */
    shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
    for(short i = 0; i < shell->command_list.count; i++) {
        /* 判断是否是按键定义并验证权限 */
        if(base[i].attr.para.type == SHELL_TYPE_KEY &&
           shell_check_permission(shell, &(base[i])) == 0)
        {
            /* 对输入的字节同按键键值进行匹配 */
            if((base[i].data.key.value & keyFilter) ==
               shell->parser.key_value &&
               (base[i].data.key.value & (0xFF << keyByteOffset)) ==
               (data << keyByteOffset))
            {
                shell->parser.key_value |= data << keyByteOffset;
                data = 0x00;
                if(keyByteOffset == 0 ||
                   (base[i].data.key.value & (0xFF << (keyByteOffset - 8))) ==
                   0x00000000)
                {
                    if(base[i].data.key.function) {
                        base[i].data.key.function(shell);
                    }
                    shell->parser.key_value = 0x00000000;
                    break;
                }
            }
        }
    }

    if(data != 0x00) {
        shell->parser.key_value = 0x00000000;
        shell_normal_input(shell, data);
    }

    if(SHELL_GET_TICK()) {
        shell->info.active_time = SHELL_GET_TICK();
    }
    SHELL_UNLOCK(shell);
}


/**
 * -----------------------------------------------
 * @brief      shell task
 * @details    shell task
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[out] data  : input data
 * @return     int   : 1 if success, 0 if fail
 * -----------------------------------------------
 */
void shell_task(void *param)
{
    shell_t *shell = (shell_t *)param;
    char data;
#if SHELL_TASK_WHILE == 1
    while(1) {
#endif
    if(shell->read && shell->read(&data, 1) == 1) {
        shell_handler(shell, data);
    }
#if SHELL_TASK_WHILE == 1
}

#endif
}

/**
 * -----------------------------------------------
 * @brief      shell users command
 * @details    show all user shell
 * -----------------------------------------------
 */
void shell_users(void)
{
    shell_t *shell = shell_get_current();
    if(shell) {
        shell_list_user(shell);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    users, shell_users, list all user);

/**
 * -----------------------------------------------
 * @brief      shell cmds command
 * @details    show all command shell
 * -----------------------------------------------
 */
void shell_cmds(void)
{
    shell_t *shell = shell_get_current();
    if(shell) {
        shell_list_command(shell);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    cmds, shell_cmds, list all cmd);

/**
 * -----------------------------------------------
 * @brief      shell vars command
 * @details    show all var shell
 * -----------------------------------------------
 */
void shell_vars(void)
{
    shell_t *shell = shell_get_current();
    if(shell) {
        shell_list_var(shell);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    vars, shell_vars, list all var);

/**
 * -----------------------------------------------
 * @brief      shell keys command
 * @details    show all key shell
 * -----------------------------------------------
 */
void shell_keys(void)
{
    shell_t *shell = shell_get_current();
    if(shell) {
        shell_list_key(shell);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    keys, shell_keys, list all key);

/**
 * -----------------------------------------------
 * @brief      shell clear command
 * @details    clear console shell
 * -----------------------------------------------
 */
void shell_clear(void)
{
    shell_t *shell = shell_get_current();
    if(shell) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_CONSOLE]);
    }
}

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
        SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN,
    clear, shell_clear, clear console);

/**
 * -----------------------------------------------
 * @brief      shell run command
 * @details    run command shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[out] cmd   : command string
 * @return     int   : 1 if success, 0 if fail
 * -----------------------------------------------
 */
int shell_run(shell_t *shell, const char *cmd)
{
    SHELL_ASSERT(shell && cmd);
    char active = shell->status.is_active;
    if(strlen(cmd) > shell->parser.buffer_size - 1) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_TOO_LONG]);
        return -1;
    } else {
        shell->parser.length = shell_string_copy(shell->parser.buffer,
                                                 (char *)cmd);
        shell_exec(shell);
        shell->status.is_active = active;
        return 0;
    }
}

#if SHELL_KEEP_RETURN_VALUE == 1
/**
 * @brief shell返回值获取
 *        获取上一次执行的命令的返回值
 *
 * @return int 返回值
 */
static int shell_ret_val_get()
{
    shell_t *shell = shell_get_current();
    return shell ? shell->info.retVal : 0;
}

static shell_node_var_attr_t shell_ret_val = {
    .get = shell_ret_val_get
};
SHELL_EXPORT_VAR(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(
                     SHELL_TYPE_VAR_NODE) | SHELL_CMD_READ_ONLY,
                 RETVAL, &shell_ret_val, return value of last command);
#endif /** SHELL_KEEP_RETURN_VALUE == 1 */
