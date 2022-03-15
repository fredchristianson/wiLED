#ifndef CONFIG_H
#define CONFIG_H

#include <Adafruit_NeoPixel.h>
#include "./lib/system/epoch_time.h"
#include "./lib/log/logger.h"
#include "./lib/util/list.h"
#include "./lib/util/util.h"
#include "./lib/json/json_interface.h"


namespace DevRelief {

    class LedPin {
        public:
        LedPin(int n, int c, bool r) {
            number=n;
            ledCount=c;
            reverse=r;
            pixelType = NEO_GRB;
            maxBrightness=50;
            pixelsPerMeter = 30;
        }

        ~LedPin() {
        }

        void destroy() {
            delete this;
        }

        int number;
        int ledCount;
        bool reverse;
        uint16_t pixelType;
        uint8_t maxBrightness;
        uint16_t pixelsPerMeter;
    };

    class ScriptDetails {
        public:
            ScriptDetails(const char * fileName, const char * name) {
                m_fileName = fileName;
                m_name = name;
            }

            virtual void destroy() { delete this;}
            const char * getName() { return m_name.text();}
            const char * getFilename() { return m_fileName.text();}

            void setName(const char * name) { m_name = name;}
            void getName(const char * fileName) { m_fileName = fileName;}

            void toJson(JsonObject* obj) {
                obj->setString("fileName",m_fileName.text());
                obj->setString("name",m_name.text());
            }
        private:
            DRString m_fileName;
            DRString m_name;
    };

    

    class Config {
        public:
            static Config* getInstance() { return instance;}
            static void setInstance(Config*cfg) { Config::instance = cfg;}

            Config() {
                SET_LOGGER(ConfigLoaderLogger);
                hostName = HOSTNAME;
                ipAddress = "unknown";
                brightness = 40;
                maxBrightness = 100;
                buildVersion = BUILD_VERSION;
                buildDate = BUILD_DATE;
                buildTime = BUILD_TIME;
            }

          
            void setAddr(const char * ip){
                ipAddress = ip;
            }

            
            const char * getAddr() const { return ipAddress.text();}

            void clearPins() {
                pins.clear();
            }

            LedPin* getPinNumber(int number) {
                for(int idx=0;idx<pins.size();idx++) {
                    LedPin* pin = pins[idx];
                    if (pin && pin->number == number) {
                        return pin;
                    }
                }
                return NULL;
            }

            LedPin* addPin(int number,int ledCount,bool reverse=false) {
                m_logger->debug("addPin %d %d %d",number,ledCount,reverse);
                if (getPinNumber(number) != NULL) {
                    m_logger->error("pin number %d added more than once");
                    return NULL;
                }
                LedPin* pin = new LedPin(number,ledCount,reverse);
                pins.add(pin);
                return pin;
            }
            const LedPin* getPin(size_t idx) const { return pins[idx];}
            size_t getPinCount() const { return pins.size();}
            const PtrList<LedPin*>& getPins() { 
                m_logger->debug("return pins");
                return pins;
            }

            int getBrightness() const { return brightness;}
            void setBrightness(int b) { brightness = b;}
            int getMaxBrightness() const { return maxBrightness;}
            void setMaxBrightness(int b) { maxBrightness = b;}

            void clearScripts() {
                scripts.clear();
            }

            size_t getScriptCount()  const{ return scripts.size();}
            
            bool addScript(const char * filename, const char * name) {
                if (name == NULL) { name = filename;}
                if (name == NULL || strlen(name) == 0) {
                    m_logger->warn("addScript requires a name");
                    return false;
                }
                m_logger->debug("add script %s",name);
                
                scripts.add(new ScriptDetails(filename, name));
                m_logger->debug("\tadded");
                return true;
            }

/*
            void setScripts(PtrList<ScriptDetails>& list) {
                scripts.clear();
                list.each([&](DRString&name) {
                    m_logger->debug("addScript %s",name.text());
                    addScript(name.get());
                });
            }
            */
            const PtrList<ScriptDetails*>& getScripts()  const{ return scripts;}

            const char * getHostname() const { return hostName.text();}
            void setHostname(const char * name) {
                hostName = name;
            }
 

            const char * getBuildVersion()const { return buildVersion.text();;}
            const char * getBuildDate()const { return buildDate.text();;}
            const char * getBuildTime()const { return buildTime.text();;}

            void setEpochSeconds(unsigned long epochSeconds){
                EpochTime::Instance.setSecondsNow(epochSeconds);
            }

    private:    
    
            DRString     hostName;
            DRString     ipAddress;
            DRString    buildVersion;
            DRString    buildDate;
            DRString    buildTime;
            PtrList<LedPin*>  pins;
            PtrList<ScriptDetails*>   scripts;
            int  brightness;
            int  maxBrightness;
            DECLARE_LOGGER();
            static Config* instance;

    };
    Config* Config::instance;
}


#endif