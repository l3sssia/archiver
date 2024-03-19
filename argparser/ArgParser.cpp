#include "ArgParser.h"
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <sstream>

using namespace ArgumentParser;

ArgParser::ArgParser(std::string  name): name_(std::move(name)) {}

bool ArgParser::ParseLongOption(const std::string& arg) const {
    int begin = 1;
    if (arg[begin] == '-') {
        begin++;
    }
    std::string option = arg.substr(begin);
    auto delim_pos = option.find('=');
    if (delim_pos == std::string::npos) {
        Flag* flag = FindFlag(option);
        if (flag == nullptr) {
            return false;
        }
        flag->PutValue(true);
        return true;
    }
    std::string name = option.substr(0, delim_pos);
    std::string value = option.substr(delim_pos + 1);
    IntArgument* int_arg = FindIntArgument(name);
    if (int_arg != nullptr) {
        int_arg->PutValue(stoi(value));
        return true;
    }
    StringArgument* string_arg = FindStringArgument(name);
    if (string_arg != nullptr) {
        string_arg->PutValue(value);
        return true;
    }
    if (name.size() == 1) {
        char short_name = name[0];
        IntArgument* int_arg_short = FindIntArgument(short_name);
        if (int_arg_short != nullptr) {
            int_arg_short->PutValue(stoi(value));
            return true;
        }
        StringArgument* string_arg_short = FindStringArgument(short_name);
        if (string_arg_short != nullptr) {
            string_arg_short->PutValue(value);
            return true;
        }
    }
    return false;
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
//    for(auto i : string_args_) {
//        if (i.IsPositional()) {
//            positional_string_ = &i;
//        }
//    }
    int last_positional = 0;
    for (size_t i = 1; i < args.size(); i++) {
        if (args.empty()) {
            continue;
        }
        if (args[i][0] == '-') {
            if (ParseLongOption(args[i])) {
                continue;
            }
            if (args[i].size() > 2) {
                if(!ParseFlags(args[i])){
                    return false;
                }
                continue;
            }
            char short_name = args[i][1];
            Argument* arg = FindStringArgument(short_name);
            if (arg != nullptr) {
                dynamic_cast<StringArgument*>(arg)->PutValue(args[++i]);
                continue;
            }
            arg = FindIntArgument(short_name);
            if (arg != nullptr) {
                dynamic_cast<IntArgument*>(arg)->PutValue(stoi(args[++i]));
                continue;
            }
            arg = FindFlag(short_name);
            if (arg != nullptr) {
                dynamic_cast<Flag*>(arg)->PutValue(true);
                continue;
            }
            return false;
        } else {
            if (!FillPositional(args[i], last_positional)) {
                return false;
            }
        }
    }
    if (Help()) {
        return true;
    }
    if (!CheckFilledArgs()) {
        return false;
    }
    return true;
}

std::string ArgParser::GetStringValue(const char& short_name, const size_t& idx) const {
    StringArgument* arg = FindStringArgument(short_name);
    if (arg == nullptr) {
        throw std::invalid_argument("String Argument with given short name not found: " + short_name);
    }
    return arg->GetValue(idx);
}

std::string ArgParser::GetStringValue(const std::string& name, const size_t& idx) const {
    StringArgument* arg = FindStringArgument(name);
    if (arg == nullptr) {
        throw std::invalid_argument("String Argument with given name not found: " + name);
    }
    return arg->GetValue(idx);
}

int ArgParser::GetIntValue(const char& short_name, const size_t& idx) const {
    IntArgument* arg = FindIntArgument(short_name);
    if (arg == nullptr) {
        throw std::invalid_argument("Int Argument with given short name not found: " + short_name);
    }
    return arg->GetValue(idx);
}

int ArgParser::GetIntValue(const std::string& name, const size_t& idx) const {
    IntArgument* arg = FindIntArgument(name);
    if (arg == nullptr) {
        throw std::invalid_argument("Int Argument with given name not found: " + name);
    }
    return arg->GetValue(idx);
}

bool ArgParser::GetFlag(const char& short_name) const {
    Flag* arg = FindFlag(short_name);
    if (arg == nullptr) {
        throw std::invalid_argument("Flag with given short name not found: " + short_name);
    }
    return arg->GetValue();
}

bool ArgParser::GetFlag(const std::string& name) const {
    Flag* arg = FindFlag(name);
    if (arg == nullptr) {
        throw std::invalid_argument("Flag with given name not found: " + name);
    }
    return arg->GetValue();
}

std::string ArgParser::HelpDescription() const {
    std::stringstream help_description;
    help_description << name_ << "\n\n";
    for (const IntArgument& arg : int_args_) {
        help_description << arg.GetHelp();
    }
    for (const StringArgument& arg : string_args_) {
        help_description << arg.GetHelp();
    }
    for (const Flag& arg : flags_) {
        help_description << arg.GetHelp();
    }
    return help_description.str();
}

void ArgParser::AddHelp(const char &short_name, const std::string &name, const std::string &help) {
    AddFlag(short_name, name, help);
    help_argument_ = &flags_.back();
}

bool ArgParser::Help() {
    if (help_argument_ == nullptr) {
        return false;
    }
    return help_argument_->GetValue();
}

bool ArgParser::FillPositional(const std::string& value, int& begin) {
    for (size_t i = begin; i < arg_names_.size(); i++) {
            std::string name = arg_names_[i];
            IntArgument* int_arg = FindIntArgument(name);
            if (int_arg != nullptr && int_arg->IsPositional()) {
                int_arg->PutValue(stoi(value));
                begin = i;
                if (!int_arg->IsMultiValue()) {
                    begin++;
                }
                return true;
            }
            StringArgument* string_arg = FindStringArgument(name);
            if (string_arg != nullptr && string_arg->IsPositional()) {
                string_arg->PutValue(value);
                begin = i;
                if (!string_arg->IsMultiValue()) {
                    begin++;
                }
                return true;
            }
            //return false; // somehow
    }
    return false;
}

StringArgument *ArgParser::FindStringArgument(char short_name) const {
    for (const auto &string_arg : string_args_) {
        if (string_arg.GetShortName() == short_name) {
            return const_cast<StringArgument*>(&string_arg);
        }
    }
    return nullptr;
}

StringArgument *ArgParser::FindStringArgument(const std::string& name) const {
    for (const auto &string_arg : string_args_) {
        if (string_arg.GetName() == name) {
            return const_cast<StringArgument*>(&string_arg);
        }
    }
    return nullptr;
}

IntArgument *ArgParser::FindIntArgument(char short_name) const {
    for (const auto &int_arg : int_args_) {
        if (int_arg.GetShortName() == short_name) {
            return const_cast<IntArgument*>(&int_arg);
        }
    }
    return nullptr;
}

IntArgument *ArgParser::FindIntArgument(const std::string& name) const {
    for (const auto &int_arg : int_args_) {
        if (int_arg.GetName() == name) {
            return const_cast<IntArgument*>(&int_arg);
        }
    }
    return nullptr;
}

Flag *ArgParser::FindFlag(char short_name) const {
    for (const auto &flag : flags_) {
        if (flag.GetShortName() == short_name) {
            return const_cast<Flag*>(&flag);
        }
    }
    return nullptr;
}

Flag *ArgParser::FindFlag(const std::string& name) const {
    for (const auto &flag : flags_) {
        if (flag.GetName() == name) {
            return const_cast<Flag*>(&flag);
        }
    }
    return nullptr;
}


bool ArgParser::ParseFlags(const std::string& arg) {
    for (size_t i = 1; i < arg.size(); i++) { // Begin after '-'
        Flag* flag = FindFlag(arg[i]);
        if (flag == nullptr) {
            return false;
        }
        flag->PutValue(true);
    }
    return true;
}

StringArgument& ArgParser::AddStringArgument(const std::string& name, const std::string& description) {
    string_args_.emplace_back(name, description);
    StringArgument* arg = &string_args_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

StringArgument& ArgParser::AddStringArgument(const char& short_name, const std::string& name, const std::string& description) {
    string_args_.emplace_back(name, description, short_name);
    StringArgument* arg = &string_args_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

IntArgument& ArgParser::AddIntArgument(const std::string& name, const std::string& description) {
    int_args_.emplace_back(name, description);
    IntArgument* arg = &int_args_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

IntArgument& ArgParser::AddIntArgument(const char& short_name, const std::string& name, const std::string& description) {
    int_args_.emplace_back(name, description, short_name);
    IntArgument* arg = &int_args_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

Flag& ArgParser::AddFlag(const std::string& name, const std::string& description) {
    flags_.emplace_back(name, description);
    Flag* arg = &flags_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

Flag& ArgParser::AddFlag(const char& short_name, const std::string& name, const std::string& description) {
    flags_.emplace_back(name, description, short_name);
    Flag* arg = &flags_.back();
    //arguments_.push_back(arg);
    arg_names_.push_back(name);
    return *arg;
}

bool ArgParser::CheckFilledArgs() {
    for (StringArgument arg : string_args_) {
        if (!arg.CheckFilled()) {
            return false;
        }
    }
    for (IntArgument arg : int_args_) {
        if (!arg.CheckFilled()) {
            return false;
        }
    }
    return true;
}

bool ArgParser::Parse(int argc, char **argv) {
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++) {
        args[i] = argv[i];
    }
    return Parse(args);
}

