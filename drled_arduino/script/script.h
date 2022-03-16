#ifndef DRSCRIPT_H
#define DRSCRIPT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_container.h"
#include "./script_context.h"
#include "../loggers.h"

namespace DevRelief
{
 
    
    class Script 
    {
    public:
        Script()
        {
            SET_LOGGER(ScriptLogger);
            m_name = "unnamed";
            
            m_duration=NULL;
            m_brightness=NULL;
            m_frequency=NULL;
            m_startMsecs = 0;
            m_rootContainer = NULL;
        }

        virtual ~Script() {
            m_logger->test("~Script %x",this);
            
            if (m_rootContainer) {
                m_logger->test("destroy rootcontainer %x",m_rootContainer);
                m_rootContainer->destroy();
            }
            if (m_duration) { m_duration->destroy();}
            if (m_brightness) { m_brightness->destroy();}
            if (m_frequency) { m_frequency->destroy();}
            m_logger->test("~Script done");
        }

        virtual void destroy() {
            m_logger->test("destroy Script");
            delete this;
        }


        void begin(IHSLStrip* strip, JsonObject* params) {
            m_logger->debug("Begin script %x %s",strip,m_name.text());
            m_realStrip = strip;
            m_startMsecs = millis();
            ScriptRootContainer* root = getRootContainer();
            root->setStrip(strip);
            root->setParams(params);
            m_logger->debug("created RootContext");
        }

        void step() {
            auto lastStep = m_rootContainer->getContext()->getLastStep();
            int durationMsecs = getDuration();
            if (durationMsecs > 0 && (m_startMsecs>0 && m_startMsecs+durationMsecs<millis())) {
                return; // past duration
            }
            int frequencyMsecs = getFrequency();
            if (lastStep && frequencyMsecs>0 && lastStep->getStartMsecs() + frequencyMsecs > millis()) {
                m_logger->test("too soon %d %d %d",lastStep?lastStep->getStartMsecs():-1, frequencyMsecs , millis());
                return; // to soon to start next step
            }
            m_logger->never("step %d %d %d",lastStep?lastStep->getStartMsecs():-1, frequencyMsecs , millis());
            
            m_realStrip->clear();

            m_logger->debug("\tdraw");
            m_rootContainer->draw();
            m_logger->debug("\tend step");

            m_realStrip->show();
            m_logger->debug("\tstep done");

        }

        void setName(const char * name) { m_name = name; }
        const char * getName() { return m_name;}

        void setDuration(IScriptValue* duration) { 
            m_logger->never("set duration");
            m_duration = duration;
        }
        void setBrightness(IScriptValue* brightness) { 
            m_logger->never("set brightness");
            m_brightness = brightness;
        }

        int getBrightness() {

            int b = m_brightness == NULL ? 40 : m_brightness->getIntValue(m_rootContainer->getContext(),40);
            m_logger->never("get brightness %d",b);
            return b;
        }
        
        int getDuration() {
            int d = m_duration == NULL ? 0 : m_duration->getMsecValue(m_rootContainer->getContext(),0);
            m_logger->never("get duration %d",d);
            return d;

        }
        int getFrequency() {
            int f = m_frequency == NULL ? 50 : m_frequency->getMsecValue(m_rootContainer->getContext(),50);
            m_logger->never("get frequncy %d",f);
            return f;

        }

        void setFrequency(IScriptValue* frequency) {
            m_logger->never("set frequency");

            m_frequency = frequency;
        }

        ScriptRootContainer* getRootContainer() { 
            if (m_rootContainer == NULL){
                m_rootContainer = new ScriptRootContainer();
            }
            return m_rootContainer;
        }
    private:
        DECLARE_LOGGER();
        DRString m_name;
        ScriptRootContainer* m_rootContainer;
        IHSLStrip* m_realStrip;
        IScriptValue* m_duration;
        IScriptValue* m_brightness;
        IScriptValue*  m_frequency;
        int m_startMsecs;
    };

   
}
#endif