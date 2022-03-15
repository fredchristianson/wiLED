#ifndef LOGGER_FILTER_H
#define LOGGER_FILTER_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"
#include "./interface.h"
#include "../util/drstring.h"
#include "../util/util.h"

/* include/exclude modules not implemented. 
 * should match all or part of the module name.
 * if include and exclude both match, the longest match wins.
 * if the match length is the same, include wins.
 */

extern EspBoardClass EspBoard;

namespace DevRelief {
class LogDefaultFilter : public ILogFilter {
    public:
        LogDefaultFilter(LogLevel level = WARN_LEVEL) {
            m_level = level;
            m_includeModules = NULL;
            m_excludeModules = NULL;
            m_isUnitTest = false;
        }

        virtual bool shouldLog(const ILogger* logger, int level, const char* module, const char*message)const {
            if (m_isUnitTest && level == TEST_LOGGER_LEVEL) { return true;}
            if (level == ALWAYS_LEVEL) {
                return true;
            }
            if (level == NEVER_LEVEL || m_level < level || logger->getLevel() < level) { 
                return false;
            }
            if (m_includeModules) { /* todo */}
            if (m_excludeModules) { /* todo */}
            return true;
        }

        void setLevel(int level) {
            m_level = level;
        };

        void setTesting(bool isUnitTest) {
            m_isUnitTest = isUnitTest;
        }

    private:
        const char* m_includeModules;
        const char* m_excludeModules;
        int m_level;
        bool m_isUnitTest;
};
}
#endif
