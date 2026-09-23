#include "core/conversation.h"

Conversation::Conversation() {
}

Conversation::~Conversation() {
    delete[] data_;
}

std::size_t Conversation::size() const noexcept {
    return size_;
}