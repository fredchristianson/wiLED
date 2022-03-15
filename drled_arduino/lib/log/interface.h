#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"

extern EspBoardClass EspBoard;

namespace DevRelief {

class ILogger;


enum LogLevel {
    DEBUG_LEVEL=100,
    INFO_LEVEL=80,
    WARN_LEVEL=60,
    ERROR_LEVEL=40,
    ALWAYS_LEVEL=1,
    TEST_LEVEL=-1,  // only & always matches during tests
    NEVER_LEVEL=-2,
    CONDITION_LEVEL=-3
};


class ILogDestination {
    public:
        virtual void write(const char * message)const=0;
};

class ILogFilter {
    public:
        virtual void setLevel(int level)=0;
        virtual void setTesting(bool isUnitTest)=0;
        virtual bool shouldLog(const ILogger*logger, int level, const char* module, const char*message)const = 0;


};


class ILogFormatter {
    public:
        virtual const char * format(const char*moduleName,int level,const char * message,va_list args)const=0;
        virtual void indent()=0;
        virtual void outdent()=0;
};


class ILogConfig {
    public:
        ILogConfig() { m_instance = this;}
        static ILogConfig* Instance() { return m_instance;}

        virtual void indent()=0;
        virtual void outdent()=0;

        virtual void setLevel(LogLevel l)=0;
        virtual int getLevel() const =0;
        virtual bool isDebug() const =0;
        virtual void setTesting(bool isUnitTest)=0;

        virtual ILogFilter* getFilter()const=0;
        virtual ILogFormatter* getFormatter()const = 0;
        virtual ILogDestination* getDestination()const=0;
    protected:
        static ILogConfig* m_instance;
};

class ILogger {
    public:
        virtual void write(int level, const char * message, va_list args )const=0;
        virtual void write(int level, const char * message,...)const=0;
    
        virtual void test(const char * message,...) const=0;  // message only writen during unit test

        virtual void debug(const char * message,...) const=0;
        virtual void info(const char * message,...) const=0;
        virtual void warn(const char * message,...) const=0;
        virtual void error(const char * message,...) const=0;
        virtual void always(const char * message,...) const=0;
        virtual void never(const char * message,...) const=0;
        virtual void conditional(bool test, const char * message,...) const=0;
        virtual void conditional(bool test, int level, const char * message,...) const=0;
        virtual void errorNoRepeat(const char * message,...) const=0;
        virtual void showMemory(const char * label="Memory") const=0;
        virtual void showMemory(int level, const char * label="Memory")  const=0;
    
        virtual int getLevel() const=0;
        virtual void setLevel(int level)=0;
};

class NullLogger : public ILogger {
    public:
        void write(int level, const char * message, va_list args ) const override {}
        void write(int level, const char * message,...) const override {}
    
        void test(const char * message,...) const override {}

        void debug(const char * ,...)  const override {}
        void info(const char * ,...)  const override {}
        void warn(const char * ,...)  const override {}
        void error(const char * ,...)  const override {}
        void always(const char * ,...)  const override {}
        void never(const char * ,...)  const override {}
        void conditional(bool test, const char * ,...) const override {}
        void conditional(bool test, int level, const char * ,...) const override {}
        void errorNoRepeat(const char * ,...) const override {}
        void showMemory(const char * label="Memory") const override{}
        void showMemory(int level, const char * label="Memory")const override {}
        int getLevel() const { return 0;};
        void setLevel(int level) { };
};

class NullLogConfig : public ILogConfig {
    public:
        NullLogConfig() : ILogConfig() { m_instance = this;}

        virtual void indent() {}
        virtual void outdent() {}

        virtual void setLevel(LogLevel l) {}
        virtual int getLevel() const  { return 0;}
        virtual bool isDebug() const  { return false;}
        virtual void setTesting(bool isUnitTest) {}

        virtual ILogFilter* getFilter()const {return NULL;}
        virtual ILogFormatter* getFormatter()const { return NULL;}
        virtual ILogDestination* getDestination()const {return NULL;}
    protected:
};

/*
 * A LogIndent object causes log messages to be indented by an additional tab
 * until it is destroyed.
 * {
 *      m_logger->debug("unindented message");
 *      LogIndent indent1;
 *      m_logger->debug("message indented by 1 tab");
 *      if (...) {
 *          LogIndent indent2;
 *          m_logger->debug("message indented by 2 tab");
 *          // indent2 destructor
 *      }
 *      m_logger->debug("message indented by 1 tab");
 * }
*/

#ifdef LOGGING_ON==1
class LogIndent {
    public:
        LogIndent() {
            ILogConfig::Instance()->indent();
            m_logger = NULL;
            m_name = NULL;
            m_level = NEVER_LEVEL;
        }
        LogIndent(ILogger* logger, const char * blockName, LogLevel level = DEBUG_LEVEL) {
            logger->write(level,"->%s - start",blockName);
            ILogConfig::Instance()->indent();
            m_name = blockName;
            m_level = level;
            m_logger = logger;
        }

        ~LogIndent() {
            ILogConfig::Instance()->outdent();
            if (m_logger && m_name) {
                m_logger->write(m_level,"<-%s - end",m_name);
            }
            ILogConfig::Instance()->outdent();
        }
    private: 
        const char * m_name;
        ILogger* m_logger;
        LogLevel m_level;
};
#else
class LogIndent {
    public:
        LogIndent() {
            
        }
        LogIndent(ILogger* logger, const char * blockName, LogLevel level = DEBUG_LEVEL) {
            
        }

        ~LogIndent() {
            
        }
#endif

ILogConfig* ILogConfig::m_instance = NULL;

#if LOGGING_ON==1
// set and declare m_logger member of a class.  
#define SET_LOGGER(logger)  extern ILogger* logger; m_logger= logger
#define DECLARE_LOGGER()    ILogger* m_logger    
// set and declare a logger member of a class with name other than m_logger.  
#define SET_CUSTOM_LOGGER(var,logger) extern ILogger* logger; var = logger
#define DECLARE_CUSTOM_LOGGER(var)  ILogger* var 

// the main logger type when logging is on
#define LOGGER_TYPE DRLogger

// if logging is on, the LM() macro leaves the (L)og (M)essage as it is
#define LM(msg) msg



#else
NullLogger nullLogger;
NullLogger* m_logger = &nullLogger; // m_logger-> can be used anywhere even though members are not declared
#define SET_LOGGER(LOGGER) /*nothing*/
#define DECLARE_LOGGER()     /*nothing*/ 
const char * emptyMessage="";
// standard loggers are named m_logger.  If LOGGIN_ON is not 1, m_logger is a global NullLogger
// custom loggers have names other than m_logger so must be declared members
#define SET_CUSTOM_LOGGER(name,LOGGER) name=&nullLogger
#define DECLARE_CUSTOM_LOGGER(name) ILogger* name

// the main logger type when logging is off
#define LOGGER_TYPE NullLogger
// if logging is off, the LM() macro replaces the log message with an empty string
#define LM(msg) emptyMessage
#endif

}

#endif

