#ifndef TASKS_H
#define TASKS_H

#include "../util/list.h"
#include "../log/interface.h"
#include "../log/logger.h"

namespace DevRelief {


PeriodicLogger periodicTaskLogger(5000,"Periodic Tasks",ERROR_LEVEL);

typedef enum TaskStatus {
    TASK_PAUSE,
    TASK_RUNNING,
    TASK_COMPLETE
};

class ITask {
    public:
        virtual void destroy() =0;
        virtual TaskStatus run()=0;
        virtual TaskStatus getStatus()const=0;
        virtual const char * getName()const=0;

        // loop() is called in the Tasks:Run().  
        // call run() if frequency is met or finish if duration expired
        virtual TaskStatus loop()=0;
};

class Tasks {
    private: 
        Tasks() {
            SET_LOGGER(TaskLogger);
        };

        void run() {
            periodicTaskLogger.debug("Tasks::run() count=%d %x",m_tasks.size(),m_logger);
            LogIndent id;
            m_tasks.each([&](ITask* task) {
                task->loop();
            });
            m_tasks.removeMatch([&](ITask*task) { 
                bool remove = task->getStatus() == TASK_COMPLETE;

                return remove;
            });
            periodicTaskLogger.debug("done count=%d",m_tasks.size());
        };

        PtrList<ITask*> m_tasks;
        DECLARE_LOGGER();

        friend class Task;

        void add(ITask* task) { m_tasks.add(task);}

        static Tasks* instance;
    public:
        static void Run() {
            instance->run();
        }
};

Tasks* Tasks::instance = new Tasks();

class Task : public ITask {
    public:
        Task(const char * name,long frequencyMsecs=0,long durationMsecs=0) {
            SET_LOGGER(TaskLogger);
            m_status = TASK_RUNNING;
            Tasks::instance->add(this);
            m_name = name;
            m_startTime = millis();
            m_frequencyMsecs = frequencyMsecs;
            m_durationMsecs = durationMsecs;
            m_lastRunMsecs = 0;
            m_logger->debug("Task started %s",getName());
        }

        virtual ~Task() {
            m_logger->debug("Task complete %s",getName());

        }

        virtual TaskStatus loop() override {
            if (m_durationMsecs>0 &&  millis()> m_startTime+m_durationMsecs) {
                return TASK_COMPLETE;
            }
            if (m_frequencyMsecs>0 && millis() < m_lastRunMsecs+m_frequencyMsecs){
                return TASK_PAUSE;
            }
            m_lastRunMsecs = millis();
            m_status = run();
            return m_status;
        }
        virtual void destroy() override { delete this;}

        TaskStatus getStatus()const override { return m_status;}
        const char * getName()const override { return m_name.text();}
    protected: 
        TaskStatus m_status;
        DRString m_name;
        long m_startTime;
        long m_frequencyMsecs;
        long m_durationMsecs;
        long m_lastRunMsecs;
        DECLARE_LOGGER();
};

}

#endif