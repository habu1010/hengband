#include "io/tokenizer.h"
#include "system/h-basic.h"
#include "test/string-helpers.h"

#include <doctest/doctest.h>

#include <string>
#include <vector>

TEST_CASE("tokenize does not treat multibyte trail bytes as separators")
{
    const std::string input{ static_cast<char>(0xe3), static_cast<char>(0x81), static_cast<char>(0x82), ':', 't', 'a', 'i', 'l' };
    const std::string expected{ static_cast<char>(0xe3), static_cast<char>(0x81), static_cast<char>(0x82) };
    const auto tokens = tokenize(input, 2);

    REQUIRE(tokens.size() == 2);
    CHECK(tokens[0] == expected);
    CHECK(tokens[1] == "tail");
}

TEST_CASE("tokenize continues to split legacy field delimiters")
{
    const auto tokens = tokenize("one/two:three", 3);

    REQUIRE(tokens.size() == 3);
    CHECK(tokens[0] == "one");
    CHECK(tokens[1] == "two");
    CHECK(tokens[2] == "three");
}

TEST_CASE("tokenize preserves delimiters after incomplete multibyte characters")
{
    const std::vector<std::string> prefixes{ "\xe3", "\xe3\x81" };
    for (const auto &prefix : prefixes) {
        for (const auto delimiter : { ':', '/' }) {
            const auto tokens = tokenize(prefix + delimiter + "tail", 2);
            REQUIRE(tokens.size() == 2);
            CHECK(tokens[0] == prefix);
            CHECK(tokens[1] == "tail");
        }

        const auto tokens = tokenize(prefix, 2);
        REQUIRE(tokens.size() == 1);
        CHECK(tokens[0] == prefix);
    }
}

namespace {
constexpr std::string_view IDEOGRAPHIC_SPACE = "\xe3\x80\x80"; //!< 全角スペース (U+3000)
constexpr std::string_view HIRAGANA_A = "\xe3\x81\x82"; //!< あ
}

TEST_CASE("tokenize_whitespace splits on runs of ASCII whitespace")
{
    const auto tokens = tokenize_whitespace(" \tone  two\r\nthree\v\f", 16);

    REQUIRE(tokens.size() == 3);
    CHECK(tokens[0] == "one");
    CHECK(tokens[1] == "two");
    CHECK(tokens[2] == "three");
}

TEST_CASE("tokenize_whitespace returns no tokens for blank input")
{
    CHECK(tokenize_whitespace("", 16).empty());
    CHECK(tokenize_whitespace(" \t\r\n", 16).empty());
}

TEST_CASE("tokenize_whitespace drops tokens beyond the limit")
{
    const auto tokens = tokenize_whitespace("a b c", 2);

    REQUIRE(tokens.size() == 2);
    CHECK(tokens[0] == "a");
    CHECK(tokens[1] == "b");
    CHECK(tokenize_whitespace("a b", 0).empty());
}

TEST_CASE("tokenize_whitespace keeps multibyte characters intact")
{
    // U+3000 (全角スペース) は区切りにしない
    const auto first = test::cat(IDEOGRAPHIC_SPACE, "a");
    const auto input = test::cat(first, " ", HIRAGANA_A);
    const auto tokens = tokenize_whitespace(input, 16);

    REQUIRE(tokens.size() == 2);
    CHECK(tokens[0] == first);
    CHECK(tokens[1] == HIRAGANA_A);
}
