#ifndef DRSCRIPTEXECUTOR_H
#define DRSCRIPTEXECUTOR_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../config.h"
#include "./script.h"

namespace DevRelief {


    class ScriptExecutor {
        public:
            ScriptExecutor() {
                SET_LOGGER(ScriptExecutorLogger);
                SET_CUSTOM_LOGGER(m_periodicLogger,FiveSecondLogger);
                m_script = NULL;
                m_ledStrip = NULL;
            }

            ~ScriptExecutor() { 
                endScript();
                delete m_ledStrip;
            }

            void turnOff() {
                endScript();
                white(0);
            }

            void white(uint8_t level) {
                endScript();
                if (m_ledStrip == NULL) {
                    m_logger->debug("No LED strip");
                    return;
                }
                // level is 0-100
                m_ledStrip->clear();
                m_ledStrip->setBrightness(40);
                m_logger->debug("Set white level %d.  LED count:%d",level,m_ledStrip->getCount());
                for(int i=0;i<m_ledStrip->getCount();i++) {
                    m_ledStrip->setSaturation(i,0);
                    m_ledStrip->setLightness(i,level);
                    m_ledStrip->setHue(i,0);
                }
                m_ledStrip->show();
            }

            void solid(JsonObject* params) {
                endScript();
                if (m_ledStrip == NULL) {
                    return;
                }
                m_ledStrip->clear();
                m_ledStrip->setBrightness(40);
                int hue = params->getInt("hue",150);
                int saturation = params->getInt("saturation",100);
                int lightness = params->getInt("lightness",50);
                m_logger->debug("Set solid %d %d %d %d",m_ledStrip->getCount(),hue,saturation,lightness);
                for(int i=0;i<m_ledStrip->getCount();i++) {
                    m_ledStrip->setSaturation(i,saturation);
                    m_ledStrip->setLightness(i,lightness);
                    m_ledStrip->setHue(i,hue);
                }
                m_ledStrip->show();
            }


            void setScript(Script * script,JsonObject* params=NULL) {
                m_logger->debug("setScript %x %x",m_ledStrip,m_script);
                endScript();
                m_script = script;
                if (script != NULL) {
                    m_logger->debug("setScript %s, %x",script->getName(),m_ledStrip);
                    script->begin(m_ledStrip,params);
                }
            }

            void endScript() {
                if (m_script) {
                    m_script->destroy();
                    m_script = NULL;
                }
            }

            void configChange(Config& config) {
                turnOff();
                setupLeds(config);
            }

            void step() {
                m_logger->never("step %x %x",m_ledStrip,m_script);
                if (m_ledStrip == NULL || m_script == NULL) {
                    m_periodicLogger->info("\tnothing to run");
                    return;
                }
                m_logger->never("\tm_script->setp()");
                //m_ledStrip->clear();
                m_script->step();
                //m_ledStrip->show();
                m_logger->never("\tfinished m_script->step()");

            }
        private:


            void setupLeds(Config& config) {
                m_logger->debug("setup HSL Strip");
                if (m_ledStrip) {
                    delete m_ledStrip;
                }
                const PtrList<LedPin*>& pins = config.getPins();
                int pixelPerMeter = pins.size()>0 ? pins[0]->pixelsPerMeter : 30;
                CompoundLedStrip*  compound = new CompoundLedStrip(pixelPerMeter);
                int ledCount = 0;
                pins.each([&](LedPin* pin) {
                    m_logger->debug("\tadd pin 0x%04X %d %d %d",pin,pin->number,pin->ledCount,pin->reverse);
                    if (pin->number >= 0) {
                        DRLedStrip * real = new PhyisicalLedStrip(pin->number,pin->ledCount,pin->pixelsPerMeter,pin->pixelType,pin->maxBrightness);
                        
                        if (pin->reverse) {
                            auto* reverse = new ReverseStrip(real);
                            compound->add(reverse);
                        } else {
                            compound->add(real);
                        }


                    }
                });

                m_ledStrip = new HSLStrip(compound);
                m_logger->info("created HSLStrip");
            }

            DECLARE_LOGGER();
            DECLARE_CUSTOM_LOGGER(m_periodicLogger);
            Script* m_script;
            HSLStrip* m_ledStrip;
    };

}
#endif