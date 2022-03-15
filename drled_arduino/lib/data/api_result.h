#ifndef DR_API_RESULT_DATA_H
#define DR_API_RESULT_DATA_H
#include "../log/interface.h"
#include "../json/parser.h"
#include "../json/json.h"
#include "../json/generator.h"
#include "../net/http_server.h"
#include "../util/buffer.h"
#include "./data_object.h"

namespace DevRelief {


class ApiResult : public DataObject {
    public:
        ApiResult(IJsonElement *json) {
            SET_LOGGER(ApiResultLogger);
            m_logger->debug("create JSON ApiResult 0x%04X",json);
            addProperty("code",200);
            addProperty("success",true);
            addProperty("message","success");
            addProperty("data",json);
            mimeType = "text/json";
        }

        ApiResult(bool success=true) {
            SET_LOGGER(ApiResultLogger);
            addProperty("code",success ? 200:500);
            addProperty("success",true);
            addProperty("message","success");

        }
        ApiResult(bool success, const char * msg, ...) {
            SET_LOGGER(ApiResultLogger);
            va_list args;
            va_start(args,msg);
            addProperty("success",success);
            addProperty("code",success ? 200:500);
            setMessage(msg,args);
        }
        
        void setData(IJsonElement*json) {
            addProperty("data",json);
        }
        bool toText(DRString& apiText){
            JsonGenerator gen(apiText);
            bool result = gen.generate(&m_jsonRoot);
            m_logger->debug("generated JSON: %s",apiText.text());
            return result;
        }

        int getCode(int defaultValue=500) {
            return m_json->getInt("code",defaultValue);
        }
        void setCode(int code) {
            addProperty("code",code);
            if (code >= 300) {
                addProperty("success",false);
            }
        }

        void setMessage(const char *msg,...) {
            va_list args;
            va_start(args,msg);
            setMessageArgs(msg,args);
        }

        void setMessageArgs(const char * msg, va_list args) {
            if (msg == NULL) {
                return;
            }
            int len = strlen(msg)*2+100;
            m_logger->debug("setting message len %d: %s",len,msg);
            DRBuffer message;
            message.reserve(len);
            vsnprintf((char*)message.data(),message.getMaxLength(),msg,args);
            m_logger->debug("formatted");
            m_logger->debug("add property %s",message.text());
            addProperty("message",message.text());
        }

        void setSuccess(bool success,int code=-1) {
            addProperty("success",success);
            if (code == -1) {
                code = success ? 200 : 500;
            }
            addProperty("code",code);
        }

        void send(Request* req){
            JsonObject* mem = m_json->createObject("memory");
            int heap = ESP.getFreeHeap();
            mem->setInt("stack",(int)ESP.getFreeContStack());
            mem->setInt("heap",heap);
            mem->setInt("max free block",ESP.getMaxFreeBlockSize());
            mem->setInt("fragmentation",ESP.getHeapFragmentation());
            if (m_lastHeapSize != 0) {
                mem->setInt("lastHeap",(int)m_lastHeapSize);
                mem->setInt("heapChange",(int)heap-m_lastHeapSize);
            }
            m_lastHeapSize = heap;
            DRString result = m_jsonRoot.toString();
            req->send(getCode(200),mimeType.text(),result.text());
        }
    private:
        DRString mimeType;
        static int m_lastHeapSize;
        DECLARE_LOGGER();
        
};

int ApiResult::m_lastHeapSize = 0;

};
#endif