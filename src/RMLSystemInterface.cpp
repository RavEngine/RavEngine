#if !RVE_SERVER
#include "RMLSystemInterface.hpp"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_clipboard.h>
#include "Window.hpp"
#include "VirtualFileSystem.hpp"
#include "App.hpp"

using namespace RavEngine;

void RMLSystemInterface::ActivateKeyboard(Rml::Vector2f caret_position, float line_height){
    const auto& mainWindow = GetApp()->GetMainWindow();
    const auto window = mainWindow->window;

#if __APPLE__
    // need to scale by window DPI. Values are in pixels but we need them in points.
    auto dpiScale = mainWindow->GetDPIScale();
    caret_position /= dpiScale;
    line_height /= dpiScale;
#endif
    
    const SDL_Rect rect{int(caret_position.x), int(caret_position.y), 1, int(line_height)};
    
    SDL_SetTextInputArea(window, &rect, 0);
    if (not SDL_TextInputActive(window)){
        SDL_StartTextInput(window);
    }
}

void RMLSystemInterface::DeactivateKeyboard(){
    const auto window = GetApp()->GetMainWindow()->window;
    SDL_StopTextInput(window);
}

bool RMLSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    switch (type) {
    case Rml::Log::Type::LT_ERROR:
    case Rml::Log::Type::LT_ASSERT:
        Debug::Fatal(message);
        break;
    default:
        Debug::Log(message);
        break;
    }

    return true;
}

void RMLSystemInterface::SetClipboardText(const Rml::String &text){
    SDL_SetClipboardText(text.c_str());
}

void RMLSystemInterface::GetClipboardText(Rml::String &text){
    auto data = SDL_GetClipboardText();
    text = data;        //avoid this copy?
    SDL_free(data);
}

double RMLSystemInterface::GetElapsedTime(){
    return GetApp()->GetCurrentTime();
}

void RMLSystemInterface::SetMouseCursor(const Rml::String &cursor_name){
    Debug::Fatal("Not implemented");
}

#endif
