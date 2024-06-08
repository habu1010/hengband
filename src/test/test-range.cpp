#include "util/range.h"
#include <cassert>
#include <iostream>

template <typename T>
void print_range(const T &range)
{
    for (auto i : range) {
        std::cout << i << std::endl;
    }
}

int main()
{
    // Rangeクラスのテスト
    Range<int> range(0, 5);
    int expected_value = 0;
    for (auto i : range) {
        if (i != expected_value) {
            std::cout << "Range test failed: expected " << expected_value << ", got " << i << std::endl;
            return 1;
        }
        ++expected_value;
        std::cout << i << std::endl;
    }
    print_range(Range(1, 100));
    constexpr auto range2 = Range(1, 100);
    constexpr auto range2_end_it = range2.end();
    assert(!range.contains(-1));
    assert(range.contains(0));
    assert(range.contains(1));
    assert(range.contains(2));
    assert(range.contains(3));
    assert(range.contains(4));
    assert(!range.contains(5));

    // RangeInclusiveクラスのテスト
    RangeInclusive<int> range_inclusive(0, 5);
    expected_value = 0;
    for (auto i : range_inclusive) {
        if (i != expected_value) {
            std::cout << "RangeInclusive test failed: expected " << expected_value << ", got " << i << std::endl;
            return 1;
        }
        ++expected_value;
        std::cout << i << std::endl;
    }
    assert(!range_inclusive.contains(-1));
    assert(range_inclusive.contains(0));
    assert(range_inclusive.contains(1));
    assert(range_inclusive.contains(2));
    assert(range_inclusive.contains(3));
    assert(range_inclusive.contains(4));
    assert(range_inclusive.contains(5));
    assert(!range_inclusive.contains(6));

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
