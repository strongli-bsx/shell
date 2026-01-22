/**
 * ********************************************************
 * \file      shell.c
 * \brief     shell func realize
 * \version   1.1
 * \author    Letter(NevermindZZT@gmail.com), Awesome
 * \copyright (c) Letter, 2020
 * \copyright (c) 2025, Awesome
 * ********************************************************
 * \note      revision note
 * |   Date    |  version  |  author   | Description
 * |2019-12-30 |    1.0    |  Letter   | init version
 * |2025-10-31 |    1.1    |  Awesome  | modify to c style
 * |2025-11-03 |    1.2    |  Awesome  | merge ext.c to shell.c
 * |2026-01-27 |    1.2    |  Awesome  | modify section setting
 * ********************************************************
 */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "zshell.h"
#include "zshell_cfg.h"

 /*-----------------------------------------------------------------------------*/
 /*! shell command section address */
#if 0 
extern const unsigned int _shell_command_start;
extern const unsigned int _shell_command_end;
const uint32_t shell_sec_start = (uint32_t)&_shell_command_start;
const uint32_t shell_sec_end = (uint32_t)&_shell_command_end;
#else
extern const uint32_t shell_sec$$Base;
extern const uint32_t shell_sec$$Limit;
const uint32_t shell_sec_start = &shell_sec$$Base;
const uint32_t shell_sec_end = &shell_sec$$Limit;
#endif /**< __GNUC__ */
/*-----------------------------------------------------------------------------*/

/**
 * ------------------
 *  shell text
 * ------------------
 * */
static const char* shell_text[] =
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
    // [SHELL_TEXT_VAR_LIST] =
    //     "\r\nVar List:\r\n",
    // [SHELL_TEXT_USER_LIST] =
    //     "\r\nUser List:\r\n",
    // [SHELL_TEXT_KEY_LIST] =
    //     "\r\nKey List:\r\n",
    [SHELL_TEXT_CMD_NOT_FOUND] =
        "\r\nCommand not Found\r\n",
    // [SHELL_TEXT_POINT_CANNOT_MODIFY] =
    //     "can't set pointer\r\n",
    // [SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY] =
    //     "can't set read only var\r\n",
    // [SHELL_TEXT_NOT_VAR] =
    //     " is not a var\r\n",
    // [SHELL_TEXT_VAR_NOT_FOUND] =
    //     "Var not Fount\r\n",
    [SHELL_TEXT_HELP_HEADER] =
        "command help of ",
    // [SHELL_TEXT_PASSWORD_HINT] =
    //     "Please input password:",
    // [SHELL_TEXT_PASSWORD_ERROR] =
    //     "\r\npassword error\r\n",
    [SHELL_TEXT_CLEAR_TOTAL] =
        "\033[2J\033[3J\033[1H",
    [SHELL_TEXT_CLEAR_CONSOLE] =
        "\033[2J\033[1H",
    // [SHELL_TEXT_CLEAR_LINE] =
    //     "\033[2K\r",
    [SHELL_TEXT_TYPE_CMD] =
        "CMD ",
    // [SHELL_TEXT_TYPE_VAR] =
    //     "VAR ",
    // [SHELL_TEXT_TYPE_USER] =
    //     "USER",
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
static shell_t* shell_list = NULL;
/*-----------------------------------------------------------------------------*/
/*! shell func declaraiton */
static void shell_write_prompt(shell_t* shell, uint8_t newline);
static void shell_write_return_value(shell_t* shell, int value);
static void shell_write_cmd_help(shell_t* shell, char* cmd);
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/*-------------               shell basic function         --------------------*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/

/**
 * -----------------------------------------------
 * @brief      write byte to shell
 * -----------------------------------------------
 * @param[in]  shell: shell obj
 * @param[in]  data : char to write
 * -----------------------------------------------
 */
static void shell_write_byte(shell_t* shell, char data)
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
uint16_t shell_write_string(shell_t* shell, const char* string)
{
    uint16_t count = 0;
    const char* p = string;

    SHELL_ASSERT(shell->write);

    while (*p++) {
        count++;
    }
    return shell->write((char*)string, count);
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
static uint16_t shell_write_cmd_desc(shell_t* shell, const char* string)
{
    uint16_t count = 0;
    const char* p = string;

    SHELL_ASSERT(shell->write);

    while (*p && *p != '\r' && *p != '\n') {
        p++;
        count++;
    }

    if (count > 36) {
        shell->write((char*)string, 36);
        shell->write("...", 3);
    }
    else {
        shell->write((char*)string, count);
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
static void shell_write_prompt(shell_t* shell, uint8_t newline)
{
    if (newline) {
        shell_write_string(shell, "\r\n");
    }
    shell_write_string(shell, shell->user_name);
    shell_write_string(shell, ":");
    shell_write_string(shell, "~");
    shell_write_string(shell, "$ ");
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
signed char shell_to_hex(unsigned int value, char* buffer)
{
    char byte;
    uint8_t i = 8;
    buffer[8] = 0;
    while (value) {
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
signed char shell_to_dec(int value, char* buffer)
{
    uint8_t i = 11;
    int v = value;
    if (value < 0) {
        v = -value;
    }
    buffer[11] = 0;
    while (v) {
        buffer[--i] = v % 10 + 48;
        v /= 10;
    }
    if (value < 0) {
        buffer[--i] = '-';
    }
    if (value == 0) {
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
static uint16_t shell_string_copy(char* dest, char* src)
{
    uint16_t count = 0;
    while (*(src + count)) {
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
static uint16_t shell_string_compare(char* dest, char* src)
{
    uint16_t match = 0;
    uint16_t i = 0;

    while (*(dest + i) && *(src + i)) {
        if (*(dest + i) != *(src + i)) {
            break;
        }
        match++;
        i++;
    }
    return match;
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
void shell_init(shell_t* shell, char* buffer, uint16_t size)
{
    /*! shell info init */
    // shell->info.sh_cmd = NULL;

    /*! shell parser init */
    shell->parser.length = 0;
    shell->parser.cursor = 0;
    shell->parser.buffer = buffer;
    shell->parser.buffer_size = size / (SHELL_HISTORY_MAX_NUMBER + 1);

    /*! shell history init */
    shell->history.offset = 0;
    shell->history.number = 0;
    shell->history.record = 0;
    for (short i = 0; i < SHELL_HISTORY_MAX_NUMBER; i++) {
        shell->history.item[i] = buffer + shell->parser.buffer_size * (i + 1);
    }

    /*! shell command list init */
    shell->command_list.base = (shell_cmd_t*)(shell_sec_start);
    shell->command_list.count =
        ((size_t)(shell_sec_end)-(size_t)(shell_sec_start)) /
        sizeof(shell_cmd_t);

    shell_list = shell;

    shell->user_name = SHELL_DEFAULT_USER;
    /*! clear console */
    shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_CONSOLE]);
    /*! show basic info */
    shell_write_string(shell, shell_text[SHELL_TEXT_INFO]);
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
void shell_list_item(shell_t* shell, shell_cmd_t* item)
{
    short spaceLength;

    spaceLength = 22 - shell_write_string(shell, item->data.cmd.name);
    spaceLength = (spaceLength > 0) ? spaceLength : 4;
    do {
        shell_write_byte(shell, ' ');
    } while (--spaceLength);
    if (item->attr.para.type <= SHELL_TYPE_CMD_FUNC) {
        shell_write_string(shell, shell_text[SHELL_TEXT_TYPE_CMD]);
    }
    shell_write_string(shell, "  ");
    for (signed char i = 7; i >= 0; i--) {
        shell_write_byte(shell, '-');
    }
    shell_write_string(shell, "  ");
    shell_write_string(shell, item->data.cmd.desc);
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
void shell_list_command(shell_t* shell)
{
    shell_cmd_t* base = (shell_cmd_t*)shell->command_list.base;
    shell_write_string(shell, shell_text[SHELL_TEXT_CMD_LIST]);
    for (short i = 0; i < shell->command_list.count; i++) {
        if (base[i].attr.para.type <= SHELL_TYPE_CMD_FUNC)
        {
            shell_list_item(shell, &base[i]);
        }
    }
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
void shell_delete_command_line(shell_t* shell, uint8_t length)
{
    while (length--) {
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
void shell_clear_command_line(shell_t* shell)
{
    for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
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
void shell_insert_byte(shell_t* shell, char data)
{
    /* 判断输入数据是否过长 */
    if (shell->parser.length >= shell->parser.buffer_size - 1) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_TOO_LONG]);
        shell_write_prompt(shell, 1);
        shell_write_string(shell, shell->parser.buffer);
        return;
    }

    /* 插入数据 */
    if (shell->parser.cursor == shell->parser.length) {
        shell->parser.buffer[shell->parser.length++] = data;
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor++;
        shell_write_byte(shell, data);
    }
    else if (shell->parser.cursor < shell->parser.length) {
        for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
            shell->parser.buffer[shell->parser.cursor + i] =
                shell->parser.buffer[shell->parser.cursor + i - 1];
        }
        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.buffer[++shell->parser.length] = 0;
        for (short i = shell->parser.cursor - 1; i < shell->parser.length; i++) {
            shell_write_byte(shell,
                shell->parser.buffer[i]);
        }
        for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--) {
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
void shell_delete_byte(shell_t* shell, signed char direction)
{
    char offset = (direction == -1) ? 1 : 0;

    if ((shell->parser.cursor == 0 && direction == 1) ||
        (shell->parser.cursor == shell->parser.length && direction == -1))
    {
        return;
    }
    if (shell->parser.cursor == shell->parser.length && direction == 1) {
        shell->parser.cursor--;
        shell->parser.length--;
        shell->parser.buffer[shell->parser.length] = 0;
        shell_delete_command_line(shell, 1);
    }
    else {
        for (short i = offset;
            i < shell->parser.length - shell->parser.cursor;
            i++)
        {
            shell->parser.buffer[shell->parser.cursor + i - 1] =
                shell->parser.buffer[shell->parser.cursor + i];
        }
        shell->parser.length--;
        if (!offset) {
            shell->parser.cursor--;
            shell_write_byte(shell, '\b');
        }
        shell->parser.buffer[shell->parser.length] = 0;
        for (short i = shell->parser.cursor; i < shell->parser.length; i++) {
            shell_write_byte(shell, shell->parser.buffer[i]);
        }
        shell_write_byte(shell, ' ');
        for (short i = shell->parser.length - shell->parser.cursor + 1; i > 0; i--)
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
int shell_split(char* string, uint16_t strLen, char* array[], char splitKey,
    short maxNum)
{
    uint8_t record = 1;
    uint8_t pairedLeft[16] = {
        0
    };
    uint8_t pariedCount = 0;
    int count = 0;

    for (short i = 0; i < maxNum; i++) {
        array[i] = NULL;
    }

    for (uint16_t i = 0; i < strLen; i++) {
        if (pariedCount == 0) {
            if (string[i] != splitKey && record == 1 && count < maxNum) {
                array[count++] = &(string[i]);
                record = 0;
            }
            else if ((string[i] == splitKey || string[i] == ' ') &&
                record == 0)
            {
                string[i] = 0;
                if (string[i + 1] != ' ') {
                    record = 1;
                }
                continue;
            }
        }

        for (uint8_t j = 0; j < sizeof(pairedChars) / 2; j++) {
            if (pariedCount > 0 && string[i] == pairedChars[j][1] &&
                pairedLeft[pariedCount - 1] == pairedChars[j][0])
            {
                --pariedCount;
                break;
            }
            else if (string[i] == pairedChars[j][0]) {
                pairedLeft[pariedCount++] = pairedChars[j][0];
                pariedCount &= 0x0F;
                break;
            }
        }

        if (string[i] == '\\' && string[i + 1] != 0) {
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
static void shell_parser_param(shell_t* shell)
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
static void shell_remove_param_quotes(shell_t* shell)
{
    uint16_t paramLength;
    for (uint16_t i = 0; i < shell->parser.param_count; i++) {
        if (shell->parser.param[i][0] == '\"') {
            shell->parser.param[i][0] = 0;
            shell->parser.param[i] = &shell->parser.param[i][1];
        }
        paramLength = strlen(shell->parser.param[i]);
        if (shell->parser.param[i][paramLength - 1] == '\"') {
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
shell_cmd_t* shell_seek_cmd(shell_t* shell,
    const char* cmd,
    shell_cmd_t* base,
    uint16_t compare_length)
{
    const char* name;
    uint16_t count = shell->command_list.count -
        ((size_t)base - (size_t)shell->command_list.base) / sizeof(shell_cmd_t);
    for (uint16_t i = 0; i < count; i++) {
        if (base[i].attr.para.type == SHELL_TYPE_KEY)
        {
            continue;
        }
        name = (&base[i])->data.cmd.name;
        if (!compare_length) {
            if (strcmp(cmd, name) == 0) {
                return &base[i];
            }
        }
        else {
            if (strncmp(cmd, name, compare_length) == 0) {
                return &base[i];
            }
        }
    }
    return NULL;
}

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
unsigned int shell_run_command(shell_t* shell, shell_cmd_t* command)
{
    int return_value = 0;
    if (command->attr.para.type == SHELL_TYPE_CMD_FUNC)
    {
        return_value = shell_register_run(shell,
            command,
            shell->parser.param_count,
            shell->parser.param);
        if (!command->attr.para.disable_return)
        {
            shell_write_return_value(shell, return_value);
        }
    }

    return return_value;
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
static void shell_write_return_value(shell_t* shell, int value)
{
    char buffer[12] = "00000000000";
    shell_write_string(shell, "Return: ");
    shell_write_string(shell, &buffer[11 - shell_to_dec(value, buffer)]);
    shell_write_string(shell, ", 0x");
    for (short i = 0; i < 11; i++) {
        buffer[i] = '0';
    }
    shell_to_hex(value, buffer);
    shell_write_string(shell, buffer);
    shell_write_string(shell, "\r\n");
}

/**
 * -----------------------------------------------
 * @brief      shell history add
 * @details    add shell history
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
static void shell_history_add(shell_t* shell)
{
    shell->history.offset = 0;
    if (shell->history.number > 0 &&
        strcmp(shell->history.item[(shell->history.record ==
            0 ? SHELL_HISTORY_MAX_NUMBER : shell->history
            .record) - 1],
            shell->parser.buffer) == 0)
    {
        return;
    }
    if (shell_string_copy(shell->history.item[shell->history.record],
        shell->parser.buffer) != 0)
    {
        shell->history.record++;
    }
    if (++shell->history.number > SHELL_HISTORY_MAX_NUMBER) {
        shell->history.number = SHELL_HISTORY_MAX_NUMBER;
    }
    if (shell->history.record >= SHELL_HISTORY_MAX_NUMBER) {
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
static void shell_history(shell_t* shell, signed char dir)
{
    if (dir > 0) {
        if (shell->history.offset-- <=
            -((shell->history.number >
                shell->history.record) ? shell->history.number : shell->history.record))
        {
            shell->history.offset =
                -((shell->history.number > shell->history.record)
                    ? shell->history.number
                    : shell->history.record);
        }
    }
    else if (dir < 0) {
        if (++shell->history.offset > 0) {
            shell->history.offset = 0;
            return;
        }
    }
    else {
        return;
    }
    shell_clear_command_line(shell);
    if (shell->history.offset == 0) {
        shell->parser.cursor = shell->parser.length = 0;
    }
    else {
        if ((shell->parser.length = shell_string_copy(shell->parser.buffer,
            shell->history.item[(shell->history.record + SHELL_HISTORY_MAX_NUMBER + shell->history.offset) %
                SHELL_HISTORY_MAX_NUMBER])) == 0)
        {
            return;
        }
        shell->parser.cursor = shell->parser.length;
        shell_write_string(shell, shell->parser.buffer);
    }
}


/**
 * -----------------------------------------------
 * @brief      shell normal input
 * @details    normal input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  data  : input byte
 * -----------------------------------------------
 */
void shell_normal_input(shell_t* shell, char data)
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
void shell_exec(shell_t* shell)
{

    if (shell->parser.length == 0) {
        return;
    }

    shell->parser.buffer[shell->parser.length] = 0;

    shell_history_add(shell);
    shell_parser_param(shell);
    shell->parser.length = shell->parser.cursor = 0;
    if (shell->parser.param_count == 0) {
        return;
    }
    shell_write_string(shell, "\r\n");

    shell_cmd_t* command = shell_seek_cmd(shell,
        shell->parser.param[0],
        shell->command_list.base,
        0);
    if (command != NULL) {
        shell_run_command(shell, command);
    }
    else {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_NOT_FOUND]);
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
void shell_up(shell_t* shell)
{
    shell_history(shell, 1);
}

KEY_REGISTER(0x1B5B4100, shell_up, up);

/**
 * -----------------------------------------------
 * @brief      shell down direction key input
 * @details    down direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_down(shell_t* shell)
{
    shell_history(shell, -1);
}

KEY_REGISTER(0x1B5B4200, shell_down, down);
#endif /** SHELL_HISTORY_MAX_NUMBER > 0 */

/**
 * -----------------------------------------------
 * @brief      shell right direction key input
 * @details    right direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_right(shell_t* shell)
{
    if (shell->parser.cursor < shell->parser.length) {
        shell_write_byte(shell, shell->parser.buffer[shell->parser.cursor++]);
    }
}

KEY_REGISTER(0x1B5B4300, shell_right, right);

/**
 * -----------------------------------------------
 * @brief      shell left direction key input
 * @details    left direction key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_left(shell_t* shell)
{
    if (shell->parser.cursor > 0) {
        shell_write_byte(shell, '\b');
        shell->parser.cursor--;
    }
}

KEY_REGISTER(0x1B5B4400, shell_left, left);

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

    const char *cmd_name = NULL;

    if (shell->parser.length == 0) {
        shell_list_command(shell);
        shell_write_prompt(shell, 1);
    } 
    else if (shell->parser.length > 0) {
        shell->parser.buffer[shell->parser.length] = 0;
        shell_cmd_t *base = (shell_cmd_t *)shell->command_list.base;
        
        for (short i = 0; i < shell->command_list.count; i++) {
            if (base[i].attr.para.type <= SHELL_TYPE_CMD_FUNC) {
                cmd_name = base[i].data.cmd.name;
            } else {
                cmd_name = "";
            }

            if (shell_string_compare(shell->parser.buffer, (char *)cmd_name) == shell->parser.length)
            {
                if (matchNum != 0) {
                    if (matchNum == 1) {
                        shell_write_string(shell, "\r\n");
                    }
                    shell_list_item(shell, &base[lastMatchIndex]);
                    
                    const char *last_cmd_name = (base[lastMatchIndex].attr.para.type <= SHELL_TYPE_CMD_FUNC) ? 
                                                base[lastMatchIndex].data.cmd.name : "";
                    length = shell_string_compare((char *)last_cmd_name, (char *)cmd_name);
                    maxMatch = (maxMatch > length) ? length : maxMatch;
                }
                lastMatchIndex = i;
                matchNum++;
            }
        }

        if (matchNum == 0) {
            return;
        }

        if (matchNum == 1) {
            shell_clear_command_line(shell);
        }

        if (matchNum != 0) {
            const char *last_match_cmd_name = (base[lastMatchIndex].attr.para.type <= SHELL_TYPE_CMD_FUNC) ? 
                                              base[lastMatchIndex].data.cmd.name : "";
            shell->parser.length = shell_string_copy(shell->parser.buffer, (char *)last_match_cmd_name);
        }

        if (matchNum > 1) {
            shell_list_item(shell, &base[lastMatchIndex]);
            shell_write_prompt(shell, 1);
            shell->parser.length = maxMatch;
        }

        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor = shell->parser.length;
        shell_write_string(shell, shell->parser.buffer);
    }
}
KEY_REGISTER(0x09000000, shell_tab, tab);

/**
 * -----------------------------------------------
 * @brief      shell backspace key input
 * @details    backspace key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_backspace(shell_t* shell)
{
    shell_delete_byte(shell, 1);
}

KEY_REGISTER(0x08000000, shell_backspace, backspace);
KEY_REGISTER(0x7F000000, shell_backspace, backspace);

/**
 * -----------------------------------------------
 * @brief      shell delete key input
 * @details    delete key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_delete(shell_t* shell)
{
    shell_delete_byte(shell, -1);
}

KEY_REGISTER(0x1B5B337E, shell_delete, delete);

/**
 * -----------------------------------------------
 * @brief      shell enter key input
 * @details    enter key input shell
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * -----------------------------------------------
 */
void shell_enter(shell_t* shell)
{
    shell_exec(shell);
    shell_write_prompt(shell, 1);
}

KEY_REGISTER(0x0A000000, shell_enter, enter);
KEY_REGISTER(0x0D000000, shell_enter, enter);

// /**
//  * -----------------------------------------------
//  * @brief      shell write command help
//  * @details    write command help shell
//  * -----------------------------------------------
//  * @param[in]  shell : shell struct
//  * @param[in]  cmd   : command string
//  * -----------------------------------------------
//  */
// static void shell_write_cmd_help(shell_t* shell, char* cmd)
// {
//     shell_cmd_t* command = shell_seek_cmd(shell, cmd, shell->command_list.base, 0);

//     if (command) {
//         shell_write_string(shell, shell_text[SHELL_TEXT_HELP_HEADER]);
//         shell_write_string(shell, command->data.cmd.name);
//         shell_write_string(shell, "\r\n");
//         shell_write_string(shell, command->data.cmd.desc);
//         shell_write_string(shell, "\r\n");
//     }
//     else {
//         shell_write_string(shell, shell_text[SHELL_TEXT_CMD_NOT_FOUND]);
//     }
// }

/**
 * -----------------------------------------------
 * @brief      shell help command
 * @details    show command help shell
 * -----------------------------------------------
 * @param[in]  argc : argument count
 * @param[in]  argv : argument vector
 * -----------------------------------------------
 */
void shell_help(void)
{
    shell_t* shell = shell_list;
    SHELL_ASSERT(shell);
    shell_list_command(shell);
}

CMD_REGISTER(SHELL_CMD_DISABLE_RETURN, help, shell_help, show command info);


/**
 * -----------------------------------------------
 * @brief      shell input handler
 * @details    handle shell input
 * -----------------------------------------------
 * @param[in]  shell : shell struct
 * @param[in]  data  : input data
 * -----------------------------------------------
 */
void shell_handler(shell_t* shell, char data)
{
    SHELL_ASSERT(data);


    char keyByteOffset = 24;
    int keyFilter = 0x00000000;
    if ((shell->parser.key_value & 0x0000FF00) != 0x00000000) {
        keyByteOffset = 0;
        keyFilter = 0xFFFFFF00;
    }
    else if ((shell->parser.key_value & 0x00FF0000) != 0x00000000) {
        keyByteOffset = 8;
        keyFilter = 0xFFFF0000;
    }
    else if ((shell->parser.key_value & 0xFF000000) != 0x00000000) {
        keyByteOffset = 16;
        keyFilter = 0xFF000000;
    }

    shell_cmd_t* base = (shell_cmd_t*)shell->command_list.base;
    for (short i = 0; i < shell->command_list.count; i++) {
        if (base[i].attr.para.type == SHELL_TYPE_KEY)
        {
            if ((base[i].data.key.value & keyFilter) ==
                shell->parser.key_value &&
                (base[i].data.key.value & (0xFF << keyByteOffset)) ==
                (data << keyByteOffset))
            {
                shell->parser.key_value |= data << keyByteOffset;
                data = 0x00;
                if (keyByteOffset == 0 ||
                    (base[i].data.key.value & (0xFF << (keyByteOffset - 8))) ==
                    0x00000000)
                {
                    if (base[i].data.key.function) {
                        base[i].data.key.function(shell);
                    }
                    shell->parser.key_value = 0x00000000;
                    break;
                }
            }
        }
    }

    if (data != 0x00) {
        shell->parser.key_value = 0x00000000;
        shell_normal_input(shell, data);
    }

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
void shell_task(void* param)
{
    shell_t* shell = (shell_t*)param;
    char data;
    while (1) {
        if (shell->read && shell->read(&data, 1) == 1) {
            shell_handler(shell, data);
        }
    }
}

/**
 * -----------------------------------------------
 * @brief      shell clear total command
 * @details    clear console & srcoll
 * -----------------------------------------------
 */
void shell_clear_total(void)
{
    shell_t* shell = shell_list;
    if (shell) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_TOTAL]);
    }
}

CMD_REGISTER(SHELL_CMD_DISABLE_RETURN,
    cclear, shell_clear_total, clear total);
/**
 * -----------------------------------------------
 * @brief      shell clear command
 * @details    clear console shell
 * -----------------------------------------------
 */
void shell_clear(void)
{
    shell_t* shell = shell_list;
    if (shell) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CLEAR_CONSOLE]);
    }
}

CMD_REGISTER(SHELL_CMD_DISABLE_RETURN,
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
int shell_run(shell_t* shell, const char* cmd)
{
    SHELL_ASSERT(shell && cmd);
    if (strlen(cmd) > shell->parser.buffer_size - 1) {
        shell_write_string(shell, shell_text[SHELL_TEXT_CMD_TOO_LONG]);
        return -1;
    }
    else {
        shell->parser.length = shell_string_copy(shell->parser.buffer, (char*)cmd);
        shell_exec(shell);
        return 0;
    }
}


/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/*---------------------------  shell register  --------------------------------*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/**
 * -----------------------------------------------
 * @brief      shell parse number type
 * @details    parse number type from string
 * -----------------------------------------------
 * @param[in]  string number string
 * @param[out] type number type
 * @return     SHELL_NUM_TYPE_E number type
 * -----------------------------------------------
 */
static SHELL_NUM_TYPE_E shell_register_num_type(char* string)
{
    char* p = string;
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
 * -----------------------------------------------
 * @brief      shell char to number
 * @details    convert char to number
 * -----------------------------------------------
 * @param[in]  code char to convert
 * @return     char number
 * -----------------------------------------------
 */
static char shell_register_char_to_num(char code)
{
    if ((code >= '0') && (code <= '9'))
    {
        return code - '0';
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
 * -----------------------------------------------
 * @brief      shell parse char
 * @details    parse char from string
 * -----------------------------------------------
 * @param[in]  string char string
 * @return     char char value
 * -----------------------------------------------
 */
static char shell_register_parse_char(char* string)
{
    char* p = (*string == '\'') ? (string + 1) : string;
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
 * -----------------------------------------------
 * @brief      shell parse string
 * @details    parse string from string
 * -----------------------------------------------
 * @param[in]  string string string
 * @return     char* string value
 * -----------------------------------------------
 */
static char* shell_register_parse_string(char* string)
{
    char* p = string;
    unsigned short index = 0;

    if (*string == '\"')
    {
        p = ++string;
    }

    while (*p)
    {
        if (*p == '\\')
        {
            *(string + index) = shell_register_parse_char(p);
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
        index++;
    }
    *(string + index) = 0;
    return string;
}

/**
 * -----------------------------------------------
 * @brief      shell parse number
 * @details    parse number from string
 * -----------------------------------------------
 * @param[in]  string number string
 * @return     size_t number value
 * -----------------------------------------------
 */
static size_t shell_register_parse_number(char* string)
{
    SHELL_NUM_TYPE_E type = NUM_TYPE_DEC;
    char radix = 10;
    char* p = string;
    char offset = 0;
    signed char sign = 1;
    size_t value_int = 0;
    float value_float = 0.0;
    size_t devide = 0;

    if (*string == '-')
    {
        sign = -1;
    }

    type = shell_register_num_type(string + ((sign == -1) ? 1 : 0));

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
        value_int = value_int * radix + shell_register_char_to_num(*p);
        devide *= 10;
        p++;
    }
    if (type == NUM_TYPE_FLOAT && devide != 0)
    {
        value_float = (float)value_int / devide * sign;
        return *(size_t*)(&value_float);
    }
    else
    {
        return value_int * sign;
    }
}


/**
 * -----------------------------------------------
 * @brief      shell parse para
 * @details    parse para from string
 * -----------------------------------------------
 * @param[in]  shell shell struct
 * @param[in]  string para string
 * @param[in]  type para type
 * @param[out] result para value
 * @return     int 0 parse success --1 parse fail
 * -----------------------------------------------
 */
static int shell_register_parse_para(shell_t* shell, char* string, char* type, size_t* result)
{
    if (type == NULL || (*string == '$' && *(string + 1)))
    {
        if (*string == '\'' && *(string + 1))
        {
            *result = (size_t)shell_register_parse_char(string);
            return 0;
        }
        else if (*string == '-' || (*string >= '0' && *string <= '9'))
        {
            *result = shell_register_parse_number(string);
            return 0;
        }
        else if (*string == '$' && *(string + 1))
        {
            // return shell_register_parse_var(shell, string, result);
        }
        else if (*string)
        {
            *result = (size_t)shell_register_parse_string(string);
            return 0;
        }
    }
    return -1;
}

/**
 * -----------------------------------------------
 * @brief      shell run command
 * @details    run shell command
 * -----------------------------------------------
 * @param[in]  shell shell struct
 * @param[in]  command cmd to run
 * @param[in]  argc cmd argc
 * @param[in]  argv cmd argv
 * @return     cmd func return value
 * -----------------------------------------------
 */
int shell_register_run(shell_t* shell, shell_cmd_t* command, int argc, char* argv[])
{
    int ret = 0;
    size_t params[SHELL_PARAMETER_MAX_NUMBER] = { 0 };
    int param_number = argc - 1;
    int i;
    for (i = 0; i < argc - 1; i++)
    {
        {
            if (shell_register_parse_para(shell, argv[i + 1], NULL, &params[i]) != 0)
            {
                return -1;
            }
        }
    }
    switch (param_number)
    {
    case 0:
        ret = command->data.cmd.function();
        break;
    case 1:
    {
        int (*func)(size_t) = command->data.cmd.function;
        ret = func(params[0]);
        break;
    }
    case 2:
    {
        int (*func)(size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1]);
        break;
    }
    case 3:
    {
        int (*func)(size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2]);
        break;
    }
    case 4:
    {
        int (*func)(size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3]);
        break;
    }
    case 5:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4]);
        break;
    }
    case 6:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t) = command->data.cmd.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5]);
        break;
    }
    default:
        ret = -1;
        break;
    }
    return ret;
}