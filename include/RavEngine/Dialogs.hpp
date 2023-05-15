#pragma once
#include <string_view>

namespace RavEngine{
    namespace Dialog{
        enum class MessageBoxType{
            Info,
            Warning,
            Error
        };
        void ShowBasic(const std::string_view& title, const std::string_view& msg, MessageBoxType type);
    }
}
