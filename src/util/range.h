#pragma once

#include <concepts>
#include <iterator>

template <std::integral T>
class RangeIterator {
public:
    using value_type = T;
    using iterator_concept = std::input_iterator_tag;

    class Sentinel {
    public:
        constexpr Sentinel(T value) noexcept
            : value_(value)
        {
        }

        constexpr T value() const noexcept
        {
            return this->value_;
        }

    private:
        T value_;
    };

    class SentinelInclusive {
    public:
        constexpr SentinelInclusive(T value) noexcept
            : value_(value)
        {
        }

        constexpr T value() const noexcept
        {
            return this->value_;
        }

    private:
        T value_;
    };

    constexpr RangeIterator(T value) noexcept
        : value(value)
    {
    }

    constexpr bool operator!=(const RangeIterator &other) const noexcept
    {
        return this->value != other.value;
    }

    constexpr T operator*() const noexcept
    {
        return this->value;
    }

    constexpr RangeIterator &operator++() noexcept
    {
        ++this->value;
        return *this;
    }

    constexpr bool is_exceeding(const Sentinel &sentinel) const noexcept
    {
        return this->value >= sentinel.value();
    }

    constexpr bool is_exceeding(const SentinelInclusive &sentinel) const noexcept
    {
        return this->value > sentinel.value();
    }

private:
    T value;
};

template <std::integral T>
using RangeIteratorSentinel = RangeIterator<T>::Sentinel;

template <std::integral T>
using RangeIteratorSentinelInclusive = RangeIterator<T>::SentinelInclusive;

template <std::integral T>
constexpr bool operator==(const RangeIterator<T> &it, const RangeIteratorSentinel<T> &sentinel) noexcept
{
    return it.is_exceeding(sentinel);
}

template <std::integral T>
constexpr bool operator==(const RangeIteratorSentinel<T> &sentinel, const RangeIterator<T> &it) noexcept
{
    return it != sentinel;
}

template <std::integral T>
constexpr bool operator==(const RangeIterator<T> &it, const RangeIteratorSentinelInclusive<T> &sentinel) noexcept
{
    return it.is_exceeding(sentinel);
}

template <std::integral T>
constexpr bool operator==(const RangeIteratorSentinelInclusive<T> &sentinel, const RangeIterator<T> &it) noexcept
{
    return it != sentinel;
}

template <std::integral T>
class Range {
public:
    using iterator = RangeIterator<T>;
    using iterator_sentinel = RangeIteratorSentinel<T>;

    constexpr Range(T min, T max) noexcept
        : min_(min)
        , max_(max)
    {
    }

    constexpr auto begin() const noexcept
    {
        return iterator(this->min_);
    }

    constexpr auto end() const noexcept
    {
        return iterator_sentinel(this->max_);
    }

    constexpr T min() const noexcept
    {
        return this->min_;
    }

    constexpr T max() const noexcept
    {
        return this->max_;
    }

    constexpr bool contains(T value) const noexcept
    {
        return this->min_ <= value && value < this->max_;
    }

private:
    T min_;
    T max_;
};

template <std::integral T>
class RangeInclusive {
public:
    using iterator = RangeIterator<T>;
    using iterator_sentinel = RangeIteratorSentinelInclusive<T>;

    constexpr RangeInclusive(T min, T max) noexcept
        : min_(min)
        , max_(max)
    {
    }

    constexpr auto begin() const noexcept
    {
        return iterator(this->min_);
    }

    constexpr auto end() const noexcept
    {
        return iterator_sentinel(this->max_);
    }

    constexpr T min() const noexcept
    {
        return this->min_;
    }

    constexpr T max() const noexcept
    {
        return this->max_;
    }

    constexpr bool contains(T value) const noexcept
    {
        return this->min_ <= value && value <= this->max_;
    }

private:
    T min_;
    T max_;
};
