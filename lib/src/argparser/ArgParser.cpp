#include <charconv>

#include "ArgParser.hpp"

namespace ArgumentParser {

ArgParser::ArgParser(std::string_view name) {
    name_ = name;
}

ArgValue<std::string>& ArgParser::AddStringArgument(std::string_view arg_name) {
    args_.CreateArg(arg_name, 0, std::string_view{}, ArgTypeName::kStringType);
    return args_.GetStrValue(arg_name);
}

ArgValue<int>& ArgParser::AddIntArgument(std::string_view arg_name) {
    args_.CreateArg(arg_name, 0, std::string_view{}, ArgTypeName::kIntType);
    return args_.GetIntValue(arg_name);
}

ArgValue<bool>& ArgParser::AddFlag(std::string_view arg_name) {
    args_.CreateArg(arg_name, 0, std::string_view{}, ArgTypeName::kBoolType);
    return args_.GetBoolValue(arg_name);
}


ArgValue<std::string>& ArgParser::AddStringArgument(char short_name, std::string_view arg_name) {
    args_.CreateArg(arg_name, short_name, std::string_view{}, ArgTypeName::kStringType);
    return args_.GetStrValue(arg_name);
}

ArgValue<int>& ArgParser::AddIntArgument(char short_name, std::string_view arg_name) {
    args_.CreateArg(arg_name, short_name, std::string_view{}, ArgTypeName::kIntType);
    return args_.GetIntValue(arg_name);
}

ArgValue<bool>& ArgParser::AddFlag(char short_name, std::string_view arg_name) {
    args_.CreateArg(arg_name, short_name, std::string_view{}, ArgTypeName::kBoolType);
    return args_.GetBoolValue(arg_name);
}


ArgValue<std::string>& ArgParser::AddStringArgument(std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, 0, arg_description, ArgTypeName::kStringType);
    return args_.GetStrValue(arg_name);
}

ArgValue<int>& ArgParser::AddIntArgument(std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, 0, arg_description, ArgTypeName::kIntType);
    return args_.GetIntValue(arg_name);
}

ArgValue<bool>& ArgParser::AddFlag(std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, 0, arg_description, ArgTypeName::kBoolType);
    return args_.GetBoolValue(arg_name);
}


ArgValue<std::string>& ArgParser::AddStringArgument(char short_name, std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, short_name, arg_description, ArgTypeName::kStringType);
    return args_.GetStrValue(arg_name);
}

ArgValue<int>& ArgParser::AddIntArgument(char short_name, std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, short_name, arg_description, ArgTypeName::kIntType);
    return args_.GetIntValue(arg_name);
}

ArgValue<bool>& ArgParser::AddFlag(char short_name, std::string_view arg_name, std::string_view arg_description) {
    args_.CreateArg(arg_name, short_name, arg_description, ArgTypeName::kBoolType);
    return args_.GetBoolValue(arg_name);
}


std::string ArgParser::GetStringValue(std::string_view key) {
    return args_.GetStrValue(key).value_;
}

int ArgParser::GetIntValue(std::string_view key) {
    return args_.GetIntValue(key).value_;
}

bool ArgParser::GetFlag(std::string_view key) {
    return args_.GetBoolValue(key).value_;
}

std::string ArgParser::GetStringValue(std::string_view key, size_t i) {
    return args_.GetStrValue(key).multivalue_.values_[i];
}

int ArgParser::GetIntValue(std::string_view key, size_t i) {
    return args_.GetIntValue(key).multivalue_.values_[i];
}

static const std::string_view bool_arg_true{"true"};
static const std::string_view bool_arg_false{"false"};

bool ArgParser::ParseShortArg(std::string_view short_arg) {
    size_t border = short_arg.find("=");
    if (border == std::string_view::npos) {
        border = short_arg.size();
    }
    if (border == 0 || border == short_arg.size() - 1) {
        return false;
    }
    std::string_view short_name = short_arg.substr(0, border);
    if (short_name.size() == short_arg.size()) {
        if (short_name.size() == 1 && short_name[0] == helper_.short_arg) {
            helper_.required = true;
            return true;
        }
        for (size_t i = 0; i < short_name.size(); ++i) {
            if (args_.GetType(short_name[i]) != ArgTypeName::kBoolType) {
                return false;
            }
            args_.GetBoolValue(short_name[i]).SetVal(true);
        }
        return true;
    }

    if (short_name.size() != 1) {
        return false;
    }
    std::string_view value = short_arg.substr(border + 1);
    switch (args_.GetType(short_name[0])) {
        case ArgTypeName::kStringType:
            args_.GetStrValue(short_name[0]).SetVal(std::string{value});
            return true;
        case ArgTypeName::kIntType:
        {
            int converted;
            std::from_chars_result conversion_result = std::from_chars(
                value.data(), value.data() + value.size(), converted
            );
            if (conversion_result.ec == std::errc::invalid_argument || 
                conversion_result.ec == std::errc::result_out_of_range) {
                return false;
            }
            args_.GetIntValue(short_name[0]).SetVal(converted);
        }
            return true;
        case ArgTypeName::kBoolType:
            if (value == bool_arg_true) {
                args_.GetBoolValue(short_name[0]).SetVal(true);
                return true;
            }
            if (value == bool_arg_false) {
                args_.GetBoolValue(short_name[0]).SetVal(false);
                return true;
            }

            return false;
    }

    return false;
}

bool ArgParser::ParseNamedArg(std::string_view arg) {
    size_t border = arg.find("=");
    if (border == std::string_view::npos) {
        border = arg.size();
    }
    if (border == 0 || border == arg.size() - 1) {
        return false;
    }
    std::string_view name = arg.substr(0, border);
    if (name.size() == arg.size()) {
        if (name == helper_.arg) {
            helper_.required = true;
            return true;
        }
        if (args_.GetType(name) != ArgTypeName::kBoolType) {
            return false;
        }
        args_.GetBoolValue(name).SetVal(true);

        return true;
    }

    std::string_view value = arg.substr(border + 1);
    switch (args_.GetType(name)) {
        case ArgTypeName::kStringType:
            args_.GetStrValue(name).SetVal(std::string{value});
            return true;
        case ArgTypeName::kIntType:
        {
            int converted;
            std::from_chars_result conversion_result = std::from_chars(
                value.data(), value.data() + value.size(), converted
            );
            if (conversion_result.ec == std::errc::invalid_argument || 
                conversion_result.ec == std::errc::result_out_of_range) {
                return false;
            }
            args_.GetIntValue(name).SetVal(converted);
        }
            return true;
        case ArgTypeName::kBoolType:
            if (value == bool_arg_true) {
                args_.GetBoolValue(name).SetVal(true);
                return true;
            }
            if (value == bool_arg_false) {
                args_.GetBoolValue(name).SetVal(false);
                return true;
            }

            return false;
    }

    return false;
}

bool ArgParser::ParsePositionalArg(std::string_view positional_name, std::string_view value) {
    if (positional_name.empty()) {
        return false;
    }

    switch (args_.GetType(positional_name)) {
        case ArgTypeName::kStringType:
            args_.GetStrValue(positional_name).SetVal(std::string{value});
            return true;
        case ArgTypeName::kIntType:
        {
            int converted;
            std::from_chars_result conversion_result = std::from_chars(
                value.data(), value.data() + value.size(), converted
            );
            if (conversion_result.ec == std::errc::invalid_argument || 
                conversion_result.ec == std::errc::result_out_of_range) {
                return false;
            }
            args_.GetIntValue(positional_name).SetVal(converted);
        }
            return true;
        case ArgTypeName::kBoolType:
            if (value == "true") {
                args_.GetBoolValue(positional_name).SetVal(true);
                return true;
            }
            if (value == "false") {
                args_.GetBoolValue(positional_name).SetVal(false);
                return true;
            }

            return false;
    }

    return true;
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
    SetHelpInfo();
    std::string_view positional = args_.FindPositional();
    for (size_t i = 1; i < args.size() && !helper_.required; ++i) {
        std::string_view cur_arg{args[i]};
        if (cur_arg.empty()) {
            // std::cerr << "Error: empty arg\n";
            return false;
        }
        if (cur_arg[0] == '-') {
            if (cur_arg.size() == 1) {
                // std::cerr << "Error: \'-\' arg\n";
                return false;
            }
            if (cur_arg[1] != '-') {
                if (!ParseShortArg(cur_arg.substr(1))) {
                    // std::cerr << "Error: invalid short arg\n";
                    return false;
                }
                continue;
            }
            if (cur_arg.size() == 2) {
                // std::cerr << "Error: \'--\' arg\n";
                return false;
            }
            if (!ParseNamedArg(cur_arg.substr(2))) {
                // std::cerr << "Error: invalid arg\n";
                return false;
            }
            continue;
        }

        if (!ParsePositionalArg(positional, cur_arg)) {
            // std::cerr << "Error: invalid positional arg\n";
            return false;
        }
    }

    if (helper_.required) {
        return true;
    }
    return args_.StoreAndCheck();
}

bool ArgParser::Parse(int argc, char** argv) {
    std::vector<std::string> args(argc);
    for (size_t i = 0; i < argc; ++i) {
        args[i] = std::string{argv[i]};
    }
    return Parse(args);
}

void ArgParser::AddHelp(char short_help_name, std::string_view help_name, std::string_view description) {
    helper_.short_arg = short_help_name;
    helper_.arg = help_name;
    description_ = description;
}

void ArgParser::SetHelpInfo() {
    std::string info = (std::string{name_} + '\n' + std::string{description_} + '\n');
    info += '\n';
    info += (args_.GetArgsInfo() + '\n');
    info += (std::string{'-'} + helper_.short_arg + std::string{",\t--"} + 
                        std::string{helper_.arg} + std::string{",\tDisplay this help and exit\n"});
    helper_.help_info = info;
}

bool ArgParser::Help() const { 
    return helper_.required;
}

std::string_view ArgParser::HelpDescription() const {
    return helper_.help_info;
}

} // namespace ArgumentParser