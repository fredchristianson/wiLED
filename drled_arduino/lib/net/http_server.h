
#ifndef DRHTTP_SERVER_H
#define DRHTTP_SERVER_H


#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <functional>
#include <memory>
#include <functional>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include "../log/logger.h"
#include "./wifi.h"


namespace DevRelief {
    typedef ESP8266WebServer Request;
    typedef ESP8266WebServer Response;

using HttpHandler = std::function<void(Request*, Response*)> ;

class HttpServer {
    public:


        HttpServer() {
            SET_LOGGER(HttpServerLogger);
            m_logger->debug(LM("HttpServer created"));

            m_server = new ESP8266WebServer(80);
            // cors added to each response in this->cors();
            //m_server->enableCORS(true);

            m_logger->info(LM("Listening to port 80 on IP %s"),WiFi.localIP().toString().c_str());

           m_server->onNotFound([&](){
               m_logger->debug("handle unknown URI %s",m_server->uri().c_str());
               cors(m_server);
               if (m_server->method() == HTTP_OPTIONS) {
                    m_server->send(200);
               } else {
                   m_server->send(404);
               }

           });

           m_server->on("/ping",HTTP_GET,[&](){
               cors(m_server);
               m_server->send(200,"ping");
           });
        }


        void begin() {
            m_logger->info(LM("HttpServer listening"));
            m_server->begin();
        }

        void handleClient() {
            if (m_server->client()) {
                m_server->client().keepAlive();
            }
            m_server->handleClient();
        }

       
        void routeBraces(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),[this,handler,server](){
                this->cors(server);
                handler(server,server);
            });
        }

        void routeBracesGet(const char * uri, HttpHandler httpHandler){
            m_logger->always(LM("routing GET to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_GET,[this,handler,server](){
                this->cors(server);
                m_logger->debug("GET %s",server->uri().c_str());
                handler(server,server);
            });
        }

        void routeBracesPost(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing POST to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_POST,[this,handler,server](){
                m_logger->debug("POST %s",server->uri().c_str());
                this->cors(server);
                handler(server,server);
            });
        }

        void routeBracesDelete(const char * uri, HttpHandler httpHandler){
            m_logger->always(LM("routing DELETE to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_DELETE,[this,handler,server](){
                m_logger->debug("DELETE %s",server->uri().c_str());
                this->cors(server);
                handler(server,server);
            });
        }

        void route(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(uri,[this,handler,server,uri](){
                this->cors(server);
                m_logger->debug(LM("path found %s"),uri);

                handler(server,server);
            });
        }
        void route(const char * uri, HTTPMethod method, HttpHandler httpHandler){
            m_logger->debug(LM("routing %s"),uri);
             auto server = m_server;
            auto handler = httpHandler;
            m_server->on(uri,method, [this,handler,server](){
                this->cors(server);
                handler(server,server);
            });
        }

        void send(const char * type, const char * value) {

            m_server->send(200,type,value);
        }

        void cors(ESP8266WebServer * server) {
            server->sendHeader(F("Access-Control-Allow-Origin"), F("*"));
            server->sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS,DELETE,PATCH"));
            server->sendHeader(F("Access-Control-Allow-Headers"), F("*"));
        }

    private:
        DECLARE_LOGGER(); 
        ESP8266WebServer * m_server;
    };
}
#endif 