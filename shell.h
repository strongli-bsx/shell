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

#ifndef __SHELL_H__
#define __SHELL_H__

#include "shell_cfg.h"
#include <stdint.h>


/*-----------------------------------------------------------------------------*/
/*! shell command type enum */
typedef enum {
    SHELL_TYPE_CMD_MAIN = 0,               /**< argc & argv */
    SHELL_TYPE_CMD_FUNC,                   /**< func */
    SHELL_TYPE_VAR_INT,                    /**< int */
    SHELL_TYPE_VAR_SHORT,                  /**< short */
    SHELL_TYPE_VAR_CHAR,                   /**< char */
    SHELL_TYPE_VAR_STRING,                 /**< string */
    SHELL_TYPE_VAR_POINT,                  /**< point */
    SHELL_TYPE_VAR_NODE,                   /**< node */
    SHELL_TYPE_USER,                       /**< user */
    SHELL_TYPE_KEY,                        /**< key */
} SHELL_CMD_TYPE_E;

/*! shell text enum */
enum {
    SHELL_TEXT_INFO,                        /**< shell info */
    SHELL_TEXT_CMD_TOO_LONG,                /**< cmd too long */
    SHELL_TEXT_CMD_LIST,                    /**< cmd list */
    SHELL_TEXT_VAR_LIST,                    /**< var list */
    SHELL_TEXT_USER_LIST,                   /**< user list */
    SHELL_TEXT_KEY_LIST,                    /**< key list */
    SHELL_TEXT_CMD_NOT_FOUND,               /**< cmd not found */
    SHELL_TEXT_POINT_CANNOT_MODIFY,         /**< point cannot modify */
    SHELL_TEXT_VAR_READ_ONLY_CANNOT_MODIFY, /**< var read only cannot modify */
    SHELL_TEXT_NOT_VAR,                     /**< cmd not var */
    SHELL_TEXT_VAR_NOT_FOUND,               /**< var not found */
    SHELL_TEXT_HELP_HEADER,                 /**< help header */
    SHELL_TEXT_PASSWORD_HINT,               /**< password hint */
    SHELL_TEXT_PASSWORD_ERROR,              /**< password error */
    SHELL_TEXT_CLEAR_CONSOLE,               /**< clear console */
    SHELL_TEXT_CLEAR_LINE,                  /**< clear line */
    SHELL_TEXT_TYPE_CMD,                    /**< cmd type */
    SHELL_TEXT_TYPE_VAR,                    /**< var type */
    SHELL_TEXT_TYPE_USER,                   /**< user type */
    SHELL_TEXT_TYPE_KEY,                    /**< key type */
    SHELL_TEXT_TYPE_NONE,                   /**< none type */
};
/*-----------------------------------------------------------------------------*/
/*! shell assert */
#define SHELL_ASSERT(expr) \
        if(!(expr)) {                                                         \
            shell_print("Assert failed: %s, line %d\n", __FILE__, __LINE__); \
            while(1) {                                                        \
            }                                                                 \
        }

/*! attrubuted section */
#define SHELL_SECTION(x)    __attribute__((section(x), aligned(1)))

/*! attrubuted used */
#define SHELL_USED          __attribute__((used))
/*-----------------------------------------------------------------------------*/
// #define SHELL_LOCK(shell)                   shell->lock(shell)
// #define SHELL_UNLOCK(shell)                 shell->unlock(shell)
#define SHELL_LOCK(s)
#define SHELL_UNLOCK(s)
/*-----------------------------------------------------------------------------*/
/*! shell cmd authority */
#define SHELL_CMD_PERMISSION(permission)   (permission & 0x000000FF)

/*! shell cmd type */
#define SHELL_CMD_TYPE(type)               ((type & 0x0000000F) << 8)

/*! shell cmd unchecked enable */
#define SHELL_CMD_ENABLE_UNCHECKED         (1 << 12)

/*! shell cmd return disable */
#define SHELL_CMD_DISABLE_RETURN           (1 << 13)

/*! shell cmd read only */
#define SHELL_CMD_READ_ONLY                (1 << 14)

/*! shell cmd param num */
#define SHELL_CMD_PARAM_NUM(num)           ((num & 0x0000000F)) << 16

/*! shell param float */
#define SHELL_PARAM_FLOAT(x)               (*(float *)(&x))
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
#define SHELL_EXPORT_CMD(_attr, _name, _func, _desc, ...)          \
        const char shellCmd##_name[] = #_name;                     \
        const char shellDesc##_name[] = #_desc;                    \
        SHELL_USED const shell_cmd_t                               \
        shell_cmd##_name SHELL_SECTION("shell_cmd") =              \
        {                                                          \
            .attr.value = _attr,                                   \
            .data.cmd.name = shellCmd##_name,                      \
            .data.cmd.function = (int (*)()) _func,                \
            .data.cmd.desc = shellDesc##_name,                     \
            ##__VA_ARGS__                                          \ 
        }

/**
 * -----------------------------------------------
 * @brief      shell var define
 * -----------------------------------------------
 * @param[in]  _attr : var attribute
 * @param[in]  _name : var name
 * @param[in]  _value: var value
 * @param[in]  _desc : var description
 * -----------------------------------------------
 */
#define SHELL_EXPORT_VAR(_attr, _name, _value, _desc)              \
        const char shellCmd##_name[] = #_name;                     \
        const char shellDesc##_name[] = #_desc;                    \
        SHELL_USED const shell_cmd_t                               \
        shellVar##_name SHELL_SECTION("shell_cmd") =               \
        {                                                          \
            .attr.value = _attr,                                   \
            .data.var.name = shellCmd##_name,                      \
            .data.var.value = (void *)_value,                      \
            .data.var.desc = shellDesc##_name                      \
        }


/**
 * -----------------------------------------------
 * @brief      shell user define
 * -----------------------------------------------
 * @param[in]  _attr : user attribute
 * @param[in]  _name : user name
 * @param[in]  _pasd : user password
 * @param[in]  _desc : user description
 * -----------------------------------------------
 */
#define SHELL_EXPORT_USER(_attr, _name, _pasd, _desc)              \
        const char shellCmd##_name[] = #_name;                     \
        const char shellPassword##_name[] = #_pasd;                \
        const char shellDesc##_name[] = #_desc;                    \
        SHELL_USED const shell_cmd_t                               \
        shellUser##_name SHELL_SECTION("shell_cmd") =              \
        {                                                          \
            .attr.value = _attr | SHELL_CMD_TYPE(SHELL_TYPE_USER), \
            .data.user.name = shellCmd##_name,                     \
            .data.user.password = shellPassword##_name,            \
            .data.user.desc = shellDesc##_name                     \
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
#define SHELL_EXPORT_KEY(_attr, _value, _func, _desc)             \
        const char shellDesc##_value[] = #_desc;                  \
        SHELL_USED const shell_cmd_t                              \
        shellKey##_value SHELL_SECTION("shell_cmd") =             \
        {                                                         \
            .attr.value = _attr | SHELL_CMD_TYPE(SHELL_TYPE_KEY), \
            .data.key.value = _value,                             \
            .data.key.function = (void (*)(shell_t *)) _func,     \
            .data.key.desc = shellDesc##_value                    \
        }

/*-----------------------------------------------------------------------------*/
/*! shell define struct */
typedef struct {

    /*! shell info */
    struct {
        const struct shell_command *sh_cmd;       /**< shell cmd pointer */
        int active_time;                          /**< shell active time */
        char *path;                               /**< current shell path */
#if SHELL_KEEP_RETURN_VALUE == 1
        int retVal; /**< 返回值 */
#endif
    } info;

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
        uint8_t is_checked : 1;             /**< password checked */
        uint8_t is_active  : 1;             /**< active shell */
        uint8_t tab_flag   : 1;             /**< tab flag */
    } status;

    /*! shell read & write function */
    signed short (*read)(char *, uint16_t);      /**< shell read function */
    signed short (*write)(char *, uint16_t);     /**< shell write function */

    /*! shell lock & unlock function */
    // int (*lock)(struct shell_def *);         /**< shell 加锁 */
    // int (*unlock)(struct shell_def *);       /**< shell 解锁 */
} shell_t;

/*! shell command define struct */
typedef struct shell_command {
    /*! shell cmd attribute */
    union {
        struct {
            uint8_t permission : 8;          /**< command authority */
            SHELL_CMD_TYPE_E type : 4;       /**< command type */
            uint8_t enable_unchecked : 1;    /**< enable with unchecked */
            uint8_t disable_return : 1;      /**< disable return value */
            uint8_t read_only : 1;           /**< read only */
            uint8_t reserve : 1;             /**< reserve */
            uint8_t param_num : 4;           /**< parameter number */
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

        /*! shell var define */
        struct {
            const char *name;               /**< var name */
            void *value;                    /**< var value */
            const char *desc;               /**< var description */
        } var;                              /**< var define */

        /** shell user define */
        struct {
            const char *name;               /**< user name */
            const char *pasd;               /**< user password */
            const char *desc;               /**< user description */
        } user;                             /**< user define */

        /*! shell key define */
        struct {
            int value;                      /**< key value */
            void (*function)(shell_t *);    /**< key execute function */
            const char *desc;               /**< key description */
        } key;                           

    } data;
} shell_cmd_t;

/*! shell node variable attribute struct */
typedef struct {
    void *var;    /**< 变量引用 */
    int (*get)(); /**< 变量get方法 */
    int (*set)(); /**< 变量set方法 */
} shell_node_var_attr_t;
/*-----------------------------------------------------------------------------*/
void shell_init(shell_t *shell, char *buffer, uint16_t size);
void shell_remove(shell_t *shell);
uint16_t shell_write_string(shell_t *shell, const char *string);
void shell_print(shell_t *shell, const char *fmt, ...);
void shell_scan(shell_t *shell, char *fmt, ...);
void shell_handler(shell_t *shell, char data);
void shell_write_end_line(shell_t *shell, char *buffer, int len);
void shell_task(void *param);
int shell_run(shell_t *shell, const char *cmd);
shell_t *shell_get_current(void);
int shell_get_var_value(shell_t *shell, shell_cmd_t *command);
shell_cmd_t *shell_seek_cmd(shell_t *shell,
                            const char *cmd,
                            shell_cmd_t *base,
                            uint16_t compare_length);

#endif/**< __SHELL_H__ */
