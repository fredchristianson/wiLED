#ifndef DRSCRIPT_TIMER_H
#define DRSCRIPT_TIMER_H

#include "../lib/log/logger.h"
#include "../lib/json/json_interface.h"
#include "../lib/json/json.h"
#include "../lib/util/drstring.h"
#include "../lib/util/util.h"
#include "../lib/util/list.h"
#include "./script_interface.h"
#include "./script_value.h"


namespace DevRelief
{

    class TimerState {
        public:
            static TimerState* copy(const TimerState* other) {
                if (other == NULL) { return NULL;}
                return new TimerState(other);
            }

            TimerState(const TimerState* other) {
                SET_LOGGER(ScriptTimerLogger);
                m_status = other->m_status;
                m_duration = other->m_duration;
                m_durationValue = other->m_durationValue;
                m_enterMillis = 0;
            }
            TimerState(ScriptStatus status, IScriptValue* durationValue) {
                SET_LOGGER(ScriptTimerLogger);
                m_logger->debug(LM("TimerState %d %s"),status, durationValue ? durationValue->toString().text() : "<no value>");
                m_status = status;
                m_duration = 0;
                m_durationValue = durationValue;
                m_enterMillis = 0;

            }

            ~TimerState() {
                if (m_durationValue) {m_durationValue->destroy();}
            }

            ScriptStatus updateStatus(IScriptContext* context) {
                int now = millis();
                m_logger->debug(LM("TimerState.updateStatus %d %d   %d > %d"),m_status, m_duration, now, (m_enterMillis+m_duration));
                if (m_duration > 0 && now > m_enterMillis+m_duration) {
                    return SCRIPT_COMPLETE;
                }
                evaluate(context,m_stepValues);

                return m_status;
            }

            void enter(IScriptContext* context) {
                m_duration = m_durationValue ? m_durationValue->getMsecValue(context,0) : -1;
                m_logger->debug(LM("TimerState.enter %d,  duration=%d"),m_status,m_duration);
                m_enterMillis = millis();
                evaluate(context,m_enterValues);
            }

            void leave(IScriptContext* context) {
                evaluate(context,m_leaveValues);
            }

            int getDuration() { return m_duration;}

            void setEnterVariables(IJsonElement* values){ 
                m_logger->debug("setEnterVariables");
                setVariables(m_enterValues,values);
            }
            void setLeaveVariables(IJsonElement* values){ 
                m_logger->debug("setLeaveVariables");
                setVariables(m_leaveValues,values);
            }
            void setStepVariables(IJsonElement* values){ 
                m_logger->debug("setStepVariables");

                setVariables(m_stepValues,values);
            }
        private:

            void setVariables(ScriptValueList& list, IJsonElement* values){
                if (values == NULL) {
                    return;
                }
                JsonObject* obj = values->asObject();
                if (obj) {
                    obj->eachProperty([&](const char * name, IJsonElement* val){
                        list.setValue(name,ScriptValue::create(val));
                    });
                }
            }

            void evaluate(IScriptContext* context, ScriptValueList& list) {
                list.each([&](NameValue* value) {
                    double val = value->getValue()->getFloatValue(context,0);
                    context->setValue(value->getName(),new ScriptNumberValue(val));
                });
            }
            ScriptStatus m_status;
            IScriptValue* m_durationValue;
            int m_duration;
            int m_enterMillis; // time this state was entered

            ScriptValueList m_enterValues;
            ScriptValueList m_leaveValues;
            ScriptValueList m_stepValues;

            DECLARE_LOGGER();
        
    };

    class ScriptTimerValue  : public IScriptTimer {
        public:

            ScriptTimerValue(const ScriptTimerValue* other) {
                SET_LOGGER(ScriptTimerLogger);
                m_runState = TimerState::copy(other->m_runState);
                m_pauseState = TimerState::copy(other->m_runState);
                m_completeState = TimerState::copy(other->m_runState);
                m_currentStatus = SCRIPT_CREATED;
                m_runCount = 0;
                m_repeatCountValue = other->m_repeatCountValue ? other->m_repeatCountValue->clone() : NULL;
                m_repeatCount = other->m_repeatCount;
            }

            ScriptTimerValue(IJsonElement* json=NULL) {
                SET_LOGGER(ScriptTimerLogger);
                m_runState = NULL;
                m_pauseState = NULL;
                m_completeState = NULL;
                m_currentStatus = SCRIPT_CREATED;
                m_runCount = 0;
                m_repeatCount = 1;
                m_repeatCountValue = NULL;
                fromJson(json);
            }
 
            virtual ~ScriptTimerValue() {
                delete m_runState;
                delete m_pauseState;
                delete m_completeState;
                if (m_repeatCountValue) { m_repeatCountValue->destroy();}
            }


            void destroy() override { delete this;}
            bool isTimer(IScriptContext* cmd) const override { return true;}
            
            double getFloatValue(IScriptContext* ctx,double defaultValue)  {
                return m_runState?m_runState->getDuration() : defaultValue;
            }

            ScriptStatus getStatus(IScriptContext* ctx,ScriptStatus defaultValue) const override { return m_currentStatus;}

            void fromJson(IJsonElement* json) {
                if (json == NULL) { m_currentStatus = SCRIPT_COMPLETE; return;}
                if (json->isObject()) {
                    fromJsonObject(json->asObject());
                }else if (json->isArray()) {
                    fromJsonArray(json->asArray());
                } else {
                    m_runState = new TimerState(SCRIPT_RUNNING,ScriptValue::create(json, NULL));
                }
            }

            void fromJsonObject(JsonObject* json) {
                if (json == NULL) { return;}
                m_repeatCountValue = ScriptValue::create(json->getPropertyValue("repeat"));
                m_runState = stateFromJson(SCRIPT_RUNNING,json->getPropertyValue("run"));
                m_pauseState = stateFromJson(SCRIPT_PAUSED, json->getPropertyValue("pause"));
            }

            TimerState* stateFromJson(ScriptStatus status, IJsonElement* json) {
                if (json == NULL) { return NULL;}
                JsonObject* obj = json->asObject();
                if (obj) {
                    IScriptValue* duration = ScriptValue::create(obj->getPropertyValue("duration"));
                    TimerState* state = new TimerState(status,duration);
                    state->setEnterVariables(obj->getPropertyValue("enter"));
                    state->setLeaveVariables(obj->getPropertyValue("leave"));
                    state->setStepVariables(obj->getPropertyValue("step"));
                    return state;
                } else {
                    return new TimerState(status,ScriptValue::create(json));
                }
            }

            void fromJsonArray(JsonArray* json) {
                if (json == NULL || json->getCount()<1) { return;}
                m_runState = new TimerState(SCRIPT_RUNNING,ScriptValue::create(json->getAt(0), NULL));
                if (json->getCount()>1) {
                    m_pauseState = new TimerState(SCRIPT_PAUSED,ScriptValue::create(json->getAt(1), NULL));
                }
                if (json->getCount()>2) {
                    m_repeatCountValue = ScriptValue::create(json->getAt(2), NULL);
                }
            }

            IJsonElement* toJson(JsonRoot* jsonRoot) override {
                m_logger->error("toJson() not implemented");
                JsonObject* obj = new JsonObject(jsonRoot);
                obj->setString("toJson","not implemented");
                return obj;
            }

            ScriptStatus updateStatus(IScriptContext* context){
                m_logger->debug(LM("Timer.updateStatus %d"),m_currentStatus);
                LogIndent li;
                if (m_runState == NULL) { return SCRIPT_COMPLETE;}
                if (m_currentStatus == SCRIPT_CREATED) {
                    m_logger->debug(LM("enterRunState"));
                    m_currentStatus = enterRunState(context);
                } else if (m_currentStatus == SCRIPT_RUNNING) {
                    m_logger->debug(LM("running"));
                    
                    if (m_runState->updateStatus(context) == SCRIPT_COMPLETE) {
                        m_logger->debug(LM("enterPauseState"));
                        m_currentStatus = enterPauseState(context);
                    } 
                } else if (m_currentStatus == SCRIPT_PAUSED && m_pauseState) {
                    if (m_pauseState->updateStatus(context) == SCRIPT_COMPLETE) {
                        m_logger->debug(LM("repeat"));
                        m_currentStatus = enterRunState(context);
                    }
                } else {
                    m_logger->debug(LM("complete"));

                    m_currentStatus = complete(context);
                }
                m_logger->debug(LM("timer status %d"),m_currentStatus);
                return m_currentStatus;
            }

            IScriptValue* eval(IScriptContext * ctx, double defaultValue) override { return new ScriptTimerValue(this);}

            bool isRecursing() const override { return false;}

            bool isString(IScriptContext* ctx)  const override{ return false; } 
            bool isNumber(IScriptContext* ctx)  const override{ return false; } 
            bool isBool(IScriptContext* ctx)  const override{ return false; } 
            bool isNull(IScriptContext* ctx)  const override{ return false; } 
            bool isUnitValue(IScriptContext* ctx)  const override{ return false; } 
            bool equals(IScriptContext*cmd, const char * match) const { return false;};

            int getIntValue(IScriptContext* cmd,  int defaultValue) { return defaultValue;} 
            bool getBoolValue(IScriptContext* cmd,  bool defaultValue) { return defaultValue;} 
            int getMsecValue(IScriptContext* cmd,  int defaultValue) { return defaultValue;} 
            UnitValue getUnitValue(IScriptContext* cmd,  double defaultValue, PositionUnit defaultUnit) { return UnitValue(defaultValue,defaultUnit);};
            ScriptStatus getStatus(IScriptContext* ctx,ScriptStatus defaultValue) { return m_currentStatus;};

            DRString stringify() override { return DRFormattedString("timer %s",m_name);}
            DRString toString() override { return DRFormattedString("timer %s",m_name);}

            IScriptValue* clone()const {
                return new ScriptTimerValue(this);
            }
        protected:
            ScriptStatus enterRunState(IScriptContext* context) {
                if (m_runState == NULL) { return SCRIPT_COMPLETE;}
                m_runState->enter(context);
                // don't call update since it does a step();
                //m_currentStatus = m_runState->updateStatus(context);
                m_currentStatus = SCRIPT_RUNNING;
                return m_currentStatus;
            }

            ScriptStatus enterPauseState(IScriptContext* context) {
                if (m_currentStatus == SCRIPT_RUNNING && m_runState){
                    m_runState->leave(context);
                }
                if (m_repeatCountValue) {
                    m_repeatCount = m_repeatCountValue->getIntValue(context,1);
                }
                m_logger->debug("increment run count");
                m_runCount.setNumberValue(m_runCount.getIntValue(context,0)+1);
                m_logger->debug("incremented run count");
                if (m_repeatCount >0 && m_runCount.getIntValue(context,0)>=m_repeatCount) {
                    m_currentStatus = SCRIPT_COMPLETE;
                } else if (m_pauseState) {
                    m_pauseState->enter(context);
                    m_currentStatus = m_pauseState->updateStatus(context);
                } else {
                    m_currentStatus = SCRIPT_COMPLETE;
                }
                return m_currentStatus;
            }

            ScriptStatus complete(IScriptContext* context) {
                if (m_completeState) {
                    m_completeState->enter(context);
                }
                m_currentStatus = SCRIPT_COMPLETE;
                return m_currentStatus;
            }

            const char * m_name;
            TimerState* m_runState;
            TimerState* m_pauseState;
            TimerState* m_completeState;
            IScriptValue* m_repeatCountValue;
            ScriptNumberValue m_runCount;
            int m_repeatCount;
            ScriptStatus m_currentStatus;
            DECLARE_LOGGER();
        
    };

    IScriptTimer* ScriptValue::createTimer(IJsonElement*json) {
        return new ScriptTimerValue(json);
    }
  
  

}
#endif