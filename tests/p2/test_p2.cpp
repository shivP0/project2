// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>

int main() {
    Conversation c;

    assert(c.size() == 0);
    assert(c.begin() == c.end());

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    assert(c.size() == 2);
    assert(c.at(0).role() == Role::User);
    assert(c.at(0).content() == "hello");
    assert(c.at(1).role() == Role::Assistant);
    assert(c.at(1).content() == "hi");

    Conversation copy(c);

    assert(copy.size() == c.size());
    assert(copy.begin() != c.begin());
    assert(copy.at(0).content() == "hello");
    assert(copy.at(1).content() == "hi");

    const Message* old_address = copy.begin();

    Conversation moved(static_cast<Conversation&&>(copy));

    assert(moved.begin() == old_address);
    assert(moved.size() == 2);
    assert(copy.size() == 0);
    assert(copy.begin() == nullptr);

    return 0;
}