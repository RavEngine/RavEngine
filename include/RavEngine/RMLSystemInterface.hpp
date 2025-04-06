#pragma once

#include <RmlUi/Core/SystemInterface.h>

namespace RavEngine {

struct RMLSystemInterface : public Rml::SystemInterface {
    double GetElapsedTime() final;
    void SetMouseCursor(const Rml::String& cursor_name) final;
    void SetClipboardText(const Rml::String& text) final;
    void GetClipboardText(Rml::String& text) final;
    void ActivateKeyboard(Rml::Vector2f caret_position, float line_height) final;
    void DeactivateKeyboard() final;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) final;
};

}
