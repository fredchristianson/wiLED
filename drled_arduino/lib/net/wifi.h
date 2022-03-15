#ifndef DRWIFIF_H
#define DRWIFIF_H
#include <ESP8266WiFi.h>
#include <lwip/dns.h>
#include "./wifi_credentials.h"
#include "../log/logger.h"
#include "../util/drstring.h"
#include "../../config.h"

namespace DevRelief {

    class DRWiFi {
    public:
        static DRWiFi* get() {
            if (DRWiFi::drWiFiSingleton == NULL) {
                DRWiFi::drWiFiSingleton = new DRWiFi();
                DRWiFi::drWiFiSingleton->initialize();
            }
            return DRWiFi::drWiFiSingleton;
        }

        bool isInitialized() {
            if (WiFi.status() == WL_CONNECTED) {
                WiFi.hostname(m_hostname.text());
                m_ipAddress = WiFi.localIP().toString().c_str();
                Config::getInstance()->setAddr(m_ipAddress.text());

                // use google DNS since mine isn't resolving everything it should.
                IP4_ADDR(&m_dnsAddr,8,8,8,8);
                dns_setserver(0,&m_dnsAddr);
                return true;
            }
            return false;
        }
    protected:
        DRWiFi(const char * hostname="dr_unnamed") {
            SET_LOGGER(WifiLogger);
            m_logger->debug("wifi created");
            m_hostname = hostname;
        }

        void initialize() {
            m_logger->info("WiFi initializing");
            WiFi.begin(wifi_ssid, wifi_password);

        }

        bool isConnected() {
            return WiFi.status() == WL_CONNECTED;
        }

        const char * getIPAddress() { return m_ipAddress.text();}

    private:
        DECLARE_LOGGER();
        static DRString m_hostname;
        static DRString m_ipAddress;
        static DRWiFi*  drWiFiSingleton;

        ip_addr m_dnsAddr;
    };

    DRWiFi * DRWiFi::drWiFiSingleton;
    DRString DRWiFi::m_hostname;
    DRString DRWiFi::m_ipAddress;
}


#endif 