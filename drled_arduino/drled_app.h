
#include <math.h>
#include <time.h>

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "./lib/application.h"
#include "./lib/log/logger.h"
#include "./lib/log/config.h"
#include "./lib/log/logger.h"
#include "./lib/data/api_result.h"
#include "./lib/task/task.h"
#include "./script/script.h"
#include "./script/executor.h"
#include "./script/data_loader.h"
#include "./app_state.h"
#include "./app_state_data_loader.h"
#include "./config.h"
#include "./config_data_loader.h"
#include "./loggers.h"

extern EspClass ESP;


namespace DevRelief {

    class NetworkInitialize : public Task {
        public: 
            NetworkInitialize(HttpServer * httpServer) 
            : Task("NetworkInitialize",500), m_ntp(m_ntpUDP,"pool.ntp.org") {
                m_logger->debug("start NetworkInitialize");
                m_httpServer = httpServer;
                m_waitWifi = true;
                m_waitTime = false;
            }
            virtual ~NetworkInitialize()  {
                m_logger->debug("end NetworkInitialize");
                // do not delete m_httpServer;
            }

            virtual TaskStatus run() {
                m_logger->debug("check WiFi state");
                LogIndent id;
                if (m_waitWifi && DRWiFi::get()->isInitialized()) {
                    m_logger->debug("WiFi ready");
                    m_httpServer->begin();
                    m_waitWifi = false;
                    m_waitTime = true;
                    m_ntp.begin();
                    m_ntp.update();
                    return TASK_RUNNING;
                }
                // once epoch time is over 1000 assume it's been updated
                if (m_waitTime && m_ntp.getEpochTime() > 1000) {
                    EpochTime::Instance.setSecondsNow(m_ntp.getEpochTime());
                    return TASK_COMPLETE;
                }
                return TASK_RUNNING;

            }

        private: 
            bool m_waitWifi;
            bool m_waitTime;
            WiFiUDP m_ntpUDP;
            NTPClient m_ntp;

            HttpServer* m_httpServer;

    };

    class DRLedApplication : public Application {
    public: 
   
   
        DRLedApplication() {
            SET_LOGGER(AppLogger);
            m_logger->showMemory("Starting");
            m_startHeap = EspBoard.getFreeHeap();
            initialize();
            resume();
        }

        ~DRLedApplication() {
        }

        // resume the api/script that was running when power was turned off
        const char * resume(bool forceStart=false, bool forceRun=false) {
            m_logger->debug("resume");

            m_executor.turnOff();
            AppStateDataLoader loader;
            m_logger->debug("\tload");
            if (loader.load(m_appState)){
                if (forceStart) {
                    m_appState.setIsStarting(false);
                }
                if (forceRun) {
                    m_appState.setIsRunning(true);
                }
                if (!m_appState.isStarting() && m_appState.isRunning() && m_appState.getType() != EXECUTE_NONE) {
                    m_logger->debug("\tset isStarting");
                    m_appState.setIsStarting(true);
                    m_logger->debug("\tsave");
                    loader.save(m_appState);
                    if (m_appState.getType() == EXECUTE_API) {
                        m_logger->debug("\texecute API %s",m_appState.getExecuteValue());
                        ApiResult result;
                        runApi(m_appState.getExecuteValue(),m_appState.getParameters()->asObject(),result);
                        m_logger->debug("\tAPI ran");
                    } else if (m_appState.getType() == EXECUTE_SCRIPT) {
                        m_logger->debug("\tscript state run");
                        JsonObject* params = m_appState.getParameters();
                        const char * name = m_appState.getExecuteValue();
                        m_logger->debug(LM("\trun script: %s"),name);
                        ApiResult result;
                        runScript(name,params,result);                    
                    }
                } 
            } else {
                m_logger->error(LM("load appstate failed"));
            }
            return m_appState.getExecuteValue();
        }


    protected:       
        void loop() {
            if (!m_initialized) {
                m_logger->debug("app not initialized");
                return;
            }
            if (m_appState.isStarting() && m_scriptStartTime+10*1000 < millis()) {
                m_appState.setIsStarting(false);
                AppStateDataLoader loader;
                loader.save(m_appState);
            }
            m_httpServer->handleClient();
            m_executor.step();
        }


        void initialize() {
            ConfigDataLoader configDataLoader;
            if (!configDataLoader.load(m_config)) {
                m_logger->error("Load failed.  Using default.");
                configDataLoader.initialize(m_config);
                configDataLoader.save(m_config);
            } else {
                m_logger->info("Loaded config.json");
            }
            m_config.setInstance(&m_config);
            m_httpServer = new HttpServer();
            setupRoutes();        
           
            m_logger->debug("show build version");
            m_executor.configChange(m_config);
            m_logger->debug("Running DRLedApplication configured: %s.  Built at %s %s",
                m_config.getBuildVersion(),
                m_config.getBuildDate(),
                m_config.getBuildTime());
            m_scriptStartTime = 0;
            m_initialized = true;
            new NetworkInitialize(m_httpServer);

        }
        

        void setupRoutes() {
            m_httpServer->route("/",[this](Request* req, Response* resp){
                resp->send(200,"text/html","home not defined");
            });


            m_httpServer->routeBracesGet( "/api/config",[this](Request* req, Response* resp){
                m_logger->debug("get /api/config");
                ConfigDataLoader configDataLoader;
                JsonRoot* jsonRoot = configDataLoader.toJson(m_config);
                JsonObject*json = jsonRoot->getTopElement()->asObject();
                m_logger->debug(LM("got config json %x %x %s"),jsonRoot,json,json->toString().text());
                LogIndent indent;
                ApiResult api(json);
                m_logger->debug(LM("new root %x"),json->getRoot());
                api.send(resp);
                jsonRoot->destroy();
            });


            m_httpServer->routeBracesPost( "/api/config",[this](Request* req, Response* resp){
                
                auto body = req->arg("plain").c_str();
                ConfigDataLoader loader;
                loader.updateConfig(m_config,body);
                m_executor.configChange(m_config);

                ApiResult result(true);
                DRString apiText;
                result.toText(apiText);
                resume();
                resp->send(200,"text/json",apiText.text());
            });


            m_httpServer->routeBracesGet( "/api/script/{}",[this](Request* req, Response* resp){
                m_executor.endScript();
                ScriptDataLoader loader;
                
                DRString path = loader.getPath(req->pathArg(0).c_str());

                m_logger->debug("GET /api/script/%s",path.text());
                bool sent = loader.loadJsonFile(path,[&](IJsonElement* json) {
                    m_logger->debug("JSON loaded %x",json);
                    ApiResult result(json);
                    result.send(resp);
                    return true;
                });
                
                if (!sent) {
                    resp->send(404,"text/json","script not loaded");
                }
                resume(true,true);
            });

            m_httpServer->routeBracesGet( "/api/run/{}",[this](Request* req, Response* resp){
                m_logger->debug("run script");
                ApiResult result;
                JsonRoot* params = getParameters(req);
                const char * name = req->pathArg(0).c_str();
                m_logger->debug("run script %s",name);
                runScript(name,params->getTopObject(),result);
                result.send(req);
                params->destroy();
            });


            m_httpServer->routeBracesPost( "/api/script/{}",[this](Request* req, Response* resp){
                m_executor.endScript();
                auto body = req->arg("plain").c_str();
                m_logger->debug("\tgot new script:%s",body);
                auto name =req->pathArg(0).c_str();
                m_logger->debug("save script %s",name);
                m_logger->debug("\tname: %s",name);
                if (name != NULL) {
                    ScriptDataLoader loader;
                    loader.save(name,body);
                    m_logger->debug("saved");
                }
                m_logger->debug("\tget params");
                JsonRoot* params = getParameters(req);
                ApiResult result;
                m_logger->debug("\trun script");
                runScript(name,params->getTopObject(),result);
                m_logger->debug("\tsend result");
                result.send(req);
                m_logger->debug("\tdelete params");
                params->destroy();
                m_logger->debug("\tdeleted params");
            });


            m_httpServer->routeBracesDelete( "/api/script/{}",[this](Request* req, Response* resp){
                m_logger->debug("delete script");
                ScriptDataLoader loader;
                auto name =req->pathArg(0).c_str();
                m_logger->debug("delete script %s",name);
                ApiResult result;
                if (!loader.deleteScript(name)) {
                    result.setSuccess(false);
                } 
                result.send(resp);
            });




            m_httpServer->routeBracesGet("/api/{}",[this](Request* req, Response* resp){
                m_logger->debug("API start");

                this->apiRequest(req->pathArg(0).c_str(),req,resp);
                m_logger->debug("API done");
            });

        }

    protected:


        void apiRequest(const char * api,Request * req,Response * resp) {
            LogIndent li(m_logger,LM("handle API"),INFO_LEVEL);
            m_logger->info(LM("api: %s"),api);
            int code=200;
            if (strcmp(api,"reboot") == 0) {
                code = 200;
                resp->send(200,"text/json","{result:true,message:\"rebooting ... \"}");   
                delay(1000);
                ESP.restart();
                return;
            }


            m_logger->never("get parameters");
            JsonRoot* paramJson = getParameters(req);
            m_logger->never("got params");
            


            JsonObject *params = paramJson->getTopObject();
            
            ApiResult result;
            m_logger->never("\tcreated ApiResult");
            

            if (runApi(api,params,result))
            {
                m_appState.setApi(api,params);
                AppStateDataLoader loader;
                loader.save(m_appState);
            }

            result.send(resp);
            paramJson->destroy();


        }

        bool runApi(const char * api, JsonObject* params, ApiResult& result){
            bool saveState=false;
            if (strcmp(api,"off") == 0) {
                m_executor.turnOff();
                saveState = true;
                result.setCode(200);
                result.setMessage("lights turned %s","off");
            } else if (strcmp(api,"on") == 0){
                int level = params->getInt("level",100);
                saveState = true;
                m_executor.white(level);
                result.setCode(200);
                result.setMessage("lights turned %s","on");
            } else if (strcmp(api,"resume") == 0){
                resume(true,true);
                result.setCode(200);
                result.setMessage("resumed last execution");
            } else if (strcmp(api,"color") == 0){
                m_executor.solid(params);
                saveState = true;
                result.setCode(200);
                result.setMessage("lights turned %s","on");
            } else if (strcmp(api,"mem") == 0){
               result.setCode(202);
            } else {
                result.setCode(404);
                result.setMessage("failed");
            }
            return saveState;
        }
    

        bool runScript(const char * name, JsonObject* params, ApiResult& result) {

            ScriptDataLoader loader;
            m_logger->debug("run script %s  params=%s",name,params->toString().text());
            m_executor.turnOff();
            Script* script  = loader.load(name);
            m_logger->debug("loaded script %s",name);

            if (script){
                m_logger->debug("\tm_executor.setScript");
                m_executor.setScript(script,params);
                m_scriptStartTime = millis();
                m_logger->debug("\tset appState %s",name);
                m_appState.setScript(name,params);
                AppStateDataLoader loader;
                m_logger->debug("\tsave appState");
                loader.save(m_appState);
                m_logger->debug("\tsaved");
                return true;
            } else {
                result.setCode(404);
                result.setMessage("script not found: %s",name);
                return false;
            }
            m_logger->debug("\trunScript failed");
            return false;
        };

      
        JsonRoot* getParameters(Request*req){
            JsonRoot * root = new JsonRoot();
            JsonObject* obj = root->getTopObject();
            m_logger->never("\tloop parameters");
            for(int i=0;i<req->args();i++) {
                if (!Util::equal(req->argName(i).c_str(),"plain")){
                    m_logger->debug("\t%s=%s",req->argName(i).c_str(),req->arg(i).c_str());
                    obj->setString(req->argName(i).c_str(),req->arg(i).c_str());
                }
            }
            return root;
        }

    private: 
        DECLARE_LOGGER();
        HttpServer * m_httpServer;
        Config m_config;
        AppState m_appState;
        ScriptExecutor m_executor;
        
        long m_scriptStartTime;
        bool m_initialized;
        long m_startHeap;
        ILogConfig* logConfig;
    };


}
