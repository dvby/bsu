#include "DBManager.h"

#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>
#include <fastcgi2/stream.h>

#include <iostream>
#include <string>
#include <sstream>

class UserProcessorClass : virtual public fastcgi::Component, virtual public fastcgi::Handler {
    DBManager db;
public:
    UserProcessorClass(fastcgi::ComponentContext *context) :
        fastcgi::Component(context) {
    }
    virtual ~UserProcessorClass() {
    }
public:
    virtual void onLoad() {
    }
    virtual void onUnload() {
    }
    virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        try {
            const string method = request->getRequestMethod();
            if (method == "GET") {
                handleGET(request, context);
            }
            else if (method == "POST") {
                handlePOST(request, context);
            }
            else if (method == "PUT") {
                handlePUT(request, context);
            }
            else if (method == "DELETE") {
                handleDELETE(request, context);
            }
            else {
                request->setStatus(405);
            }
            request->setStatus(200);
        }

        catch (const std::string& s) {
            request->setContentType("text/plain");
            fastcgi::RequestStream stream(request);
            stream << s << std::endl;
            request->setStatus(500);
        }

    }
private:
    std::string getId(fastcgi::Request* request) const {
        const std::string prefix = "/user/";
        if (request->getScriptName().size() < prefix.size()) {
            return std::string();
        } else {
            return request->getScriptName().substr(prefix.size());
        }
    }

    void handleGET(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        request->setContentType("application/json");
        std::stringbuf buffer(db.ReadUser(id).ToJson());
        request->write(&buffer);
    }

    void handlePOST(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        if (id.size() > 0) {
            //Create post for user
            std::string post;
            fastcgi::DataBuffer bodyBuffer = request->requestBody();
            bodyBuffer.toString(post);
            Post created =  db.CreatePost(id, post);

            std::stringbuf buffer(created.ToJson());
            request->setContentType("application/json");
            request->write(&buffer);
        } else {
            std::string user;
            fastcgi::DataBuffer bodyBuffer = request->requestBody();
            bodyBuffer.toString(user);
            User created = db.CreateUser(user);

            std::stringbuf buffer(created.ToJson());
            request->setContentType("application/json");
            request->write(&buffer);
        }
    }

    void handlePUT(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        std::string user;
        fastcgi::DataBuffer bodyBuffer = request->requestBody();
        bodyBuffer.toString(user);

        std::stringbuf buffer(db.UpdateUser(id, user).ToJson());
        request->setContentType("application/json");
        request->write(&buffer);
    }

    void handleDELETE(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        db.DeleteUser(id);
    }
};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("simple_factory", UserProcessorClass)
FCGIDAEMON_REGISTER_FACTORIES_END()
