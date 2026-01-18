#include "ArgParser.hpp"

namespace ArgumentParser {

void ArgParser::ArgContainer::CreateArg(std::string_view name, char short_name, 
                                        std::string_view arg_description, ArgTypeName arg_type) {
    switch (arg_type) {
        case ArgTypeName::kStringType:
            string_args_.insert_or_assign(name, ArgValue<std::string>{});
            string_args_.at(name).SetAdditionalTraits(short_name, arg_description);
            break;
        case ArgTypeName::kIntType:
            int_args_.insert_or_assign(name, ArgValue<int>{});
            int_args_.at(name).SetAdditionalTraits(short_name, arg_description);
            break;
        case ArgTypeName::kBoolType:
            flags_.insert_or_assign(name, ArgValue<bool>{});
            flags_.at(name).SetAdditionalTraits(short_name, arg_description);
            flags_.at(name).SetVal(false);
    }
    if (short_name != 0) {
        arg_names_.insert_or_assign(short_name, std::string_view{name});
    }
}

ArgTypeName ArgParser::ArgContainer::GetType(std::string_view name) const {
    if (string_args_.find(name) != string_args_.end()) {
        return ArgTypeName::kStringType;
    } 
    if (int_args_.find(name) != int_args_.end()) {
        return ArgTypeName::kIntType;
    }
    if (flags_.find(name) != flags_.end()) {
        return ArgTypeName::kBoolType;
    }
    return ArgTypeName::kNull;
}

ArgTypeName ArgParser::ArgContainer::GetType(char short_name) const {
    if (arg_names_.find(short_name) == arg_names_.end()) {
        return ArgTypeName::kNull;
    }
    return ArgParser::ArgContainer::GetType(arg_names_.at(short_name));
}

ArgValue<std::string>& ArgParser::ArgContainer::GetStrValue(std::string_view name) {
    return string_args_.at(name);
}

ArgValue<std::string>& ArgParser::ArgContainer::GetStrValue(char short_name) {
    return string_args_.at(arg_names_.at(short_name));
}

ArgValue<int>& ArgParser::ArgContainer::GetIntValue(std::string_view name) {
    return int_args_.at(name);
}

ArgValue<int>& ArgParser::ArgContainer::GetIntValue(char short_name) {
    return int_args_.at(arg_names_.at(short_name));
}

ArgValue<bool>& ArgParser::ArgContainer::GetBoolValue(std::string_view name) {
    return flags_.at(name);
}

ArgValue<bool>& ArgParser::ArgContainer::GetBoolValue(char short_name) {
    return flags_.at(arg_names_.at(short_name));
}

std::string_view ArgParser::ArgContainer::FindPositional() const {
    for (const auto& [key, value] : string_args_) {
        if (value.multivalue_.positional_) {
            return key;
        }
    }
    for (const auto& [key, value] : int_args_) {
        if (value.multivalue_.positional_) {
            return key;
        }
    }
    for (const auto& [key, value] : flags_) {
        if (value.multivalue_.positional_) {
            return key;
        }
    }
    return std::string_view{};
}

bool ArgParser::ArgContainer::StoreAndCheck() const {
    bool exit_code = true;
    for (const auto& [key, value] : string_args_) {
        if (value.multivalue_.created_) {
            const auto& multivalue = value.multivalue_;
            if (multivalue.min_args_count_ > value.multivalue_.values_.size()) {
                return false;
            }
            if (value.multivalue_.save_to_) {
                *multivalue.save_to_ = multivalue.values_;
            }
            continue;
        }
        
        if (!value.value_set_) {
            exit_code = false;
        }
        if (value.save_to_) {
            *value.save_to_ = value.value_;
        }
    }

    for (const auto& [key, value] : int_args_) {
        if (value.multivalue_.created_) {
            const auto& multivalue = value.multivalue_;
            if (multivalue.min_args_count_ > multivalue.values_.size()) {
                exit_code = false;
            }
            if (multivalue.save_to_) {
                *multivalue.save_to_ = multivalue.values_;
            }
            continue;
        }
        
        if (!value.value_set_) {
            exit_code = false;
        }
        if (value.save_to_) {
            *value.save_to_ = value.value_;
        }
    }

    for (const auto& [key, value] : flags_) {
        if (value.multivalue_.created_) {
            const auto& multivalue = value.multivalue_;
            if (multivalue.min_args_count_ > multivalue.values_.size()) {
                exit_code = false;
            }
            if (multivalue.save_to_) {
                *multivalue.save_to_ = multivalue.values_;
            }
            continue;
        }
        
        if (value.save_to_) {
            *value.save_to_ = value.value_;
        }
    }

    return exit_code;
}

std::string ArgParser::ArgContainer::GetArgsInfo() const {
    std::string res;
    for (const auto& [key, value] : string_args_) {
        if (value.short_name_) {
            res += (std::string{"-"} + value.short_name_ + ',');
        }
        if (key[0] == '_')
            res += (std::string{"\t"} + "<string>");
        else
            res += (std::string{"\t--"} + std::string{key} + std::string{"=<string>"});
        if (!value.description_.empty()) {
            res += (std::string{",\t"} + std::string{value.description_});
        }
        if (value.value_set_) {
            res += (std::string(" [default = ") + value.value_ + ']');
        }
        if (value.multivalue_.created_) {
            res += ((std::string{" [repeated, min args = "}) + 
                     std::to_string(value.multivalue_.min_args_count_) + 
                     ']');
        }
        res += '\n';
    }

    for (const auto& [key, value] : int_args_) {
        if (value.short_name_) {
            res += (std::string{"-"} + value.short_name_ + ',');
        }
        res += (std::string{"\t--"} + std::string{key} + std::string{"=<int>"});
        if (!value.description_.empty()) {
            res += (std::string{",\t"} + std::string{value.description_});
        }
        if (value.value_set_) {
            res += (std::string(" [default = ") + std::to_string(value.value_) + ']');
        }
        if (value.multivalue_.created_) {
            res += ((std::string{" [repeated, min args = "}) + 
                     std::to_string(value.multivalue_.min_args_count_) + 
                     ']');
        }
        res += '\n';
    }

    for (const auto& [key, value] : flags_) {
        if (value.short_name_) {
            res += (std::string{"-"} + value.short_name_ + ',');
        }
        res += (std::string{"\t--"} + std::string{key});
        if (!value.description_.empty()) {
            res += (std::string{",\t"} + std::string{value.description_});
        }
        if (value.value_set_) {
            std::string default_val = (value.value_) ? std::string{"true"} : std::string{"false"};
            res += (std::string(" [default = ") + default_val + ']');
        }
        res += '\n';
    }

    return res;
}

} // namespace ArgumentParser