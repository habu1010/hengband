/*!
 * @brief BGM・効果音の設定ファイル (.cfg) の読み込みのテスト
 */

#include "main/cfg-reader.h"
#include "test/temporary-json-files.h"

#include <doctest/doctest.h>

#include <map>
#include <string>
#include <utility>

namespace {

constexpr auto TEST_TYPE = 7;

//! 2 つまでのキーを持つセクションを作る
cfg_section make_section(bool *has_data = nullptr)
{
    return { "Sound", TEST_TYPE, [](int index) -> tl::optional<std::string> {
                if (index >= 2) {
                    return tl::nullopt;
                }
                return index == 0 ? "first" : "second";
            },
        has_data };
}

//! 決まった値を返す CfgValueGetter を作る
CfgValueGetter make_getter(std::map<std::pair<std::string, std::string>, std::string> values)
{
    return [values = std::move(values)](const std::string &section, const std::string &key) {
        const auto it = values.find({ section, key });
        return it == values.end() ? std::string() : it->second;
    };
}

}

TEST_CASE("CfgReader keeps only files that exist in the directory")
{
    test::TemporaryJsonFiles files;
    files.write_raw("a.wav", "");
    files.write_raw("b.wav", "");
    const CfgReader reader(files.directory, { "sound.cfg" });

    auto has_data = false;
    const auto data = reader.read_sections({ make_section(&has_data) },
        make_getter({ { { "Sound", "first" }, " missing.wav\ta.wav  " }, { { "Sound", "second" }, "missing.wav" } }));

    CHECK(has_data);
    REQUIRE(data.has_key(TEST_TYPE, 0));
    CHECK(data.get_rand(TEST_TYPE, 0) == "a.wav");
    CHECK_FALSE(data.has_key(TEST_TYPE, 1));
    CHECK_FALSE(data.get_rand(TEST_TYPE, 1).has_value());
}

TEST_CASE("CfgReader chooses among the listed files")
{
    test::TemporaryJsonFiles files;
    files.write_raw("a.wav", "");
    files.write_raw("b.wav", "");
    const CfgReader reader(files.directory, { "sound.cfg" });

    const auto data = reader.read_sections({ make_section() }, make_getter({ { { "Sound", "second" }, "a.wav b.wav" } }));

    CHECK_FALSE(data.has_key(TEST_TYPE, 0));
    REQUIRE(data.has_key(TEST_TYPE, 1));
    auto count_a = 0;
    auto count_b = 0;
    for (auto i = 0; i < 64; ++i) {
        const auto filename = data.get_rand(TEST_TYPE, 1);
        REQUIRE(filename.has_value());
        if (*filename == "a.wav") {
            ++count_a;
        } else if (*filename == "b.wav") {
            ++count_b;
        }
    }

    CHECK(count_a + count_b == 64);
    CHECK(count_a > 0);
    CHECK(count_b > 0);
}

TEST_CASE("CfgReader does not set has_data without any file")
{
    test::TemporaryJsonFiles files;
    const CfgReader reader(files.directory, { "sound.cfg" });

    auto has_data = false;
    const auto data = reader.read_sections({ make_section(&has_data) }, make_getter({ { { "Sound", "first" }, "missing.wav" } }));

    CHECK_FALSE(has_data);
    CHECK_FALSE(data.has_key(TEST_TYPE, 0));
}

TEST_CASE("CfgReader uses the first cfg file that exists")
{
    test::TemporaryJsonFiles files;
    files.write_raw("sound.cfg", "");

    const CfgReader reader(files.directory, { "sound_debug.cfg", "sound.cfg" });
    CHECK(reader.get_cfg_path() == files.directory / "sound.cfg");

    files.write_raw("sound_debug.cfg", "");
    const CfgReader debug_reader(files.directory, { "sound_debug.cfg", "sound.cfg" });
    CHECK(debug_reader.get_cfg_path() == files.directory / "sound_debug.cfg");
}
