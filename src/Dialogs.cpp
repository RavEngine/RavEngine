#include "Dialogs.hpp"
#include "App.hpp"
#include <SDL_messagebox.h>

using namespace std;
using namespace RavEngine;

static inline auto MessageBoxToSDLFlag(Dialog::MessageBoxType type){
    switch (type) {
        case Dialog::MessageBoxType::Info:
            return SDL_MESSAGEBOX_INFORMATION;
        case Dialog::MessageBoxType::Warning:
            return SDL_MESSAGEBOX_WARNING;
        case Dialog::MessageBoxType::Error:
            return SDL_MESSAGEBOX_ERROR;
        default:
            Debug::Fatal("Invalid MessageBoxType {}", type);
    }
}

void Dialog::ShowBasic(const std::string& title, const std::string& msg, MessageBoxType type){
    SDL_ShowSimpleMessageBox(MessageBoxToSDLFlag(type), title.c_str(), msg.c_str(), nullptr);
}
