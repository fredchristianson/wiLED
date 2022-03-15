#ifndef SCRIPT_LOADER_TEST_H
#define SCRIPT_LOADER_TEST_H


#include "../lib/test/test_suite.h"
#include "../script/data_loader.h"
#include "../script/script.h"

#if RUN_TESTS==1
namespace DevRelief {

const char *LOAD_SIMPLE_SCRIPT = R"script(
        {
            "name": "simple",
            "commands": [
            {
                "type": "hsl",
                "hue": 50
            }
            ]
        }        
    )script";    


class ScriptLoaderTestSuite : public TestSuite{
    public:

        static bool Run(ILogger* logger) {
            ScriptLoaderTestSuite test(logger);
            test.run();
            return test.isSuccess();
        }

        void run() {
            runTest("testScriptCommandMemLeak",[&](TestResult&r){memLeakScriptCommand(r);});
            runTest("testScriptTextToJsonLoaderMemLeak",[&](TestResult&r){testScriptTextToJsonLoaderMemLeak(r);});
            runTest("testScriptLoaderMemLeak",[&](TestResult&r){memLeak(r);});
        }

        ScriptLoaderTestSuite(ILogger* logger) : TestSuite("ScriptLoader Tests",logger){
        }

    protected: 


    void memLeakScriptCommand(TestResult& result);
    void testScriptTextToJsonLoaderMemLeak(TestResult& result);
    void memLeak(TestResult& result);
};

void ScriptLoaderTestSuite::memLeakScriptCommand(TestResult& result) {
    auto script = new Script();
    auto root = script->getRootContainer();
    auto* element = new HSLElement();
    root->add(element);
    script->destroy();
}


void ScriptLoaderTestSuite::testScriptTextToJsonLoaderMemLeak(TestResult& result) {
    m_logger->showMemory();
    m_logger->debug("before create ScriptDataLoader");
    ScriptDataLoader loader;
    m_logger->showMemory();
    m_logger->debug("call loader.toJson");
    JsonRoot* root = loader.toJson(LOAD_SIMPLE_SCRIPT);
    m_logger->debug("got JsonRoot");
    m_logger->showMemory();
    delete root;
    m_logger->debug("deleted root");
    m_logger->showMemory();

}
void ScriptLoaderTestSuite::memLeak(TestResult& result) {
    m_logger->showMemory();
    m_logger->debug("before create ScriptDataLoader");
    ScriptDataLoader loader;
    m_logger->showMemory();
    m_logger->debug("call loader.toJson");
    JsonRoot* root = loader.toJson(LOAD_SIMPLE_SCRIPT);
    m_logger->debug("got JsonRoot");
    m_logger->showMemory();
    m_logger->debug("call parseJson");
    Script* script = loader.parseJson(root);
    m_logger->debug("got script %x",script);
    m_logger->showMemory();
    m_logger->debug("destroy script");
    if (script) {script->destroy();}
    m_logger->debug("destroyed");
    m_logger->showMemory();
    m_logger->debug("destroy json root");
    delete root;
    m_logger->debug("deleted root");
    m_logger->showMemory();

}




}
#endif 

#endif
