#pragma once
#include <string>
#include <vector>

namespace ArgumentParser {
    const char kDefaultShortName = '\0';

    class Argument {
     public:
        Argument() = default;
        Argument(std::string  name, std::string description, const char& short_name);
        [[nodiscard]] virtual char GetShortName() const;
        [[nodiscard]] virtual std::string GetName() const;
        [[nodiscard]] virtual std::string GetHelp() const = 0;
        virtual Argument& Positional();
        [[nodiscard]] bool IsPositional() const;
     protected:
        std::string name_;
        char short_name_ = '\0';
        std::string description_;
        bool is_positional_ = false;
    };

    class StringArgument : public Argument {
     public:
        StringArgument();
        explicit StringArgument(const std::string& name, const std::string& description = "", const char& short_name = kDefaultShortName);
        [[nodiscard]] std::string GetValue(size_t idx = 0) const;
        [[nodiscard]] std::string GetHelp() const override;
        StringArgument& PutValue(const std::string& value);
        StringArgument& StoreValue(std::string& value);
        StringArgument& StoreValues(std::vector<std::string>& values);
        StringArgument& MultiValue(int min_args_count = 0);
        StringArgument& Positional() override;
        StringArgument& Default(const std::string& default_value);
        bool CheckFilled();
        bool IsMultiValue() const;
     private:
        std::string default_value_;
        std::string* stored_value_{};
        std::vector<std::string>* stored_values_{};
        std::vector<std::string> values_{};
        bool is_multi_value_ = false;
        bool is_stored_ = false;
        bool is_defaulted_ = false;
        int min_args_count_ = 1;
    };

    class IntArgument : public Argument {
     public:
        IntArgument() = default;
        explicit IntArgument(const std::string& name, const std::string& description = "", const char& short_name = kDefaultShortName);
        int GetValue(size_t idx = 0) const;
        std::string GetHelp() const override;
        IntArgument& PutValue(int value);
        IntArgument& StoreValue(int& value);
        IntArgument& StoreValues(std::vector<int>& values);
        IntArgument& MultiValue(int min_args_count = 0);
        IntArgument& Positional() override;
        IntArgument& Default(int default_value);
        bool CheckFilled();
        bool IsMultiValue() const;
     private:
        int default_value_;
        int* stored_value_{};
        std::vector<int>* stored_values_{};
        std::vector<int> values_{};
        bool is_stored_ = false;
        bool is_defaulted_ = false;
        bool is_multi_value_ = false;
        int min_args_count_ = 1;
    };

    class Flag : public Argument {
     public:
        Flag() = default;
        explicit Flag(const std::string& name, const std::string& description = "", const char& short_name = kDefaultShortName);
        bool GetValue() const;
        std::string GetHelp() const override;
        Flag& PutValue(bool value);
        Flag& StoreValue(bool& value);
        Flag& Default(const bool& value);
     private:
        bool is_stored_ = false;
        bool is_defaulted_ = false;
        bool default_value_ = false;
        bool value_ = false;
        bool* stored_value_ = nullptr;  // указатель на значение, которое нужно хранить
    };

} // ArgumentParser

