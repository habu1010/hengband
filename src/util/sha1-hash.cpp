#include "util/sha1-hash.h"

#include <fstream>
#include <sstream>

namespace util {

namespace {
    constexpr uint32_t sha1_circular_shift(uint32_t bits, uint32_t word)
    {
        return (word << bits) | (word >> (32 - bits));
    }

    constexpr std::array<uint32_t, SHA1Hash::HASH_SIZE / 4> INITIAL_INTERMEDIATE_HASH{ {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0,
    } };
}

struct SHA1Hash::Impl {
    void process_message_block();
    void pad_message();

    std::array<uint32_t, SHA1Hash::HASH_SIZE / 4> intermediate_hash_ = INITIAL_INTERMEDIATE_HASH;
    uint32_t length_low_ = 0;
    uint32_t length_high_ = 0;
    int_least16_t message_block_index_ = 0;
    std::array<uint8_t, 64> message_block_{};

    bool computed = false;
    bool corrupted = false;
};

SHA1Hash::SHA1Hash()
    : pimpl_(std::make_unique<Impl>())
{
}

void SHA1Hash::reset()
{
    pimpl_->intermediate_hash_ = INITIAL_INTERMEDIATE_HASH;
    pimpl_->length_low_ = 0;
    pimpl_->length_high_ = 0;
    pimpl_->message_block_index_ = 0;
    pimpl_->computed = false;
    pimpl_->corrupted = false;
}

void SHA1Hash::input(const uint8_t *message_array, size_t length)
{
    if (length == 0) {
        return;
    }

    if (pimpl_->computed || pimpl_->corrupted) {
        pimpl_->corrupted = true;
        return;
    }

    while (length-- && !pimpl_->corrupted) {
        pimpl_->message_block_[pimpl_->message_block_index_++] = (*message_array & 0xFF);
        pimpl_->length_low_ += 8;
        // pimpl_->length_low &= 0xFFFFFFFF;
        if (pimpl_->length_low_ == 0) {
            pimpl_->length_high_++;
            // pimpl_->length_high &= 0xFFFFFFFF;
            if (pimpl_->length_high_ == 0) {
                pimpl_->corrupted = true;
            }
        }

        if (pimpl_->message_block_index_ == 64) {
            pimpl_->process_message_block();
        }

        message_array++;
    }
}

bool SHA1Hash::result(SHA1HashResult &message_digest_array)
{
    if (pimpl_->corrupted) {
        return false;
    }

    if (!pimpl_->computed) {
        pimpl_->pad_message();
        pimpl_->message_block_.fill(0);
        pimpl_->length_low_ = 0;
        pimpl_->length_high_ = 0;
        pimpl_->computed = true;
    }

    for (size_t i = 0; i < message_digest_array.size(); ++i) {
        message_digest_array[i] = static_cast<uint8_t>((pimpl_->intermediate_hash_[i >> 2] >> 8 * (3 - (i & 0x03))) & 0xFF);
    }

    return true;
}

void SHA1Hash::Impl::process_message_block()
{
    constexpr std::array<uint32_t, 4> k{ {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6,
    } };

    std::array<uint32_t, 80> w;

    for (auto i = 0; i < 16; ++i) {
        w[i] = (message_block_[i * 4] << 24) |
               (message_block_[i * 4 + 1] << 16) |
               (message_block_[i * 4 + 2] << 8) |
               (message_block_[i * 4 + 3]);
    }

    for (auto i = 16; i < 80; ++i) {
        w[i] = sha1_circular_shift(1, w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);
    }

    auto a = intermediate_hash_[0];
    auto b = intermediate_hash_[1];
    auto c = intermediate_hash_[2];
    auto d = intermediate_hash_[3];
    auto e = intermediate_hash_[4];

    auto compute = [&a, &b, &c, &d, &e](uint32_t f, uint32_t w_i, uint32_t k_i) {
        const auto temp = sha1_circular_shift(5, a) + f + e + k_i + w_i;
        e = d;
        d = c;
        c = sha1_circular_shift(30, b);
        b = a;
        a = temp;
    };

    for (auto i = 0; i < 20; ++i) {
        compute((b & c) | ((~b) & d), w[i], k[0]);
    }

    for (auto i = 20; i < 40; ++i) {
        compute(b ^ c ^ d, w[i], k[1]);
    }

    for (auto i = 40; i < 60; ++i) {
        compute((b & c) | (b & d) | (c & d), w[i], k[2]);
    }

    for (auto i = 60; i < 80; ++i) {
        compute(b ^ c ^ d, w[i], k[3]);
    }

    intermediate_hash_[0] += a;
    intermediate_hash_[1] += b;
    intermediate_hash_[2] += c;
    intermediate_hash_[3] += d;
    intermediate_hash_[4] += e;

    message_block_index_ = 0;
}

void SHA1Hash::Impl::pad_message()
{
    if (message_block_index_ > 55) {
        message_block_[message_block_index_++] = 0x80;
        while (message_block_index_ < 64) {
            message_block_[message_block_index_++] = 0;
        }

        process_message_block();

        while (message_block_index_ < 56) {
            message_block_[message_block_index_++] = 0;
        }
    } else {
        message_block_[message_block_index_++] = 0x80;
        while (message_block_index_ < 56) {
            message_block_[message_block_index_++] = 0;
        }
    }

    message_block_[56] = (length_high_ >> 24) & 0xFF;
    message_block_[57] = (length_high_ >> 16) & 0xFF;
    message_block_[58] = (length_high_ >> 8) & 0xFF;
    message_block_[59] = (length_high_)&0xFF;
    message_block_[60] = (length_low_ >> 24) & 0xFF;
    message_block_[61] = (length_low_ >> 16) & 0xFF;
    message_block_[62] = (length_low_ >> 8) & 0xFF;
    message_block_[63] = (length_low_)&0xFF;

    process_message_block();
}

std::string sha1_hash(const std::string &str)
{
    SHA1Hash sha1_hash;
    sha1_hash.input(reinterpret_cast<const uint8_t *>(str.data()), str.size());
    SHA1HashResult result{};
    sha1_hash.result(result);

    std::stringstream ss;
    for (const auto &byte : result) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

std::string sha1_hash(const std::filesystem::path &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file");
    }

    SHA1Hash sha1_hash;
    std::array<uint8_t, 1024> buffer{};
    while (ifs) {
        ifs.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        sha1_hash.input(buffer.data(), ifs.gcount());
    }

    SHA1HashResult result{};
    sha1_hash.result(result);
    
}

void test_sha1_hash()
{
    constexpr std::array<uint8_t, 3> message{ { 'a', 'b', 'c' } };
    constexpr std::array<uint8_t, 20> expected{ {
        0xA9,
        0x99,
        0x3E,
        0x36,
        0x47,
        0x06,
        0x81,
        0x6A,
        0xBA,
        0x3E,
        0x25,
        0x71,
        0x78,
        0x50,
        0xC2,
        0x6C,
        0x9C,
        0xD0,
        0xD8,
        0x9D,
    } };

    util::SHA1Hash sha1_hash;
    sha1_hash.input(message.data(), message.size());
    std::array<uint8_t, 20> result{};
    sha1_hash.result(result);

    if (result != expected) {
        throw std::runtime_error("SHA1Hash test failed");
    }

    std::cout << "SHA1Hash test passed" << std::endl;
}

int main()
{
    test_sha1_hash();
    return 0;
}
