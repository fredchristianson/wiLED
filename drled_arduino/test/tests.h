#ifndef TESTS_H
#define TESTS_H

#if RUN_TESTS == 1
#include "../lib/test/test_suite.h"
#include "../lib/log/logger.h"
#include "../lib/log/config.h"
#include "./json_suite.h"
#include "./string_suite.h"
#include "./api_suite.h"
#include "./script_loader_suite.h"
#include "./script_suite.h"
#include "./app_state_suite.h"
#include "./timer_suite.h"
#endif 

namespace DevRelief {

#if RUN_TESTS!=1
    class Tests {
        public:
        static bool Run() {
            return true;
        }


    };
#else    
    class Tests {
        public:
        static bool Run() {

            Tests* tests = new Tests();
            bool result = tests->run();
            delete tests;
            return true; //result;
        }

        Tests() {

        }

        bool run() {
            SET_LOGGER(TestLogger);
            LogConfig::Instance()->setTesting(true);
            EspBoard.delayMsecs(1000); // wait for serial logger to settle
            m_logger->always("force logging to allocate static buffers in order to detect real memory leaks: %f %d %s %d",.123,4567,"abc",true);
            int startHeap = EspBoard.getFreeHeap();
            m_logger->showMemory();
            bool success = true;
            
            #ifdef RUN_JSON_TESTS
            success = JsonTestSuite::Run(m_logger) && success;
            #endif

            #ifdef RUN_STRING_TESTS
            success = StringTestSuite::Run(m_logger) && success;
            #endif

            #if RUN_ANIMATION_TESTS==1
            success = AnimationTestSuite::Run(m_logger) && success;
            #endif
            #if SCRIPT_LOADER_TESTS==1
            success = ScriptLoaderTestSuite::Run(m_logger) && success;
            #endif
            #if RUN_API_TESTS==1
            success = ApiTestSuite::Run(m_logger) && success;
            #endif
            #if RUN_SCRIPT_TESTS==1
            success = ScriptTestSuite::Run(m_logger) && success;
            #endif
            #if RUN_APP_STATE_TESTS==1
            success = AppStateTestSuite::Run(m_logger) && success;
            #endif

            #if RUN_TIMER_TESTS==1
            success = TimerTestSuite::Run(m_logger) && success;
            #endif
            //success = runTest("testSharedPtr",&Tests::testSharedPtr) && success;
            //success = runTest("testStringBuffer",&Tests::testStringBuffer) && success;
            //success = runTest("testDRString",&Tests::testDRString) && success;
            //success = runTest("testDRStringCopy",&Tests::testDRStringCopy) && success;
            //success = runTest("testData",&Tests::testData) && success;
            //success = runTest("testConfigLoader",&Tests::testConfigLoader) && success;
            //success = runTest("testList",&Tests::testList) && success;
            //success = runTest("testPtrList",&Tests::testPtrList) && success;

            m_logger->debug("test suites done");

            int endHeap = ESP.getFreeHeap();
            m_logger->debug("end heap %d",endHeap);
            if (endHeap != startHeap) {
                m_logger->error("Leaked %d bytes of memory",startHeap-endHeap);
                success = false;
            }

            m_logger->info("tests complete: %s",(success ? "success": "fail"));

            LogConfig::Instance()->setTesting(false);
            return success;
        }

   
        private:
            DECLARE_LOGGER();        
    };
#endif                   
}
#endif
