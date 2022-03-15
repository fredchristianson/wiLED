
#include "./env.h"
#include "./lib/application.h"
#include "./lib/log/logger.h"
#include "./loggers.h"
#include "./lib/task/task.h"
#include "./drled_app.h"

#if RUN_TESTS
#include "./test/tests.h"
#endif

using namespace DevRelief;



Application * app=NULL;
ILogConfig* logConfig=NULL;

void setup() {

#if LOGGING_ON==1
  logConfig = new LogConfig(new LogSerialDestination(), new LogDefaultFilter(DEBUG_LEVEL));
#else 
  logConfig = new NullLogConfig();
#endif 

#if RUN_TESTS==1
  if (!Tests::Run()) {
    DevRelief::AppLogger->error("Tests failed.  Not running application.");
  }
#endif  
  app = new DRLedApplication();
  wdt_enable(WDTO_4S);
}


void loop() {
  Tasks::Run();
  if (app) {
    app->loop();
  }
  wdt_reset();
}
