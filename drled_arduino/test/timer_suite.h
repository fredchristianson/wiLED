#ifndef TIMER_TEST_H
#define TIMER_TEST_H

#include "../lib/test/test_suite.h"
#include "../lib/json/parser.h"
#include "../script/script_timer.h"
#include "../script/script_context.h"

#if RUN_TESTS==1
namespace DevRelief {
    const char *SIMPLE_TIMER = R"script(
        {
            "timer": 5000
        }        
    )script"; 

    const char *ARRAY_TIMER = R"script(
        {
            "timer": [5000,2000,3]
        }        
    )script"; 

    const char *OBJECT_TIMER = R"script(
        {
            "timer": {
                "run": {
                    "duration": 100,
                    "enter": {
                        "xhue": ["+","var(xhue)",50],
                        "len": 1
                    },
                    "leave": {
                        "a": 0  
                    },
                    "step": {
                        "len": ["+","var(len)",1]
                    }
                },
                "pause": {
                    "duration": 100
                },
                "repeat": 2
            }
        }        
    )script"; 

class TimerTestSuite : public TestSuite{
    public:
        static TimerTestSuite::TestFn jsonTests[];

        static bool Run(ILogger* logger) {
            #if RUN_TIMER_TESTS
            TimerTestSuite test(logger);
            test.run();
            return test.isSuccess();
            #else
            return true;
            #endif
        }

        void run() {
            runTest("testMemLeakSimple",[&](TestResult&r){testMemLeakSimple(r);});
            runTest("testMemLeakArray",[&](TestResult&r){testMemLeakArray(r);});
            runTest("testMemLeakObject",[&](TestResult&r){testMemLeakObject(r);});
            runTest("testRun",[&](TestResult&r){testRun(r);});
        }

        TimerTestSuite(ILogger* logger) : TestSuite("Timer Tests",logger){

        }

    protected: 


    void testMemLeakSimple(TestResult& result);
    void testMemLeakArray(TestResult& result);
    void testMemLeakObject(TestResult& result);
    void testRun(TestResult& result);
};

void TimerTestSuite::testMemLeakSimple(TestResult& result) {
    JsonParser parser;
    JsonRoot* root = parser.read(SIMPLE_TIMER);
    JsonObject* obj = root->getTopObject();
    ScriptTimerValue timer(obj->getPropertyValue("timer"));
    root->destroy();
}

void TimerTestSuite::testMemLeakArray(TestResult& result) {
    JsonParser parser;
    JsonRoot* root = parser.read(ARRAY_TIMER);
    JsonObject* obj = root->getTopObject();
    ScriptTimerValue timer(obj->getPropertyValue("timer"));
    root->destroy();
}


void TimerTestSuite::testMemLeakObject(TestResult& result) {
    JsonParser parser;
    JsonRoot* root = parser.read(OBJECT_TIMER);
    JsonObject* obj = root->getTopObject();
    ScriptTimerValue timer(obj->getPropertyValue("timer"));
    root->destroy();
}

void TimerTestSuite::testRun(TestResult& result) {
    JsonParser parser;
    JsonRoot* root = parser.read(OBJECT_TIMER);
    JsonObject* obj = root->getTopObject();
    ScriptTimerValue timer(obj->getPropertyValue("timer"));
    RootContext context;
    ScriptStatus status = timer.getStatus(&context,SCRIPT_ERROR);
    result.assertEqual(status,SCRIPT_CREATED,"initial status");
    status = timer.updateStatus(&context);

    result.assertEqual(status,SCRIPT_RUNNING,"first run");
    IScriptValue * val = context.getValue("xhue");
    result.assertNotNull(val,"xhue IScriptValue");
    int xhue = val->getIntValue(&context,-1);
    result.assertEqual(xhue,50,"xhue value");
    val = context.getValue("len");
    result.assertNotNull(val,"len  IScriptValue");
    int len = val->getIntValue(&context,-1);
    result.assertEqual(len,1,"first update len value");
    

    status = timer.updateStatus(&context);
    result.assertEqual(status,SCRIPT_RUNNING,"step");
    
    val = context.getValue("xhue");
    result.assertNotNull(val,"xhue IScriptValue");
    xhue = val->getIntValue(&context,-1);
    result.assertEqual(xhue,50,"xhue value");
    val = context.getValue("len");
    result.assertNotNull(val,"len  IScriptValue");
    len = val->getIntValue(&context,-1);
    result.assertEqual(len,2,"second update len value");

    delay(110);
    status = timer.updateStatus(&context);
    result.assertEqual(status,SCRIPT_PAUSED,"first pause");
    delay(110);
    status = timer.updateStatus(&context);
    result.assertEqual(status,SCRIPT_RUNNING,"second run");

    val = context.getValue("xhue");
    result.assertNotNull(val,"xhue IScriptValue");
    xhue = val->getIntValue(&context,-1);
    result.assertEqual(xhue,100,"xhue value");
    val = context.getValue("len");
    result.assertNotNull(val,"len  IScriptValue");
    len = val->getIntValue(&context,-1);
    result.assertEqual(len,1,"second run len value");

    status = timer.updateStatus(&context);
    result.assertEqual(status,SCRIPT_RUNNING,"step");
    delay(110);
    status = timer.updateStatus(&context);
    result.assertEqual(status,SCRIPT_COMPLETE,"complete");

    root->destroy();
}


}
#endif 

#endif