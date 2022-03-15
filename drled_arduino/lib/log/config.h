#ifndef LOGGER_CONFIG_H
#define LOGGER_CONFIG_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"
#include "./interface.h"
#include "./filter.h"
#include "./formatter.h"
#include "./destination.h"

extern EspBoardClass EspBoard;

namespace DevRelief {

class LogConfig : public ILogConfig {
    public:
        LogConfig(ILogDestination*dest, ILogFilter* filter=NULL, ILogFormatter* formatter=NULL) {
            m_indentTabCount = 0;
            m_isTesting = false;
            m_destination = dest;
            m_formatter = formatter?formatter : dest ? new LogDefaultFormatter(): NULL;
            m_filter = filter ? filter : dest ? new LogDefaultFilter() : NULL;
        }

        void setTesting(bool on) { if (m_filter) { m_filter->setTesting(on);}}

        void indent() {
            //Serial.println("config indent");
            if (m_formatter) {
                m_formatter->indent();
            }

        }

        void outdent() {
            //Serial.println("config outdent");
            if (m_formatter) {
                m_formatter->outdent();
            }

        }

        void setLevel(LogLevel l) override { m_level = l;}
        int getLevel() const override { return m_level;}
        bool isDebug() const override { return m_level==100;}
        virtual ILogFilter* getFilter()const { return m_filter;}
        virtual ILogFormatter* getFormatter()const { return m_formatter;}
        virtual ILogDestination* getDestination()const{ return m_destination;}
    private:
        bool m_isTesting;
        int m_indentTabCount;
        ILogFilter* m_filter;
        ILogDestination* m_destination;
        ILogFormatter* m_formatter;
        LogLevel m_level;

};

}
#endif