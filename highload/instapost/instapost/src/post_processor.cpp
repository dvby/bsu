#include "DBManager.h"

#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>
#include <fastcgi2/stream.h>

#include <exception>
#include <iostream>
#include <string>
#include <sstream>

class PostProcessorClass : virtual public fastcgi::Component, virtual public fastcgi::Handler {
    DBManager db;
public:
    PostProcessorClass(fastcgi::ComponentContext *context) :
        fastcgi::Component(context) {
    }
    virtual ~PostProcessorClass() {
    }
public:
    virtual void onLoad() {
    }
    virtual void onUnload() {
    }
    virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        try {
            const std::string method = request->getRequestMethod();
            if (method == "GET") {
                handleGET(request, context);
            }
            else if (method == "POST") {
                request->setStatus(405);
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
        const std::string prefix = "/post/";
        if (request->getScriptName().size() < prefix.size()) {
            return std::string();
        } else {
            return request->getScriptName().substr(prefix.size());
        }
    }

    void handleGET(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        std::stringbuf buffer(db.ReadPost(id).ToJson());
        request->setContentType("application/json");
        request->write(&buffer);
    }

    void handlePUT(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        // Post post = db.ReadPost(id);

        //TODO
        // post.Title = request->hasArg("title") ? request->getArg("title") : post.Title;
        //TODO
        // post.Content = request->hasArg("content") ? request->getArg("content") : post.Content;

        std::string post;
        fastcgi::DataBuffer bodyBuffer = request->requestBody();
        bodyBuffer.toString(post);

        std::stringbuf buffer(db.UpdatePost(id, post).ToJson());
        request->setContentType("application/json");
        request->write(&buffer);
    }

    void handleDELETE(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        std::string id = getId(request);
        db.DeletePost(id);
    }
};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("simple_factory", PostProcessorClass)
FCGIDAEMON_REGISTER_FACTORIES_END()
