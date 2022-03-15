#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include "../log/interface.h"

namespace DevRelief {

#if RUN_TESTS==1
    int nextTestObjectId=100;

    const char * SUCCEEDED = "succeeded";
    const char * FAILED = "failed";
    const char * UNKNOWN_TEST="-???-";

    class TestResult {
        public:
            TestResult(ILogger * logger) { 
                m_success = true;
                m_logger = (LOGGER_TYPE*) logger;    
            }

            void fail(const char * msg = UNKNOWN_TEST) { 
                if (msg != NULL) {
                    m_logger->error("test failed: %s",msg);
                }
                m_success = false;
            }
            
            void addResult(bool success,const char * msg) {
                if (!success) {
                    fail(msg);
                }
            }

            bool isSuccess() { return m_success;}

            bool assertEqual(int a, int b,const char * msg=UNKNOWN_TEST) {
                bool result = (a == b);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertEqual %s [ %s]:  %d == %d",(result ? SUCCEEDED : FAILED), msg,a,b);
                return result;
            }

            bool assertBetween(int value, int low, int high,const char * msg=UNKNOWN_TEST) {
                bool result = (value >=low && value <= high);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertBetween %s [ %s]:  %d <= %d <= %d",(result ? SUCCEEDED : FAILED), msg,low,value,high);
                return result;
            }

            bool assertEqual(const char * a, const char * b,const char * msg=UNKNOWN_TEST) {
                bool result = true;
                if (a==b) {
                    result = true;
                } else if ((a==NULL&&b!=NULL) || (a!=NULL&&b==NULL)){
                    result = false;
                } else {
                    result = strcmp(a,b) == 0;
                }
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertEqual strings %s [ %s]:  %s == %s",(result ? SUCCEEDED : FAILED), msg,a?a:"NULL",b?b:"NULL");
                return result;
            }

            bool assertEqual(void * a,void * b,const char * msg=UNKNOWN_TEST) {
                bool result = (a == b);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertEqual %s [ %s]:  %d == %d",(result ? SUCCEEDED : FAILED), msg,a,b);
                return result;
            }


            bool assertNotEqual(int a,int b,const char * msg=UNKNOWN_TEST) {
                bool result = (a != b);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertNotEqual %s [ %s]:  %d == %d",(result ? SUCCEEDED : FAILED), msg,a,b);
                return result;
            }


            bool assertNotEqual(const char* a,const char* b,const char * msg=UNKNOWN_TEST) {
                bool result = true;
                if (a == b) {
                    result = false;
                } else if ((a == NULL && b != NULL) ||(b == NULL && a != NULL)){
                    result = false;
                } else {
                    result = strcmp(a,b) != 0;
                }
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertNotEqual %s [ %s]:  %d != %d",(result ? SUCCEEDED : FAILED), msg,a,b);
                return result;
            }

            bool assertNotEqual(void* a,void * b,const char * msg=UNKNOWN_TEST) {
                bool result = (a != b);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertMptEqual %s [ %s]:  %d == %d",(result ? SUCCEEDED : FAILED), msg,a,b);
                return result;
            }


            bool assertNull(void*a,const char * msg=UNKNOWN_TEST) {
                bool result = (a == NULL);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertNull %s [ %s]:  %d",(result ? SUCCEEDED : FAILED), msg,a);
                return result;
            }

            bool assertNotNull(void*a,const char * msg=UNKNOWN_TEST) {
                bool result = (a != NULL);
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertNotNull %s [ %s]:  %d",(result ? SUCCEEDED : FAILED), msg,a);
                return result;
            }

            bool assertTrue(bool a,const char * msg=UNKNOWN_TEST) {
                bool result = a;
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertTrue %s [ %s]:  %d",(result ? SUCCEEDED : FAILED), msg,a);
                return result;
            }

            bool assertFalse(bool a,const char * msg=UNKNOWN_TEST) {
                bool result = !a;
                addResult(result,msg);
                m_logger->write(result ? INFO_LEVEL:ERROR_LEVEL,"assertFalse %s [ %s]:  %d",(result ? SUCCEEDED : FAILED), msg,a);
                return result;
            }

        private:
            DECLARE_LOGGER();
            bool m_success;
    };

    class TestObject {
        public:
        TestObject(){
            id = nextTestObjectId++;
            value = 0;
        }
        TestObject(int v){
            id = nextTestObjectId++;
            value = v;
        }

        int id;
        int value;
    };

    class TestSuite {
        public:
            typedef void (TestSuite::*TestFn)(TestResult &);

            TestSuite(const char * name, ILogger* logger,bool logTestMessages=false){
                m_logTestMessages = logTestMessages;
                m_name = name;
                m_logger = (LOGGER_TYPE*) logger;
            }
            virtual ~TestSuite(){
             
            }

            bool runTest(const char * name, auto  test){
                ILogConfig::Instance()->setTesting(m_logTestMessages);
                TestResult result(m_logger);
                int mem = ESP.getFreeHeap();
                m_logger->info("Run test: %s",name);
                LogIndent indent;
                m_logger->showMemory("memory before test");

                (test)(result);
                m_logger->debug("test complete");
                int endMem = ESP.getFreeHeap();
                m_logger->showMemory("memory after test");
                if (endMem != mem) {
                    m_logger->error("Memory Leak: %d bytes",endMem-mem);
                    result.fail("memory leak");
                }
                success = result.isSuccess() && success;
                ILogConfig::Instance()->setTesting(false);

                return result.isSuccess();
            }

            bool isSuccess() { return success;}
        protected:
            const char * m_name;
            DECLARE_LOGGER();
            bool m_logTestMessages;
            bool success;
    };

   
#endif                   
}
#endif
