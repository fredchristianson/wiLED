#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

#include "../log/logger.h"
#include "../json/parser.h"

namespace DevRelief
{

class DataObject {
    public:
        DataObject() {
            SET_LOGGER(DataObjectLogger);
            m_json = m_jsonRoot.getTopObject();

        }

        virtual ~DataObject() {
            
        }


        void addProperty(const char* name, IJsonElement * val){
            m_logger->debug("addProperty %x->%x",val,val->getRoot());
            m_json->set(name,val);
            m_logger->debug("\tadded property %x->%x",val,val->getRoot());
            m_logger->debug("\t property %s",JsonElement::toJsonString(val).text());
        }
        void addProperty(const char* name, int val){
            m_json->setInt(name,val);
        }

        void addProperty(const char* name, const char * val){
            m_json->setString(name,val);
        }

        void addProperty(const char* name, double val){
            m_json->setFloat(name,val);
        }

        void addProperty(const char* name, bool val){
            m_json->setBool(name,val);
        }

        

    protected:
        JsonRoot    m_jsonRoot;
        JsonObject* m_json;
        DECLARE_LOGGER();
};


} // namespace DevRelief

#endif