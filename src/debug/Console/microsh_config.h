/**
 * \file            microsh_config.h
 * \brief           microSH default configurations file
 */

/*
 * Copyright (c) 2022 Dmitry KARASEV
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of microSH - Shell for Embedded Systems library.
 *
 * Author:          Dmitry KARASEV <karasevsdmitry@yandex.ru>
 * Version:         2.0.0-dev
 */

#ifndef MICROSH_HDR_DEFAULT_CONFIG_H
#define MICROSH_HDR_DEFAULT_CONFIG_H

/* Uncomment to ignore user options (or set macro in compiler flags) */
/* #define MICROSH_IGNORE_USER_OPTS */

/* Include application options */
#ifndef MICROSH_IGNORE_USER_CONFIGS
// #include "microsh_user_config.h"
#endif /* MICROSH_IGNORE_USER_CONFIGS */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * \defgroup        MICROSH_CONFIG Configuration
 * \brief           microSH configurations
 * \{
 */

/**
 * \brief           Maximum number of different commands to be registered
 */
#ifndef MICROSH_CFG_NUM_OF_CMDS
#define MICROSH_CFG_NUM_OF_CMDS 11
#endif

/**
 * \brief           Enable logging of command execution result
 */
#ifndef MICROSH_CFG_LOGGING_CMD_EXEC_RESULT
#define MICROSH_CFG_LOGGING_CMD_EXEC_RESULT 1
#endif

/**
 * \brief           Enable console sessions with authentication process
 */
#ifndef MICROSH_CFG_CONSOLE_SESSIONS
#define MICROSH_CFG_CONSOLE_SESSIONS 0
#endif

/**
 * \brief           Maximum number of sessions credentials
 */
#ifndef MICROSH_CFG_MAX_CREDENTIALS
#define MICROSH_CFG_MAX_CREDENTIALS 5
#endif

/**
 * \brief           Number of attempts to enter the session password
 */
#ifndef MICROSH_CFG_MAX_AUTH_ATTEMPTS
#define MICROSH_CFG_MAX_AUTH_ATTEMPTS 3
#endif

	/**
	 * \}
	 */

#define MICROSH_VERSION_MAJOR 2
#define MICROSH_VERSION_MINOR 0
#define MICROSH_VERSION_PATCH 0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MICROSH_HDR_DEFAULT_CONFIG_H */
