#ifndef JSON_TEST_H
#define JSON_TEST_H


#include "../lib/test/test_suite.h"
#include "../script/data_loader.h"

#if RUN_TESTS == 1
namespace DevRelief
{
    const char *SIMPLE_SCRIPT = R"script(
            {
                "name": "simple"
            }        
        )script";    
        
        const char *ARRAY_SCRIPT = R"script(
            {
                "name": "with elements array",
                "elements": [
                {
                    "type": "hsl",
                    "hue": 250
                }
                ]
            }        
        )script";

  
    class JsonTestSuite : public TestSuite
    {
    public:
        static TestSuite::TestFn jsonTests[];

        static bool Run(ILogger *logger)
        {
#if RUN_JSON_TESTS
            JsonTestSuite test(logger);
            test.run();
            return test.isSuccess();
#else
            return true;
#endif
        }

        void run()
        {

            runTest("testJsonMemoryFree", [&](TestResult &r)
                    { testJsonMemory(r); });
            runTest("testParseSimple", [&](TestResult &r)
                    { testParseSimple(r); });
            runTest("testParseArray", [&](TestResult &r)
                    { testParseArray(r); });
            runTest("testGenerateSimple", [&](TestResult &r)
                    { testGenerateSimple(r); });
 
                                  
        }

        JsonTestSuite(ILogger *logger) : TestSuite("JSON Tests", logger,false)
        {
        }

    protected:
        void testJsonMemory(TestResult &result);
        void testParseSimple(TestResult &result);
        void testParseArray(TestResult &result);
        void testGenerateSimple(TestResult &result);
        //void testJsonValue(TestResult &result);
        //void testPosition(TestResult &result);
    };

    void JsonTestSuite::testJsonMemory(TestResult &result)
    {
        LogIndent li(m_logger,"testJsonMemory");
        JsonRoot root;
        JsonObject* obj = root.getTopObject();
        obj->setString("test","foo");
        obj->setInt("testi",1);

    }

    void JsonTestSuite::testParseSimple(TestResult &result)
    {
        LogIndent li(m_logger,"testParseSimple");
        m_logger->showMemory();
        JsonParser parser;
        m_logger->debug("Parse SIMPLE_SCRIPT");
        m_logger->showMemory();
        JsonRoot* root = parser.read(SIMPLE_SCRIPT);
        m_logger->debug("\tgot SIMPLE_SCRIPT");
        m_logger->showMemory();
        root->destroy();
        m_logger->debug("\tdestroyed root");
        m_logger->showMemory();
    }
    void JsonTestSuite::testParseArray(TestResult &result)
    {
        JsonParser parser;
        m_logger->debug("Parse ARRAY_SCRIPT");
        m_logger->showMemory();
        JsonRoot* root = parser.read(ARRAY_SCRIPT);
        m_logger->debug("\tgot ARRAY_SCRIPT");
        m_logger->showMemory();
        root->destroy();
        m_logger->debug("\tdestroyed ARRAY_SCRIPT");
        m_logger->showMemory();
    }

    void JsonTestSuite::testGenerateSimple(TestResult &result)
    {
        m_logger->debug("create JsonRoot");
        m_logger->showMemory();
        JsonRoot*  root=new JsonRoot();
        JsonObject* obj = root->getTopObject();
        obj->setString("test","foo");
        obj->setInt("testi",1);
        DRString* buffer = new DRString();
        m_logger->debug("create generator");
        m_logger->showMemory();
        JsonGenerator gen(*buffer);
        m_logger->debug("generate text");
        m_logger->showMemory();
        gen.generate(root);
        m_logger->debug("generated text. length=%d",buffer->getLength());
        m_logger->showMemory();
        delete buffer;
        m_logger->debug("deleted buffer");
        m_logger->showMemory();
        root->destroy();
        m_logger->debug("deleted root");
        m_logger->showMemory();
    }

}
#endif

#endif