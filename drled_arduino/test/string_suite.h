#ifndef STRING_TEST_H
#define STRING_TEST_H

#include "../lib/test/test_suite.h"

#if RUN_TESTS==1
namespace DevRelief {

class StringTestSuite : public TestSuite{
    public:
        static StringTestSuite::TestFn jsonTests[];

        static bool Run(ILogger* logger) {
            #if RUN_STRING_TESTS
            StringTestSuite test(logger);
            test.run();
            return test.isSuccess();
            #else
            return true;
            #endif
        }

        void run() {
            runTest("testReturnDRStringValue",[&](TestResult&r){testReturnDRStringValue(r);});
        }

        StringTestSuite(ILogger* logger) : TestSuite("JSON Tests",logger){

        }

    protected: 


    void testReturnDRStringValue(TestResult& result);
};

void StringTestSuite::testReturnDRStringValue(TestResult& result) {
    m_logger->debug("Running JSON Value tests");
}


}
#endif 

#endif