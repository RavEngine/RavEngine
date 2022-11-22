
// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#ifndef AudioSetting_h
#define AudioSetting_h

#include "LabSound/core/AudioBus.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lab
{
// value defaults to zero, assumed set as integer.
// floatAssigned() is provided so that a user interface or RPC binding can configure
// automatically for float or unsigned integer values.
//
// a value changed callback is provided so that if tools set values, they can
// be responded to.
class AudioSetting
{
public:
    enum class Type
    {
        None,
        Bool,
        Integer,
        Float,
        Enumeration,
        Bus
    };

private:
    std::string _name;
    std::string _shortName;

    Type _type;
    float _valf = 0;
    uint32_t _vali = 0;
    bool _valb = false;
    std::shared_ptr<AudioBus> _valBus;

    std::function<void()> _valueChanged;
    char const * const * _enums = nullptr;

public:
    explicit AudioSetting(const std::string & n, const std::string & sn, Type t)
        : _name(n)
        , _shortName(sn)
        , _type(t)
    {
    }

    explicit AudioSetting(char const * const n, char const * const sn, Type t)
        : _name(n)
        , _shortName(sn)
        , _type(t)
    {
    }

    explicit AudioSetting(const std::string & n, const std::string & sn, char const * const * enums)
        : _name(n)
        , _shortName(sn)
        , _type(Type::Enumeration)
        , _enums(enums)
    {
    }

    explicit AudioSetting(char const * const n, char const * const sn, char const * const * enums)
        : _name(n)
        , _shortName(sn)
        , _type(Type::Enumeration)
        , _enums(enums)
    {
    }

    std::string name() const { return _name; }
    std::string shortName() const { return _shortName; }
    Type type() const { return _type; }

    char const * const * enums() const { return _enums; }
    int enumFromName(char const* const e)
    {
        if (!_enums || !e)
            return -1;

        int enum_idx = 0;
        for (char const* const* names_p = _enums; *names_p != nullptr; ++names_p, ++enum_idx)
        {
            if (!strcmp(e, *names_p))
                return enum_idx;
        }
        return -1;
    }

    bool valueBool() const { return _valb; }
    float valueFloat() const { return _valf; }
    uint32_t valueUint32() const { return _vali; }
    std::shared_ptr<AudioBus> valueBus() const { return _valBus; }

    void setBool(bool v, bool notify = true)
    {
        if (v == _valb) return;
        _valb = v;
        if (notify && _valueChanged) _valueChanged();
    }

    void setFloat(float v, bool notify = true)
    {
        if (v == _valf) return;
        _valf = v;
        if (notify && _valueChanged) _valueChanged();
    }

    void setUint32(uint32_t v, bool notify = true)
    {
        if (v == _vali) return;
        _vali = v;
        if (notify && _valueChanged) _valueChanged();
    }

    void setEnumeration(int v, bool notify = true)
    {
        if (v == _vali) return;
        if (v < 0) return;
        _vali = static_cast<int>(v);
        if (notify && _valueChanged) _valueChanged();
    }

    void setEnumeration(char const*const v, bool notify = true)
    {
        if (v) setEnumeration(enumFromName(v), notify);
    }

    void setString(char const*const, bool notify = true)
    {
        if (notify && _valueChanged) _valueChanged();
    }

    // nb: Invoking setBus will create and cache a duplicate of the supplied bus.
    void setBus(const AudioBus * incoming, bool notify = true)
    {
        std::unique_ptr<AudioBus> new_bus = AudioBus::createByCloning(incoming);
        _valBus = std::move(new_bus);
        if (notify && _valueChanged)
            _valueChanged();
    }

    void setValueChanged(std::function<void()> fn) { _valueChanged = fn; }
};

}  // namespace lab

#endif
