#pragma once
#include "Argument.h"

namespace ArgumentParser {

    class ArgParser {
    public:
        explicit ArgParser(std::string  name);

        bool Parse(const std::vector<std::string>& args);
        bool Parse(int argc, char** argv);

        std::string GetStringValue(const char&, const size_t& idx = 0) const;
        std::string GetStringValue(const std::string& name, const size_t& idx = 0) const;
        int GetIntValue(const char& short_name, const size_t& idx = 0) const;
        int GetIntValue(const std::string& name, const size_t& idx = 0) const;
        bool GetFlag(const char& short_name) const;
        bool GetFlag(const std::string& name) const;

        std::string HelpDescription() const;

        Flag& AddFlag(const char& short_name, const std::string& name, const std::string& description = "");
        Flag& AddFlag(const std::string& name, const std::string& description = "");

        IntArgument& AddIntArgument(const char& short_name, const std::string& name, const std::string& description = "");
        IntArgument& AddIntArgument(const std::string& name, const std::string& description = "");

        StringArgument& AddStringArgument(const char& short_name, const std::string& name, const std::string& description = "");
        StringArgument& AddStringArgument(const std::string& name, const std::string& description = "");

        void AddHelp(const char& short_name, const std::string& name, const std::string& help = "");
        bool Help();

    private:
        std::string name_; // Название парсера
        std::vector<Flag> flags_{};
        std::vector<IntArgument> int_args_{};
        std::vector<StringArgument> string_args_{};
        //std::vector<Argument*> arguments_{}; // Вектор всех аргументов
        std::vector<std::string> arg_names_{};
        std::string help_string_; // Описание программы
        Flag* help_argument_ = nullptr; // Флаг, отвечающий за help
        ArgParser() = default;

        StringArgument* FindStringArgument(char short_name) const;
        StringArgument* FindStringArgument(const std::string& name) const;

        IntArgument* FindIntArgument(char short_name) const;
        IntArgument* FindIntArgument(const std::string& name) const;

        Flag* FindFlag(char short_name) const;
        Flag* FindFlag(const std::string& name) const;

        bool ParseLongOption(const std::string& arg) const;

        bool FillPositional(const std::string& value, int& begin);
        bool ParseFlags(const std::string& arg);
        bool CheckFilledArgs();
        //StringArgument* positional_string_;
    };

} // namespace ArgumentParser