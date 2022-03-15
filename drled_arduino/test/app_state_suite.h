#ifndef APP_STATE_SUITE_TEST_H
#define APP_STATE_SUITE_TEST_H

#include "../lib/test/test_suite.h"
#include "../app_state.h"
#include "../app_state_data_loader.h"

#if RUN_TESTS==1
namespace DevRelief {

class AppStateTestSuite : public TestSuite{
    public:
        static AppStateTestSuite::TestFn jsonTests[];

        static bool Run(ILogger* logger) {
            #if RUN_API_TESTS
            AppStateTestSuite test(logger);
            test.run();
            return test.isSuccess();
            #else
            return true;
            #endif
        }

        void run() {
            runTest("testStateLoader",[&](TestResult&r){testStateLoader(r);});
           // runTest("testParameters",[&](TestResult&r){testParameters(r);});
        }

        AppStateTestSuite(ILogger* logger) : TestSuite("Api Tests",logger){

        }

    protected: 


    void testStateLoader(TestResult& result);
   // void testParameters(TestResult& result);
};

void AppStateTestSuite::testStateLoader(TestResult& result) {
    AppState state;
    state.setApi("test",NULL);
    AppStateDataLoader loader;
    loader.save(state,"test_state");
    AppState state1;
    loader.load(state1,"test_state");
    result.assertTrue(Util::equal(state.getExecuteValue(),state1.getExecuteValue()));
}



}
#endif 

#endif