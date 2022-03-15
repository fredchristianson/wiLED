#ifndef API_SUITE_TEST_H
#define API_SUITE_TEST_H

#include "../lib/test/test_suite.h"
#include "../lib/data/api_result.h"

#if RUN_TESTS==1
namespace DevRelief {

class ApiTestSuite : public TestSuite{
    public:
        static ApiTestSuite::TestFn jsonTests[];

        static bool Run(ILogger* logger) {
            #if RUN_API_TESTS
            ApiTestSuite test(logger);
            test.run();
            return test.isSuccess();
            #else
            return true;
            #endif
        }

        void run() {
            runTest("testApiMemLeak",[&](TestResult&r){testApiMemLeak(r);});
            runTest("testApiParamsMemLeak",[&](TestResult&r){testApiParamsMemLeak(r);});
        }

        ApiTestSuite(ILogger* logger) : TestSuite("Api Tests",logger){

        }

    protected: 


    void testApiMemLeak(TestResult& result);
    void testApiParamsMemLeak(TestResult& result);
};

void ApiTestSuite::testApiMemLeak(TestResult& result) {
    ApiResult apiResult;
    apiResult.setCode(200);
    apiResult.setMessage("lights turned %s","off");
}

void ApiTestSuite::testApiParamsMemLeak(TestResult& result) {
    ApiResult apiResult;
    JsonRoot root;
    JsonObject* obj = root.getTopObject();
    obj->setString("test","test");
    apiResult.setData(obj);
    apiResult.setMessage("lights turned %s","off");
}


}
#endif 

#endif