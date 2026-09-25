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
#include <fstream>

class TestInput : public InputSource {
public:
    TestInput(const std::string& text) : text_(text) {}

    std::string read_line() override {
        return text_;
    }

    bool is_eof() const override {
        return false;
    }

private:
    std::string text_;
};

class TestOutput : public OutputSink {
public:
    void write(std::string_view text) override {
        output_ += text;
    }

    const std::string& output() const {
        return output_;
    }

private:
    std::string output_;
};


void test_empty_conversation() {
    Conversation c;

    assert(c.size() == 0);
    assert(c.begin() == c.end());
}


void test_append_and_access() {
    Conversation c;

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    assert(c.size() == 2);
    assert(c.at(0).role() == Role::User);
    assert(c.at(0).content() == "hello");
    assert(c.at(1).role() == Role::Assistant);
    assert(c.at(1).content() == "hi");
}


void test_copy_constructor() {
    Conversation c;

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    Conversation copy(c);

    assert(copy.size() == c.size());
    assert(copy.begin() != c.begin());
    assert(copy.at(0).content() == "hello");
    assert(copy.at(1).content() == "hi");
}


void test_move_constructor() {
    Conversation c;

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    const Message* old_address = c.begin();

    Conversation moved(static_cast<Conversation&&>(c));

    assert(moved.begin() == old_address);
    assert(moved.size() == 2);
    assert(c.size() == 0);
    assert(c.begin() == nullptr);
}


void test_copy_assignment() {
    Conversation c;

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    Conversation copyAssigned;
    copyAssigned.append(Message(Role::System, "old"));

    copyAssigned = c;

    assert(copyAssigned.size() == c.size());
    assert(copyAssigned.begin() != c.begin());
    assert(copyAssigned.at(0).content() == "hello");
    assert(copyAssigned.at(1).content() == "hi");
}


void test_move_assignment() {
    Conversation c;

    c.append(Message(Role::User, "hello"));
    c.append(Message(Role::Assistant, "hi"));

    Conversation moveAssigned;
    moveAssigned.append(Message(Role::System, "old"));

    const Message* address_before_move = c.begin();

    moveAssigned = static_cast<Conversation&&>(c);

    assert(moveAssigned.begin() == address_before_move);
    assert(moveAssigned.size() == 2);
    assert(c.size() == 0);
    assert(c.begin() == nullptr);
}


void test_bounds() {
    Conversation c;

    bool threw = false;

    try {
        c.at(100);
    }
    catch (...) {
        threw = true;
    }

    assert(threw);
}


void test_growth() {
    Conversation growing;

    for (int i = 0; i < 100; i++) {
        growing.append(Message(Role::User, "test"));
    }

    assert(growing.size() == 100);

    for (std::size_t i = 0; i < growing.size(); i++) {
        assert(growing.at(i).content() == "test");
    }
}


void test_scanner_clean_text() {
    SentinelScanner scanner("<|end_conversation|>");

    auto out1 = scanner.feed("Hello there!");
    auto out2 = scanner.flush();

    assert(out1.sentinel_found == false);
    assert(out2.sentinel_found == false);
    assert(out1.safe_text + out2.safe_text == "Hello there!");
}


void test_scanner_whole_sentinel() {
    SentinelScanner scanner("<|end_conversation|>");

    auto out = scanner.feed("Goodbye.<|end_conversation|>");

    assert(out.sentinel_found == true);
    assert(out.safe_text == "Goodbye.");
}


void test_scanner_split_sentinel() {
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "Goodbye." + sentinel;

    for (std::size_t split = 0; split <= text.size(); split++) {
        SentinelScanner scanner(sentinel);

        auto out1 = scanner.feed(text.substr(0, split));
        auto out2 = scanner.feed(text.substr(split));

        assert(out1.sentinel_found || out2.sentinel_found);
        assert(out1.safe_text + out2.safe_text == "Goodbye.");
    }
}


void test_scanner_false_alarm() {
    SentinelScanner scanner("<|end_conversation|>");

    auto out1 = scanner.feed("Hello <|end_world|>");
    auto out2 = scanner.flush();

    assert(out1.sentinel_found == false);
    assert(out2.sentinel_found == false);
    assert(out1.safe_text + out2.safe_text ==
           "Hello <|end_world|>");
}


void test_scanner_one_character_at_a_time() {
    SentinelScanner scanner("<|end_conversation|>");

    std::string safe;
    bool found = false;
    std::string text = "Goodbye.<|end_conversation|>";

    for (char ch : text) {
        std::string oneChar(1, ch);

        auto out = scanner.feed(oneChar);

        safe += out.safe_text;

        if (out.sentinel_found) {
            found = true;
            break;
        }
    }

    assert(found);
    assert(safe == "Goodbye.");
}


void test_scanner_stress() {
    SentinelScanner scanner("<|end_conversation|>");

    std::string safe;

    for (int i = 0; i < 100000; i++) {
        auto out = scanner.feed("<");

        assert(!out.sentinel_found);
        safe += out.safe_text;
    }

    auto finalOut = scanner.flush();
    safe += finalOut.safe_text;

    assert(safe.size() == 100000);
}


void test_harness_turn_limit() {
    auto model =
        std::make_unique<ScriptedModelClient>("scripts/greeting.script");

    HarnessConfig config;
    config.max_turns = 1;
    config.system_message = model->system_message();

    Harness harness(std::move(model), config);

    TestInput input("hello");
    TestOutput output;

    StopReason result = harness.run(input, output);

    assert(result.kind == StopReason::Kind::TurnLimit);

    const Conversation& conv = harness.conversation();

    assert(conv.size() == 3);
    assert(conv.at(0).role() == Role::System);
    assert(conv.at(0).content() == "Be concise.");
    assert(conv.at(1).role() == Role::User);
    assert(conv.at(1).content() == "hello");
    assert(conv.at(2).role() == Role::Assistant);
}


void test_harness_sentinel() {
    auto model =
        std::make_unique<ScriptedModelClient>("scripts/greeting.script");

    HarnessConfig config;
    config.max_turns = 10;
    config.system_message = model->system_message();

    Harness harness(std::move(model), config);

    TestInput input("hello");
    TestOutput output;

    StopReason result = harness.run(input, output);

    assert(result.kind == StopReason::Kind::Sentinel);

    const Conversation& conv = harness.conversation();

    assert(conv.size() == 7);
    assert(conv.at(6).role() == Role::Assistant);
    assert(conv.at(6).content() ==
    "Goodbye!<|end_conversation|>");

    assert(output.output().find("<|end_conversation|>") ==
    std::string::npos);
}

void test_transcript_round_trip() {
    const std::string path = "test_transcript.txt";

    std::ofstream file(path);

    file << "role: system\n";
    file << "Be concise.\n";
    file << "---\n";
    file << "role: user\n";
    file << "hello\n";
    file << "---\n";
    file << "role: assistant\n";
    file << "Hi there!\n";
    file << "---\n";
    file << "role: user\n";
    file << "goodbye\n";
    file << "---\n";
    file << "role: assistant\n";
    file << "Goodbye!<|end_conversation|>\n";

    file.close();

    ReplayModelClient replay(path);

    assert(replay.system_message() == "Be concise.");

    Conversation conv;
    conv.append(Message(Role::System, "Be concise."));
    conv.append(Message(Role::User, "hello"));

    Message first = replay.generate(conv);

    assert(first.role() == Role::Assistant);
    assert(first.content() == "Hi there!");

    conv.append(first);
    conv.append(Message(Role::User, "goodbye"));

    Message second = replay.generate(conv);

    assert(second.role() == Role::Assistant);
    assert(second.content() ==
    "Goodbye!<|end_conversation|>");
}

int main() {
    test_empty_conversation();
    test_append_and_access();
    test_copy_constructor();
    test_move_constructor();
    test_copy_assignment();
    test_move_assignment();
    test_bounds();
    test_growth();
    test_scanner_clean_text();
    test_scanner_whole_sentinel();
    test_scanner_split_sentinel();
    test_scanner_false_alarm();
    test_scanner_one_character_at_a_time();
    test_scanner_stress();


    test_harness_turn_limit();
    test_harness_sentinel();
    test_transcript_round_trip();

    return 0;
}