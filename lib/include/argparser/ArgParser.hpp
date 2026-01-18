#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <vector>
#include <string>
#include <unordered_map>

namespace ArgumentParser {

enum class ArgTypeName {
    kStringType,
    kIntType,
    kBoolType,
    kNull
};

class ArgParser;

template <typename ArgType>
class ArgValue;

template <typename ArgType>
class ValCollection {
    friend class ArgValue<ArgType>;
    friend class ArgParser;

    public:
        void StoreValues(std::vector<ArgType>& ref) {
            save_to_ = &ref;
        }

        ValCollection<ArgType>& Positional() {
            positional_ = true;
            return *this;
        }

    private:
        std::vector<ArgType> values_;
        std::vector<ArgType>* save_to_ = nullptr;
        size_t min_args_count_ = 0;
        bool positional_ = false;
        bool created_ = false;
};

template <typename ArgType>
class ArgValue {
    friend class ArgParser;

    public:
        void Default(const ArgType& def_val) {
            SetVal(def_val);
        }

        void StoreValue(ArgType& ref) {
            save_to_ = &ref;
        }

        ValCollection<ArgType>& MultiValue(size_t min_args_count) {
            multivalue_.min_args_count_ = min_args_count;
            multivalue_.created_ = true;
            return multivalue_;
        }

        ValCollection<ArgType>& MultiValue() {
            multivalue_.created_ = true;
            return multivalue_;
        }

    private:
        ArgType value_;
        ArgType* save_to_ = nullptr;
        ValCollection<ArgType> multivalue_;
        bool value_set_ = false;
        char short_name_ = 0;
        std::string_view description_;

        void SetVal(const ArgType& value) {
            if (multivalue_.created_) {
                multivalue_.values_.push_back(value);
            } else {
                value_ = value;
            }
            value_set_ = true;
        }

        void SetAdditionalTraits(char short_name, std::string_view description) {
            short_name_ = short_name;
            description_ = description;
        }
};

/* Класс ArgParser
    - Изменение строк, переданных в качества имён/описания аргументов, ведёт к некорректной работе парсера.
    Предпочтительный способ задания имён - передача строкового литерала
    - Все добавленные параметры (помимо флагов) должны быть заданы пользователем, иначе парсинг неудачен.
    - Добавление более 1 позиционного аргумента приводит к некорректной работе парсера
*/
class ArgParser {
    public:
        ArgParser(std::string_view name);

        ArgValue<std::string>& AddStringArgument(std::string_view arg_name);
        ArgValue<int>& AddIntArgument(std::string_view arg_name);
        ArgValue<bool>& AddFlag(std::string_view arg_name);

        ArgValue<std::string>& AddStringArgument(char short_name, std::string_view arg_name);
        ArgValue<int>& AddIntArgument(char short_name, std::string_view arg_name);
        ArgValue<bool>& AddFlag(char short_name, std::string_view arg_name);

        ArgValue<std::string>& AddStringArgument(std::string_view arg_name, 
                                                std::string_view arg_description);
        ArgValue<int>& AddIntArgument(std::string_view arg_name, std::string_view arg_description);
        ArgValue<bool>& AddFlag(std::string_view arg_name, std::string_view arg_description);

        ArgValue<std::string>& AddStringArgument(char short_name, std::string_view arg_name, 
                                                 std::string_view arg_description);
        ArgValue<int>& AddIntArgument(char short_name, std::string_view arg_name, 
                                      std::string_view arg_description);
        ArgValue<bool>& AddFlag(char short_name, std::string_view arg_name, 
                                std::string_view arg_description);

        std::string GetStringValue(std::string_view key);
        int GetIntValue(std::string_view key);
        bool GetFlag(std::string_view key);

        std::string GetStringValue(std::string_view key, size_t i);
        int GetIntValue(std::string_view key, size_t i);

        bool Parse(const std::vector<std::string>& args);
        bool Parse(int argc, char** argv);

        void AddHelp(char short_name, std::string_view help_name, std::string_view description);
        bool Help() const;
        std::string_view HelpDescription() const;

    private:

        class ArgContainer{
            public:
                void CreateArg(
                    std::string_view name, 
                    char short_name, 
                    std::string_view arg_description, 
                    ArgTypeName arg_type
                );

                ArgTypeName GetType(std::string_view name) const;
                ArgTypeName GetType(char short_name) const;

                ArgValue<std::string>& GetStrValue(std::string_view name);
                ArgValue<std::string>& GetStrValue(char short_name);

                ArgValue<int>& GetIntValue(std::string_view name);
                ArgValue<int>& GetIntValue(char short_name);

                ArgValue<bool>& GetBoolValue(std::string_view name);
                ArgValue<bool>& GetBoolValue(char short_name);

                std::string_view FindPositional() const;

                bool StoreAndCheck() const;

                std::string GetArgsInfo() const;

            private:
                std::unordered_map<std::string_view, ArgValue<std::string>> string_args_;
                std::unordered_map<std::string_view, ArgValue<int>> int_args_;
                std::unordered_map<std::string_view, ArgValue<bool>> flags_;
                std::unordered_map<char, std::string_view> arg_names_;
        };

        struct Helper{
            bool required = false;
            std::string_view arg;
            char short_arg;
            std::string help_info;
        };

        std::string_view name_;
        std::string_view description_;
        ArgContainer args_;
        Helper helper_;

        bool ParseShortArg(std::string_view short_arg);
        bool ParseNamedArg(std::string_view arg);
        bool ParsePositionalArg(std::string_view positional, std::string_view value);
        void SetHelpInfo();
};

} // namespace ArgumentParser

#endif  // ARGPARSER_HPP
