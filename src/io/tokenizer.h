#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> tokenize(std::string_view buf, size_t num);
std::vector<std::string_view> tokenize_whitespace(std::string_view str, size_t num);
