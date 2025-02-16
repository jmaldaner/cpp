#include <format>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ArgumentParser.h"

using namespace std;

string ArgumentSpec::name() const {
    return _name;
}

const vector<string>& ArgumentSpec::aliases() const {
    return _aliases;
}

string ArgumentSpec::comment() const {
    return _comment;
}

int ArgumentSpec::is_optional_bool() const {
    return _is_optional_bool;
}

optional<string> ArgumentSpec::default_value() const {
    return _default_value;
}

any (*const ArgumentSpec::parser() const) (const char*) const {
    return _parser;
}

any parseString(const char* input) {
    return string(input);
}

any (*ArgumentSpec::string_parser) (const char* input) = parseString;

any parseInt(const char* input) {
    return stoi(input);
}

any (*ArgumentSpec::int_parser) (const char* input) = parseInt;

any parseBool(const char* input) {
    return string("true").compare(input) == 0;
}

any (*ArgumentSpec::bool_parser) (const char* input) = parseBool;

ArgumentSpec ArgumentSpec::forString(string name, vector<string> aliases, string comment, string defaultValue) {
    return ArgumentSpec(name, aliases, comment, defaultValue, string_parser);
}

ArgumentSpec ArgumentSpec::forInt(string name, vector<string> aliases, string comment, int defaultValue) {
    return ArgumentSpec(name, aliases, comment, to_string(defaultValue), int_parser);
}

ArgumentSpec ArgumentSpec::forString(string name, vector<string> aliases, string comment) {
    return ArgumentSpec(name, aliases, comment, false, string_parser);
}

ArgumentSpec ArgumentSpec::forInt(string name, vector<string> aliases, string comment) {
    return ArgumentSpec(name, aliases, comment, false, int_parser);
}

ArgumentSpec ArgumentSpec::forBool(string name, vector<string> aliases, string comment) {
    return ArgumentSpec(name, aliases, comment, true, bool_parser);
}

ArgumentSpec::ArgumentSpec() {
    _aliases = vector<string>();
};

ArgumentSpec::ArgumentSpec(string name, vector<string> aliases, string comment, string defaultValue, any (*parser) (const char*)) {
    _name = name;
    _aliases = vector<string>(aliases);
    _comment = comment;
    _is_optional_bool = false;
    _default_value = defaultValue;
    _parser = parser;
}


ArgumentSpec::ArgumentSpec(string name, vector<string> aliases, string comment, bool isBool, any (*parser) (const char*)) {
    _name = name;
    _aliases = vector<string>(aliases);
    _comment = comment;
    _is_optional_bool = isBool;
    _default_value = isBool ? optional<string>("false") : nullopt;
    _parser = parser;
}

optional<ArgumentSpec> ArgumentParser::match_definition(const string& argument) {
    optional<string> lookupKey = nullopt;
    if (argument.compare(0, 2, "--") == 0) {
        lookupKey = argument.substr(2);
    } else if (argument[0] == '-') {
        lookupKey = argument.substr(1);
    }
    if (!lookupKey.has_value()) {
        return nullopt;
    }
    unordered_map<string, ArgumentSpec>::iterator it = specs->find(lookupKey.value());
    return it != specs->end() ? make_optional(it->second) : nullopt;
}

unique_ptr<unordered_map<string, ArgumentSpec>> index_specs(const vector<ArgumentSpec>& specs) {
    unique_ptr<unordered_map<string, ArgumentSpec>> ret = make_unique<unordered_map<string, ArgumentSpec>>();
    for (vector<ArgumentSpec>::const_iterator it = specs.begin(); it != specs.end(); it++) {
        ArgumentSpec spec = *it;
        if (spec.name().length() == 0) {
            throw std::invalid_argument("Invalid flag spec: missing name");
        }
        (*ret)[spec.name()] = spec;
        for (vector<string>::const_iterator alias = spec.aliases().begin(); alias != spec.aliases().end(); alias++) {
            (*ret)[*alias] = spec;
        }
    }
    return ret;
}

ArgumentParser::ArgumentParser(int argc, char* argv[], const vector<ArgumentSpec>& specs) {
    this->specs = index_specs(specs);
    this->args = make_unique<unordered_map<string, any>>();
    optional<ArgumentSpec> spec = nullopt;
    optional<const char*> specValue = nullopt;
    for (int i = 1; i < argc; i++) {
        if (!spec.has_value()) {
            spec = match_definition(argv[i]);
            if (!spec.has_value()) {
                std::ostringstream sss;
                sss << "Unknown parameter " << argv[i];
                throw std::invalid_argument(sss.str());
            }   
        } else {
            if (!specValue.has_value()) {
                specValue = optional<const char*>(argv[i]);
            } else {
                std::ostringstream sss;
                sss << "Multiple values in sequence for attribute " << spec.value().name();
                sss << ". Use quotes.";
                throw std::invalid_argument(sss.str());
            }
        }
        if (specValue.has_value() || spec.value().is_optional_bool()) {
            (*args)[spec.value().name()] = spec.value().is_optional_bool() ? make_any<bool>(true) : spec.value().parser()(specValue.value());
            spec = nullopt;
            specValue = nullopt;
        }
    }
    if (spec.has_value() || specValue.has_value()) {
        std::ostringstream sss;
        sss << "Missing paramter value --" << spec.value().name();
        throw std::invalid_argument(sss.str());
    }
}

string ArgumentParser::get_string(const char* argument) const {
    return any_cast<string>(get(argument));
}

int ArgumentParser::get_int(const char* argument) const {
    return any_cast<int>(get(argument));
}

bool ArgumentParser::get_bool(const char* argument) const {
    return any_cast<bool>(get(argument));
}

any ArgumentParser::get(const char* argument) const {
    unordered_map<string, any>::const_iterator it = args->find(argument);
    if (it == args->end()) {
        unordered_map<string, ArgumentSpec>::const_iterator spec = specs->find(argument);
        if (spec == specs->end()) {
            std::ostringstream sss;
            sss << "Invalid arugment " << argument;
            throw std::invalid_argument(sss.str());
        }
        if (spec->second.default_value().has_value()) {
            return spec->second.parser()(spec->second.default_value().value().c_str());
        } else {
            std::ostringstream sss;
            sss << "Missing mandatory parameter " << spec->second.name();
            throw std::invalid_argument(sss.str());
        }
    } else {
        return it->second;
    }
}



