// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "sfizz_message.h"

namespace sfz {

template <char Tag> struct OscDataTraits;
template <char Tag> using OscType = typename OscDataTraits<Tag>::type;
template <char Tag> using OscDecayedType = typename OscDataTraits<Tag>::decayed_type;

class Client {
public:
    explicit Client(void* data) : data_(data) {}
    void* getClientData() const { return data_; }
    void setReceiveCallback(sfizz_receive_t* receive) { receive_ = receive; }
    bool canReceive() const { return receive_ != nullptr; }
    void receive(int delay, const char* path, const char* sig, const sfizz_arg_t* args);

    template <char... Sig>
    void receive(int delay, const char* path, OscDecayedType<Sig>... values);

private:
    template <char Tag>
    sfizz_arg_t make_arg(OscDecayedType<Tag> value);
    void* data_ = nullptr;
    sfizz_receive_t* receive_ = nullptr;
};

} // namespace sfz

#include "Messaging.hpp"
