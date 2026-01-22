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

#ifndef __SHELL_H__
#define __SHELL_H__

#include "zshell_cfg.h"
#include <stdint.h>


/*-----------------------------------------------------------------------------*/
/*! shell command type enum */
typedef enum shell_cmd_type_e{
    SHELL_TYPE_CMD_FUNC,                   /**< func */
    SHELL_TYPE_KEY,                        /**< key */
} SHELL_CMD_TYPE_E;

/*! shell text enum */
enum shell_text_e {
    SHELL_TEXT_INFO,                        /**< shell info */
    SHELL_TEXT_CMD_TOO_LONG,                /**< cmd too long */
    SHELL_TEXT_CMD_LIST,                    /**< cmd list */
    SHELL_TEXT_CMD_NOT_FOUND,               /**< cmd not found */
    SHELL_TEXT_HELP_HEADER,                 /**< help header */
    SHELL_TEXT_CLEAR_TOTAL,                 /**< clear total */
    SHELL_TEXT_CLEAR_CONSOLE,               /**< clear console */
    SHELL_TEXT_TYPE_CMD,                    /**< cmd type */
    SHELL_TEXT_TYPE_KEY,                    /**< key type */
    SHELL_TEXT_TYPE_NONE,                   /**< none type */
};
/*-----------------------------------------------------------------------------*/
#define SHELL_SEC_NAME                      "shell_sec"
/*-----------------------------------------------------------------------------*/
/*! shell assert */
#define SHELL_ASSERT(expr) \
        if(!(expr)) {                                                         \
            printf("Assert failed: %s, line %d\n", __FILE__, __LINE__);       \
            while(1) {                                                        \
            }                                                                 \
        }

/*! attrubuted section */
#define SHELL_SECTION(x)                __attribute__((section(x), aligned(1)))

/*! attrubuted used */
#define SHELL_USED                       __attribute__((used))
/*-----------------------------------------------------------------------------*/
/*! shell cmd type */
#define SHELL_CMD_TYPE(type)               ((type & 0x0000000F) << 8)

/*! shell cmd unchecked enable */
#define SHELL_CMD_ENABLE_RETURN            (0 << 4)

/*! shell cmd return disable */
#define SHELL_CMD_DISABLE_RETURN           (1 << 4)

/*-----------------------------------------------------------------------------*/
/**
 * -----------------------------------------------
 * @brief      shell cmd define
 * -----------------------------------------------
 * @param[in]  _attr: cmd attribute
 * @param[in]  _name: cmd name
 * @param[in]  _func: cmd function
 * @param[in]  _desc: cmd description
 * -----------------------------------------------
 */
#define CMD_REGISTER(_attr, _name, _func, _desc, ...)          \
        const char cmd##_name[] = #_name;                     \
        const char cmd_desc##_name[] = #_desc;                    \
        SHELL_USED const shell_cmd_t                               \
        shell_cmd##_name SHELL_SECTION(SHELL_SEC_NAME) =              \
        {                                                          \
            .attr.value = _attr | SHELL_TYPE_CMD_FUNC,                                   \
            .data.cmd.name = cmd##_name,                      \
            .data.cmd.function = (int (*)()) _func,                \
            .data.cmd.desc = cmd_desc##_name,                     \
            ##__VA_ARGS__                                          \ 
        }
/**
 * -----------------------------------------------
 * @brief      shell key define
 * -----------------------------------------------
 * @param[in]  _attr : key attribute
 * @param[in]  _value: key value
 * @param[in]  _func : key function
 * @param[in]  _desc : key description
 * -----------------------------------------------
 */
#define KEY_REGISTER(_value, _func, _desc)             \
        const char cmd_desc##_value[] = #_desc;                  \
        SHELL_USED const shell_cmd_t                              \
        shell_key##_value SHELL_SECTION(SHELL_SEC_NAME) =             \
        {                                                         \
            .attr.value = SHELL_TYPE_KEY, \
            .data.key.value = _value,                             \
            .data.key.function = (void (*)(shell_t *)) _func,     \
            .data.key.desc = cmd_desc##_value                    \
        }

/*-----------------------------------------------------------------------------*/
/*! shell define struct */
typedef struct {

    const char *user_name;                        /**< user name */
    /*! shell parser */
    struct {
        uint16_t length;                          /**< input length */
        uint16_t cursor;                          /**< current cursor */
        char *buffer;                             /**< input buffer */
        char *param[SHELL_PARAMETER_MAX_NUMBER];  /**< param */
        uint16_t buffer_size;                     /**< input buffer size */
        uint16_t param_count;                     /**< parameter number */
        int key_value;                            /**< input key value */
    } parser;

    /*! shell history cmds */
    struct {
        char *item[SHELL_HISTORY_MAX_NUMBER];     /**< history cmds */
        uint16_t number;                          /**< history cmds number */
        uint16_t record;                          /**< current history record */
        signed short offset;                      /**< current history offset */
    } history;

    /*! shell command list */
    struct {
        void *base;                               /**< cmd list base addr */
        uint16_t count;                           /**< cmd num */
    } command_list;
    
    /*! shell status */
    struct {
        uint8_t tab_flag   : 1;             /**< tab flag */
    } status;

    /*! shell read & write function */
    signed short (*read)(char *, uint16_t);      /**< shell read function */
    signed short (*write)(char *, uint16_t);     /**< shell write function */

} shell_t;

/*! shell command define struct */
typedef struct shell_command {
    /*! shell cmd attribute */
    union {
        struct {
            SHELL_CMD_TYPE_E type : 4;       /**< command type */
            uint8_t disable_return : 1;      /**< disable return value */
        } para;
        int value;
    } attr; 

    /*! shell content */
    union {
        /*! shell cmd define */
        struct {
            const char *name;               /**< cmd name */
            int (*function)();              /**< cmd execute function */
            const char *desc;               /**< cmd description */
        } cmd;                              /**< cmd define */

        /*! shell key define */
        struct {
            int value;                      /**< key value */
            void (*function)(shell_t *);    /**< key execute function */
            const char *desc;               /**< key description */
        } key;                           

    } data;
} shell_cmd_t;

/*-----------------------------------------------------------------------------*/
void shell_init(shell_t *shell, char *buffer, uint16_t size);

uint16_t shell_write_string(shell_t *shell, const char *string);

void shell_handler(shell_t *shell, char data);

void shell_task(void *param);

int shell_run(shell_t *shell, const char *cmd);

shell_cmd_t *shell_seek_cmd(shell_t *shell,
                            const char *cmd,
                            shell_cmd_t *base,
                            uint16_t compare_length);

/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/                            
/*------------------------------- shell register -----------------------------*/                            
/*-----------------------------------------------------------------------------*/                            
/*-----------------------------------------------------------------------------*/                            
/**
 * ---------------------------------------
 *  number type
 * ---------------------------------------
 */
typedef enum shell_num_type_e
{
    NUM_TYPE_DEC,                      /**< decimal */
    NUM_TYPE_BIN,                      /**< binary */
    NUM_TYPE_OCT,                      /**< octal */
    NUM_TYPE_HEX,                      /**< hex */
    NUM_TYPE_FLOAT                     /**< float */
} SHELL_NUM_TYPE_E;

int shell_register_run(shell_t *shell, shell_cmd_t *command, int argc, char *argv[]);

#endif/**< __SHELL_H__ */
