#include "Argument.h"
#include <stdexcept>
#include <sstream>

using namespace ArgumentParser;

Argument::Argument(std::string  name, std::string  description = "", const char& short_name = kDefaultShortName)
    : name_(std::move(name)),
      short_name_(short_name),
      description_(std::move(description)) {}


std::string Argument::GetName() const {
    return name_;
}

char Argument::GetShortName() const {
    return short_name_;
}

bool Argument::IsPositional() const {
    return is_positional_;
}

Argument& Argument::Positional() {
    is_positional_ = true;
    return *this;
}

std::string IntArgument::GetHelp() const {
    std::stringstream help_description;
    if (short_name_ != kDefaultShortName) {
        help_description << "-" << short_name_ << ",  ";
    } else {
        help_description << "     ";
    }
    help_description << "--" << name_ << "=<int>,  ";
    help_description << description_;
    if (is_defaulted_) {
        help_description << " [default = " << default_value_ << "]";
    }
    help_description << '\n';
    return help_description.str();
}

std::string Flag::GetHelp() const {
    std::stringstream help_description;
    if (short_name_ != kDefaultShortName) {
        help_description << "-" << short_name_ << ",  ";
    } else {
        help_description << "     ";
    }
    help_description << "--" << name_ << "  ";
    help_description << description_;
    if (!default_value_) {
        help_description << " [default = false]";
    } else {
        help_description << " [default = true]";
    }
    help_description << '\n';
    return help_description.str();
}

std::string StringArgument::GetHelp() const {
    std::stringstream help_description;
    if (short_name_ != kDefaultShortName) {
        help_description << "-" << short_name_ << ",  ";
    } else {
        help_description << "     ";
    }
    help_description << "--" << name_ << "=<string>,  ";
    help_description << description_;
    if (is_defaulted_) {
        help_description << " [default = " << default_value_ << "]";
    }
    help_description << '\n';
    return help_description.str();
}

StringArgument::StringArgument() = default;

StringArgument::StringArgument(const std::string& name, const std::string& description, const char &short_name)
    : Argument(name, description, short_name) {}

std::string StringArgument::GetValue(size_t idx) const {
    if (!is_stored_) {
        if (idx >= values_.size()) {
            throw std::range_error("Invalid index of int argument value is given");
        }
        return values_[idx];
    }
    if (!is_multi_value_) {
        return *stored_value_;
    }
    if (idx >= stored_values_->size()) {
        throw std::range_error("Invalid index of int argument value is given");
    }
    return (*stored_values_)[idx];
}

int IntArgument::GetValue(size_t idx) const {
    if (!is_stored_) {
        if (idx >= values_.size()) {
            throw std::range_error("Invalid index of int argument value is given");
        }
        return values_[idx];
    }
    if (!is_multi_value_) {
        return *stored_value_;
    }
    if (idx >= stored_values_->size()) {
        throw std::range_error("Invalid index of int argument value is given");
    }
    return (*stored_values_)[idx];
}

Flag::Flag(const std::string& name, const std::string& description, const char& short_name)
    : Argument(name, description, short_name) {}

bool Flag::GetValue() const {
    if (is_stored_) {
        return *stored_value_;
    }
    if (is_defaulted_) {
        return default_value_;
    }
    return value_;
}

Flag& Flag::StoreValue(bool& value) {
    stored_value_ = &value;
    is_stored_ = true;
    return *this;
}

Flag& Flag::PutValue(bool value) {
    if (is_stored_) {
        *stored_value_ = value;
    } else {
        value_ = value;
    }
    return *this;
}

Flag& Flag::Default(const bool& value) {
    default_value_ = value;
    PutValue(value);
    return *this;
}

IntArgument::IntArgument(const std::string &name, const std::string &description, const char &short_name)
    : Argument(name, description, short_name) {}

IntArgument& IntArgument::StoreValue(int& value) {
    if (is_multi_value_) {
        throw std::logic_error("Cannot store value for multi value IntArgument");
    }
    stored_value_ = &value;
    is_stored_ = true;
    return *this;
}

IntArgument& IntArgument::StoreValues(std::vector<int>& values){
    if (!is_multi_value_) {
        throw std::logic_error("Cannot store multi value for single value IntArgument");
    }
    stored_values_ = &values;
    is_stored_ = true;
    return *this;
}

IntArgument& IntArgument::MultiValue(int min_args_count) {
    min_args_count_ = min_args_count;
    is_multi_value_ = true;
    return *this;
}

IntArgument& IntArgument::Positional() {
    is_positional_ = true;
    return *this;
}
IntArgument& IntArgument::PutValue(int value) {
    if (is_stored_) {
        if (is_multi_value_) {
            stored_values_->push_back(value);
        } else {
            *stored_value_ = value;
        }
    } else {
        if (is_multi_value_ || values_.empty()) {
            values_.push_back(value);
        } else {
            values_[0] = value;
        }
    }
    return *this;
}

IntArgument& IntArgument::Default(int default_value) {
    is_defaulted_ = true;
    default_value_ = default_value;
    PutValue(default_value);
    return *this;
}

StringArgument& StringArgument::Positional() {
    is_positional_ = true;
    return *this;
}

StringArgument& StringArgument::PutValue(const std::string& value) {
    if (is_stored_) {
        if (is_multi_value_) {
            stored_values_->push_back(value);
        } else {
            *stored_value_ = value;
        }
    } else {
        if (is_multi_value_ || values_.empty()) {
            values_.push_back(value);
        } else {
            values_[0] = value;
        }
    }
    return *this;
}
StringArgument& StringArgument::Default(const std::string& default_value) {
    default_value_ = default_value;
    is_defaulted_ = true;
    PutValue(default_value);
    return *this;
}

StringArgument& StringArgument::StoreValue(std::string& value) {
    if (is_multi_value_) {
        throw std::logic_error("Cannot store value for multi value StringArgument");
    }
    stored_value_ = &value;
    is_stored_ = true;
    return *this;
}

StringArgument& StringArgument::StoreValues(std::vector<std::string>& values) {
    if (!is_multi_value_) {
        throw std::logic_error("Cannot store multi value for single value StringArgument");
    }
    stored_values_ = &values;
    is_stored_ = true;
    return *this;
}

StringArgument& StringArgument::MultiValue(int min_args_count) {
    min_args_count_ = min_args_count;
    is_multi_value_ = true;
    return *this;
}

bool StringArgument::CheckFilled() {
    if (is_stored_) {
        if (is_multi_value_) {
            return (stored_values_->size() >= min_args_count_);
        }
        return true;
    }
    return values_.size() >= min_args_count_;
}

bool StringArgument::IsMultiValue() const {
    return is_multi_value_;
}

bool IntArgument::CheckFilled() {
    if (is_stored_) {
        if (is_multi_value_) {
            return (stored_values_->size() >= min_args_count_);
        }
        return true;
    }
    return values_.size() >= min_args_count_;
}

bool IntArgument::IsMultiValue() const {
    return is_multi_value_;
}
