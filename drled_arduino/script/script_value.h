#ifndef DRSCRIPT_VALUE_H
#define DRSCRIPT_VALUE_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
#include "../lib/util/drstring.h"
#include "../lib/util/util.h"
#include "../lib/util/list.h"
#include "../lib/led/led_strip.h"
#include "../lib/led/color.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./animation.h"


namespace DevRelief
{

    const char * functionNames[] = {
        "rand","add","+","subtract","sub","-","multiply","*","mult",
        "divide","div","/","mod","%","min","max","randOf","seq","sequence",
        NULL
    };

    // ScriptValueReference is a pointer to another ScriptValue 
    // the pointer can be deleted while the real value remains
    class ScriptValueReference : public IScriptValue {
        public:
            ScriptValueReference(IScriptValue* ref) {
                m_reference = ref;
            }

            virtual ~ScriptValueReference() { /* do not delete m_reference*/}
            void destroy() override { delete this;}

            int getIntValue(IScriptContext* ctx,  int defaultValue)  { return m_reference->getIntValue(ctx,defaultValue);}
            double getFloatValue(IScriptContext* ctx,  double defaultValue)  { return m_reference->getFloatValue(ctx,defaultValue);} 
            bool getBoolValue(IScriptContext* ctx,  bool defaultValue)  { return m_reference->getBoolValue(ctx,defaultValue); }
            int getMsecValue(IScriptContext* ctx,  int defaultValue)  { return m_reference->getMsecValue(ctx,defaultValue);}
            ScriptStatus getStatus(IScriptContext* ctx,ScriptStatus defaultValue)const { return m_reference->getStatus(ctx,defaultValue);};
            UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) { return m_reference->getUnitValue(ctx,defaultValue,defaultUnit);}

            bool isString(IScriptContext* ctx) const { return m_reference->isString(ctx);}
            bool isNumber(IScriptContext* ctx) const { return m_reference->isNumber(ctx);}
            bool isBool(IScriptContext* ctx) const { return m_reference->isBool(ctx);}
            bool isNull(IScriptContext* ctx) const { return m_reference->isNull(ctx);}
            bool isUnitValue(IScriptContext* ctx) const { return m_reference->isUnitValue(ctx);}
            bool isTimer(IScriptContext* ctx) const { return m_reference->isTimer(ctx);}

            bool equals(IScriptContext*ctx, const char * match) const override { return m_reference->equals(ctx,match);}


            // evaluate this IScriptValue with the given command and return a new
            // IScriptValue.  mainly useful to get a random number one time
            IScriptValue* eval(IScriptContext*ctx, double defaultValue) { m_reference->eval(ctx,defaultValue); }


            bool isRecursing() const { return m_reference->isRecursing();} 

            IJsonElement* toJson(JsonRoot* jsonRoot) { m_reference-> toJson(jsonRoot);}
            // for debugging
            DRString toString() { return m_reference->toString();}           

            DRString stringify() { return m_reference->stringify();}

            IScriptValue* clone()const override { return m_reference->clone();}
        private:
            IScriptValue* m_reference;
    };

    class ScriptValue : public IScriptValue {
        public:
            static IScriptValue* create(IJsonElement*json, JsonObject* parent=NULL);
            static IScriptValue* createFromString(const char * string);
            static IScriptValue* createFromArray(JsonArray* json, JsonObject* parent=NULL);
            static IScriptValue* createFromObject(JsonObject* json);
            static ScriptPatternElement* createPatternElement(IJsonElement*json);
            static IAnimationEase* createEase(JsonObject* json);
            static IAnimationDomain* createDomain(JsonObject* json, IAnimationRange* range);
            static IScriptValue* createPattern(JsonArray* pattern, bool smooth, bool repeat, bool unfold, JsonObject* json=NULL);
            static IScriptTimer* createTimer(IJsonElement*json);
            ScriptValue() {
                SET_LOGGER(ScriptValueLogger);
            }
            virtual ~ScriptValue() {
                m_logger->never("~ScriptValue");
            }

            void destroy() override { delete this;}

            bool isRecursing() const override { return false;}

            bool isString(IScriptContext* ctx)  const override{
              return false;  
            } 
            bool isNumber(IScriptContext* ctx)  const override{
              return false;  
            } 
            bool isBool(IScriptContext* ctx)  const override{
              return false;  
            } 

            bool isNull(IScriptContext* ctx)  const override{
              return false;  
            } 

            bool isTimer(IScriptContext* ctx)  const override{
              return false;  
            } 

            bool isUnitValue(IScriptContext* ctx) const override { return false;}
            UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override{
                UnitValue uv(getFloatValue(ctx,defaultValue),defaultUnit);
                return uv;
            }

            bool equals(IScriptContext*ctx,const char * match) const override { return false;}

            //IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override;
            
            IJsonElement* toJson(JsonRoot* jsonRoot) override {
                m_logger->error("toJson() not implemented");
                JsonObject* obj = new JsonObject(jsonRoot);
                obj->setString("toJson","not implemented");
                return obj;
            }

            IScriptValue* eval(IScriptContext * ctx, double defaultValue) override;

            int getIntValue(IScriptContext* ctx,int defaultValue)  {
                return getFloatValue(ctx,defaultValue);
            }

            bool getBoolValue(IScriptContext* ctx, bool defaultValue) override { return defaultValue;}
            int getMsecValue(IScriptContext* ctx,  int defaultValue) override { return defaultValue;}
            DRString stringify() override { return "";}

            ScriptStatus getStatus(IScriptContext* context, ScriptStatus defaultValue=SCRIPT_RUNNING) const override { return defaultValue;}
        protected:

      
            DECLARE_LOGGER();
    };

 
  
    class NameValue
    {
    public:
        NameValue(const char *name, IScriptValue *value)
        {
            m_name = Util::allocText(name);
            m_value = value;
        }

        virtual ~NameValue()
        {
            Util::freeText(m_name);
            m_value->destroy();
        }

        virtual void destroy() { delete this;}

        const char *getName() { return m_name; }
        IScriptValue *getValue() { return m_value; }

        void replaceValue(IScriptValue* newValue) {
            if (m_value == newValue) {
                return;
            }
            if (m_value) { m_value->destroy();}
            m_value = newValue;
        }
    private:
        const char * m_name;
        IScriptValue *m_value;
    };
    // ScriptVariableGenerator: ??? rand, trig, ...
    class FunctionArgs {
        public:
            FunctionArgs() {}
            virtual ~FunctionArgs() {}

            FunctionArgs* clone()  {
                FunctionArgs* other = new FunctionArgs();
                args.each([&](IScriptValue* val) {
                    add(val->clone());
                });
            }

            void add(IScriptValue* val) { args.add(val);}
            IScriptValue* get(int index) { return args.get(index);}
            size_t length() { return args.size();}
            PtrList<IScriptValue*> args;
    };

    int randTotal = millis();

    class ScriptFunction : public ScriptValue
    {
    public:
        static bool isFunctionName(const char * val) {
            if (val == NULL || val[0] == NULL) {return false;}
            for(int i=0;functionNames[i] != NULL;i++) {
                if (Util::equal(val,functionNames[i])){
                    return true;
                }
            }
            return false;
        }
    public:
        ScriptFunction(const char *name, FunctionArgs* args=NULL) : m_name(name)
        {
           // randomSeed(analogRead(0)+millis());
            if (args == NULL) {
                m_args = new FunctionArgs();
            } else {
                m_args = args;
            }
            m_funcState = -1;
        }

        virtual ~ScriptFunction()
        {
            delete m_args;
        }

        IScriptValue* clone()const override {
            return new ScriptFunction(m_name,m_args?m_args->clone() : NULL);
        }
        void addArg(IScriptValue*val) {
            m_args->add(val);
        }
        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return (int)getFloatValue(ctx,(double)defaultValue);
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return invoke(ctx,defaultValue);
        }

       bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            double d = getFloatValue(ctx,0);
            bool b = d != 0;
            m_logger->never("func bool: %s",b?"true":"false");
            return b;
        }

        DRString toString() { return DRString("Function: ").append(m_name); }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            if (Util::equal("millis",m_name)) {
                return millis();
            }
            return getFloatValue(ctx,defaultValue);
        }
        bool isNumber(IScriptContext* ctx) const override { return true;}
        
        IJsonElement* toJson(JsonRoot* root) override {
            JsonArray* json = root->createArray();
            json->addString(m_name.text());
            m_args->args.each([&](IScriptValue*arg) {
                json->addItem(arg->toJson(root));
            });
            return json;
        }

        
    protected:
        double invoke(IScriptContext * ctx,double defaultValue) {
            double result = 0;
            const char * name = m_name.get();
            if (Util::equal("rand",name)){
                result = invokeRand(ctx,defaultValue);
            } else if (Util::equal("add",name) || Util::equal("+",name)) {
                result = invokeAdd(ctx,defaultValue);
            } else if (Util::equal("subtract",name) ||Util::equal("sub",name) || Util::equal("-",name)) {
                result = invokeSubtract(ctx,defaultValue);
            } else if (Util::equal("multiply",name) || Util::equal("mult",name) || Util::equal("*",name)) {
                result = invokeMultiply(ctx,defaultValue);
            } else if (Util::equal("divide",name) || Util::equal("div",name) || Util::equal("/",name)) {
                result = invokeDivide(ctx,defaultValue);
            } else if (Util::equal("mod",name) || Util::equal("%",name)) {
                result = invokeMod(ctx,defaultValue);
            } else if (Util::equal("min",name)) {
                result = invokeMin(ctx,defaultValue);
            } else if (Util::equal("max",name)) {
                result = invokeMax(ctx,defaultValue);
            } else if (Util::equal("randOf",name)) {
                result = invokeRandomOf(ctx,defaultValue);
            } else if (Util::equal("seq",name)||Util::equal("sequence",name)) {
                result = invokeSequence(ctx,defaultValue);
            } else {
                m_logger->error("unknown function: %s",name);
            }
            m_logger->never("function: %s=%f",m_name.get(),result);
            return result;
        }

        double invokeRand(IScriptContext*ctx ,double defaultValue) {
            int low = getArgValue(ctx,0,0);
            int high = getArgValue(ctx, 1,low);
            if (high == low) {
                low = 0;
            }
            if (high < low) {
                int t = low;
                low = high;
                high = t;
            }
            int val = random(low,high+1);
            return val;
            
        }

        double invokeAdd(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first + second;
        }

        double invokeSubtract(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            if (m_args->length()==1) {
                return -first;
            }
            double second = getArgValue(ctx,1,defaultValue);
            return first - second;
        }

        double invokeMultiply(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first * second;
        }

        double invokeDivide(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return (second == 0) ? 0 : first / second;
        }

        
        double invokeMod(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return (double)((int)first % (int)second);
        }
        
        double invokeMin(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first < second ? first : second;
        }
        
        double invokeMax(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first > second ? first : second;
        }

        double invokeRandomOf(IScriptContext*ctx,double defaultValue) {
            int count = m_args->length();
            int idx = random(count);
            return getArgValue(ctx,idx,defaultValue);
        }

        double invokeSequence(IScriptContext*ctx,double defaultValue) {
            
            int start = getArgValue(ctx,0,0);
            int end = getArgValue(ctx,1,100);
            int step = getArgValue(ctx,2,1);

            if (m_funcState < start){ 
                m_funcState = start;
            } else {
                m_funcState += step;
            }
            if (m_funcState > end){
                m_funcState = start;
            }

            m_logger->never("Sequence %d %d %d ==> %f",start,end,step,m_funcState);
            return m_funcState;
        }

        double getArgValue(IScriptContext*ctx, int idx, double defaultValue){
            if (m_args == NULL) {
                return defaultValue;
            }
            IScriptValue* val = m_args->get(idx);
            if (val == NULL) { return defaultValue;}
            return val->getFloatValue(ctx,defaultValue);
        }

        DRString m_name;
        FunctionArgs * m_args;
        double m_funcState; // different functions can use in their way
    };

    class ScriptNumberValue : public ScriptValue
    {
    public:
        ScriptNumberValue(double value=0) : m_value(value)
        {

        }



        ScriptNumberValue(IScriptContext*ctx, IScriptValue* base,double defaultValue) {
            double val = base->getFloatValue(ctx,defaultValue);
            m_value = val;
        }

        virtual ~ScriptNumberValue()
        {
        }

        virtual int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            int v = m_value;
            m_logger->test("getIntValue %f %d",m_value,v);
            return v;
        }

        virtual double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return m_value;
        }

        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            return m_value != 0;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return m_value;
        }
        bool isNumber(IScriptContext* ctx) const override { return true;}

        virtual DRString toString() { return DRString::fromFloat(m_value); }
        IJsonElement* toJson(JsonRoot*root) override { return new JsonFloat(root,m_value);}
        DRString stringify() override { return DRString::fromFloat(m_value);}
        IScriptValue* clone() const override{ return new ScriptNumberValue(m_value);}

        void setNumberValue(double val) { m_value = val;}
    protected:
        double m_value;
    };

    class ScriptBoolValue : public ScriptValue
    {
    public:
        ScriptBoolValue(bool value) : m_value(value)
        {
            m_logger->debug("ScriptBoolValue()");
        }

        virtual ~ScriptBoolValue()
        {
            m_logger->debug("~ScriptBoolValue()");
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return m_value ? 1 : 0;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return m_value ? 1 : 0;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override {
            return m_value;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return defaultValue;
        }
        bool isBool(IScriptContext* ctx) const override { return true;}
        IJsonElement* toJson(JsonRoot*root) override { return new JsonBool(root,m_value);}

        DRString toString() override { 
            const char * val =  m_value ? "true":"false"; 
            DRString drv(val);
            return drv;
        }

        DRString stringify() override { return m_value ? "true":"false";}
        IScriptValue* clone() const override{ return new ScriptBoolValue(m_value);}

    protected:
        bool m_value;
    };

    
    class ScriptNullValue : public ScriptValue
    {
    public:
        ScriptNullValue() 
        {
            m_logger->debug("ScriptNullValue()");
        }

        virtual ~ScriptNullValue()
        {
            m_logger->debug("~ScriptNullValue()");
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return defaultValue;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return defaultValue;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override {
            return defaultValue;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return defaultValue;
        }

        bool isBool(IScriptContext* ctx) const override { return false;}
        bool isNull(IScriptContext* ctx) const  override{
            return true;  
        } 

        IJsonElement* toJson(JsonRoot*root) override { return new JsonNull(root);}

        DRString toString() override { 
            m_logger->debug("ScriptNulllValue.toString()");
            DRString drv("ScriptNullValue");
            m_logger->debug("\tcreated DRString");
            return drv;
        }

        DRString stringify() override { return "null";}
        IScriptValue* clone() const override { return new ScriptNullValue();}
    protected:

    };


    class ScriptStringValue : public ScriptValue
    {
    public:
        ScriptStringValue(const char *value) : m_value(value)
        {
            m_logger->never("ScriptStringValue 0x%04X %s",this,value ? value : "<null>");
        }

        virtual ~ScriptStringValue()
        {
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return atoi(n);
            }
            return defaultValue;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return atof(n);
            }
            return defaultValue;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return Util::equal(n,"true");
            }
            return defaultValue;
        }

        bool equals(IScriptContext*ctx, const char * match) const override { 
            m_logger->never("ScriptStringValue.equals %s==%s",m_value.get(),match);
            return Util::equal(m_value.text(),match);
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return Util::toMsecs(m_value);
        }

        UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override{
            UnitValue uv(getFloatValue(ctx,defaultValue),stringToUnit(m_value.text(),defaultUnit));
            return uv;
        }

        bool isString(IScriptContext* ctx) const override { return true;}
        IJsonElement* toJson(JsonRoot*root) override { return new JsonString(root,m_value);}

        const char * getValue() { return m_value.text();}


        DRString toString() override { return m_value; }
        DRString stringify() override { return m_value;}
        IScriptValue* clone()const override {
            return new ScriptStringValue(m_value);
        }
    protected:
        DRString m_value;
    };

    class AnimatedValue : public ScriptValue {
        public:
            AnimatedValue( ) {
                
            }

            ~AnimatedValue()  {
                if (m_animator) { m_animator->destroy();}
            }

            void setAnimator(IValueAnimator* animator) {
                m_animator = animator;
            }


            
            int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
                return getIntValue(ctx,defaultValue);
            }
            bool isNumber(IScriptContext* ctx) const override { return false;}

            virtual int getIntValue(IScriptContext* ctx,  int defaultValue)
            {
                return getFloatValue(ctx,(double)defaultValue);
            }

            virtual double getFloatValue(IScriptContext* ctx,  double defaultValue)
            {
                UnitValue uv = getUnitValue(ctx,(double)defaultValue,POS_INHERIT);
                return uv.getValue();
            }


            virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
            {
                return getFloatValue(ctx,defaultValue?1:0) != 0;
            }

            IScriptValue* clone()const override {
                m_logger->error(LM("AnimatedValue.clone() not implemented"));
                return NULL;
            }
        protected:
            IValueAnimator* m_animator;
    };

 

    class PatternInterpolation {
        public:
            PatternInterpolation() {
                SET_LOGGER(ScriptValueLogger);
            }
            virtual ~PatternInterpolation() {
            }

            void destroy() { delete this;}

            virtual UnitValue getValue(double pct, IScriptContext* ctx, LinkedList<ScriptPatternElement*>& elements,int pixelCount, double defaultValue, PositionUnit defaultUnit)=0;

            virtual bool toJson(JsonObject* json)const =0;
        protected:
            DECLARE_LOGGER();

    };

   class PatternValue : public AnimatedValue
    {
    public:
        PatternValue() : AnimatedValue()
        {

            m_pixelCount = 0;
            m_interpolation = NULL;

        }

        virtual ~PatternValue()
        {
           if (m_interpolation) { m_interpolation->destroy();}
        }

        IScriptValue* clone()const override {
            m_logger->error(LM("PatternValue.clone() not implemented"));
            return NULL;
        }

        void addElement(ScriptPatternElement* element) {
            if (element == NULL) {
                m_logger->error("ScriptPatternElement cannot be NULL");
                return;
            }
            m_elements.add(element);
            m_pixelCount += element->getPixelCount();
        }

        
        UnitValue getUnitValue(IScriptContext*ctx, double defaultValue, PositionUnit defaultUnit) {
            m_logger->never("PatternValue.getUnitValue %x",m_animator);

            if (m_animator == NULL) {
                return UnitValue(defaultValue,defaultUnit);
            }
            m_pixelCount = 0;
            m_elements.each([&](ScriptPatternElement* element) {
                element->update(ctx);
                m_pixelCount += element->getPixelCount();
            });
            m_animator->update(ctx);

            double pct = m_animator->getRangeValue(ctx);
            PositionUnit unit = m_animator->getUnit();
            if (unit == POS_UNSET) {
                unit = defaultUnit;
            }
            UnitValue value = m_interpolation->getValue(pct,ctx,m_elements,m_pixelCount,defaultValue,unit); 
            m_logger->never("patternvalue: %f",value.getValue());
            //UnitValue val = getValueAt(ctx,  value,defaultValue,POS_INHERIT);
            return value;
        }
   
        IJsonElement* toJson(JsonRoot* jsonRoot) { 
            m_logger->never("PatternValue.toJson");
            JsonObject* json = jsonRoot->getTopObject();
            m_logger->never("\tcreate array");
            JsonArray* pattern = json->createArray("pattern");
            m_logger->never("\tpopulate array");
            m_elements.each([&](ScriptPatternElement* element){
                m_logger->never("\taddItem %x",element);
                pattern->addItem(element->toJson(jsonRoot));
            });
            m_logger->never("\tm_animate->toJson %x",m_animator);
            if (m_animator) {
                m_animator->toJson(json);
            }
            if (m_interpolation) {
                m_interpolation->toJson(json);
            }
            return json;
        }
        
        // for debugging
        virtual DRString toString()
        {
            DRString result("pattern: ");
            m_elements.each([&](ScriptPatternElement*e){
                result.append(e->toString());
                result.append(",");
            });
            return result;
        }

        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
        {
            // bool probably doesn't make sense for a pattern.  return true if there are elements
        return m_pixelCount > 0;
        }

        IScriptValue* eval(IScriptContext * ctx, double defaultValue) override{
            JsonRoot root;
            //m_logger->pos('d');
            IJsonElement* json = toJson(&root);
            //m_logger->pos('e');
            IScriptValue* copy = ScriptValue::create(json);
            //m_logger->pos('f');
            return copy;

        }

        size_t getCount() { return m_pixelCount;}

        
        void setInterpolation(PatternInterpolation*interpolation) { m_interpolation = interpolation;}

    protected:
        StepWatcher m_watcher;
        PatternInterpolation* m_interpolation;
        PtrList<ScriptPatternElement*> m_elements;
        size_t m_pixelCount;
       
    };
    
    void ScriptPatternElement::update(IScriptContext* ctx) {
        if (m_repeatCount == NULL) {
            m_pixelCount = 1;
            return;
        }
        UnitValue uv = m_repeatCount->getUnitValue(ctx,1,POS_INHERIT);
        PositionUnit unit = uv.getUnit();
        if (unit == POS_INHERIT) {
            unit = ctx->getCurrentElement()->getPosition()->getUnit();
        }
        if (unit == POS_PERCENT) {
            int length = ctx->getAnimationPositionDomain()->getDistance();
            int repeat = uv.getValue();
            m_pixelCount = round(length * repeat/100.0);
        } else {
            m_pixelCount = uv.getValue();
        }
    }

    class InterpolationSegment {
        public:
        InterpolationSegment() {
            startElementIndex=0;
            endElementIndex=0;
            startPercent=0;
            endPercent=1;
        }
        ~InterpolationSegment() {}
        void destroy() { delete this;}
        int startElementIndex;
        int endElementIndex;
        double startPercent;
        double endPercent;
    };


    class SmoothInterpolation : public PatternInterpolation {
        public:
            SmoothInterpolation() :PatternInterpolation() {
                m_logger->debug(LM("SmoothInterpolation "));
                 
            }
            virtual ~SmoothInterpolation() {

            }

            UnitValue getValue(double pct, IScriptContext* ctx, LinkedList<ScriptPatternElement*>& elements,int pixelCount, double defaultValue, PositionUnit defaultUnit) {
                if (m_stepWatcher.isChanged(ctx)) {
                    m_logger->debug(LM("update SmoothInterpolation segments"));
                    setupSegments(elements,pixelCount);
                }
                InterpolationSegment* segment = findSegment(pct,elements);
                if (segment != NULL) {
                    ScriptPatternElement* start = elements.get(segment->startElementIndex);
                    ScriptPatternElement* end = elements.get(segment->endElementIndex);
                    if (start != NULL && start->getValue() != NULL && end == NULL){
                        m_logger->never("no start.  return end value");
                        return start->getValue()->getUnitValue(ctx,defaultValue,defaultUnit);
                    } else if (end != NULL && end->getValue() != NULL && start == NULL){
                        m_logger->never("no end.  return start value");
                        return end->getValue()->getUnitValue(ctx,defaultValue,defaultUnit);
                    } else {
                        double segmentPct = (pct-segment->startPercent)/(segment->endPercent-segment->startPercent);
                        UnitValue uv = interpolate(ctx,start->getValue(),end->getValue(),segmentPct,defaultValue,defaultUnit);
                        m_logger->never("interpolate spct=%.2f  pct=%.2f  start=%.2f  end=%.2f   result=%.2f  (default=%.2f)",segmentPct,pct,segment->startPercent,segment->endPercent,uv.getValue(),defaultValue);
                        return uv;
                    }

                }
                m_logger->never("no segment found %f",defaultValue);

                return UnitValue(defaultValue,defaultUnit);
               
               
            }

            bool toJson(JsonObject* json)const override {
                json->setBool("smooth",true);
                return true;
            }
        protected:
            void setupSegments(LinkedList<ScriptPatternElement*>& elements,int totalPixels) {
                int elementCount = elements.size();
                if (elementCount<3) {
                    m_segments.clear();
                    if (elementCount==0) { return;}
                    InterpolationSegment* seg = new InterpolationSegment();
                    seg->startElementIndex = 0;
                    seg->endElementIndex = -1;
                    seg->startPercent = 0;
                    seg->endPercent = 1;
                    if (elementCount==2) {
                        seg->endElementIndex=1;
                    }
                    m_segments.add(seg);
                    return;

                }
               if (elements.size() != m_segments.size()+1){
                    m_segments.clear();
                    for(int i=0;i<elements.size()-1;i++) {
                        m_segments.add(new InterpolationSegment());
                    }
                }
                double pixelOffset = 0;
                int startElementIndex = 0;
                int endElementIndex = 0;
                double startPct = 0;
                double endPct = 0;
                int segmentIndex = 0;

                for(int elementIndex=0;elementIndex<elementCount-1;elementIndex++) {
                    InterpolationSegment* segment = m_segments.get(elementIndex);
                    ScriptPatternElement* start = elements.get(elementIndex);                    
                    ScriptPatternElement* end = elements.get(elementIndex+1);
                    segment->startElementIndex = elementIndex;
                    segment->endElementIndex = elementIndex+1;

                    segment->startPercent = startPct;

                    double endPixelOffset =  pixelOffset + start->getPixelCount() + end->getPixelCount()/2.0;
                    if (elementIndex+2>=elementCount) {
                        endPixelOffset =  totalPixels;
                        segment->endPercent = 1;
                    } else {
                        segment->endPercent = 1.0*endPixelOffset/totalPixels;
                    }
                    pixelOffset += start->getPixelCount();
                    startPct = segment->endPercent;
                }

            }

            InterpolationSegment* findSegment(double pct,LinkedList<ScriptPatternElement*>& elements) {
                m_logger->never("find segment %.2f of %d",pct,m_segments.size());
               if (pct == 0 || m_segments.size() <= 2) {
                    return m_segments.get(0);
                }
                if (pct >= 1) {
                    return m_segments.get(m_segments.size()-1);
                }
                for(int i=0;i<m_segments.size();i++) {
                    InterpolationSegment* segment = m_segments.get(i);
                    if (segment->startPercent<=pct && segment->endPercent > pct) {
                        return segment;
                    }
                    m_logger->never("\tno match %.2f < %.2f < %.2f",segment->startPercent,pct,segment->endPercent);

                }
                return NULL;
            }

            virtual UnitValue interpolate(IScriptContext*ctx, IScriptValue*start,IScriptValue*end, double pct, double defaultValue, PositionUnit defaultUnit){
                if (start == NULL && end == NULL) {
                    m_logger->never("no value for start or end");
                    return UnitValue(defaultValue,defaultUnit);
                } else if (start == NULL){
                    m_logger->never("no value for start");
                    return end->getUnitValue(ctx,defaultValue,defaultUnit);
                } else if (end == NULL){
                    m_logger->never("no value for end");
                    return start->getUnitValue(ctx,defaultValue,defaultUnit);
                }
                UnitValue suv = start->getUnitValue(ctx,defaultValue,defaultUnit);
                UnitValue euv = end->getUnitValue(ctx,defaultValue,defaultUnit);
                double sval = suv.getValue();
                double eval = euv.getValue();
                double diff = eval - sval;
                double result = sval + diff*pct;
                m_logger->never("smooth interpolated value %.2f-%.2f  %.2f  %.2f",start->getFloatValue(ctx,-1),end->getFloatValue(ctx,-1),pct,result);

                return UnitValue(result,suv.getUnit());

            }     

            StepWatcher m_stepWatcher;  
            PtrList<InterpolationSegment*> m_segments;    
    };

    class StepInterpolation : public PatternInterpolation {
        public:
            StepInterpolation() :PatternInterpolation() {
            }
            virtual ~StepInterpolation() {

            }

            UnitValue getValue(double pct, IScriptContext* ctx, LinkedList<ScriptPatternElement*>& elements,int totalPixels, double defaultValue, PositionUnit defaultUnit) {
                if (m_stepWatcher.isChanged(ctx)) {
                    setupSegments(elements,totalPixels);
                }
                InterpolationSegment* segment = findSegment(pct,elements);
                if (segment) {
                    ScriptPatternElement*element = elements.get(segment->startElementIndex);
                    IScriptValue* val = element ? element->getValue() : NULL;
                    if (val) {
                        return val->getUnitValue(ctx,defaultValue,defaultUnit);
                    }
                }
                return UnitValue(defaultValue,defaultUnit);
                
            }

            bool toJson(JsonObject* json) const override {
                return true;
            }
        protected:
            void setupSegments(LinkedList<ScriptPatternElement*>& elements,int totalPixels) {
                if (elements.size() != m_segments.size()){
                    m_segments.clear();
                    for(int i=0;i<elements.size();i++) {
                        m_segments.add(new InterpolationSegment());
                    }
                }
                double pixelOffset = 0;
                int index = 0;
                int pct = 0;
                elements.each([&](ScriptPatternElement* element){
                    InterpolationSegment* segment = m_segments.get(index);
                    segment->startElementIndex = index;
                    segment->endElementIndex = index;
                    segment->startPercent = pixelOffset/totalPixels;
                    index++;
                    pixelOffset += element->getPixelCount();
                    segment->endPercent = pixelOffset/totalPixels;
                });
            }

            InterpolationSegment* findSegment(double pct,LinkedList<ScriptPatternElement*>& elements) {
                if (pct <= 0) {
                    return m_segments.get(0);
                }
                if (pct >= 1) {
                    return m_segments.get(m_segments.size()-1);
                }
                for(int i=0;i<m_segments.size();i++) {
                    InterpolationSegment* segment = m_segments.get(i);
                    if (segment->startPercent<=pct && segment->endPercent > pct) {
                        return segment;
                    }
                }
                return NULL;
            }
            PtrList<InterpolationSegment*> m_segments;        
            StepWatcher m_stepWatcher;   
    };

    class PatternRange : public AnimationRange {
        public:
            PatternRange(PatternValue * pattern,bool unfold) : AnimationRange(unfold) {
                SET_LOGGER(ScriptValueLogger);
                m_pattern = pattern;
            }

            virtual ~PatternRange() {

            }

            void destroy() { delete this;}

    



        protected:
            DECLARE_LOGGER();
            PatternValue* m_pattern;
    };

    class StretchPatternRange : public PatternRange {
        public: 
            StretchPatternRange(PatternValue * pattern,bool unfold) : PatternRange(pattern,unfold) {

            }

            virtual ~StretchPatternRange() {}
            
            void update(IScriptContext* ctx){
                if (m_pattern) {
                    m_high = m_pattern->getCount()-1;
                }
            }

            /* extends pattern to full length of strip */
            double getValue(double position) {
                int count = m_pattern ? m_pattern->getCount() : 0;
                if (count < 2) { return 0;}
                m_logger->never("getValue");
                double val = AnimationRange::getValue(position);
                m_logger->never("animate stretch %.2f/%.2f=%.2f",val,count-1,(val/(count-1)));
                return val/(count-1);
                // int index = (val/(count-1))*count; // stretch
                // m_logger->never("PatternRange %.2f-%.2f %.2f=>%.2f",getMinValue(),getMaxValue(),position,val);
                // return index;
            }            
    };

    class RepeatPatternRange : public PatternRange {
        public:
            RepeatPatternRange(PatternValue* pattern,bool alternate) : PatternRange(pattern,false) {
                // cannot unfold like other animations.  need to alternate drawing the pattern forward then backward
                m_alternate = alternate;  
            }
            virtual ~RepeatPatternRange() {}

            /* 1-to-1 mapping from strip LEDs to value */
            void update(IScriptContext* ctx){
               // m_high = (ctx->getStrip()->getLength())-1; 
               IAnimationDomain* domain = ctx->getAnimationPositionDomain();
               int length = domain->getMax()-domain->getMin()+1;
               m_high = length;
            }

            /* repeat pattern */
            double getValue(double position) {
                int count = m_pattern ? m_pattern->getCount() : 0;
                if (count < 2) { return 0;}

                double val = AnimationRange::getValue(position);
                double index = ((int)round(val)) % (count);
                if (m_alternate) {
                    // mod the countx2.  first half runs pattern forward.  2nd have goes backward;
                    index = ((int)round(val)) % ((count-1)*2);
                    m_logger->never("alternate index=%f  val=%f count=%d",index,val,count);
                    if (index >= (count-1)) { 
                        index = (count-1)*2-index;
                    }
                    m_logger->never("\tindex=%f",index);
                } else {
                    m_logger->never("\tno alternate %f %f %d",index, val, count);
                }
                m_logger->never("\tresult=%f",index/count);
                return index/(count);
                
                // m_logger->never("RepeatPatternRange count=%d pos=%f val=%f index=%d",count,position,val,index);
                // return index;
            }

        protected:
            bool m_alternate;
    };

    class ScriptVariableValue : public IScriptValue
    {
    public:
        ScriptVariableValue(bool isSysValue, const char *value,IScriptValue* defaultValue) 
        {
            SET_LOGGER(ScriptValueLogger);
            m_name = Util::allocText(value);
            m_logger->debug("Created ScriptVariableValue %s %d.", value, isSysValue);
            m_defaultValue = defaultValue;
            m_isSysValue = isSysValue;
            m_recurse = false;
        }

        ScriptVariableValue(const ScriptVariableValue* other){
            m_name = Util::allocText(other->m_name);
            m_defaultValue = other->m_defaultValue ? other->m_defaultValue->clone() : NULL;
            m_isSysValue = other->m_isSysValue;
            m_recurse = false;
        }

        virtual ~ScriptVariableValue()
        {
            Util::freeText(m_name);
            if (m_defaultValue) { m_defaultValue->destroy();}
        }


        void destroy() override { delete this;}

        virtual int getIntValue(IScriptContext*ctx,  int defaultValue)  override
        {
            return getFloatValue(ctx,defaultValue);
        }

        virtual double getFloatValue(IScriptContext*ctx,  double defaultValue)  override
        {
            if (m_recurse) {
                m_logger->never("variable getFloatValue() recurse");
                return m_defaultValue ? m_defaultValue->getFloatValue(ctx,defaultValue) : defaultValue;
            }
            if (m_isSysValue && Util::equal("offset",m_name)){
                IElementPosition*pos = ctx->getPosition();
                if (pos && pos->hasLength()) {
                    return pos->getOffset().getValue();
                } else {
                    return 0;
                }
            }
            if (m_isSysValue && Util::equal("length",m_name)){
                IElementPosition*pos = ctx->getPosition();
                if (pos && pos->hasLength()) {
                    return pos->getLength().getValue();
                } else {
                    return ctx->getStrip()->getLength();
                }
            }
            if (m_isSysValue && Util::equal("led",m_name)){
                return ctx->getAnimationPositionDomain()->getValue();
            }
            if (m_isSysValue && Util::equal("step",m_name)){
                int step =  ctx->getStep()->getNumber();
                m_logger->never("sys(step)=%d  %x",step,ctx);
                return step;
            }
            m_recurse = true;

            IScriptValue * val = getScriptValue(ctx);
            double result = val ? val->getFloatValue(ctx,defaultValue) : defaultValue;
            m_recurse = false;
            return result;
        }

        virtual bool getBoolValue(IScriptContext*ctx,  bool defaultValue) override
        {
            if (m_recurse) {
                m_logger->never("variable getFloatValue() recurse");
                return m_defaultValue ? m_defaultValue->getBoolValue(ctx,defaultValue) : defaultValue;
            }
            m_recurse = true;
            IScriptValue * val = getScriptValue(ctx);
            bool result =  val->getBoolValue(ctx,defaultValue);
           
            m_recurse = false;
            return result;
        }


        ScriptStatus getStatus(IScriptContext*ctx,  ScriptStatus defaultValue) const override {
            return defaultValue;
        }

        bool equals(IScriptContext*ctx, const char * match) const override {
            IScriptValue * val = getScriptValue(ctx);
            return val ? val->equals(ctx,match) : false;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            IScriptValue * val = getScriptValue(ctx);

            return val ? val->getMsecValue(ctx,defaultValue) : defaultValue;
        }

        UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override { 
            IScriptValue * val = getScriptValue(ctx);

            return val ? val->getUnitValue(ctx,defaultValue,defaultUnit) : UnitValue(defaultValue,defaultUnit);
        }

        bool isNumber(IScriptContext* ctx) const { 
            IScriptValue * val = getScriptValue(ctx);
            return val ? val->isNumber(ctx) : false;

         }
        bool isString(IScriptContext* ctx) const { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isString(ctx);

         }
        bool isBool(IScriptContext* ctx) const { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isBool(ctx);
         }
         bool isNull(IScriptContext* ctx) const { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isNull(ctx);
         }

        bool isUnitValue(IScriptContext* ctx) const override { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isUnitValue(ctx);
         }

         bool isTimer(IScriptContext* ctx) const override { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isTimer(ctx);
         }

         

        IScriptValue* eval(IScriptContext * ctx, double defaultValue) override{
            return new ScriptNumberValue(getFloatValue(ctx,defaultValue));

        }

        IJsonElement* toJson(JsonRoot* jsonRoot) override {
            DRFormattedString val("%s(%s)",(m_isSysValue?"sys":"var"),m_name);
            if (m_defaultValue) {
                DRString text = m_defaultValue->stringify();
                if (text.getLength()>0) {
                    val.append("|");
                    val.append(text);
                }
                
            }
            return new JsonString(jsonRoot,val.text());
        }
        
        virtual DRString toString() { return DRString("Variable: ").append(m_name); }

        bool isRecursing() const { return m_recurse;}

        DRString stringify() { return "";} // cannot stringify vars
        IScriptValue* clone()const override {
            m_logger->error(LM("AnimatedValue.clone() not implemented"));
            return new ScriptVariableValue(this);
        }
    protected:
        IScriptValue* getScriptValue(IScriptContext*context) const {
            IScriptValue* val = m_isSysValue ? context->getSysValue(m_name) : context->getValue(m_name);
            if (val == NULL)  {
                val = m_defaultValue;
            }

            return val ? val : &NULL_VALUE;
        }
        const char * m_name;
        bool m_isSysValue;
        IScriptValue*  m_defaultValue;
        DECLARE_LOGGER();
        bool m_recurse;

        static ScriptNullValue NULL_VALUE;
    };

    ScriptNullValue ScriptVariableValue::NULL_VALUE;

    class ScriptValueList : public IScriptValueProvider {
        public:
            ScriptValueList() {
                SET_LOGGER(ScriptValueLogger);
                m_logger->never("create ScriptValueList()");
            }



            virtual ~ScriptValueList() {
                m_logger->debug("delete ~ScriptValueList()");
                clear();
                m_logger->debug("cleared");
            }

            bool hasValue(const char *name) override  {
                return getValue(name) != NULL;
            }
            
            IScriptValue *getValue(const char *name)  override {
                m_logger->never("getValue %s",name);
                NameValue** first = m_values.first([&](NameValue*&nv) {

                    return strcmp(nv->getName(),name)==0;
                });
                if (first) {
                    IScriptValue* found = (*first)->getValue();
                    if (found) {
                        m_logger->never("\tfound %s",found->toString().text());
                        return found;
                    }
                }

                m_logger->never("\tnot found");
                return NULL;
            }

            void setValue(const char * name,IScriptValue * value) {
                if (Util::isEmpty(name) || value == NULL) {
                    return;
                }
                NameValue** find = m_values.first([&](NameValue*&nv) {
                    return strcmp(nv->getName(),name)==0;
                });
                if (find) {
                    (*find)->replaceValue(value);
                    return;
                }
                m_logger->never("add NameValue %s  0x%04X",name,value);
                NameValue* nv = new NameValue(name,value);
                m_values.add(nv);
            }

            void each(auto&& lambda) const {
                m_values.each(lambda);
            }
 
            void destroy() { delete this;}

            int count() { return m_values.size();}

            void initialize(ScriptValueList* source,IScriptContext*ctx) override {
                m_logger->never("initialize ScriptValueList from source %x",source);
                m_values.clear();
                if(source == NULL) { return;}
                source->each([&](NameValue* nv) {
                    IScriptValue* val = nv->getValue();
                    if (val != NULL) {
                        IScriptValue* newVal = val->eval(ctx);
                        NameValue*newNV = new NameValue(nv->getName(),newVal);
                        m_values.add(newNV);
                    }
                });
            }

            void clear() { m_values.clear();}
        private:
            PtrList<NameValue*> m_values;
            DECLARE_LOGGER();
   };

   IScriptValue* ScriptValue::eval(IScriptContext * ctx, double defaultValue) {
        return new ScriptNumberValue(getFloatValue(ctx,defaultValue));
   }

   IScriptValue* ScriptValue::create(IJsonElement*json,JsonObject* parent){
       if (json == NULL) { 
           return NULL;
       }
       IScriptValue * result = NULL;
       if (json->asValue()) {
           IJsonValueElement* jsonVal = json->asValue();
            if (json->isNumber()) {
                result = new ScriptNumberValue(jsonVal->getFloat(0));
            } else if (json->isString()) {
                //return new ScriptStringValue(jsonVal->getString(""));
                result = createFromString(jsonVal->getString(""));
            } else if (json->isBool()) {
                result = new ScriptBoolValue(jsonVal->getBool(false));
            } else if (json->isNull()) {
                result = new ScriptNullValue();
            }
       } else if (json->asArray()) {
           result = createFromArray(json->asArray(),parent);
       } else if (json->asObject()) {
           result = createFromObject(json->asObject());
       }
       return result;
   }

   IScriptValue* ScriptValue::createFromString(const char * string){
       while(string != NULL && isspace(*string)) {
           string++;
       }
       if (string == NULL || string[0] == 0) { return NULL;}
       IScriptValue* result = NULL;
       if (Util::startsWith(string,"var(") || Util::startsWith(string,"sys(")){
           LinkedList<DRString> nameDefault;
           bool isSysVar = string[0] == 's';
           Util::split(string+4,')',nameDefault);
           if (nameDefault.size() == 1) {
               result = new ScriptVariableValue(isSysVar, nameDefault.get(0).text(),NULL);
           } else if (nameDefault.size() == 2) {
               const char * defString = nameDefault.get(1).text();
               while (defString != NULL && defString[0] != 0 && defString[0] == '|'){
                   defString++;
               }

                result = new ScriptVariableValue(isSysVar, nameDefault.get(0),createFromString(defString));
           } else {
           }
       } else if (Util::equal(string,"null")) { result = new ScriptNullValue();}
       else if (Util::equal(string,"true")) { result = new ScriptBoolValue(true);}
       else if (Util::equal(string,"false")) { result = new ScriptBoolValue(false);}
       else { result = new ScriptStringValue(string);}
       return result;
   }

   IScriptValue* ScriptValue::createFromArray(JsonArray* json,JsonObject* parent){
        
       if (json == NULL || json->getCount() == 0) { return NULL;}
       int count = json->getCount();
       
       IJsonElement* first = json->getAt(0);
       
       if (first == NULL || first->asValue() == NULL) { return new ScriptNullValue();}
       // if the first element is a string, that is the function name.  otherwise this is a range;
       IJsonValueElement* val = first->asValue();
       const char * name = val->getString(NULL);
        bool isFunc = ScriptFunction::isFunctionName(name);
        if (isFunc) {
            ScriptFunction* func = new ScriptFunction(name);
            for(int i=1;i<count;i++){
                func->addArg(create(json->getAt(i),parent));
            }
            return func;
       } else {
            bool unfold = parent ? parent->getBool("unfold",false) : false;
            bool repeat = parent ? parent->getBool("repeat",false) : false;
            bool smooth = parent ? parent->getBool("smooth",true) : true;
           IScriptValue* range = createPattern(json,smooth,repeat,unfold,parent);
           return range;
       }
   }

   IScriptValue* ScriptValue::createFromObject(JsonObject* json){
       if (json == NULL) { return NULL;}
        IScriptValue* result = NULL;
        JsonArray* pattern = json->getArray("pattern");
        JsonArray* range = json->getArray("range");
        bool unfoldDefault = false;
        bool repeatDefault = true;
        bool smoothDefault = false;
        if (pattern == NULL && range != NULL) {
            pattern = range;
            repeatDefault = false;
            smoothDefault = true;
        }
        if (pattern) {
            bool unfold = json->getBool("unfold",unfoldDefault);
            bool repeat = json->getBool("repeat",repeatDefault);
            bool smooth = json->getBool("smooth",smoothDefault);
            IScriptValue* patternValue = createPattern(pattern,smooth,repeat,unfold,json);
            result = patternValue;
        } else {

        }
        return result;
   }

    IScriptValue* ScriptValue::createPattern(JsonArray* pattern, bool smooth, bool repeat, bool unfold, JsonObject* json){
        
            PatternValue* patternValue = new PatternValue();
            PositionUnit unit = POS_UNSET;
            pattern->each([&](IJsonElement*elemenValue){
                auto patternElement = createPatternElement(elemenValue);
                if (unit == POS_UNSET) {
                    unit = patternElement->getUnit();
                }
                patternValue->addElement(patternElement);
            });
            IAnimationEase* ease = json ? createEase(json) : NULL;

            PatternInterpolation* pi = NULL;
            if (smooth){
                pi = new SmoothInterpolation();
             } else {
                 pi = new StepInterpolation();
             }
            patternValue->setInterpolation(pi);
            IAnimationRange* range = NULL;
            bool isTime = (json && (json->getPropertyValue("duration") || json->getPropertyValue("speed")));
            if (repeat &&  !isTime) {
                range = new RepeatPatternRange(patternValue,unfold);
            } else {
                range = new StretchPatternRange(patternValue,unfold);
            }
            range->setUnit(unit);
            IAnimationDomain* domain = json ? createDomain(json,range) : new ContextPositionDomain();
            IValueAnimator* animator = new Animator(domain,range,ease);
            patternValue->setAnimator(animator);
            return patternValue;
    }

    IAnimationEase* ScriptValue::createEase(JsonObject* json){
        IAnimationEase* ease = NULL;
        double easeValue =1;
        IJsonElement* easeJson = json->getPropertyValue("ease");
        IJsonValueElement* easeVal = easeJson ? easeJson->asValue() : NULL;
        IJsonElement* easeIn = json->getPropertyValue("ease-in");
        IJsonValueElement* easeInVal = easeIn ? easeIn->asValue() : NULL;
        IJsonElement* easeOut = json->getPropertyValue("ease-out");
        IJsonValueElement* easeOutVal = easeOut ? easeOut->asValue() : NULL;
        if (easeVal) {
            if (Util::equal(easeVal->getString(),"linear")){
                return new LinearEase();
            }
        }
        IScriptValue* inValue=NULL;
        IScriptValue* outValue=NULL;
        if (easeInVal != NULL) {
            inValue = ScriptValue::create(easeIn);
        } else {
            inValue = ScriptValue::create(easeJson);
        }
        if (easeOutVal != NULL) {
            outValue = ScriptValue::create(easeOut);
        } else {
            outValue = ScriptValue::create(easeJson);
        }
        if (inValue == NULL && outValue == NULL) {
            return NULL;
        }
        return new CubicBezierEase(inValue,outValue);
        
    }

    IAnimationDomain* ScriptValue::createDomain(JsonObject* json, IAnimationRange* range){
        IAnimationDomain* domain = NULL;
        IJsonElement* speedJson = json->getPropertyValue("speed");
        IJsonElement* durationJson = json->getPropertyValue("duration");
        IJsonElement* repeatJson = json->getPropertyValue("repeat");
        IJsonElement* delayJson = json->getPropertyValue("delay");
        if (speedJson) {
            IScriptValue* speedValue = create(speedJson);
            if (speedValue) {
                SpeedDomain* speedDomain = new SpeedDomain(speedValue,range);
                domain = speedDomain;
                speedDomain->setRepeat(create(repeatJson));
                speedDomain->setDelay(create(repeatJson));

            }
        } else if (durationJson) {
            IScriptValue* durationValue = create(durationJson);
            if (durationJson) {
                DurationDomain* durationDomain = new DurationDomain(durationValue);
                durationDomain->setRepeat(create(repeatJson));
                durationDomain->setDelay(create(delayJson));
                domain = durationDomain;
            }
        } else {
            domain = new ContextPositionDomain();
        }
        
        return domain;
    }




   ScriptPatternElement* ScriptValue::createPatternElement(IJsonElement*json){
        if (json == NULL) { return NULL;}
        ScriptPatternElement* element = NULL;
        IScriptValue * val=NULL;
        IScriptValue* count = NULL;
        PositionUnit unit = POS_UNSET;
        if (json->isObject()) {
            JsonObject* obj = json->asObject();
            if (obj->getPropertyValue("value")){
                unit = stringToUnit(obj->getString("unit",NULL),POS_PIXEL);
                count = create(obj->getPropertyValue("count"));
                val = create(obj->getPropertyValue("value"));
            } else {
                val = create(obj);
                count = new ScriptNumberValue(1);
            }
        } else if (json->isString()) {
            const char * str = json->asValue()->getString(NULL);
            if (str != NULL) { 
                LinkedList<DRString> valCount;
                Util::split(str,'x',valCount);
                val = createFromString(valCount.get(0));
                unit = stringToUnit(valCount.get(0));
                if (valCount.size()==2) {
                    const char * countAndUnit = valCount.get(1).text();
                    count = new ScriptStringValue(countAndUnit);
                }
            }
        } else {
            val = create(json);
        }
        if (val == NULL) {
            val = new ScriptNullValue();
        }
        element = new ScriptPatternElement(count,unit, val);
        return element;
   }



}
#endif