#include "DBManager.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/stdx.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

const std::string DBNAME = "testdb1";
const std::string USERS_COLLECTION_NAME = "users";
const std::string POSTS_COLLECTION_NAME = "posts";
const size_t MAX_POSTS = 10;
const size_t MAX_RETRIES = 10;

boost::thread_specific_ptr<mongocxx::client> DBManager::clientPtr;

//////////////////////
//////// POST ////////
//////////////////////

Post DBManager::CreatePost(const std::string& authorId, const std::string& body) {
    auto post = bsoncxx::from_json(body);
    auto id = bsoncxx::oid().to_string();

    auto docValue = document{}
        << "_id" << id
        << "author" << authorId
        << bsoncxx::builder::concatenate_doc{ post.view() }
        << finalize;

    auto& conn = getClient();
    auto collection = conn[DBNAME][POSTS_COLLECTION_NAME];

    for (size_t i = 0; i < MAX_RETRIES; ++i) {
        auto maybeInserted = collection.insert_one(docValue.view());
        if (maybeInserted) {
            return ReadPost(id);
        }
    }
    throw std::string("Cannot insert post");
}

Post DBManager::ReadPost(const std::string& id) {
    auto& conn = getClient();
    auto collection = conn[DBNAME][POSTS_COLLECTION_NAME];

    auto maybe_result = collection.find_one(document{} << "_id" << id << finalize);
    if (maybe_result) {
        auto view = maybe_result->view();
        auto authorId = view["author"].get_utf8().value.to_string();
        auto title = view["title"].get_utf8().value.to_string();
        auto content = view["content"].get_utf8().value.to_string();
        return Post(id, authorId, title, content);
    }
    throw std::string("Cannot read post, possibly wrong id");
}

Post DBManager::UpdatePost(const std::string& id, const std::string& body) {
    auto post = bsoncxx::from_json(body);

    auto& conn = getClient();
    auto collection = conn[DBNAME][POSTS_COLLECTION_NAME];

    auto result = collection.update_one(document{} << "_id" << id << finalize,
                                        document{} << "$set" << open_document
                                        << bsoncxx::builder::concatenate_doc{ post.view() }
                                        << close_document << finalize);
    if (result) {
        return ReadPost(id);
    }
    throw std::string("Cannot update post, possibly wrong id");
}

void DBManager::DeletePost(const std::string& id) {
    auto& conn = getClient();
    auto collection = conn[DBNAME][POSTS_COLLECTION_NAME];

    auto result = collection.delete_one(document{} << "_id" << id << finalize);
    if (!result) {
        throw std::string("Cannot delete post, possibly wrong id");
    }
}

//////////////////////
//////// USER ////////
//////////////////////

User DBManager::CreateUser(const std::string& body) {
    auto user = bsoncxx::from_json(body);

    auto id = bsoncxx::oid().to_string();

    auto docValue = document{}
        << "_id" << id
        << bsoncxx::builder::concatenate_doc{ user.view() }
        << finalize;

    auto& conn = getClient();
    auto collection = conn[DBNAME][USERS_COLLECTION_NAME];

    for (size_t i = 0; i < MAX_RETRIES; ++i) {
        auto maybeInserted = collection.insert_one(docValue.view());
        if (maybeInserted) {
            return ReadUser(id);
        }
    }
    throw std::string("Cannot insert user");
}

User DBManager::ReadUser(const std::string& id) {
    auto& conn = getClient();
    auto collection = conn[DBNAME][USERS_COLLECTION_NAME];

    auto maybe_result = collection.find_one(document{} << "_id" << id << finalize);
    if (maybe_result) {
        auto view = maybe_result->view();
        auto name = view["name"].get_utf8().value.to_string();

        vector<std::string> posts;
        auto postsCollection = conn[DBNAME][POSTS_COLLECTION_NAME];
        mongocxx::options::find opts;
        opts.limit(MAX_POSTS);
        auto cursor = postsCollection.find(document{} << "author" << id << finalize, opts);
        for (auto&& doc : cursor) {
            posts.push_back(bsoncxx::to_json(doc));
        }

        return User(id, name, posts);
    }
    throw std::string("Cannot read user, possibly wrong id");
}

User DBManager::UpdateUser(const std::string& id, const std::string& body) {
    auto user = bsoncxx::from_json(body);

    auto& conn = getClient();
    auto collection = conn[DBNAME][USERS_COLLECTION_NAME];

    auto result = collection.update_one(document{} << "_id" << id << finalize,
                                        document{} << "$set" << open_document
                                        << bsoncxx::builder::concatenate_doc{ user.view() }
                                        << close_document << finalize);
    if (result) {
        return ReadUser(id);
    }
    throw std::string("Cannot update user, possibly wrong id");
}

void DBManager::DeleteUser(const std::string& id) {
    auto& conn = getClient();
    auto collection = conn[DBNAME][USERS_COLLECTION_NAME];

    auto result = collection.delete_one(document{} << "_id" << id << finalize);
    if (!result) {
        throw std::string("Cannot delete user, possibly wrong id");
    }
}
