#ifndef EPOCH_TIME_H
#define EPOCH_TIME_H

// sets the epoch time once and return hour:minute:seconds 
// any time after, updated with change in millis()
class EpochTime {
        public:
            static EpochTime Instance;
            EpochTime() {
                m_secondsAtMillis = 0;
                m_millis = 0;
                m_gmtOffsetMinutes = 0;
            }

            void setGmtOffsetMinutes(int offsetMinutes) {
                m_gmtOffsetMinutes = offsetMinutes;
            }

            int getGmtOffsetMinutes() const { return m_gmtOffsetMinutes;}

            void setSecondsNow(ulong seconds) {
                m_millis = millis();
                m_secondsAtMillis = seconds;
            }

            ulong getHour() {
                long h = getEpochSeconds();
                return (h/3600)%24;
            }

            ulong getMinute() {
                long h = getEpochSeconds();
                return (h/60)%60;
            }

            ulong getSecond() {
                long h = getEpochSeconds();
                return (h)%60;
            }

            long getEpochSeconds() {
                if (m_millis==0) {
                    return (millis()/1000)%60;
                }
                long diff = millis()-m_millis;
                long seconds = m_secondsAtMillis +  m_gmtOffsetMinutes*60 + (diff/1000)%60;
                return seconds;
            }
        private:
            ulong m_gmtOffsetMinutes;
            ulong m_secondsAtMillis;
            ulong m_millis;
    };

EpochTime EpochTime::Instance;

#endif