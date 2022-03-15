#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"
#include "../util/util.h"
#include "./interface.h"

extern EspBoardClass EspBoard;

namespace DevRelief {



char lastErrorMessage[100];
long lastErrorTime=0;
bool logTestingMessage = false;

class DRLogger : public ILogger {
public:
    DRLogger(const char * name, int level = 100) {
        m_name = NULL;
        m_lastError = NULL;
        m_lastErrorTime = 0;
        setModuleName(name);
        m_level = level;
    }


    virtual ~DRLogger() {
        Util::freeText(m_name);
    }

    void setModuleName(const char * name) {
        if (name == NULL) {
            name = "???";
        }
        m_name = Util::allocText(name);
    }


    void write(int level, const char * message, va_list args ) const override {
        ILogConfig* cfg = ILogConfig::Instance();
        if (cfg == NULL || message == NULL || message[0] == 0) {
            // need LogConfig before doing anything.
            return;
        }
        ILogDestination* dest = cfg->getDestination();
        if (dest == NULL) {
            Serial.println("no Log dest");
            return;
        }
        ILogFilter* filter = cfg->getFilter();
        if (!shouldLog(level,m_name,message) || (filter && !filter->shouldLog(this,level,m_name,message))) {
            return;
        }
        const char* output = message;
        ILogFormatter* formatter = cfg->getFormatter();
        if (formatter) {
            output = formatter->format(m_name,level,message,args);
        }

        if (dest) {
            dest->write(output);
        }
        
       
    }


    void write(int level, const char * message,...) const override  {
        va_list args;
        va_start(args,message);
        write(level,message,args);
    }

    void test(const char * message,...) const override  {
        if (!logTestingMessage) { return;}
        va_list args;
        va_start(args,message);
        write(TEST_LEVEL,message,args);
    }



    void debug(const char * message,...) const override  {

        va_list args;
        va_start(args,message);
        write(DEBUG_LEVEL,message,args);
      
    }



    void info(const char * message,...) const override  {
        va_list args;
        va_start(args,message);
        write(INFO_LEVEL,message,args);
      
    }


    void warn(const char * message,...)  const override {
        va_list args;
        va_start(args,message);
        write(WARN_LEVEL,message,args);
    }


    
    void error(const char * message,...) const override  {
        va_list args;
        va_start(args,message);
        write(ERROR_LEVEL,message,args);
    }

    // mainly for new messages during development.  don't need to turn debug level on and get old debug messages.
    // quick search-replace of "always"->"debug" when done
    void always(const char * message,...) const override  {
        va_list args;
        va_start(args,message);
        write(ALWAYS_LEVEL,message,args);
    }

     void never(const char * message,...) const override  {
     }

    void conditional(bool test, const char * message,...) const override  {
        if (!test) { return;}
        va_list args;
        va_start(args,message);
        write(CONDITION_LEVEL,message,args);
    }

    void conditional(bool test, int level, const char * message,...) const override  {
        if (!test) { return;}
        va_list args;
        va_start(args,message);
        write(level,message,args);
    }

    void errorNoRepeat(const char * message,...) const override  {
        if (message == m_lastError){
            return; //don't repeat the error message
        }
        if (EspBoard.currentMsecs()>m_lastErrorTime+500) {
            return; //don't show any errors too fast - even different message;
        }
        ((DRLogger*)this)->m_lastError = message;
        lastErrorTime = EspBoard.currentMsecs();
        va_list args;
        va_start(args,message);
        write(ERROR_LEVEL,message,args);
    }

    void showMemory(const char * label="Memory") const override {
        write(INFO_LEVEL,"%s: stack=%d,  heap=%d, max block size=%d, fragmentation=%d",label,EspBoard.getFreeContStack(),EspBoard.getFreeHeap(),EspBoard.getMaxFreeBlockSize(),EspBoard.getHeapFragmentation());
    }

    void showMemory(int level, const char * label="Memory")  const override {
        write(level,"%s: stack=%d,  heap=%d, max block size=%d, fragmentation=%d",label,EspBoard.getFreeContStack(),EspBoard.getFreeHeap(),EspBoard.getMaxFreeBlockSize(),EspBoard.getHeapFragmentation());
    }

    int getLevel() const { return m_level;}
    void setLevel(int level) { m_level = level;}

protected:
    virtual bool shouldLog(int level,const char*m_name,const char* message)const {
        return (m_level >= level);
    }
 
    int m_level;
    char * m_name;
    const char * m_lastError;
    int m_lastErrorTime;

};

/*
 * PeriodicLogger has a maximum frequency it will write.  
 * It can be used as any other DRLogger, but all messages
 * will be ignored until the frequency elapses.  
 * This allows something to be writen at the start of every loop()
 * but the write only happens at a specified frequency
 */
class PeriodicLogger : public DRLogger {
    public:
    PeriodicLogger(int maxFrequencyMsecs, const char * name, int level = 100) : DRLogger(name,level) {

        m_maxFrequencyMsecs = maxFrequencyMsecs;
        m_lastWriteTime = 0;
    }


    protected:
    bool shouldLog(int level,const char*m_name,const char* message)const {
        if (EspBoard.currentMsecs() < m_lastWriteTime+m_maxFrequencyMsecs){
            return false;
        }        
        if (m_level < level) { return false;}
        ((PeriodicLogger*)this)->m_lastWriteTime = EspBoard.currentMsecs();
        return true;
    }

    private:
        int m_maxFrequencyMsecs;
        int m_lastWriteTime;
};

}
#endif