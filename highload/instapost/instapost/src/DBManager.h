#pragma once

#include "post.h"
#include "user.h"

#include <boost/thread/tss.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <string>

class DBManager {
public:
    DBManager() {
        mongocxx::instance inst{};
    }

    Post CreatePost(const std::string& authorId, const std::string& post);

    Post ReadPost(const string& id);

    Post UpdatePost(const std::string& id, const std::string& post);

    void DeletePost(const string& id);


    User CreateUser(const std::string& user);

    User ReadUser(const string& id);

    User UpdateUser(const std::string& id, const std::string& user);

    void DeleteUser(const string& id);
private:
    static boost::thread_specific_ptr<mongocxx::client> clientPtr;

    static void reset() {
        clientPtr.reset(new mongocxx::client(mongocxx::uri{}));
    }

    static mongocxx::client& getClient() {
        if (!clientPtr.get())
            reset();
        return *clientPtr.get();
    }
};
