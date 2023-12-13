#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#ifndef NDEBUG
#include <stdexcept>
#define THROWING true
#else
#define THROWING false
#endif

#if __cplusplus > 201103
#define BITSTRUCT_CONSTEXPR constexpr
#else
#define BITSTRUCT_CONSTEXPR
#endif

namespace bit {
namespace impl {
template <size_t size> struct bytes_t;
template <> struct bytes_t<1> {
  using type = uint8_t;
};
template <> struct bytes_t<2> {
  using type = uint16_t;
};
template <> struct bytes_t<4> {
  using type = uint32_t;
};
template <> struct bytes_t<8> {
  using type = uint64_t;
};
template <size_t size> using Bytes = typename bytes_t<size>::type;
template <size_t MSB_trunc, typename T> constexpr T truncate(T num) noexcept {
  num <<= MSB_trunc;
  num >>= MSB_trunc;
  return num;
}
} // namespace impl

template <size_t s> struct Bitstruct {
  template <size_t begin, size_t extent, typename data_type = uint8_t>
  struct Bitref {
    static_assert(std::is_trivial<data_type>::value,
                  "Data type must be trivial");
    using Word = impl::Bytes<sizeof(data_type)>;
    constexpr static auto bitsize = sizeof(Word) * 8;
    constexpr Bitref(Word &w) : data(w){};
    Word &data;

    BITSTRUCT_CONSTEXPR Bitref &operator=(data_type i) noexcept(!THROWING) {
      static_assert(!std::is_const<data_type>::value,
                    "cannot assign to const value");
      auto &w = reinterpret_cast<Word &>(i);
#ifndef NDEBUG
      auto max = 1 << extent;
      if (w >= max) {
        throw std::runtime_error("Data type i is out of bounds");
      }
#endif
      w = impl::truncate<sizeof(Word) * 8 - extent>(w);
      auto mask =
          impl::truncate<sizeof(Word) * 8 - extent>(Word(0xFFFFFFFFFFFFFFFF));
      mask <<= begin;
      mask = ~mask;
      // zeros out bits about to be overwritten
      data &= mask;
      data |= w << begin;
      return *this;
    }

    BITSTRUCT_CONSTEXPR operator data_type() const noexcept {
      Word d = data;
      d = impl::truncate<sizeof(Word) * 8 - (extent + begin)>(d);
      d >>= begin;
      return data_type(d);
    }
  };

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_CONSTEXPR Bitref<bit, extent, T> get() noexcept {
    static_assert(sizeof(T) <= sizeof(data), "Data type is too small");
    static_assert(bit < sizeof(data) * 8, "Bit index is out of bounds");
    static_assert(bit + extent <= sizeof(data) * 8,
                  "Bit extent is out of bounds");
    using Word = impl::Bytes<sizeof(T)>;
    return Bitref<bit, extent, T>(reinterpret_cast<Word &>(data[bit / 8]));
  }
  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_CONSTEXPR Bitref<bit, extent, const T> get() const noexcept {
    return get<bit, extent, const T>();
  }

private:
  std::array<uint8_t, s / 8> data{};
};

} // namespace bit

#undef BITSTRUCT_CONSTEXPR