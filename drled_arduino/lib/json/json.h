#ifndef JSON_H
#define JSON_H

#include "../log/logger.h"
#include "../util/drstring.h"
#include "./json_interface.h"

namespace DevRelief {


class JsonArray;
class JsonObject;

class JsonBase : public IJsonElement {
    public:
        JsonBase() {
            m_jsonId = -1;
            SET_LOGGER(JsonLogger);
        }
        virtual ~JsonBase() {

        }

        void destroy() override { 
            delete this;
        }

        bool isArray() override { return false;}
        bool isObject() override { return false;}
        bool isString() override { return false;}
        bool isInt() override { return false;}
        bool isBool() override { return false;}
        bool isFloat() override { return false;}
        bool isVariable() override { return false;}
        bool isNumber() override { return false;}
        bool isNull() override { return false;}

        JsonArray* asArray() override { 
            if (this->isArray()) { return (JsonArray*)this;}
            return NULL;
        }
        JsonObject* asObject() override { 
            if (this->isObject()) { return (JsonObject*)this;}
            return NULL;
        }
        IJsonValueElement* asValue() override { return (IJsonValueElement*)this;}

        int getJsonId() { return m_jsonId;}
        void setJsonId(int id) { m_jsonId = id;}

        DRString toString();

    protected:
        int m_jsonId;
        DECLARE_LOGGER();
};

class JsonRoot : public JsonBase {
    public:
        JsonRoot() : JsonBase() {
            m_value = NULL;
            m_nextJsonId = 1;
            setJsonId(this);
        }



        virtual ~JsonRoot() {
            if (m_value && m_value->getRoot() == this){
                m_value->destroy();
            }
        }

        void setJsonId(JsonBase* element) {
            element->setJsonId(m_nextJsonId++);
        }

        char * allocString(const char * val, size_t len) {
            if (len == 0) {
                return NULL;
            }
            char * str = (char*)malloc(len+1);
            strncpy(str,val,len+1);
            str[len] = 0;
        
            return str;
        }

        void freeString(const char * val) {
            if (val != NULL) {
                free((void*)val);
            }
        }

        JsonRoot* getRoot() override { return this;}

        void detatch(IJsonElement* child) {
            if (child != m_value) {
            }
            m_value = NULL;
        }

        JsonObject* createObject();
        JsonArray* createArray();

        IJsonElement* getTopElement(){
            return m_value;
        }

        JsonObject* getTopObject();

        JsonArray* getTopArray();
        
        void setTopElement(IJsonElement*top){
            if (m_value != NULL) { m_value->destroy();}
            m_value = top;
        }

        
        JsonObject* asObject() override { 
            if (m_value && m_value->isObject()){
                return (JsonObject*)m_value;
            }
            return NULL;
        }

        JsonArray* asArray() override { 
            if (m_value && m_value->isArray()){
                return (JsonArray*)m_value;
            }
            return NULL;
        }

        JsonType getType() override { return JSON_ROOT;}
        bool hasValue() override { return false;}

        void setRoot(JsonRoot* root) override { 
            m_logger->error("JsonRoot::setRoot() should not be called");
            /* nothing to do.  should not happen;*/
        }
    protected:
        int m_nextJsonId;
        IJsonElement * m_value;
};

class JsonElement : public JsonBase {
    public:
        static DRString toJsonString(IJsonElement* element);
        JsonElement(JsonRoot* root,JsonType t) :m_root(root) {
            m_type = t;
            root->setJsonId(this);

        }
        virtual ~JsonElement() {
         }

        IJsonValueElement* asValue() override { return NULL;}
        bool hasValue() override { return asValue() != NULL;}
        JsonRoot* getRoot()  override  { return  m_root;}
        void setRoot(JsonRoot* root) override { 
            m_logger->debug("JsonElement::setRoot %d %x %x->%x",(int)this->m_type,this,m_root,root);
            m_root = root;
        }

        JsonType getType() override { return m_type;}
    protected:
        JsonRoot* m_root;
        JsonType m_type;
};


class JsonValueElement : public JsonElement, public IJsonValueElement {
    public:
        JsonValueElement(JsonRoot* root,JsonType t) : JsonElement(root,t) {}
        virtual ~JsonValueElement() {};
        IJsonValueElement* asValue() override { return this;}
        
        bool getBool(bool defaultValue) override { return defaultValue;}
        int getInt(int defaultValue) override { 
            return defaultValue;
        }
        const char * getString(const char *defaultValue=NULL) override { return defaultValue;}
        double getFloat(double defaultValue) override { return defaultValue;}


};

class JsonInt : public JsonValueElement {
    public:
        JsonInt(JsonRoot* root, int value) : JsonValueElement(root,JSON_INTEGER) {

            m_value = value;
        
        }

        virtual ~JsonInt() { 
            
        }

        bool isInt() override { return true;}
        bool isNumber() override { return true;}

        const char * getString(const char *defaultValue=NULL) override {
            return defaultValue;
        }

        int getInt(int defaultValue) override { 
            return m_value;
        }
        double getFloat(double defaultValue) override { return m_value;}

    protected:
        int m_value;
};


class JsonFloat : public  JsonValueElement {
    public:
        JsonFloat(JsonRoot* root, double value) : JsonValueElement(root,JSON_FLOAT) {
            m_value = value;
        
        }

        virtual ~JsonFloat() { 
        }
  
        bool isFloat() override { return true;}
        bool isNumber() override { return true;}

        int getInt(int defaultValue) override { return m_value;}
        double getFloat(double defaultValue) override { return m_value;}
        const char * getString(const char *defaultValue=NULL) override {
            return defaultValue;
        }

        
     protected:
        double m_value;
};

class JsonBool : public  JsonValueElement {
    public:
        JsonBool(JsonRoot* root, bool value) : JsonValueElement(root,JSON_BOOLEAN) {

            m_value = value;
        
        }

        virtual ~JsonBool() { 
        }

        virtual bool isBool() { return true;}
        bool getBool(bool defaultValue) override { return m_value;}

    protected:
        bool m_value;
};


class JsonString : public JsonValueElement {
    public:
        JsonString(JsonRoot* root, const char * value) : JsonValueElement(root,JSON_STRING) {
            size_t len = value == NULL ? 0 : strlen(value);
            m_value = root->allocString(value,len);
        }
        JsonString(JsonRoot* root, const char * value, size_t len) : JsonValueElement(root,JSON_STRING) {
            m_value = root->allocString(value,len);
        }

        virtual ~JsonString() { 
            getRoot()->freeString(m_value);
        }


        bool isString() override { return true;}
        bool isNumber() override { 
            if (m_value == NULL || !isdigit(m_value[0])) { return false;}
            const char * p = m_value;
            while(p != NULL && p[0] != 0 && (p[0] == '.' || isdigit(p[0]))) {
                p++;
            }
            return p[0] == 0;
        }

        int getInt(int defaultValue) override { 
            if (isNumber()) { return atoi(m_value);}
            return defaultValue;
        }

        double getFloat(double defaultValue) override { 
            if (isNumber()) { return atoi(m_value);}
            return defaultValue;
        }

        const char * getString(const char * defaultValue) { return m_value;}


    protected:
        char* m_value;
};



class JsonNull : public JsonValueElement {
    public:
        JsonNull(JsonRoot* root) : JsonValueElement(root,JSON_NULL){

        }
        bool isNull() override { return true;}

};



class JsonProperty : public JsonElement {
    public:
        JsonProperty(JsonRoot* root, const char * name,size_t nameLength,IJsonElement* value) : JsonElement(root,JSON_PROPERTY) {
            m_name = root->allocString(name,nameLength);
            m_value = value;
            m_next = NULL;
        }
        
        JsonProperty(JsonRoot* root, const char * name,IJsonElement* value) : JsonElement(root,JSON_PROPERTY) {
            size_t nameLength = strlen(name)+1;
            m_name = root->allocString(name,nameLength);
            m_value = value;
            m_next = NULL;

        }
        virtual ~JsonProperty() { 
           if (m_value && m_value->getRoot() == getRoot()){
                m_root->freeString(m_name); 
                m_value->destroy();
            } 
        }

        


        IJsonValueElement* asValue() { return m_value->asValue();}
        JsonProperty* getNext() { return m_next;}
        IJsonElement * getValue() { return m_value;}
        const char * getName() { return m_name;}
        
        int getCount() { return m_next == NULL ? 1 : 1+m_next->getCount();}
        IJsonElement* getAt(size_t idx) {
            return idx==0 ? m_value : m_next == NULL ? NULL :m_next->getAt(idx-1);
        }

        void setNext(JsonProperty*  next) {
            if (m_next != NULL) {
                m_next->destroy();
            }
            m_next = next;
        }

        void addEnd(JsonProperty* next) {
            if (m_next == NULL) {
                m_next = next;
            } else {
                m_next->addEnd(next);
            }
        }

        void setValue(IJsonElement*val) {
            if (m_value) { m_value->destroy();}
            m_value = val;
        }

        // JsonProperty is reponsible for deleting the value.
        // use this to keep the value after the property is deleted
        void forgetValue() {
            m_value = NULL;
        }

        bool getBool(bool defaultValue) { 
            return hasValue() ? asValue()->getBool(defaultValue) : defaultValue;}
        int getInt(int defaultValue) { 
            if (hasValue()) {
                IJsonValueElement* ve = asValue();
                int val = ve->getInt(defaultValue);
                return val;
            }

            return hasValue() ? asValue()->getInt(defaultValue) : defaultValue;}
        const char * getString(const char *defaultValue=NULL) { 
            return hasValue() ? asValue()->getString(defaultValue) : defaultValue;}
        double getFloat(double defaultValue) { 
            return hasValue() ? asValue()->getFloat(defaultValue) : defaultValue;}

        void setRoot(JsonRoot* root) override { 
            m_logger->debug("JsonProperty::setRoot %d %x %x->%x",(int)this->m_type,this,m_root,root);

            JsonElement::setRoot(root);
            if (m_value) {
                m_value->setRoot(root);
            }
            if (m_next) {
                m_next->setRoot(root);
            }
        }

    private:
        char* m_name;
        IJsonElement* m_value;
        JsonProperty* m_next;
};

class JsonObject : public JsonElement {
    public:
        JsonObject(JsonRoot* root) : JsonElement(root,JSON_OBJECT) {
            m_firstProperty = NULL;
            setInt("jsonId",getJsonId());
        }
        virtual ~JsonObject() {
            JsonProperty* prop = m_firstProperty;
            while (prop != NULL) {
                JsonProperty*next = prop->getNext();
                if (prop->getRoot() == m_root) {
                    prop->destroy();
                } else {
                    m_logger->error("property has different root");
                }
                prop = next;
            }
       }
        
        bool isObject() override  { return true;}
        JsonObject* asObject() override { return this;}

        void setRoot(JsonRoot* root) override { 
            m_logger->debug("JsonObject::setRoot %d %x %x->%x",(int)this->m_type,this,m_root,root);

            JsonElement::setRoot(root);
            if (m_firstProperty) {
                m_firstProperty->setRoot(root);
            }
        }

        void eachProperty(auto lambda) {
            JsonProperty* prop = m_firstProperty;
            while(prop != NULL) {
                lambda(prop->getName(),prop->getValue());
                prop = prop->getNext();
            }
        }

        JsonProperty* add(JsonProperty* prop){
            if (m_firstProperty == NULL) {
                m_firstProperty = prop;
            } else {
                JsonProperty * end = m_firstProperty;
                while(end->getNext() != NULL) {
                    end = end->getNext();
                }
                end->setNext(prop);
             
            }
            return prop;
        }

        JsonProperty* set(const char *nameStart,size_t nameLen,JsonElement * value){
            value->setRoot(m_root);
            JsonProperty* prop = new JsonProperty(m_root,nameStart,nameLen,value);
            add(prop);
            return prop;
        }

        JsonObject* createObject(const char * propertyName) {
            JsonObject* obj = new JsonObject(getRoot());
            set(propertyName,obj);
            return obj;
        }

        JsonArray* createArray(const char * propertyName);
        
        JsonProperty* set(const char *name,IJsonElement * value){
            if (value->getRoot() != m_root) {
                value->setRoot(m_root);
            }
            JsonProperty*prop = getProperty(name);
            if (prop != NULL) {
                prop->setValue(value);
            } else {

                prop = new JsonProperty(m_root,name,value);
                add(prop);
              
            }
            return prop;
        }

        JsonProperty* setBool(const char *name,bool value) {
            JsonBool * pval = new JsonBool(getRoot(),value);
            return set(name,pval);
        }
        JsonProperty* setInt(const char *name,int value) {
            JsonInt * pval = new JsonInt(getRoot(),value);
            return set(name,pval);
        }
        JsonProperty* setString(const char *name,const char *value) {
            JsonString * pval = new JsonString(getRoot(),value);
            return set(name,pval);
        }
        JsonProperty* setFloat(const char *name,double value) {
            JsonFloat * pval = new JsonFloat(getRoot(),value);
            return set(name,pval);
        }




        bool getBool(const char *name,bool defaultValue) {
            JsonProperty* prop = getProperty(name);
            return prop ? prop->getBool(defaultValue) : defaultValue;
        }
        int getInt(const char *name,int defaultValue){
            JsonProperty* prop = getProperty(name);
            return prop ? prop->getInt(defaultValue) : defaultValue;
        }
        const char * getString(const char *name,const char *defaultValue){
            JsonProperty* prop = getProperty(name);
            return prop ? prop->getString(defaultValue) : defaultValue;
        }
        double getFloat(const char *name,double defaultValue){
            JsonProperty* prop = getProperty(name);
            return prop ? prop->getInt(defaultValue) : defaultValue;
        }

        JsonArray* getArray(const char * name){
            JsonProperty* prop = getProperty(name);
            return prop && prop->getValue() ? prop->getValue()->asArray() : NULL;
        }
        JsonObject* getChild(const char * name){
            JsonProperty* prop = getProperty(name);
            return prop && prop->getValue() ? prop->getValue()->asObject() : NULL;
        }

        JsonProperty * getProperty(const char * name) {
            for(JsonProperty*prop=m_firstProperty;prop!=NULL;prop=prop->getNext()){
                if (strcmp(prop->getName(),name)==0) {
                    return prop;
                }
            }
            return NULL;
        }
        JsonProperty* getFirstProperty() { return m_firstProperty;}

        bool hasProperty(const char * name) { return getProperty(name);}
        IJsonElement * getPropertyValue(const char * name) {
            JsonProperty*e = m_firstProperty;
            while(e != NULL && strcmp(e->getName(),name) != 0) {
                e = e->getNext();
            }
            return e == NULL ? NULL : e->getValue();
        }

        int getCount() { return m_firstProperty == NULL ? 0 : m_firstProperty->getCount();}
        IJsonElement* getAt(size_t idx) {
            return m_firstProperty == NULL ? NULL : m_firstProperty->getAt(idx);
        }

        void clear(){
            if (m_firstProperty) { 
                m_firstProperty->destroy();
            }
            m_firstProperty=NULL;
        }
    protected:
        JsonProperty* m_firstProperty;
};



class JsonArrayItem : public JsonElement {
    public:
        JsonArrayItem(JsonRoot* root, IJsonElement * value) :JsonElement(root,JSON_ARRAY_ITEM) {

            m_value = value;
            m_next = NULL;

        }

        virtual ~JsonArrayItem() {
            if (m_value && m_value->getRoot() == getRoot()){
                m_value->destroy();
            }

        }


        int getCount() { return m_next == NULL ? 1 : 1+m_next->getCount();}
        
        IJsonElement* getAt(size_t idx) {
            return idx==0 ? m_value : m_next == NULL ? NULL :m_next->getAt(idx-1);
        }

        JsonArrayItem* getNext() { return m_next;}
        IJsonElement* getValue() { return m_value;}

        void add(JsonArrayItem *last) {
            if (m_next != NULL) {
                m_next->add(last);
            } else {
                m_next = last;
            }
        }


        IJsonElement* getAt(int index) {
            if (index == 0) { return m_value;}
            return m_next->getAt(index-1);
        }

        void setRoot(JsonRoot* root) override { 
            m_logger->debug("JsonArrayItem::setRoot %d %x %x->%x",(int)this->m_type,this,m_root,root);

            JsonElement::setRoot(root);
            if (m_value) {
                m_value->setRoot(root);
            }
            if (m_next) {
                m_next->setRoot(root);
            }
        }

        
    protected:
        IJsonElement* m_value;
        JsonArrayItem* m_next;
};

class JsonArray : public JsonElement {
    public:
        JsonArray(JsonRoot* root) : JsonElement(root,JSON_ARRAY) {
            m_firstItem = NULL;
        
        }
        virtual ~JsonArray() {
            JsonArrayItem* item = m_firstItem;
            while (item != NULL) {
                JsonArrayItem*next = item->getNext();
                if (item->getRoot() == m_root) {
                    item->destroy();
                }
                item = next;
            }
        
        }

        virtual bool isArray() { return true;}
        virtual JsonArray* asArray() { return this;}

        void setRoot(JsonRoot* root) override { 
            m_logger->debug("JsonArray::setRoot %d %x %x->%x",(int)this->m_type,this,m_root,root);

            JsonElement::setRoot(root);
            if (m_firstItem) {
                m_firstItem->setRoot(root);
            }
        }
        JsonArrayItem* addItem(IJsonElement * value){
            value->setRoot(m_root);
            JsonArrayItem* item = new JsonArrayItem(m_root,value);
            if (m_firstItem == NULL) {
                m_firstItem = item;
            } else {
                m_firstItem->add(item);
            }
            return item;
        }

        JsonObject* addNewObject() {
            JsonObject* obj = new JsonObject(getRoot());
            addItem(obj);
            return obj;
        }

        JsonArrayItem* addString(const char * val){
            return addItem(new JsonString(getRoot(),val));
        }
        JsonArrayItem* addInt(int val){
            return addItem(new JsonInt(getRoot(),val));
        }
        JsonArrayItem* addFloat(double val){
            return addItem(new JsonFloat(getRoot(),val));
        }
        JsonArrayItem* addBool(bool val){
            return addItem(new JsonBool(getRoot(),val));
        }

        int getCount() { return m_firstItem == NULL ? 0 : m_firstItem->getCount();}
        IJsonElement* getAt(size_t idx) {
            return m_firstItem == NULL ? NULL : m_firstItem->getAt(idx);
        }
        JsonArrayItem* getFirstItem() {
            return m_firstItem;
        }

        void each(auto&& lambda) const {
            JsonArrayItem*item = m_firstItem;
            while(item != NULL) {
                IJsonElement* value = item->getValue();
                lambda(value);
                item = item->getNext();
            }
        }

        IJsonElement* getAt(int index) {
            return m_firstItem ? m_firstItem->getAt(index):NULL;
        }

    protected: 
        JsonArrayItem * m_firstItem;
    };

        JsonObject* JsonRoot::createObject(){
            return new JsonObject(this);
        }
        JsonArray* JsonRoot::createArray() {
            return new JsonArray(this);
        }
        JsonObject* JsonRoot::getTopObject() {
            if (m_value == NULL) {
                m_value = createObject();
            }
            return m_value->asObject();
        }

        JsonArray* JsonRoot::getTopArray() {
            if (m_value == NULL) {
                m_value = createArray();
            }
            return m_value->asArray();
        }
        
        JsonArray* JsonObject::createArray(const char * propertyName){
            JsonArray* array = new JsonArray(getRoot());
            set(propertyName,array);
            return array;
        }

}
#endif
