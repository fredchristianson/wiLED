#ifndef DRLED_APP_STATE_H
#define DRLED_APP_STATE_H

#include "./lib/log/logger.h"
#include "./lib/json/json.h"

namespace DevRelief {

    typedef enum ExecuteType  {
        EXECUTE_NONE=0,
        EXECUTE_API=1,
        EXECUTE_SCRIPT=2
    };

    class AppState {
        public:
 
            AppState() {
                SET_LOGGER(AppStateLogger);
                m_executeType = EXECUTE_NONE;
                m_starting = true;
                m_running = false;
            }

            ~AppState() { }


            void setIsStarting(bool is) { 
                m_logger->debug(LM("setIsStarting %d"),is?1:0);
                m_starting = is;
            }

            void setIsRunning(bool is) { 
                m_logger->debug(LM("setIsRunning %d"),is?1:0);
                m_running = is;
            }
            
            void setExecuteType(ExecuteType type) { m_executeType = type;}
            void setExecuteValue(const char * val) { 
                m_logger->debug("\t\tsetExecuteValue(%s)",val?val : "<null>");
                m_executeValue = val;
                m_logger->debug("\t\tsetExecuteValue() done %s",val?val : "<null>");
            }
            void setParameters(JsonObject* params) {
                if (params == NULL){
                    m_logger->debug(LM("AppState parameters = NULL"));
                } else {
                    m_logger->debug(LM("set parameters %s"),params->toString().text());
                }
                JsonObject* paramObject = m_parameterRoot.getTopObject();
                copyParameters(paramObject,params);
            }

            void setApi(const char * api, JsonObject*params) {
                setExecuteValue(api);
                // when setApi is called, the API is running and has started
                setIsRunning(true);
                setIsStarting(false);
                setExecuteType(EXECUTE_API);
                copyParameters(m_parameterRoot.getTopObject(),params);
            }
             
            void setScript(const char * name, JsonObject*params) {
                m_logger->debug(LM("set script %s"),name?name:"<no name>");
                setExecuteValue(name);
                m_logger->debug(LM("\tname set %s => %s"),name?name:"<no name>",m_executeValue.text());
                m_logger->debug(LM("\tset isRunning %s"),name);
                setIsRunning(true);
                // set as starting until it has run long
                // enough to see it works (10 seconds?).
                // otherwise can get in a reboot-loop
                setIsStarting(true);
                setExecuteType(EXECUTE_SCRIPT);
                m_logger->debug(LM("\tcopy parameters"));
                copyParameters(m_parameterRoot.getTopObject(),params);
                m_logger->debug(LM("\tcopied parameters"));
                m_logger->debug(LM("\tsetScript done %s"),m_executeValue.text());
            }

            void copyParameters(JsonObject*  toObj,JsonObject*params=NULL) {
                m_logger->debug(LM("copyParameters %x"),toObj);
                if (toObj == params) {
                    return; // no change
                }
                if (toObj == NULL) {
                    m_logger->info(LM("\tno toObj"));
                     return;
                }
                toObj->clear();
                m_logger->debug(LM("\tcleared"));
                if (params == NULL){
                    m_logger->debug(LM("\tget current"));
                    params = m_parameterRoot.getTopObject();
                }
                if (params == NULL) {
                    m_logger->debug(LM("\tno params"));

                    return;
                }

                m_logger->debug(LM("\teachProperty"));

                params->eachProperty([&](const char * name, IJsonElement*val){
                    m_logger->debug(LM("\tcopy %s"),name);
                    IJsonValueElement* ve = val->asValue();
                    if (ve) {
                        toObj->setString(name,ve->getString(NULL));
                    } else {
                        m_logger->error("AppState only supports string parameter values");
                    }
                });
                m_logger->debug(LM("done"));

            }

            JsonObject* getParameters() { return m_parameterRoot.getTopObject();}
            const char* getExecuteValue()const { return m_executeValue.text();}
            bool isStarting() { return m_starting;}
            bool isRunning() { return m_running;}
            ExecuteType getType() { return m_executeType;}
    private:            
            DRString        m_executeValue;
            bool            m_starting;
            bool            m_running;
            ExecuteType     m_executeType;
            JsonRoot        m_parameterRoot;
            DECLARE_LOGGER();
    };

}


#endif