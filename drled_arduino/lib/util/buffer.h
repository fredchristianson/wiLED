#ifndef DR_BUFFER_H
#define DR_BUFFER_H

#include "../log/logger.h"
#define MAX_PATH 100

namespace DevRelief {


class DRBuffer {
public:
    DRBuffer(size_t length=0){
        SET_LOGGER(DRBufferLogger);
        m_data = NULL;
        m_maxLength = 0;
        m_length = length;
        reserve(m_length);
    }

    virtual ~DRBuffer() {
        if (m_data) {
            m_logger->info("delete DRBuffer data");
            if (m_data != NULL){
                free(m_data);
            }
        }
        
    }

    void clear() {
        if (m_data) {
            free(m_data);
            m_data = NULL;
            m_maxLength = 0;
            m_length = 0;
        }
    }
    const uint8_t* data() const {
        m_logger->debug("return data.  length=%d, maxLength=%d",m_length,m_maxLength);
        return m_data;
    }

    void setText(const char * text) {
        if (text == NULL) {
            reserve(1);
            m_data[0] = 0;
        }
        int len=strlen(text)+1;
        strcpy((char*)reserve(len),text);
        m_length = len;
        
    };

    const char * text() {
        reserve(1); // make sure there's room for an empty string
        m_data[m_length] = 0;
        return (const char *) m_data;
    }

    uint8_t* reserve(size_t length) {
        if (length > m_maxLength) {
            if (length < 128) {
                length = 128; // don't allocate small chunks
            }
            m_logger->info("increase buffer length. old length=%d, new length=%d",m_maxLength,length);
            uint8_t* newData = (uint8_t*)malloc(length+1);
            m_logger->info("allocated buffer");
            if (m_data != NULL) {
                if (length > 0) {
                    memcpy(newData,m_data,length);
                }
                free(m_data);

            } else {
                memset(newData,0,length);
            }
            m_data = newData;
            m_maxLength = length;
            m_logger->info("first byte of m_data: %d",m_data[0]);
        }
        return m_data;
    }



    char * increaseLength(size_t length) {
        m_logger->info("incread length.  length=%d+=>%d, maxLength=%d",m_length,length,m_maxLength);
        size_t oldLength = m_length;
        m_length += length;
        reserve(m_length+length+1);
        m_data[m_length] = 0;
        m_logger->info("\tdone length=%d, maxLength=%d",m_length,m_maxLength);
        return (char*) m_data+oldLength;
    }

    void setLength(size_t length) {
        reserve(length+1);
        m_length = length;
        m_data[m_length]=0;
        m_logger->info("set length.  length=%d, maxLength=%d",m_length,m_maxLength);
    }

    size_t getLength() {
        m_logger->info("get length.  length=%d, maxLength=%d",m_length,m_maxLength);
        return m_length;
    }


    size_t getMaxLength() { return m_maxLength;}
protected:
    DECLARE_LOGGER();

private:
    uint8_t * m_data;
    size_t m_maxLength;
    size_t m_length;
};

class DRStringBuffer : public DRBuffer{
    public:
    DRStringBuffer() : DRBuffer() {
        m_values = NULL;
        m_count = 0;
        //m_logger->setModuleName("DRStringBuffer");
        
        //m_logger->setLevel(DEBUG_LEVEL);
        reserve(128);
    }


    const char * nextMatch(const char* data,const char* seperators){
        const char *match = data;
        while(*match != 0) {
            match++;
            const char*s=seperators;
            while(*s != 0 && *s!=*match){
                s++;
            }
            if (*s != 0) {
                return match;
            }
        }
        return NULL;
    }
    const char ** split(const char* data,const char* seperators) {
        m_count = 0;
        m_logger->never("split %s ~~ %s",data,seperators);
        const char * pos = data;
        size_t len = strlen(data)+1;
        size_t count = 1;
        const char * match;
        while((match=nextMatch(pos,seperators)) != NULL) {
            count += 1;
            pos = match+1;
        }
        m_logger->never("\tcount: %d",count);
        
        size_t alignBytes = 4-(len%4);
        char * strings = (char*)reserve(len+(count+1)*sizeof(char*)+alignBytes);
        memcpy(strings,data,len);
        strings[len] = 0;
        const char ** array = (const char**)(strings+len+alignBytes);
        const char ** result = array;
        *array = strings;
        match = strings;
        bool done = false;
        m_values = result;
        m_count = 0;
        while(!done) {
            char * end = (char *) nextMatch(match,seperators);
            if (end == NULL) {
                m_logger->never("found end");
                done = true;
            } else {
                *end = 0;
            }
            m_logger->never("\tfound: %s",match);
            *array = match;
            m_logger->never("\t\tadded pointer");
            array++;
            m_count++;
            m_logger->never("\t\tarray++");
            match = done ? NULL : end+1;
        }
        m_logger->never("\tdone");
        *array = NULL;
        return result;
    }

    size_t count() { return m_count;}
    const char * getAt(size_t idx) {
        if (idx<0 || idx>= m_count) {
            return NULL;
        }
        return m_values[idx];
    }
    private:
        size_t m_count;
        const char ** m_values;
};

}
#endif