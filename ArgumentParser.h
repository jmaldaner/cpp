#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <any>
#include <optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class ArgumentSpec {

    private:
        string _name;
        vector<string> _aliases;
        string _comment;
        int _is_optional_bool;
        optional<string> _default_value;
        any (*_parser) (const char*);

    public:
        string name() const;
        const vector<string>& aliases() const;
        string comment() const;
        int is_optional_bool() const;
        optional<string> default_value() const;
        any (*const parser() const) (const char*) const;

        ArgumentSpec();
        ArgumentSpec(string name, vector<string> aliases, string comment, bool isBool, any (*parser) (const char*));
        ArgumentSpec(string name, vector<string> aliases, string comment, string defaultValue, any (*parser) (const char*));

        static any (*string_parser) (const char* input);
        static any (*int_parser) (const char* input);
        static any (*bool_parser) (const char* input);

        static ArgumentSpec forString(string name, vector<string> aliases, string comment);
        static ArgumentSpec forInt(string name, vector<string> aliases, string comment);
        static ArgumentSpec forString(string name, vector<string> aliases, string comment, string defaultValue);
        static ArgumentSpec forInt(string name, vector<string> aliases, string comment, int defaultValue);
        static ArgumentSpec forBool(string name, vector<string> aliases, string comment);
};

class ArgumentParser {

    private:
        unique_ptr<unordered_map<string, ArgumentSpec>> specs;
        unique_ptr<unordered_map<string, any>> args;

        optional<ArgumentSpec> match_definition(const string & argument);

    public:
        ArgumentParser(int argc, char* argv[], const vector<ArgumentSpec>& specs);

        string get_string(const char* argument) const;
        int get_int(const char* argument) const;
        bool get_bool(const char* argument) const;
        any get(const char* argument) const;
};

#endif
