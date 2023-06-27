#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace util {

using SHA1HashResult = std::array<uint8_t, SHA1Hash::HASH_SIZE>;

class SHA1Hash {

public:
    static constexpr auto HASH_SIZE = 20;

    SHA1Hash();

    void reset();
    void input(const uint8_t *message_array, size_t length);
    bool result(SHA1HashResult &message_digest_array);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

SHA1HashResult sha1_hash(const std::string &str);
SHA1HashResult sha1_hash(const std::filesystem::path &path);
std::string to_string(const SHA1HashResult &hash);

}
