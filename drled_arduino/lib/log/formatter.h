#ifndef LOGGER_FORMATTER_H
#define LOGGER_FORMATTER_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"
#include "../system/epoch_time.h"
#include "../util/util.h"
#include "./interface.h"

extern EspBoardClass EspBoard;

namespace DevRelief {
class LogDefaultFormatter : public ILogFormatter {
    public:
        LogDefaultFormatter(int maxLen = 512) {
            m_maxOutputSize = 512;
            m_outputBuffer = Util::allocText(m_maxOutputSize);
            m_padding ="                                   ";
            m_tabs = "\t\t\t\t\t\t";
            m_maxTabs = 6;
            m_indentTabCount = 0;
        }

        virtual ~LogDefaultFormatter() {
            Util::freeText(m_outputBuffer);
        }

        void indent() {
            if (m_indentTabCount < m_maxTabs) {
                m_indentTabCount++;
            }
        }

        void outdent() {
            if (m_indentTabCount>0) {
                m_indentTabCount--;
            }
        }

        const char * format(const char*moduleName,int level,const char * message,va_list args) const override{
            if (moduleName == NULL) { moduleName = "???";}
            // long now = EspBoard.currentMsecs()/1000;
            // int hours = now/3600;
            // now = now % 3600;
            // int minutes = now/60;
            // int seconds = now % 60;
            EpochTime& time = EpochTime::Instance;

            const char * tabs = m_indentTabCount<=0 ? "" : (m_tabs + m_maxTabs-m_indentTabCount);
            int len = snprintf(m_outputBuffer,m_maxOutputSize,"%6s-%d:%02d:%02d - %15.15s: %s",
                        getLevelName(level),
                        time.getHour(),time.getMinute(),time.getSecond(),
                        moduleName,tabs);
            vsnprintf(m_outputBuffer+len,m_maxOutputSize-len,message,args);
            return m_outputBuffer;
        }
        
    private:
        const char * getLevelName(int level) const{
            if (level > 80) {
                return "DEBUG";
            }
            if (level > 60) {
                return "INFO ";
            }
            if (level > 40) {
                return "WARN ";
            }
            if (level ==  ALWAYS_LEVEL) {
                return "ALWAYS";
            } 
            if (level == TEST_LEVEL) {
                return "TEST";
            }
            if (level == CONDITION_LEVEL) {
                return "IF";
            }
            return "ERROR";
            
        }

    size_t m_maxOutputSize; 
    char* m_outputBuffer;
    char * m_padding;
    char * m_tabs;
    size_t m_indentTabCount;
    size_t m_maxTabs;
};
}
#endif