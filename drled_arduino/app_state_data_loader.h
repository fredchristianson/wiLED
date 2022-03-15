#ifndef APP_STATE_DATA_LOADER_H
#define APP_STATE_DATA_LOADER_H

#include "./lib/log/interface.h"
#include "./lib/data/data_object.h"
#include "./lib/data/data_loader.h"
#include "./lib/json/parser.h"
#include "./app_state.h"


namespace DevRelief {

const char * STATE_PATH_BASE="/state/";

class AppStateDataLoader : public DataLoader {
    public:
        AppStateDataLoader() {
            SET_LOGGER(AppStateLoaderLogger);
        }

        DRString getPath(const char * name) {
            DRString path= STATE_PATH_BASE;
            path += name;
            path += ".json";
            m_logger->debug(LM("AppStateDataLoader getPath(%s)==>%s"),name,path.text());
            return path;
        }


        bool save(AppState& state, const char * name = "state"){
            m_logger->debug(LM("save AppState"));

            JsonRoot* jsonRoot = toJson(state);
            
            bool success = writeJsonFile(getPath(name),jsonRoot->getTopElement());
            m_logger->debug(LM("\twrite %s"),success?"success":"failed");
            jsonRoot->destroy();
            return success;
        }

        bool load(AppState& state,const char * name ="state") {
            return loadJsonFile(getPath(name),[&](JsonObject * obj) {
                if (obj != NULL) {
                    state.setExecuteType((ExecuteType)obj->getInt("type",EXECUTE_NONE));
                    state.setExecuteValue(obj->getString("value",(const char *)NULL));
                    state.setIsRunning(obj->getBool("is-running",false));
                    state.setIsStarting(obj->getBool("is-starting",false));
                    state.setParameters(obj->getChild("parameters"));
                    auto paramJson = state.getParameters();
                    DRString json = paramJson? paramJson->toString() : DRString();
                    m_logger->debug(LM("Load AppState: %s %s %d %s %s"),
                                state.isStarting()?"starting":"",
                                state.isRunning()?"running":"",
                                (int)state.getType(),
                                state.getExecuteValue(),
                                json.text());

                    return true;
                }
                return false;
            });
        }

        JsonRoot* toJson(AppState& state) {
            JsonRoot* jsonRoot = new JsonRoot();
            JsonObject* obj = jsonRoot->getTopObject();
            obj->setInt("type",(int)state.getType());
            obj->setBool("is-running",state.isRunning());
            obj->setBool("is-starting",state.isStarting());
            obj->setString("value",state.getExecuteValue());
            JsonObject* params = obj->createObject("parameters");
            state.copyParameters(params);
            return jsonRoot;            
        }

      
    protected:

    private:
};
};
#endif
