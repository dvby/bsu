#include "DBManager.h"

#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>

#include <iostream>
#include <string>
#include <sstream>

class DefaultProcessorClass : virtual public fastcgi::Component, virtual public fastcgi::Handler {
    DBManager db;
public:
    DefaultProcessorClass(fastcgi::ComponentContext *context) :
        fastcgi::Component(context) {
    }
    virtual ~DefaultProcessorClass() {
    }
public:
    virtual void onLoad() {
    }
    virtual void onUnload() {
    }
    virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string body;

        fastcgi::DataBuffer bodyBuffer = request->requestBody();
        bodyBuffer.toString(body);

        std::stringbuf buffer(body);
        request->setContentType("application/json");
        request->write(&buffer);
    }
};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("simple_factory", DefaultProcessorClass)
FCGIDAEMON_REGISTER_FACTORIES_END()
