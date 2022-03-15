#ifndef LOGGERS_H
#define LOGGERS_H

#include "./env.h"
#include "./lib/log/logger.h"

namespace DevRelief
{
    

#ifdef DEBUG
ILogger* AdafruitLogger = new DRLogger("AdafruitLED",ADAFRUIT_LED_LOGGER_LEVEL);
ILogger* AnimationLogger = new DRLogger("Animation",ANIMATION_LOGGER_LEVEL);
ILogger* ApiResultLogger = new DRLogger("ApiResult",API_RESULT_LOGGER_LEVEL);
ILogger* AppLogger = new DRLogger("DRLed",APP_LOGGER_LEVEL);
ILogger* AppStateLogger = new DRLogger("AppState",APP_STATE_LOGGER_LEVEL);
ILogger* AppStateLoaderLogger = new DRLogger("AppStateDataLoader",APP_STATE_LOGGER_LEVEL);
ILogger* ColorLogger = new DRLogger("Color",COLOR_LOGGER_LEVEL);
ILogger* CompoundLogger = new DRLogger("CompoundStrip",COMPOUND_STRIP_LOGGER_LEVEL);
ILogger* ConfigLogger = new DRLogger("Config",CONFIG_LOGGER_LEVEL);
ILogger* ConfigLoaderLogger = new DRLogger("Config Loader",CONFIG_LOADER_LOGGER_LEVEL);
ILogger* DataObjectLogger = new DRLogger("DataObject",DATA_OBJECT_LOGGER_LEVEL);
ILogger* DRBufferLogger = new DRLogger("DRBuffer",DR_BUFFER_LOGGER_LEVEL);
ILogger* FileSystemLogger = new DRLogger("FileSystem",FILE_SYSTEM_LOGGER_LEVEL);
ILogger* GeneratorLogger = new DRLogger("JsonGenerator",JSON_GENERATOR_LOGGER_LEVEL);
ILogger* HSLStripLogger = new DRLogger("HSLStrip",HSL_STRIP_LOGGER_LEVEL);
ILogger* HttpServerLogger = new DRLogger("HTTPServer",HTTP_SERVER_LOGGER_LEVEL);
ILogger* JsonLogger = new DRLogger("Json",JSON_LOGGER_LEVEL);
ILogger* JsonParserLogger = new DRLogger("Json",JSON_PARSER_LOGGER_LEVEL);
ILogger* JsonGeneratorLogger = new DRLogger("Json",JSON_GENERATOR_LOGGER_LEVEL);
ILogger* LEDLogger = new DRLogger("LED",LED_LOGGER_LEVEL);
ILogger* LinkedListLogger = new DRLogger("LinkedList",LINKED_LIST_LOGGER_LEVEL);
ILogger* MemoryLogger = new DRLogger("Memory",MEMORY_LOGGER_LEVEL);
ILogger* PtrListLogger = new DRLogger("PtrList",PTR_LIST_LOGGER_LEVEL);
ILogger* ParserLogger = new DRLogger("JsonParser",JSON_PARSER_LOGGER_LEVEL);
ILogger* ScriptLogger = new DRLogger("Script",SCRIPT_LOGGER_LEVEL);
ILogger* ScriptContainerLogger = new DRLogger("ScriptContainer",SCRIPT_CONTAINER_LOGGER_LEVEL);
ILogger* ScriptElementLogger = new DRLogger("ScriptElement",SCRIPT_ELEMENT_LOGGER_LEVEL);
ILogger* ScriptHSLStripLogger = new DRLogger("ScriptHSLStrip",SCRIPT_HSLSTRIP_LOGGER_LEVEL);
ILogger* ScriptLoaderLogger = new DRLogger("ScriptLoader",SCRIPT_LOADER_LOGGER_LEVEL);
ILogger* ScriptPositionLogger = new DRLogger("ScriptPosition",SCRIPT_POSITION_LOGGER_LEVEL);
ILogger* ScriptTimerLogger = new DRLogger("Timer",SCRIPT_TIMER_LOGGER_LEVEL);
ILogger* ScriptValueLogger = new DRLogger("ScriptValue",SCRIPT_VALUE_LOGGER_LEVEL);
ILogger* ScriptExecutorLogger = new DRLogger("ScriptExecutor",SCRIPT_EXECUTOR_LOGGER_LEVEL);
ILogger* SharedPtrLogger = new DRLogger("SharedPtr",SHARED_PTR_LOGGER_LEVEL);
ILogger* StripElementLogger = new DRLogger("StripElement",STRIP_ELEMENT_LOGGER_LEVEL);
ILogger* StringLogger = new DRLogger("DRString",DRSTRING_LOGGER_LEVEL);
ILogger* TaskLogger = new DRLogger("Tasks",TASK_LOGGER_LEVEL);
ILogger* TestLogger = new DRLogger("Tests",TEST_LOGGER_LEVEL);
ILogger* UtilLogger = new DRLogger("Util",UTIL_LOGGER_LEVEL);
ILogger* WifiLogger = new DRLogger("WiFi",WIFI_LOGGER_LEVEL);

ILogger* FiveSecondLogger = new PeriodicLogger(5000,"5 Second Logger",ERROR_LEVEL); // log at most every 5 seconds
#endif

}

#endif