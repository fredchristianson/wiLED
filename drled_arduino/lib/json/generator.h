#ifndef JSONGENERATOR_H
#define JSONGENERATOR_H

#include "../log/logger.h"
#include "../util/buffer.h"
#include "../util/util.h"
#include "./json.h"

namespace DevRelief {


class JsonGenerator : public IParseGen {
public:
    JsonGenerator(DRString& buffer) : m_buf(buffer) {
        m_buf = buffer;
        m_depth = 0;
        m_pos = 0;
        SET_LOGGER(JsonGeneratorLogger);
    }

    ~JsonGenerator() {
        
    }

    bool generate(IJsonElement* element) {
        m_logger->debug("Generate JSON %d",element->getType());
        m_pos = 0;
        m_depth = 0; 
        m_logger->debug("clear buffer");
        
        m_buf.clear();
        m_logger->debug("\tcleared buffer");
        if (element == NULL) {
            m_logger->error("\telement is NULL");
            return false;
        }
        if (element->getType() == JSON_ROOT){
            m_logger->debug("write top element");
            writeElement(((JsonRoot*)element)->getTopElement());
        } else {
            m_logger->debug("write self");
            writeElement((JsonElement*)element);
        }
        m_logger->debug("generated: %s",m_buf.text());
        return true;
    }

    void writeElement(IJsonElement* element){
        if (element == NULL) {
            writeText("null");
            writeNewline();
            return;
        }
        if (EspBoard.getFreeHeap() < 2000) {
            m_logger->error("out of memory generating JSON");
            return;
        }
        int type = element->getType();
        m_logger->debug("write element type %d",type);
        switch(type) {
            case JSON_UNKNOWN:
                writeText("unknown element type");
                writeInt(type);
                writeNewline();
                break;
            case JSON_NULL:
                writeText("null");
                writeInt(type);
                writeNewline();
                break;
            case JSON_INTEGER:
                writeInteger((JsonInt*)element);
                break;
            case JSON_FLOAT:
                writeFloat((JsonFloat*)element);
                break;
            case JSON_STRING:
                writeString((JsonString*)element);
                break;
            case JSON_BOOLEAN:
                writeBool((JsonBool*)element);
                break;
            case JSON_OBJECT:
                writeObject((JsonObject*)element);
                break;
            case JSON_ARRAY:
                writeArray((JsonArray*)element);
                break;
            case JSON_ROOT:
                writeElement(((JsonRoot*)element)->getTopElement());
                break;
            case JSON_PROPERTY:
                writeProperty((JsonProperty*)element);
                break;
            case JSON_ARRAY_ITEM:
                writeArrayItem((JsonArrayItem*)element);
                break;
            default:
                writeText("unknown type: ");
                writeInt(type);
                writeNewline();
                break;
        }
    }

    void writeObject(JsonObject * object) {
        writeText("{");
        m_depth += 1;
        m_logger->info("write object [%d] count=%d",object->getJsonId(),object->getCount());
        for(JsonProperty*prop=object->getFirstProperty();prop!=NULL;prop=prop->getNext()){
            if (strcmp(prop->getName(),"jsonId")!=0) {
                m_logger->info("\tprop %s [%d]",(prop == NULL ? "no prop":prop->getName()),prop->getJsonId());
                writeNewline();

                writeProperty(prop);

                if (prop->getNext() != NULL) {
                    writeText(",");
                }
            }
        }
        m_logger->debug("\twrote properties");
        m_depth -= 1;
        writeNewline();
        writeText("}");
    }

    void writeProperty(JsonProperty * prop) {
        if (prop == NULL) {
            m_logger->error("writeProperty got null");
        }
        m_logger->debug("write property %s",prop->getName());
        writeString(prop->getName());
        m_logger->debug("\twrote name %s",prop->getName());
        
        writeText(": ");
        m_logger->debug("\twrote colon");
        IJsonElement* val = prop->getValue();
        m_logger->debug("\twrite element %d",val);
        writeElement(val);
        m_logger->debug("\twrote value");
    }

    void writeArray(JsonArray* array) {
        writeText("[");
        m_depth += 1;
        for(JsonArrayItem* item=array->getFirstItem();item!= NULL;item=item->getNext()){
            writeNewline();
            writeArrayItem(item);
            if (item->getNext() != NULL) {
                writeText(",");
            }
        }
        m_depth -= 1;
        writeNewline();
        writeText("]");
    }

    void writeArrayItem(JsonArrayItem* item) {
        writeElement(item->getValue());
    }
  
    void writeInteger(JsonInt* element) {
        if (element == NULL) { writeText("null");}
        writeInt(element->getInt(0));
    }
    
    void writeFloat(JsonFloat* element) {
        if (element == NULL) { writeText("null");}
        writeFloat(element->getFloat(0));
    }
        
    void writeBool(JsonBool* element) {
        if (element == NULL) { writeText("null");}
        writeText(element->getBool(false)?"true":"false");
    }
    
    void writeString(JsonString* element) {
        if (element == NULL) { writeText("null");}
        const char * txt = element->getString(NULL);
        if (txt == NULL) {        
            writeText("null");
            return;
        }
        writeText("\"");
        const char * end = strchr(txt,'\"');
        while(end != NULL) {
            size_t len = end-txt+1;
            char* pos = m_buf.increaseLength(len+1);
            memcpy(pos,txt,len);
            pos[len] = '\\';
            pos[len+1] = '\"';
            txt = end+1;
            end = strchr(txt,'\"');
        }
        writeText(txt);
        writeText("\"");
    }
    
    void writeText(const char * text) {
        if (text == NULL) {
            return;
        }
        size_t len = strlen(text);
        if (len == 0) {
            return;
        }
        m_logger->debug("writeText %d %d ~%.15s~",len,m_buf.getLength(),text);
        char *pos = m_buf.increaseLength(len);
        memcpy(pos,text,len);
        pos[len] = 0;
        m_logger->never("\tJSON: %d %.300s", m_buf.getLength(),m_buf.text());
    }

    void writeString(const char * text) {
        writeText("\"");
        writeText(text);
        writeText("\"");
    }

    void writeInt(int i) {
        snprintf(m_tmp,32,"%d",i);
        writeText(m_tmp);
    }

    void writeFloat(double f) {
        snprintf(m_tmp,32,"%f",f);
        writeText(m_tmp);
    }

    void writeNewline() {
        writeText("\n");
        writeTabs();
    }

    void writeTabs(){
        int cnt = m_depth;
        while(cnt-- > 0) {
            writeText("\t");
        }
    }

protected:
    char m_tmp[32];
    DRString& m_buf;    
    int m_depth;
    size_t m_pos;
    DECLARE_LOGGER();
};


DRString JsonBase::toString(){
    DRString dr;
    JsonGenerator gen(dr);
    gen.generate(this);
    return dr;
}

DRString JsonElement::toJsonString(IJsonElement* element){
    if (element == NULL) {
        return NULL;
    }
    return ((JsonBase*)element)->toString();
}

}
#endif
