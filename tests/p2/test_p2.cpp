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


    
    Conversation copyAssigned;
    copyAssigned.append(Message(Role::System, "old"));

    copyAssigned = c;

    assert(copyAssigned.size() == c.size());
    assert(copyAssigned.begin() != c.begin());
    assert(copyAssigned.at(0).content() == "hello");
    assert(copyAssigned.at(1).content() == "hi");

   
    Conversation moveAssigned;
    moveAssigned.append(Message(Role::System, "old"));

    const Message* address_before_move = copyAssigned.begin();

    moveAssigned = static_cast<Conversation&&>(copyAssigned);

    assert(moveAssigned.begin() == address_before_move);
    assert(moveAssigned.size() == 2);
    assert(copyAssigned.size() == 0);
    assert(copyAssigned.begin() == nullptr);

   
    bool threw = false;

    try {
        c.at(100);
    }
    catch (...) {
        threw = true;
    }

    assert(threw);

   
    Conversation growing;

    for (int i = 0; i < 100; i++) {
        growing.append(Message(Role::User, "test"));
    }

    assert(growing.size() == 100);

    for (std::size_t i = 0; i < growing.size(); i++) {
        assert(growing.at(i).content() == "test");
    }


    SentinelScanner scanner1("<|end_conversation|>");

    auto clean1 = scanner1.feed("Hello there!");
    auto clean2 = scanner1.flush();

    assert(clean1.sentinel_found == false);
    assert(clean2.sentinel_found == false);
    assert(clean1.safe_text + clean2.safe_text == "Hello there!");


    SentinelScanner scanner2("<|end_conversation|>");

    auto whole = scanner2.feed("Goodbye.<|end_conversation|>");

    assert(whole.sentinel_found == true);
    assert(whole.safe_text == "Goodbye.");


    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "Goodbye." + sentinel;

    for (std::size_t split = 0; split <= text.size(); split++) {
        SentinelScanner scanner(sentinel);

        auto out1 = scanner.feed(text.substr(0, split));
        auto out2 = scanner.feed(text.substr(split));

        assert(out1.sentinel_found || out2.sentinel_found);
        assert(out1.safe_text + out2.safe_text == "Goodbye.");
    }


    SentinelScanner scanner3("<|end_conversation|>");

    auto false1 = scanner3.feed("Hello <|end_world|>");
    auto false2 = scanner3.flush();

    assert(false1.sentinel_found == false);
    assert(false2.sentinel_found == false);
    assert(false1.safe_text + false2.safe_text ==
           "Hello <|end_world|>");

    return 0;
}