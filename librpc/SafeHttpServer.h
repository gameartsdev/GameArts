/**
 
 *
 * @file SafeHttpServer.h
 * @author: caryliao
 * @date 2018-10-26
 */
#pragma once

#include "jsonrpccpp/server/abstractserverconnector.h"
#include <microhttpd.h>
#include <map>
#include <string>

namespace dev
{
class SafeHttpServer : public jsonrpc::AbstractServerConnector
{
    typedef int (*pCallBack)(void*, struct MHD_Connection*, const char*, const char*, const char*,
        const char*, size_t*, void**);

public:
    /// "using HttpServer" won't work with msvc 2013, so we need to copy'n'paste constructor
    SafeHttpServer(std::string const& _address, int _port,
        std::string const& _sslcert = std::string(), std::string const& _sslkey = std::string(),
        int _threads = 64);
    virtual ~SafeHttpServer() {}

    virtual bool StartListening();
    virtual bool StopListening();
    virtual bool SendResponse(std::string const& _response, void* _addInfo = nullptr);
    virtual bool SendOptionsResponse(void* _addInfo);

    void SetUrlHandler(const std::string& url, jsonrpc::IClientConnectionHandler* handler);
    void setAllowedOrigin(std::string const& _origin) { m_allowedOrigin = _origin; }
    std::string const& allowedOrigin() const { return m_allowedOrigin; }
    virtual pCallBack getCallback() { return SafeHttpServer::callback; }

private:
    int port;
    int threads;
    bool running;
    std::string path_sslcert;
    std::string path_sslkey;
    std::string sslcert;
    std::string sslkey;

    struct MHD_Daemon* daemon;

    std::map<std::string, jsonrpc::IClientConnectionHandler*> urlhandler;

    static int callback(void* cls, struct MHD_Connection* connection, const char* url,
        const char* method, const char* version, const char* upload_data, size_t* upload_data_size,
        void** con_cls);

    jsonrpc::IClientConnectionHandler* GetHandler(const std::string& url);

    std::string m_allowedOrigin;
    std::string m_address;
};

}  // namespace dev
