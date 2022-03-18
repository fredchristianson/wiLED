#ifndef STRIP_ELEMENT_H
#define STRIP_ELEMENT_H

// these ScriptElements add strips in the hierarchy to modify LED operations
#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_container.h"
#include "../loggers.h"

namespace DevRelief
{
 
    
    class ModifiedHSLStrip : public ScriptHSLStrip {
        public:
            ModifiedHSLStrip() : ScriptHSLStrip() {
                SET_LOGGER(StripElementLogger);
            }

            virtual ~ModifiedHSLStrip() {

            }

        protected:


    };

    class MirrorStrip : public ScriptHSLStrip {
        public:
            MirrorStrip() : ScriptHSLStrip() {
                m_lastLed = 0;
            }

            virtual ~MirrorStrip() {

            }

            void updatePosition() { 
                
                // int len = m_parentLength;
                // if (m_position->hasLength()){
                //     UnitValue uv = m_position->getLength();
                //     LogIndent li(m_logger,"MirrorStrip.updatePosition",NEVER_LEVEL);
                //     len = unitToPixel(uv);
                //     m_logger->never("defined len %d  %d %3.3f",len,(int)uv.getUnit(),uv.getValue());
                // }
                // m_lastLed = m_offset+len-1;
                // m_length = len/2;
                // m_logger->never("mirror last=%d len=%d",m_lastLed,m_length);
            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setHue(hue,tidx,op);
                m_parent->setHue(hue,mirrorIndex(tidx),top);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                m_logger->never("ScriptHSLStrip.setSaturation op=%d",translateOp(op)); 
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setSaturation(saturation,tidx,op);
                m_parent->setSaturation(saturation,mirrorIndex(tidx),top);
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setLightness(lightness,tidx,op);
                m_parent->setLightness(lightness,mirrorIndex(tidx),top);
                
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setRGB(rgb,tidx,op);
                m_parent->setRGB(rgb,mirrorIndex(tidx),top);
                
            }   

            int getParentLength() override {
                return m_parent->getLength()/2;
            }

        protected:
            int mirrorIndex(int idx) {
                return m_parent->getLength() - (idx-m_offset-m_relativeOffset);
            }
            friend class MirrorElement;
            int m_lastLed;

    };

    class CopyStrip : public ScriptHSLStrip {
        public:
            CopyStrip() : ScriptHSLStrip() {
                m_count = 0;
                m_repeatOffset = 0;
            }

            virtual ~CopyStrip() {

            }

            void setCount(int count) { 
                m_count = count;
                if (m_count == 0) {
                    m_length = 0;
                    m_repeatOffset = 0;
                } else {
                    m_length = m_parentLength/m_count;
                    m_repeatOffset = m_count==0 ? 0  : m_length;
                    m_overflow = OVERFLOW_ALLOW;
                    m_logger->never("copy: %d %d %d %d",m_count,m_length,m_parentLength,m_repeatOffset);
                }
            }


            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setHue(hue,tidx+i*m_repeatOffset,top);
                }
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setSaturation(saturation,tidx+i*m_repeatOffset,top);
                }
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setLightness(lightness,tidx+i*m_repeatOffset,top);
                }
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setRGB(rgb,tidx+i*m_repeatOffset,top);
                }                
            }              

        protected:
            friend class CopyElement;

            int m_count;
            int m_repeatOffset;

    };

      class RepeatStrip : public ScriptHSLStrip {
        public:
            RepeatStrip() : ScriptHSLStrip() {
                m_repeatLength = 0;
                m_repeatCount = 0;
            }

            virtual ~RepeatStrip() {

            }

            void setRepeatLength(int length) { 
                m_repeatLength = length;
                if (m_parentLength >0) {
                    m_repeatCount = m_parentLength/m_repeatLength;
                }
            }


            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<=m_repeatCount;i++) {
                    if (tidx+i*m_repeatLength >= m_length) break;
                    m_parent->setHue(hue,tidx+i*m_repeatLength,top);
                }
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<=m_repeatCount;i++) {
                    if (tidx+i*m_repeatLength >= m_length) break;
                    m_parent->setSaturation(saturation,tidx+i*m_repeatLength,top);
                }
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<=m_repeatCount;i++) {
                    if (tidx+i*m_repeatLength >= m_length) break;
                    m_parent->setLightness(lightness,tidx+i*m_repeatLength,top);
                }
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<=m_repeatCount;i++) {
                    if (tidx+i*m_repeatLength >= m_length) break;
                    m_parent->setRGB(rgb,tidx+i*m_repeatLength,top);
                }                
            }              

        protected:
            friend class CopyElement;

            int m_repeatCount;
            int m_repeatLength;

    };
    
    class StripElement : public ScriptContainer {
        public:
            StripElement(const char * type, IScriptHSLStrip* strip,ScriptContainer*parent) 
            : m_context(parent->getContext()), ScriptContainer(type,&m_context,strip, &m_elementPosition) {

            }

            void valuesToJson(JsonObject* json) const override {
                ScriptContainer::valuesToJson(json);
            }

            void valuesFromJson(JsonObject* json) override {
                ScriptContainer::valuesFromJson(json);

            }            

        protected:
            ScriptElementPosition m_elementPosition;
            ChildContext m_context;
    };

    /* every LED operation is drawn "normal" and mirrored around the center of the parent strip.
     * the length of the strip is 1/2 the parent length so %-based values only cover the original, not mirrored leds
     */
    class MirrorElement : public StripElement {
        public:
            MirrorElement(ScriptContainer*parent) : StripElement("mirror",&m_mirrorStrip,parent){

            }

            void valuesToJson(JsonObject* json) const override {
                StripElement::valuesToJson(json);
            }

            void valuesFromJson(JsonObject* json) override {
                StripElement::valuesFromJson(json);

            }    

            virtual bool beforeDrawChildren() { 
                m_mirrorStrip.updatePosition();
                return true;
            }                
        private:
            MirrorStrip  m_mirrorStrip;
    };

    /* the parent strip is divided into "m_count" sections that are copies the original.
        the length of this strip is (1/m_count X parent_length) calculated at the start of each step.  
        so percent-based values work on one section.
     */
    class CopyElement : public StripElement {
        public:
            CopyElement(ScriptContainer*parent) : StripElement("Copy",&m_copyStrip,parent){
                m_countValue = NULL;
                m_count = 1;
                m_logger->never("create CopyElement");
            }

            virtual ~CopyElement() { 
                if (m_countValue) { m_countValue->destroy();}
            }

            virtual bool beforeDrawChildren() { 
                if (m_countValue) {
                    m_count = m_countValue->getIntValue(getContext(),1);
                }
                m_copyStrip.setCount(m_count);
                return true;
            }

            void valuesToJson(JsonObject* json) const override {
                StripElement::valuesToJson(json);
                if (m_countValue) {
                    json->set("count",m_countValue->toJson(json->getRoot()));
                }
            }

            void valuesFromJson(JsonObject* json) override {
                StripElement::valuesFromJson(json);
                m_countValue = ScriptValue::create(json->getPropertyValue("count"));

            }            
        private:
            CopyStrip m_copyStrip;
            int m_count;
            IScriptValue* m_countValue;
    };

    /* this is like a position-repeating pattern, except elements (including containers) are
     * repeated to fill the parent.  
     * repeated containers need to specify a length.  
     * only children with "flow"==true are counted
     * todo: is it worth calculating the container length based on it's children.  
     *          or use the length calculated in the previous draw which may change from time-based ranges.
     */
    class RepeatElement : public StripElement {
        public:
            RepeatElement(ScriptContainer*parent) : StripElement("Repeat",&m_repeatStrip,parent){
                m_logger->never("create RepeatElement");
            }

            virtual ~RepeatElement() { 
            }

            virtual bool beforeDrawChildren() { 
                int childrenLength = 0;
                m_children.each([&](IScriptElement* child) {
                    IElementPosition* cpos = child->getPosition();
                    if (cpos->hasLength() && cpos->isFlow()) {
                        UnitValue uv = cpos->getLength();
                        int len = m_repeatStrip.unitToPixel(uv);
                        childrenLength+=len;
                    }
                });
                m_repeatStrip.setRepeatLength(childrenLength);
                return true;
            }
           
        private:
            RepeatStrip m_repeatStrip;
    };

}



#endif