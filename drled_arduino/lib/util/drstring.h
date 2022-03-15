#ifndef DR_STRING_H
#define DR_STRING_H

#include "../log/interface.h"
#include "../util/shared_ptr.h"

namespace DevRelief {



class DRStringData {
    public:
        DRStringData(size_t length) : m_length(0), m_maxLength(0), m_data(NULL) {
            
            ensureLength(length);
            m_length = length;
        }

        virtual ~DRStringData() {
            delete m_data;
        }

        virtual void destroy() { delete this;}

        const char * get() const {
            ((DRStringData*)this)->ensureLength(1);
            return m_data;
        }

        size_t updateLength() {
            if (m_data == NULL) {
                m_length = 0;
            } else {
                m_length = strlen(m_data);
            }
            return m_length;
        }
        char * ensureLength(size_t minLength){
            
            if (m_maxLength >= minLength) {
                
                return m_data;
            }

            size_t newLength = minLength+1;
            
            if (newLength > EspBoard.getMaxFreeBlockSize()) {
                // cannot use DRLogger because of circular dependencies for now.
                #ifdef LOGGING_ON
                    Serial.printf("out of memory %d > %d\n\n",newLength, EspBoard.getMaxFreeBlockSize());
                #endif
            }
            char * newData = (char*) malloc(sizeof(char)*newLength);
            

            memset(newData,0,newLength);
            if (m_data != NULL){
                if (m_length > 0) {
                    
                    memcpy(newData,m_data,m_length);
                }
                
                free(m_data);
            }
            m_data = newData;
            m_maxLength = newLength-1;
            
            return m_data;
        }

        char * setLength(size_t newLength) {
            ensureLength(newLength);
            
            m_length = newLength;
            if (m_data != NULL) {
                
                m_data[newLength] = 0;
            }
            return m_data;
        }

        char * increaseLength(size_t additional) {
            // return pointer to new data
            
            size_t actualLength = (m_data == NULL) ? 0 : strlen(m_data);
            size_t origLength = m_length;
            if (actualLength+additional <= m_maxLength) {
                // have enough space;
                m_length=actualLength + additional;
                
                
                return m_data+actualLength;

            }
            size_t addLength = additional;
            if (addLength < 32) {
                addLength = 32;
            } 
            if (addLength < m_length/2) {
                addLength = m_length/2;
            }
            
            ensureLength(m_length+addLength);
            m_length = actualLength+additional;
            
            
            return m_data+actualLength;

        }

        char * data() { 
            
            return m_data;
        }
        const char * data() const { return m_data;}
        size_t getLength() const { return m_length;}
        size_t getMaxLength() const { return m_maxLength;}
    private:
        size_t m_length;
        size_t m_maxLength;
        char * m_data;
};

class DRString : public Printable {
    public: 
        static DRString fromFloat(double val);
        DRString(const char * = NULL);
        DRString(const char *, size_t len);
        DRString(char c, size_t repeatCount);
        DRString(const DRString& other);
        ~DRString();

        size_t printTo(Print& p) const {
            return p.print(get());
        }

        const char* operator->() const { 
            
            return m_data.get()->data();
        }

        operator const char*() const { 
            
            return m_data.get()->data();
        }

        const char * get() const { 
            
            if (m_data.get() == NULL){
                
                return "";
            }
            
            return m_data.get()->get();
        }

        DRString& append(const char * other);
        const char * operator+=(const char * other){return append(other);}
        const char * operator+=(const DRString& other) {return append(other.text());}

        const char * operator=(const char * other){
            // there are problems if the current value overlaps with the other value;
            // make a copy of other and use the copy as this value
            DRString copy(other);
            m_data = copy.m_data;
            return text();
        }

        void clear() {
            
            if (m_data.get() == NULL) {
                
            }
            
            m_data.get()->setLength(0);
            
        }
        char * increaseLength(size_t charsNeeded) { 
            return m_data.get()->increaseLength(charsNeeded);
        }

        const char * text() const { return get();}
        size_t getLength() { return m_data.get()->getLength();}

        // trime whitespace + optional chars
        DRString& trimStart(const char * chars=NULL);
        DRString& trimEnd(const char * chars=NULL);
    protected:
        SharedPtr<DRStringData> m_data;

};

DRString DRString::fromFloat(double val){
    DRString result;
    char * buf = result.m_data->ensureLength(32);
    sprintf(buf,"%f",val);
    result.m_data->updateLength();
    return result;
}

DRString::DRString(const DRString& other) : m_data(other.m_data){
    
    
}

DRString::DRString(const char * orig) {
    
    size_t len = orig == NULL ? 0 : strlen(orig);
    m_data = new DRStringData(len);
    if (orig != NULL) { 
        
        strncpy(m_data.get()->data(),orig,len);
        
    }

}

DRString::DRString(const char * orig, size_t length) {
    
    size_t len = orig == NULL ? 0 : length;
    m_data = new DRStringData(len);
    if (orig != NULL) { 
        strncpy(m_data.get()->data(),orig,len);
    }
}


DRString::DRString(char c, size_t repeatCount) {
    
    
    size_t len = repeatCount;
    m_data = new DRStringData(len);
    char * data = m_data.get()->data();
    for(int i=0;i<len;i++) {
        data[i] = c;
    }
    
}

DRString::~DRString() {
    
    if (m_data.get() == NULL) {
        
    } else {
        
        
    }
}

DRString& DRString::append(const char * other){
    if (other == NULL) { return *this;}
    
    size_t olen = strlen(other);
    if (olen == 0) {
        return *this;
    }
    char * extra = m_data.get()->increaseLength(olen);
    memcpy(extra,other,olen);
    return *this;
}


class DRFormattedString : public DRString{
    public:
        DRFormattedString(const char * format,...){
            va_list args;
            va_start (args,format);
            formatString(format,args);
        }

    private:
        void formatString(const char * format, va_list args) {
            char buf[2];
            int len = vsnprintf(buf,1,format,args)+1;
            m_data.get()->ensureLength(len+1);
            vsnprintf(m_data.get()->data(),len,format,args);
        }
};



}
#endif