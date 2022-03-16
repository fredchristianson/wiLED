#ifndef ELEMENT_POSITION_H
#define ELEMENT_POSITION_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"

namespace DevRelief {

    // most elements won't have defined properties.  
    // a single default object can hold all of the defaults values
    // so only elements with specified positions need members/memory
    //
    // todo: make different versionse of PositionProperties with just the common ones (e.g. offset,length)
    //       and complete properties with rarely used (e.g. stripNumber, reverse)



    class PositionProperties {
        public:
            PositionProperties() {
                SET_LOGGER(ScriptPositionLogger);
                m_logger->debug("PositionProperties %x",this);
                m_offsetValue = NULL;
                m_lengthValue = NULL;
                m_stripNumberValue = NULL;
                m_reverseValue = NULL;
                m_gapValue = NULL;
                m_unit = POS_INHERIT;
                m_hslOperation = INHERIT;
                m_clip = false;
                m_wrap = false;   
                m_offset = UnitValue(0,POS_PERCENT);
                m_length = UnitValue(100,POS_PERCENT);
                m_absolute = false;
                m_cover = false;
                m_center = false;
                m_flow = true;
                m_reverse = false;
                m_gap = UnitValue(0,POS_INHERIT);
            }

            ~PositionProperties() {
                m_logger->never("~PositionProperties");
                m_logger->never("destroy offset");
                if (m_offsetValue) { m_offsetValue->destroy();}
                m_logger->never("destroy length");
                if (m_lengthValue) { m_lengthValue->destroy();}
                m_logger->never("destroy stripNumber");
                if (m_stripNumberValue) { m_stripNumberValue->destroy();}
                if (m_reverseValue) { m_reverseValue->destroy();}
                m_logger->never("destroy done");
                if(m_gapValue) { m_gapValue->destroy();}
            }

            bool toJson(JsonObject* json) {
                m_logger->debug("PositionProperties.toJson %x %x",this);
                if (m_lengthValue) { 
                    m_logger->debug("\tlength");
                    json->set("length",m_lengthValue->toJson(json->getRoot()));
                }
                if (m_offsetValue) { 
                    m_logger->debug("\toffset");
                    json->set("offset",m_offsetValue->toJson(json->getRoot()));
                }
                if (m_stripNumberValue) { 
                    m_logger->debug("\tstrip number");
                    json->set("strip",m_stripNumberValue->toJson(json->getRoot()));
                }
                if (m_gapValue) { 
                    json->set("gap",m_gapValue->toJson(json->getRoot()));
                }

                m_logger->debug("\tunit %x",m_unit);

                json->setString("unit", unitToString(m_unit));
                m_logger->debug("\twrap %d",m_wrap);
                json->setBool("wrap", isWrap());
                m_logger->debug("ElementPostion.toJson clip %x %d",this,m_clip);
                json->setBool("clip", isClip());
                m_logger->debug("\tcenter %d",m_center);
                json->setBool("center", isCenter());
                m_logger->debug("\tflow %d",m_flow);
                json->setBool("flow", isFlow());
                m_logger->debug("\tcover %d",m_cover);
                json->setBool("cover", isCover());
                json->setBool("absoute", m_absolute);
                json->setBool("reverse", m_reverse);
                
                m_logger->debug("toJson done: %s",json->toString().text());
                return true;                
            }

            void evaluateValues(IScriptContext* context) {
                if (m_offsetValue) { m_offset = m_offsetValue->getUnitValue(context,0,POS_INHERIT);}
                if (m_lengthValue) { m_length = m_lengthValue->getUnitValue(context,100,POS_INHERIT);}
                if (m_stripNumberValue) { m_stripNumber = m_stripNumberValue->getIntValue(context,0);}
                if (m_gapValue) { 
                    m_gap = m_gapValue->getUnitValue(context,0,POS_INHERIT);
                    m_logger->never("updategap %f",m_gap.getValue());
                }
                if (m_reverseValue) { 
                    m_reverse = m_reverseValue->getBoolValue(context,false);
                    m_logger->never("got reverse value: %d",m_reverse);
                }

            }

            PositionUnit getUnit() const {
                m_logger->never("PositionProperties.getUnit %d",m_unit);
                return m_unit;
            }

            PositionOverflow getOverflow() const { 
                return m_wrap ? OVERFLOW_WRAP: m_clip ? OVERFLOW_CLIP : OVERFLOW_ALLOW;
            }

            bool isClip() const { return m_clip;}
            bool isWrap() const { return m_wrap;}
            bool isCenter() const { return m_center;}
            bool isFlow() const { return m_flow;}
            bool isCover() const { return m_cover;}
            bool isReverse() const { return m_reverse;}
            bool useRootStrip() const { 
                return m_absolute;}
            HSLOperation getHSLOperation() const { return m_hslOperation;}
            bool hasOffset() const { return m_offsetValue != NULL;}
            UnitValue getOffset() const { return m_offset;}
            bool hasLength() const {
                m_logger->never("hasLength %x",m_lengthValue);
                return m_lengthValue != NULL;
            }
            UnitValue getLength() const { return m_length;}

            UnitValue getGap() const { return m_gap;}
            bool hasStrip()const  { return m_stripNumberValue != NULL;}
            int getStrip()const { return m_stripNumber;}

            void setClip(IJsonElement* json)  { 
                if (json) {
                    m_logger->debug("setClip %x %s",this, JsonElement::toJsonString(json).text());
                    m_clip = getBool(json,false);
                    m_logger->debug("\tjson %d",m_clip);
                }
            }
            void setWrap(IJsonElement* json)  { if (json) { m_wrap = getBool(json,false);}}
            void setCenter(IJsonElement* json)  { if (json) { m_center = getBool(json,false);}}
            void setCover(IJsonElement* json)  { if (json) { m_cover = getBool(json,false);}}

            // setFlow must be called after center/cover to get the right default
            void setFlow(IJsonElement* json)  { if (json) {m_flow = getBool(json,!m_center && !m_cover);}}
            void setPositionAbsolute(IJsonElement* json)  {if (json) { m_absolute = getBool(json,false);}}

            void setGap(IJsonElement* json)  { 
                m_gapValue = ScriptValue::create(json);
                m_logger->never("pos setGap %s",m_gapValue?m_gapValue->toString().text() : "NULL");
            }
            void setOffset(IJsonElement* json)  { m_offsetValue = ScriptValue::create(json);}
            void setLength(IJsonElement* json)  { m_lengthValue = ScriptValue::create(json);}
            void setStrip(IJsonElement* json)  { m_stripNumberValue = ScriptValue::create(json);}
            void setUnit(IJsonElement*json) {if (json) {m_unit = parseJsonUnit(json);}}
            void setReverse(IJsonElement*json) { if (json) {
                 //m_reverse = getBool(json,false);
                 m_reverseValue = ScriptValue::create(json);
                 }
            }
            void setHSLOperation(IJsonElement*json){
                m_logger->never("PositionProperty.setHSLOperation %x",json);
                if (json) {m_hslOperation = parseJsonOperation(json);}
                m_logger->never("PositionProperty.setHSLOperation %d",m_hslOperation);
            }
            void setHSLOperation(HSLOperation op) { m_hslOperation = op;}
            void setWrap(bool wrap) { m_wrap = wrap;}
            void setClip(bool clip) { 
                m_logger->debug("setClip(bool) %x %d",this,clip);
                m_clip = clip;
            }
            void setReverse(bool reverse) { m_reverse = reverse;}
            void setCover(bool cover) { m_cover = cover;}
            void setUnit(PositionUnit unit) { 
                m_logger->never("set unit %d",unit);
                m_unit = unit;
            }
            void setOffset(int val) { m_offset = val;}
            void setLength(int val) { m_length = val;}
            void setGap(int val) { m_gap = val;}


        protected:
            bool getBool(IJsonElement* json, bool defaultValue) { 
                if (json==NULL) {
                    m_logger->debug("json is NULL");
                    return defaultValue;
                }
                IJsonValueElement* val = json->asValue();
                if (val == NULL) { 
                    m_logger->debug("json is not a value");

                    return defaultValue;
                }
                bool b = val->getBool(defaultValue);
                m_logger->debug("\tvalue=%d",(int)b);
                return b;
            }

            HSLOperation parseJsonOperation(IJsonElement*json) {
                if (json == NULL) { 
                    m_logger->never("no HSL operation specified");
                    return INHERIT;
                }
                auto opVal = json->asValue();
                HSLOperation op = INHERIT;
                if (opVal) {
                    m_logger->never("got HSLOp %s",opVal->getString());
                    op =  (HSLOperation)Util::mapText2Int(
                    "replace:0,add:1,subtract:2,sub:2,average:3,avg:3,min:4,max:5",opVal->getString(),INHERIT);
                }
                m_logger->never("return op %d",op);
                return op;
            }

           PositionUnit parseJsonUnit(IJsonElement* json) {
                PositionUnit unit = POS_INHERIT;
                m_logger->never("PositionProperties.setUnit");
                auto unitVal = json ? json->asValue() : NULL;
                if (unitVal) {
                    unit = stringToUnit(unitVal->getString());
                    m_logger->never("JSON unit %s ==> %d",unitVal->getString(),unit);
                } 
                m_logger->never("\tunit=%d",unit);
                return unit;
            }

            DECLARE_LOGGER();
            // evaluatable values
            IScriptValue* m_offsetValue;
            IScriptValue* m_lengthValue;
            IScriptValue* m_stripNumberValue;
            IScriptValue* m_reverseValue;
            IScriptValue* m_gapValue;

            // evaluated values
            UnitValue m_offset;
            UnitValue m_length;
            UnitValue m_gap;
            bool    m_clip;
            bool    m_wrap;
            uint8_t m_stripNumber;
            PositionUnit    m_unit;
            bool m_center;
            bool m_cover;
            bool m_flow;
            bool m_absolute;   
            bool m_reverse;     

            HSLOperation m_hslOperation;
    };

    PositionProperties DEFAULT_PROPERTIES;

    class ElementPositionBase : public IElementPosition {
        public: 
            ElementPositionBase() {
                SET_LOGGER(ScriptPositionLogger);
                m_properties = &DEFAULT_PROPERTIES;
            
            }

            virtual ~ElementPositionBase() {
                m_logger->never("~ElementPositionBase  %x %x",m_properties, &DEFAULT_PROPERTIES);
                if (m_properties != &DEFAULT_PROPERTIES) {
                    delete m_properties;
                }
            }

            void destroy() override { 
                m_logger->never("destroy ElementPositionBase");                
                delete this; 
            }

            
            bool fromJson(JsonObject*json) override {
                m_logger->debug("ElementPositionBase.fromJson %x %x",this,m_properties);
                LogIndent li;

                IJsonElement * offsetValue = json->getPropertyValue("offset");
                IJsonElement * lengthValue = json->getPropertyValue("length");
                IJsonElement * stripNumberValue = json->getPropertyValue("strip");
                IJsonElement * clip = json->getPropertyValue("clip");
                IJsonElement * wrap = json->getPropertyValue("wrap");
                IJsonElement * absolute = json->getPropertyValue("absolute");
                IJsonElement * cover = json->getPropertyValue("cover");
                IJsonElement * center = json->getPropertyValue("center");
                IJsonElement * flow = json->getPropertyValue("flow");
                IJsonElement * unit = json->getPropertyValue("unit");
                IJsonElement * reverse = json->getPropertyValue("reverse");
                IJsonElement * op = json->getPropertyValue("op");
                IJsonElement * gap = json->getPropertyValue("gap");

                if (offsetValue || lengthValue || stripNumberValue ||
                    clip || wrap || absolute || cover || center || flow || unit || reverse || gap){
                        if(m_properties == NULL || m_properties == &DEFAULT_PROPERTIES) {
                            m_logger->debug("create properties %x (default=%x)",m_properties,&DEFAULT_PROPERTIES);
                            m_properties = new PositionProperties();
                        }
                        m_properties->setOffset(offsetValue);
                        m_properties->setLength(lengthValue);
                        m_properties->setStrip(stripNumberValue);
                        m_properties->setClip(clip);
                        m_properties->setWrap(wrap);
                        m_properties->setPositionAbsolute(absolute);
                        m_properties->setCover(cover);
                        m_properties->setCenter(center);
                        m_properties->setFlow(flow);
                        m_properties->setUnit(unit);
                        m_properties->setReverse(reverse);
                        m_properties->setHSLOperation(op);
                        m_properties->setGap(gap);
                    }

                return true;
            }

 
            bool toJson(JsonObject* json) const override  {
                m_logger->debug("ElementPositionBase.toJson %x %x",this,m_properties);
                LogIndent li;
                if (m_properties == &DEFAULT_PROPERTIES){
                    m_logger->info("DEFAULT_PROPERTIES");
                    return true;
                }
                return m_properties->toJson(json);

            }     

            void evaluateValues(IScriptContext* context) {
                m_properties->evaluateValues(context);
            }

            PositionOverflow getOverflow() const override { 
                return m_properties->getOverflow();
            }

            bool isClip() const { return m_properties->isClip();}
            bool isWrap() const { return m_properties->isWrap();}
            bool isCenter() const { return m_properties->isCenter();}
            bool isFlow() const { return m_properties->isFlow() && !m_properties->hasStrip() && !m_properties->useRootStrip() && !m_properties->isCover() && (hasOffset() || hasLength());}
            bool isCover() const { return m_properties->isCover();}
            bool isReverse() const { return m_properties->isReverse();}
            bool isPositionAbsolute() const { return m_properties->useRootStrip();}
            bool isPositionRelative() const { return !m_properties->useRootStrip();}
            UnitValue getGap() const  { return m_properties->getGap();}
            bool hasOffset() const { return  m_properties->hasOffset();}
            UnitValue getOffset() const { return m_properties->getOffset();}
            bool hasLength() const { 
                m_logger->never("ElementPosition.hasLength %d",m_properties->hasLength());
                return m_properties->hasLength();
            }
            UnitValue getLength() const { return m_properties->getLength();}
            HSLOperation getHSLOperation() const { return m_properties->getHSLOperation();}

            bool hasStrip()const  { return m_properties->hasStrip();}
            int getStrip()const { return m_properties->getStrip();}



        protected:
 
        protected:
            DECLARE_LOGGER();    
            PositionProperties* m_properties;
    };

    class RootElementPosition : public ElementPositionBase {
        public: 
            RootElementPosition() {
                m_properties = new PositionProperties();
                m_properties->setUnit(POS_PERCENT);

                m_properties->setWrap(true);
                m_properties->setCover(false);
                m_properties->setHSLOperation(REPLACE);

            }

            virtual ~RootElementPosition() {

            }

 
            IElementPosition* getParent()const override {return NULL;};
            void setParent(IElementPosition*parent) {
                m_logger->debug("RootElementPosition cannot have a parent");
            }


            void setRootPosition(int offset, int length) {
                m_properties->setOffset(offset);
                m_properties->setLength(length);
            }

            PositionUnit getUnit() const override {
                m_logger->never("RootElementPosition.getUnit()" );
                PositionUnit unit = m_properties->getUnit();
                m_logger->never("\tunit=%d",unit);
                m_logger->never("\treturn unit=%d",unit);
                return unit;
            }

        protected:
            int m_ledCount;
    };


    class ScriptElementPosition : public ElementPositionBase {
        public:
            ScriptElementPosition() {
                m_parent = NULL;
            }

 
            virtual ~ScriptElementPosition() {
                m_logger->debug("~destroy ScriptElementPosition");

            }

            void destroy() override { 
                m_logger->debug("destroy ScriptElementPosition");
                delete this;
            }

            PositionUnit getUnit() const override {
                m_logger->never("ScriptElementPosition.getUnit()");
                PositionUnit unit = m_properties ? m_properties->getUnit() : POS_INHERIT;
                if (unit == POS_INHERIT) { 
                    m_logger->never("\tget parent unit %s", (m_parent ? "" : "no parent"));
                    unit = m_parent ? m_parent->getUnit() : POS_INHERIT;
                    m_logger->never("\tgot parent unit %d",unit);
                }
                return unit;
            }

            IElementPosition* getParent()const override {return m_parent;}
            void setParent(IElementPosition*parent) { m_parent = parent;}
        protected:
            IElementPosition* m_parent;


 

           
    };
};

#endif
