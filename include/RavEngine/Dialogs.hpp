#pragma once
#include <string>

namespace RavEngine{
    namespace Dialog{
        enum class MessageBoxType{
            Info,
            Warning,
            Error
        };
        void ShowBasic(const std::string& title, const std::string& msg, MessageBoxType type);
    }
}
