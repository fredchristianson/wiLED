#ifndef SCRIPT_DATA_LOADER_H
#define SCRIPT_DATA_LOADER_H

#include "../lib/log/logger.h"
#include "../lib/data/data_loader.h"
#include "../lib/json/parser.h"
#include "./script.h"
#include "./script_element.h"
#include "./script_container.h"
#include "./data_generator.h"

namespace DevRelief {

    class ScriptDataLoader : public DataLoader {
        public:
            ScriptDataLoader(){
                SET_LOGGER(ScriptLoaderLogger);
            }

            DRString getPath(const char * name) {
                int len = 0;
                while(Util::isNameChar(name[len])) {
                    len++;
                }
                DRString pname("/script/");
                pname.append(DRString(name,len));
                pname.append(".json");
                return pname;
            }


            Script* load(const char * name) {
                m_logger->debug("load script file: %s",name);
                Script* script = NULL;
                loadJsonFile(getPath(name), [&](JsonObject*obj){
                    script = parseJson(obj);
                    return true;
                });
                return script;
            }


            bool deleteScript(const char * name) {
                Script* script = NULL;
                DRString path =getPath(name);
                m_logger->debug("delete script file: %s (%s)",name,path.text());
                return deleteFile(path);
            }


            bool save(const char * name, Script& script) {
                m_logger->debug("save script file: %s",name);
                JsonRoot* newJson = toJson(script);
                m_logger->debug("Result script: %s",newJson->toString().get());
                bool result = writeJsonFile(getPath(name),newJson);
                newJson->destroy();
                return result;
            }

            bool save(const char * name, const char * text) {
                return writeFile(getPath(name),text);
                return false;
            }

            Script* parse(const char * text) {
                JsonParser parser;
                JsonRoot* root = parser.read(text);
                Script * script = parseJson(root);
                root->destroy();
                return script;
            }

            Script* parseJson(JsonRoot* json) {
                if (json == NULL || json->asObject() == NULL) {
                    m_logger->debug("No JsonRoot");
                    return NULL;
                }
                JsonObject* scriptJson = json->asObject();
                return parseJson(scriptJson);
            }

            Script* parseJson(JsonObject* scriptJson) {
                m_logger->debug("parseJson");
                Script* script = new Script();
                m_logger->debug("set name");
                script->setName(scriptJson->getString("name","unnamed"));
                m_logger->debug("set duration");
                script->setDuration(scriptJson->getInt("duration",0));
                m_logger->debug("set frequency");
                script->setFrequency(scriptJson->getInt("frequency",50));
                m_logger->debug("get elements");
                JsonArray* elements = scriptJson->getArray("elements");
                m_logger->debug("get container");
                ScriptRootContainer * container = script->getRootContainer();
                container->fromJson(scriptJson);
                m_logger->debug("parse elements in container %x",container);
                m_logger->debug("parse done");

#ifdef OFF
                m_logger->debug("script toJson");
                    JsonRoot* newJson = toJson(*script);
                    m_logger->debug("Result script: %s",newJson->toString().get());
                    newJson->destroy();
#endif
                m_logger->debug("\tparseJson done -> %x",script);
                return script;
            }

            JsonRoot* toJson(const char * text) {
                JsonParser parser;
                JsonRoot* root = parser.read(text);
                return root;
            }

            JsonRoot* toJson(Script& script) {
                m_logger->debug("toJson");
                ScriptDataGenerator gen;
                JsonRoot* root = gen.toJson(&script);
                m_logger->debug("\ttoJson --> %x",root);
                return root;
            }

     

        private:
            DECLARE_LOGGER();
    };

};

#endif
