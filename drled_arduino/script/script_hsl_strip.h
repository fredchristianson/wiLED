#ifndef SCRIPT_HSL_STRIP_H
#define SCRIPT_HSL_STRIP_H

#include "../lib/led/led_strip.h"
#include "./script_interface.h"
#include "../config.h"

namespace DevRelief{

    class ScriptHSLStrip : public IScriptHSLStrip {
        public:
            static int DEFAULT_PIXELS_PER_METER;
            ScriptHSLStrip() {
                SET_LOGGER(ScriptHSLStripLogger);
                m_offset = 0;
                m_length = 0;
                m_relativeOffset = 0;
                m_overflow = OVERFLOW_CLIP;
                m_parent = NULL;
                m_parentLength = 0;
                m_unit = POS_INHERIT;
                m_flowIndex = 0;
                m_position = NULL;
                m_reverse = false;

            }

            virtual ~ScriptHSLStrip() {

            }

            void destroy() { delete this;}

            int getPixelsPerMeter(int strip=-1) override { 
                if (m_parent) {
                    return m_parent->getPixelsPerMeter(strip);
                }
                return DEFAULT_PIXELS_PER_METER;
            }

            int getOffset() override { return m_offset;}
            int getLength() override { return m_length;}

            void setParent(IScriptHSLStrip* parent) override { 
                if (parent == this) { return;}
                m_parent = parent;
            }
            IScriptHSLStrip* getParent() const override {  return m_parent;}

            void setHue(int16_t hue,int index, HSLOperation op) override {
                m_logger->debug("ScriptHSLStrip.setHue(%d,%d)",hue,index);
                if (!isPositionValid(index)) { 
                    m_logger->debug("Invalid index %d, %d",index,m_length);
                    return;
                }
                m_parent->setHue(hue,translateIndex(index),translateOp(op));
                m_logger->debug("hue set %x",this);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                m_logger->never("ScriptHSLStrip.setSaturation op=%d",translateOp(op)); 
                m_parent->setSaturation(saturation,translateIndex(index),translateOp(op));
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_parent->setLightness(lightness,translateIndex(index),translateOp(op));
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_parent->setRGB(rgb,translateIndex(index),translateOp(op));
            }  
  
            int getFlowIndex() const { 
                return m_flowIndex;
            }
            void setFlowIndex(int index) { 
                UnitValue unitGap = m_position->getGap();
                int gap = unitToPixel(unitGap);
                m_flowIndex = index+gap;
                if (m_position && !m_position->hasLength()) {
                    if (m_position->isFlow() && m_parent) {
                        m_parent->setFlowIndex(m_flowIndex+m_offset);
                    }
                }
            }
       
            int unitToPixel(const UnitValue& uv, int strip=-1) {
                double val = uv.getValue();
                PositionUnit unit = uv.getUnit();
                m_logger->never("unitToPixel %d %d    %f",unit,m_unit,val);
                if (unit == POS_INHERIT) {
                    if (m_unit == POS_INHERIT) {
                        unit = m_position->getUnit();
                    } else {
                        unit = m_unit;
                    }
                }
                int pixels = val;
                double meterMultiplier = 0;
                switch(unit) {
                    case POS_PIXEL: 
                        // no change
                        break;
                    case POS_INCH: 
                        meterMultiplier = 1/39.31;
                        break;
                    case POS_CENTIMETER: 
                        meterMultiplier = 1/100.0;
                        break;
                    case POS_METER: 
                        meterMultiplier = 1;
                        break;
                    case POS_INHERIT: 
                    case POS_PERCENT: 
                    default: 
                        val = val/100.0*m_parentLength;
                        break;
                }
                if (meterMultiplier != 0 && m_parent) {
                    // this may get the wrong value if there are multiple strips with different pixels per meter
                    double pixelsPerMeter = m_parent->getPixelsPerMeter(strip);
                    val = val * meterMultiplier * pixelsPerMeter;
                }
                m_logger->never("\tpixels=%f",val);
                return val;

            }



            void updatePosition(IElementPosition * pos, IScriptContext* context) override  {
                m_reverse = pos->isReverse();
                
                m_position = pos;
                m_relativeOffset = 0;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0
                LogIndent li(m_logger,"HSLStrip.updatePosition",NEVER_LEVEL);
                m_logger->never("m_unit before update %d",m_unit);
                m_unit = pos->getUnit();
                m_logger->never("after update unit=%d",m_unit);
                if (pos->isPositionAbsolute()) {
                    m_parent = context->getRootStrip();
                }
                m_parentLength = getParentLength();
                int relativeOffset = 0;
                int strip = -1;
                // todo: combining strip and offset or flow has invalid results
                if (pos->hasStrip()) {
                    strip = pos->getStrip();
                    auto config = Config::getInstance();
                    auto pin = config->getPin(strip);
                    if (pin == 0) {
                        m_length = 0;
                        m_logger->error(LM("pin not found %d"),strip);
                        return;
                    }
                    m_parentLength = pin->ledCount;
                    for(int i=0;i<strip;i++) {
                        auto p = config->getPin(i);
                        relativeOffset += p->ledCount;
                    }
                }
                if (pos->isCover()) {
                    m_offset = 0;
                    m_length = m_parentLength;
                } else {
                    m_length = pos->hasLength() ? unitToPixel(pos->getLength(),strip) : m_parentLength-m_parent->getFlowIndex();
                    m_logger->debug("set length %d %d %d",m_length,m_parentLength,m_parent->getFlowIndex());
                    m_offset = pos->hasOffset() ? unitToPixel(pos->getOffset(),strip) : 0;
                    if (pos->isCenter()) {
                        int margin = (m_parentLength - m_length)/2;
                        relativeOffset += margin;
                        m_logger->debug("center %d %d %d-%d/2",margin,relativeOffset,m_parentLength,m_length);
                    } else {
                        if (pos->isFlow()) {
                            relativeOffset += m_parent->getFlowIndex();
                        } else {
                        }
                    }
                }
                m_relativeOffset = relativeOffset;
                m_overflow = pos->getOverflow();
                m_logger->debug("Overflow %d (%d/%d)",m_overflow,m_offset,m_length);
                if (pos->isFlow()) {
                    m_parent->setFlowIndex(m_offset+m_relativeOffset+m_length);
                }
                m_logger->debug("offset=%d relativeOffset=%d length=%d  parentLength=%d  op=%d unit=%d overflow=%d",m_offset,m_relativeOffset,m_length,m_parentLength,pos->getHSLOperation(),m_unit,m_overflow);
            }

            virtual bool isPositionValid(int index) {
                
                if (m_parent == NULL ) { return false;}
                if (m_overflow != OVERFLOW_CLIP) { 
                    return true;
                }
                return index >= 0 || index < m_length;
            }

            virtual int translateIndex(int origIndex){
               int index = m_reverse ? (m_length-origIndex-1) : origIndex;
                index = m_offset+index;
                if (m_overflow == OVERFLOW_WRAP) {
                    index = index%m_length;
                    if (index<0) { index = m_length-index-1;}
                    m_logger->debug("wrap %d/%d %d %d ==> %d",m_offset,m_relativeOffset, m_length,origIndex,index);
                } else if (m_overflow == OVERFLOW_CLIP) {
                    // shouldn't happen since isPositionValid() was false if tidx out of range
                    if (index < m_offset) { index = m_offset;}
                    if (index >= m_offset+m_length) {index = m_offset+m_length-1;}
                }
                int tidx = index + m_relativeOffset;
                m_logger->debug("translated index  %d (%d)  offset=%d length=%d ==>%d",origIndex,index,m_offset, m_length,tidx);
                return tidx;
            }

            
            virtual HSLOperation translateOp(HSLOperation op) {
                m_logger->debug("ScriptHSLStrip.translateOp %d",op);
                return op;
            }

            virtual int getParentLength() {
                return m_parent->getLength();
            }

            IScriptHSLStrip* m_parent;
            IElementPosition* m_position;
            int m_parentLength;
            int m_length;
            int m_relativeOffset;  // offset relative to parent
            int m_offset;  // offset relative to m_relativeOffset
            int m_flowIndex;
            bool m_reverse;


            PositionUnit m_unit;
            PositionOverflow m_overflow;
            DECLARE_LOGGER();
    };

    int ScriptHSLStrip::DEFAULT_PIXELS_PER_METER=30;

    class RootHSLStrip : public ScriptHSLStrip {
        public:
            RootHSLStrip( ) : ScriptHSLStrip(){
                m_base = NULL;
                m_offset = 0;
                m_overflow = OVERFLOW_WRAP;
                m_parentLength = 0;
            }

            virtual ~RootHSLStrip() {

            }

            int getPixelsPerMeter(int strip=-1) override {
                int ppm = 0;
                if (m_base) {
                    if (strip >=0){
                        auto config = Config::getInstance();
                        auto pin = config->getPin(strip);
                        if (pin) {
                            return pin->pixelsPerMeter;
                        }
                    }
                    ppm = m_base->getPixelsPerMeter();
                    
                }
                return ppm > 0 ? ppm : DEFAULT_PIXELS_PER_METER;
            }


            void updatePosition(IElementPosition * pos, IScriptContext* context) override  {
                m_logger->debug(LM("RootHSLStrip.updatePosition %d %d"),pos->isWrap(),pos->isClip());
                m_position = pos;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0
                m_parentLength = m_base->getCount();

                LogIndent id(m_logger,"RootHSLStrip.update ",NEVER_LEVEL);
                m_logger->never("\tplen %d",m_parentLength);
                m_logger->never("\tunit %d %d",m_unit, pos->getUnit());
                m_unit = pos->getUnit();
                m_logger->never("\tupdated unit %d %d",m_unit, pos->getUnit());
                if (pos->hasLength()) {
                    m_length = unitToPixel(pos->getLength());
                } else {
                    m_length = m_parentLength;
                }
                m_logger->debug("root len %d",m_length);
                if (pos->hasOffset()) {
                    m_offset = unitToPixel(pos->getOffset());
                } else {
                    m_offset = 0;
                }
                m_reverse = pos->isReverse();
                

                m_logger->debug("\toffset %d",m_offset);
                m_overflow = pos->getOverflow();
                m_logger->debug("\toverflow %d",m_overflow);
                m_logger->debug("\toverflow=%d offset=%d length=%d unit=%d",m_overflow,m_offset,m_length,m_unit);
            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}
                m_logger->debug("RootStrip.setHue %d %d",hue,index);
                m_base->setHue(translateIndex(index),hue,translateOp(op));
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}     
                m_logger->never("RootHSLStrip.setSaturation op=%d",translateOp(op));        
                m_base->setSaturation(translateIndex(index),saturation,translateOp(op));
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_base->setLightness(translateIndex(index),lightness,translateOp(op));
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_base->setRGB(translateIndex(index),rgb,translateOp(op));
            }  

            void setHSLStrip(IHSLStrip* base) {
                m_base = base;
                m_length = base ? base->getCount() : 0;
                m_offset = 0;
                m_parentLength = m_length;
            }


            virtual bool isPositionValid(int index) {
                return index >= 0 || index < m_length;
            }


        protected:
            virtual HSLOperation translateOp(HSLOperation op) {
                m_logger->debug("root translate %x op %d %d",m_position,op,m_position->getHSLOperation());
                
                if (op == INHERIT || op == UNSET) {
                    op = m_position ? m_position->getHSLOperation() : ADD;
                }
                return op;
            }

            IHSLStrip* m_base;
    };

    class ContainerElementHSLStrip : public ScriptHSLStrip {
        public:
            ContainerElementHSLStrip() : ScriptHSLStrip() {

            }

            virtual ~ContainerElementHSLStrip() {

            }

    };

    class DrawLED : public IHSLStripLED {
        public:
            DrawLED(IScriptHSLStrip* strip, IScriptContext*context,HSLOperation op) {
                m_strip = strip;
                m_context = context;
                m_operation = op;
                SET_LOGGER(ScriptHSLStripLogger);
            }

            void setIndex(int p) {
                m_index = p;
            }

            int index() { return m_index;}


            void setHue(int hue) override {
                m_logger->debug("Set hue %d %d",m_index,hue);
                m_strip->setHue(hue,m_index,m_operation);
            }
            void setSaturation(int saturation) override {
                m_strip->setSaturation(saturation,m_index,m_operation);
            }
            void setLightness(int lightness) override {
                m_strip->setLightness(lightness,m_index,m_operation);
            }
            void setRGB(const CRGB& rgb)  override {
                m_strip->setRGB(rgb,m_index,m_operation);
            }

            IScriptContext* getContext() const override { return m_context;}
            IScriptHSLStrip* getStrip() const override { return m_strip;}
            virtual int getIndex()const {return m_index;}
        private:
            int m_index;
            HSLOperation m_operation;
            IScriptHSLStrip* m_strip;
            IScriptContext * m_context;
            DECLARE_LOGGER();
    };

    class DrawStrip : public ScriptHSLStrip {
        public:
            DrawStrip(IScriptContext* context, IScriptHSLStrip*parent, IElementPosition*position) : ScriptHSLStrip(){
                m_context = context;
                m_parent = parent;
                m_position = position;
                m_position->evaluateValues(context);
                updatePosition(position,context);
            }

            virtual ~DrawStrip() {

            }

            void eachLED(auto&& drawer) {
                
                HSLOperation op = m_position->getHSLOperation();
                m_logger->debug("eachLED HSL op: %d",op);
                DrawLED led(this,m_context,op);
                if (m_length == 0) {
                    return;
                }
                PositionDomain* domain = m_context->getAnimationPositionDomain();
                if (domain) { domain->setPosition(0,0,m_length-1); }
                int count = abs(m_length);
                int neg = m_length<0?-1 : 1;
                m_logger->debug("drawStrip %d",count);
                for(int i=0;i<count;i++){
                    if (domain) {domain->setPos(i);}
                    led.setIndex(i*neg);
                    drawer(led);
                }
            }



            // void setHue(int16_t hue, int position, HSLOperation op) { m_parent->setHue(hue,position+m_offset,translateOp(op));}
            // void setSaturation(int16_t saturation, int position, HSLOperation op) { m_parent->setSaturation(saturation,position+m_offset,translateOp(op));}
            // void setLightness(int16_t lightness, int position, HSLOperation op) { m_parent->setLightness(lightness,position+m_offset,translateOp(op));}
            // void setRGB(const CRGB& rgb, int position, HSLOperation op) { m_parent->setRGB(rgb,position+m_offset,translateOp(op));}


        private:
            IScriptContext* m_context;
            IElementPosition*m_position;


    };

    

}

#endif