/**
 * ********************************************************
 * \file      shell_cfg.h
 * \brief     shell basic configuration
 * \version   5.0.0
 * \author    Awesome
 * \copyright (c) 2019 Letter
 * \copyright (c) 2025, Awesome
 * ********************************************************
 * \note      revision note
 * |   Date    |  version  |  author   | Description
 * |2019-12-30 |    1.0    |  Letter   | init version
 * |2025-11-03 |    1.1    |  Awesome  | add new feature
 * ********************************************************
 */

#ifndef __SHELL_CFG_H__
#define __SHELL_CFG_H__

#define  SHELL_VERSION                        "5.0.0"      /**< shell version */

#define  SHELL_DEFAULT_USER                   "letter"     /**< default user */

#define  SHELL_DEFAULT_USER_PASSWORD          "123"        /**< default user password */

#define  SHELL_MAX_NUMBER                      5           /**< max number of shell struct */

#define  SHELL_HELP_LIST_USER                  0           /**< whether to list user in help */

#define  SHELL_HELP_LIST_VAR                   0           /**< whether to list variable in help */

#define  SHELL_HELP_LIST_KEY                   0           /**< whether to list key in help */

#define  SHELL_HELP_SHOW_PERMISSION            1           /**< whether to show permission in help */

#define  SHELL_ENTER_LF                        1           /**< whether to use LF as enter */

#define  SHELL_ENTER_CR                        1           /**< whether to use CR as enter */

#define  SHELL_ENTER_CRLF                      0           /**< whether to use CRLF as enter， CANNOT use with SHELL_ENTER_LF or SHELL_ENTER_CR */

#define  SHELL_PARAMETER_MAX_NUMBER            8           /**< max number of shell parameter */

#define  SHELL_HISTORY_MAX_NUMBER              5           /**< max number of shell history */

#define  SHELL_SUPPORT_END_LINE                0           /**< whether to support end line */

#define  SHELL_TASK_WHILE                      1           /**< whether to use default shell task while loop */

#define  SHELL_DOUBLE_CLICK_TIME               200         /**< double click time(ms), used in SHELL_LONG_HELP, double click tab to complete help */

#define  SHELL_QUICK_HELP                      1           /**< whether to use quick help, double click tab to show cmd info */

#define  SHELL_KEEP_RETURN_VALUE               0           /**< whether to keep return value of last command */

#define  SHELL_PRINT_BUFFER                    128         /**< shell formatted output buffer size */

#define  SHELL_SCAN_BUFFER                     0           /**< shell formatted input buffer size */

#define  SHELL_USING_LOCK                      0           /**< whether to use shell lock */

#define  SHELL_CLS_WHEN_LOGIN                  1           /**< whether to clear screen when login */

#define  SHELL_LOCK_TIMEOUT               (0 * 60 * 1000)  /**< shell lock timeout(ms), used in double click tab */


#ifndef SHELL_GET_TICK
/**
 * @brief get system tick(ms)
 *        define this macro to get system tick(ms), such as `HAL_GetTick()`
 * @note this macro is not defined, double click tab to complete help and shell timeout lock are not supported
 */
#define     SHELL_GET_TICK()                   0
#endif /** SHELL_GET_TICK */


#endif
