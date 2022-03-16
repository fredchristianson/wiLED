#ifndef CONFIG_DATA_LOADER_H
#define CONFIG_DATA_LOADER_H

#include "./lib/log/logger.h"
#include "./lib/system/epoch_time.h"
#include "./lib/data/data_object.h"
#include "./lib/data/data_loader.h"
#include "./lib/json/parser.h"
#include "./config.h"


namespace DevRelief {

const char * CONFIG_PATH_BASE="/";


class ConfigDataLoader : public DataLoader {
    public:
        ConfigDataLoader() {
            SET_LOGGER(ConfigLoaderLogger);
        }

        DRString getScriptPath(const char * name) {
            DRString path = "/script/";
            m_logger->debug("getScriptPath %s",path.text());
            path += name;
            if (!Util::endsWith(name,".json")){
                path += ".json";
            }
            m_logger->debug(LM("ConfigDataLoader getScriptPath(%s)==>%s"),name,path.text());
            
            return path;

        }
        DRString getPath(const char * name) {
            DRString path= CONFIG_PATH_BASE;
            path += name;
            if (!Util::endsWith(name,".json")){
                path += ".json";
            }
            m_logger->debug(LM("ConfigDataLoader getPath(%s)==>%s"),name,path.text());
            return path;
        }

        bool initialize(Config& config) {
            config.clearScripts();
            config.clearPins();
            config.setBrightness(40);
            config.setMaxBrightness(100);
            addScripts(config);
            return true;
        }

        bool addScripts(Config&config) {
            LinkedList<DRString> files;
            m_logger->debug(LM("adding scripts"));

            if (m_fileSystem.listFiles("/script",files)){
                m_logger->debug(LM("\tcall config.setScripts"));
                config.clearScripts();
                files.each([&](DRString& file) {
                    const char * fname = file.text();

                    int len = strlen(fname);
                    if (len > 5 && strcmp(fname+len-5,".json")==0) {
                        DRString fileName(fname,len-5);
                        DRString name = getScriptName(fname,fileName);
                        config.addScript(fileName, name);
                    } else {
                        m_logger->error("unknown file in script dir %s",fname);
                    }
                });
                
            }
            return true;
        }

        DRString getScriptName(const char * filename, const char * defaultName) {
            m_logger->debug("getScriptName %s",filename);
            DRString path = getScriptPath(filename);
            DRBuffer buf(200);
            int len = m_fileSystem.readBinary(path,buf.reserve(199),199);
            if (len>0){
                buf.setLength(len);
                char* text = (char*)buf.data();
                text[len-1] = 0;
                m_logger->debug("find name in %s",text);
                JsonParser parser;
                DRString name = findName(text);
                m_logger->debug("found name %s",name.text());
                if (name.getLength()>0) {
                    return name;
                }
            }
            m_logger->debug("use default name %s",defaultName);

            return defaultName;
            
        }

        DRString findName(char * name) {
            const char *pos = Util::next(name,"\"name\"");
            if (pos) {
                m_logger->debug("found name property: %s",pos);
                const char * start = Util::next(pos+8,"\"");
                if (start) {
                    start += 1;
                    m_logger->debug("found name value: %s",start);
                    const char* end = Util::next(start,"\"");
                    if (end) {
                        m_logger->debug("found end value: %s",start);
                        return DRString(start,end-start);
                    }
                }
            }
            return "";
        }


        bool updateConfig(Config& config, const char * jsonText){
            JsonParser parser;
            m_logger->debug(LM("read config"));
            JsonRoot * root = parser.read(jsonText);
            if (root == NULL) {
                m_logger->debug(LM("no JSON"));
                return false;
            } else {
                m_logger->debug(LM("get Config from JSON"));
                if (readJson(config,root->getTopObject())) {
                    m_logger->debug(LM("save Config"));
                    root->destroy();
                    return save(config);
                }
                root->destroy();
            }
            return false;
        }

        bool save(Config& config){
            m_logger->debug(LM("save Config"));
            JsonRoot* jsonRoot = toJson(config);
            
            bool success = writeJsonFile(getPath("config"),jsonRoot->getTopElement());
            m_logger->debug(LM("\twrite %s"),success?"success":"failed");
             jsonRoot->destroy();
            return success;
        }

        bool load(Config& config) {
            return loadJsonFile(getPath("config"),[&](JsonObject * obj) {
                if (obj != NULL) {
                    return readJson(config,obj);
                }
                return false;
            });
        }

        bool readJson(Config& config, JsonObject* root) {
           JsonElement * top = root;
            m_logger->debug(LM("\tgot top %s"),(top?"yes":"no"));
            if (top == NULL) {
                return false;
            }
            JsonObject*object = top->asObject();
            if (object == NULL) {
                m_logger->error("no JSON element found");
                return false;
            }
            m_logger->debug(LM("get hostname"));
            config.setHostname(object->getString("hostname",config.getHostname()));
            m_logger->debug(LM("get ipAddress"));
            config.setAddr(object->getString("ipAddress",config.getAddr()));
            m_logger->debug(LM("get brightness"));
            config.setBrightness(object->getInt("brightness",config.getBrightness()));
            m_logger->debug(LM("get maxBrightness"));
            config.setMaxBrightness(object->getInt("maxBrightness",config.getMaxBrightness()));
            EpochTime::Instance.setGmtOffsetMinutes(object->getInt("gmtOffsetMinutes",-5*60));

            JsonArray* pins = object->getArray("pins");
            if (pins) {
                config.clearPins();
                config.clearScripts();
                m_logger->debug(LM("get pins"));

                    m_logger->debug(LM("pins: %s"),pins->toString().text());
                    pins->each([&](IJsonElement*&item) {
                        m_logger->debug(LM("got pin"));
                        JsonObject* pin = item->asObject();
                        m_logger->debug(LM("\tpin: %s"),pin->toString().get());
                        if (pin){
                            m_logger->debug(LM("add pin %d"),pin->getInt("number",-1));
                            LedPin* configPin = config.addPin(pin->getInt("number",-1),pin->getInt("ledCount",0),pin->getBool("reverse",false)); 
                            if (configPin) {
                                configPin->maxBrightness = pin->getInt("maxBrightness",40);
                                configPin->pixelType = getPixelType(pin->getString("pixelType","NEO_GRP"));
                                configPin->pixelsPerMeter = pin->getInt("pixelsPerMeter",30);
                            }
                        } else {
                            m_logger->error("pin is not an Object");
                        }
                    });
            } else {
                m_logger->debug(LM("no pins found"));
            }

            /* scripts are loaded from the filesystem, not stored*/
            m_logger->debug(LM("\tdone reading JSON"));
            return true;
        }


        JsonRoot* toJson(Config&config) {
            m_logger->debug(LM("load config json"));
            LogIndent indent;
            JsonRoot* root=new JsonRoot;  
            JsonObject * json = root->getTopObject();
            json->setString("buildVersion",config.getBuildVersion());
            json->setString("buildDate",config.getBuildDate());
            json->setString("buildTime",config.getBuildTime());
            json->setString("hostname",config.getHostname());
            json->setInt("gmtOffsetMinutes",EpochTime::Instance.getGmtOffsetMinutes());

            json->setString("ipAddress",config.getAddr());
            json->setInt("brightness",config.getBrightness());
            json->setInt("maxBrightness",config.getMaxBrightness());
            JsonArray* pins = root->createArray();
            json->set("pins",pins);
            m_logger->debug(LM("filling pins from config"));
            config.getPins().each( [this,logger=m_logger,pins](LedPin* pin) {
                logger->debug(LM("\thandle pin 0x%04X"),pin); 
                logger->debug(LM("\tnumber %d"),pin->number); 
                JsonObject* pinElement = new JsonObject((pins->getRoot()));
                pinElement->setInt("number",pin->number);
                pinElement->setInt("ledCount",pin->ledCount);
                pinElement->setBool("reverse",pin->reverse);
                pinElement->setInt("maxBrightness",pin->maxBrightness);
                pinElement->setString("pixelType",getPixelType(pin->pixelType));
                pinElement->setInt("pixelsPerMeter",pin->pixelsPerMeter);
                pins->addItem(pinElement);
            });
            m_logger->debug(LM("pins done"));

            JsonArray* scripts = root->createArray();
            json->set("scripts",scripts);
            addScripts(config);
            config.getScripts().each( [&](ScriptDetails*script) {
                m_logger->debug(LM("add script %s %s"),script->getFilename(), script->getName()); 
                JsonObject* obj = scripts->addNewObject();
                script->toJson(obj);
            });
            m_logger->debug(LM("scripts done"));
            m_logger->debug(LM("load config done %x"),root);

            return root;
        }

      
        neoPixelType getPixelType(const char * name) {
            if (strcmp(name,"NEO_RGB") == 0) {return NEO_RGB;}
            if (strcmp(name,"NEO_RBG") == 0) {return NEO_RBG;}
            if (strcmp(name,"NEO_GRB") == 0) {return NEO_GRB;}
            if (strcmp(name,"NEO_GBR") == 0) {return NEO_GBR;}
            if (strcmp(name,"NEO_BRG") == 0) {return NEO_BRG;}
            if (strcmp(name,"NEO_BGR") == 0) {return NEO_BGR;}

            return NEO_GRB;
        }

        const char *getPixelType(neoPixelType type) {
            if (type==NEO_RGB) { return "NEO_RGB";}
            if (type==NEO_RBG) { return "NEO_RBG";}
            if (type==NEO_GRB) { return "NEO_GRB";}
            if (type==NEO_GBR) { return "NEO_GBR";}
            if (type==NEO_BRG) { return "NEO_BRG";}
            if (type==NEO_BGR) { return "NEO_BGR";}
            return "NEO_GRB";
        }



    protected:

    private:
};
};
#endif
