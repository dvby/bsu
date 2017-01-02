#pragma once

#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;

struct User {
    string Id;
    string Name;
    vector<string> Posts;
    string Location;

    User(const string& id, const string& name)
        : Id(id)
        , Name(name)
    {
        Location = "/user/" + Id;
    }

    User(const string& id, const string& name, const vector<string>& posts)
        : Id(id)
        , Name(name)
        , Posts(posts)
    {
        Location = "/user/" + Id;
    }

    string ToJson() const {
        std::ostringstream ss;
        ss << "{\n";
        ss << "\t\"id\" : \"" << Id << "\",\n";
        ss << "\t\"location\" : \"" << Location << "\",\n";
        ss << "\t\"name\" : \"" << Name << "\",\n";
        ss << "\t\"posts\" : [";
        if (Posts.size() > 0) {
            ss << "\n\t\t\"" << Posts[0];
            for (size_t i = 1; i < Posts.size(); ++i) {
                ss << "\",\n\t\t\"" << Posts[i];
            }
            ss << "\"\n\t";
        }
        ss << "]\n}";
        return ss.str();
    }
};
