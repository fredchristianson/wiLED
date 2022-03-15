#ifndef SCRIPT_TEST_SUITE_H
#define SCRIPT_TEST_SUITE_H


#include "../lib/test/test_suite.h"
#include "../script/data_loader.h"
#include "../script/script.h"

#if RUN_TESTS==1
namespace DevRelief {

const char *HSL_SIMPLE_SCRIPT = R"script(
        {
            "name": "simple",
            "elements": [
            {
                "type": "hsl",
                "hue": 50
            }
            ]
        }        
    )script";    

class DummyStrip : public HSLFilter {
    public:
        DummyStrip(): HSLFilter(NULL) {}
        virtual ~DummyStrip() {}

        int getPixelsPerMeter() override { return 30;}
};

class ScriptTestSuite : public TestSuite{
    public:

        static bool Run(ILogger* logger) {
            ScriptTestSuite test(logger);
            test.run();
            return test.isSuccess();
        }

        void run() {
            runTest("scriptLifecycle",[&](TestResult&r){scriptLifecycle(r);});
        }

        ScriptTestSuite(ILogger* logger) : TestSuite("Script Tests",logger){
        }

    protected: 



    void scriptLifecycle(TestResult& result);
};


void ScriptTestSuite::scriptLifecycle(TestResult& result) {
    m_logger->debug("ScriptTestScript::scriptLifecycle");
    m_logger->showMemory();
    ScriptDataLoader loader;
    m_logger->debug("\tcreated loader");
    m_logger->showMemory();
    Script* script = loader.parse(HSL_SIMPLE_SCRIPT);
    m_logger->debug("\tparsed script %x",script);
    m_logger->showMemory();
    auto dummy  = new DummyStrip();
    HSLStrip strip(dummy);
    m_logger->debug("\tcreated strip");
    m_logger->showMemory();
    script->begin(&strip,NULL);
    m_logger->debug("\tbegin");
    m_logger->showMemory();
    script->step();
    m_logger->debug("\tstep");
    m_logger->showMemory();
    script->step();
    m_logger->debug("\tstep");
    m_logger->showMemory();
    script->destroy();
    m_logger->showMemory();
    m_logger->debug("\tdestroyed");
    m_logger->showMemory();
}


}
#endif 

#endif
