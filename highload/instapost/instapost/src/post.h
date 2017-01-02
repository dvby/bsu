#pragma once

#include <string>
#include <sstream>
using std::string;

struct Post {
    string Id;
    string AuthorId;
    string Title;
    string Content;
    string Location;

    Post(const string& id, const string& authorId)
        : Id(id)
        , AuthorId(authorId)
        , Title("test title")
        , Content("test content")
    {
        Location = "/post/" + Id;
    }

    Post(const string& id, const string& authorId, const string& title, const string& content)
        : Id(id)
        , AuthorId(authorId)
        , Title(title)
        , Content(content)
    {
        Location = "/post/" + Id;
    }

    string ToJson() const {
        std::ostringstream ss;
        ss << "{\n";
        ss << "\t\"id\" : \"" << Id << "\",\n";
        ss << "\t\"location\" : \"" << Location << "\",\n";
        ss << "\t\"author\" : \"" << AuthorId << "\",\n";
        ss << "\t\"title\" : \"" << Title << "\",\n";
        ss << "\t\"content\" : \"" << Content << "\"\n";
        ss << "}";
        return ss.str();
    }
};


