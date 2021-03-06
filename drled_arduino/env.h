#ifndef ENV_H
#define ENV_H

#define PROD 1
#define DEV 2
#define ENV PROD

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__
#define BUILD_VERSION "2.0.1"
#define HOSTNAME "dr_led"

// LOGGING_ON should be 1 to enable logging.  0 optimizes all logging calls and constants (messages) out.

#define LOGGING_ON 1
#define ADAFRUIT_LED_LOGGER_LEVEL   ERROR_LEVEL
#define ANIMATION_LOGGER_LEVEL      WARN_LEVEL
#define API_RESULT_LOGGER_LEVEL     ERROR_LEVEL
#define APP_LOGGER_LEVEL            INFO_LEVEL
#define APP_STATE_LOGGER_LEVEL      WARN_LEVEL
#define APP_STATE_LOADER_LOGGER_LEVEL      WARN_LEVEL
#define COLOR_LOGGER_LEVEL          WARN_LEVEL
#define CONFIG_LOGGER_LEVEL          WARN_LEVEL
#define CONFIG_LOADER_LOGGER_LEVEL  WARN_LEVEL
#define COMPOUND_STRIP_LOGGER_LEVEL WARN_LEVEL
#define DATA_OBJECT_LOGGER_LEVEL    WARN_LEVEL
#define DR_BUFFER_LOGGER_LEVEL      ERROR_LEVEL
#define DRSTRING_LOGGER_LEVEL       ERROR_LEVEL
#define FILE_SYSTEM_LOGGER_LEVEL    ERROR_LEVEL
#define HSL_STRIP_LOGGER_LEVEL      ERROR_LEVEL
#define HTTP_SERVER_LOGGER_LEVEL    WARN_LEVEL
#define JSON_LOGGER_LEVEL           WARN_LEVEL
#define JSON_GENERATOR_LOGGER_LEVEL ERROR_LEVEL
#define JSON_PARSER_LOGGER_LEVEL    ERROR_LEVEL
#define LED_LOGGER_LEVEL            ERROR_LEVEL
#define LINKED_LIST_LOGGER_LEVEL    ERROR_LEVEL
#define MEMORY_LOGGER_LEVEL             WARN_LEVEL
#define PTR_LIST_LOGGER_LEVEL           ERROR_LEVEL
#define SCRIPT_LOGGER_LEVEL             WARN_LEVEL
#define SCRIPT_CONTAINER_LOGGER_LEVEL   WARN_LEVEL
#define SCRIPT_ELEMENT_LOGGER_LEVEL     WARN_LEVEL
#define SCRIPT_HSLSTRIP_LOGGER_LEVEL    WARN_LEVEL
#define SCRIPT_EXECUTOR_LOGGER_LEVEL    WARN_LEVEL
#define SCRIPT_LOADER_LOGGER_LEVEL      WARN_LEVEL
#define SCRIPT_POSITION_LOGGER_LEVEL     WARN_LEVEL
#define SCRIPT_TIMER_LOGGER_LEVEL     ERROR_LEVEL
#define SCRIPT_VALUE_LOGGER_LEVEL       WARN_LEVEL
#define STRIP_ELEMENT_LOGGER_LEVEL      INFO_LEVEL
#define SHARED_PTR_LOGGER_LEVEL         WARN_LEVEL  
#define TASK_LOGGER_LEVEL               ERROR_LEVEL
#define TEST_LOGGER_LEVEL               INFO_LEVEL
#define UTIL_LOGGER_LEVEL               ERROR_LEVEL
#define WIFI_LOGGER_LEVEL               ERROR_LEVEL

#if ENV==PROD
    #define ENV_PROD
    #define RUN_TESTS 0
    #define LOGGING_ON 0
#else
    #define ENV_DEV
    #define DEBUG


    // RUN_TESTS should be 1 to run tests on start.  otherwise they are not run
    #define RUN_TESTS 0
    #define RUN_STRING_TESTS 0
    #define RUN_JSON_TESTS 0
    #define RUN_ANIMATION_TESTS 0
    #define SCRIPT_LOADER_TESTS 0
    #define RUN_API_TESTS 0
    #define RUN_APP_STATE_TESTS 0
    #define RUN_SCRIPT_TESTS 0
    #define RUN_TIMER_TESTS 1
#endif

#endif