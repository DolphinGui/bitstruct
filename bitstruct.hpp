#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_BITSTRUCT
#define BITSTRUCT_PUBLIC __declspec(dllexport)
#else
#define BITSTRUCT_PUBLIC __declspec(dllimport)
#endif
#else
#ifdef BUILDING_BITSTRUCT
#define BITSTRUCT_PUBLIC __attribute__((visibility("default")))
#else
#define BITSTRUCT_PUBLIC
#endif
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
} // namespace impl

template <size_t s> struct BITSTRUCT_PUBLIC Bitstruct {
  template <size_t begin, size_t extent, typename data_type = uint8_t>
  struct Bitref {
    static_assert(std::is_trivial<data_type>::value,
                  "Data type must be trivial");
    using Word = impl::Bytes<sizeof(data_type)>;
    constexpr static auto bitsize = sizeof(Word) * 8;
    constexpr Bitref(Word &w) : data(w){};
    Word &data;

    BITSTRUCT_CONSTEXPR Bitref &operator=(data_type i) noexcept {
      static_assert(!std::is_const<data_type>::value,
                    "cannot assign to const value");
      auto &w = reinterpret_cast<Word &>(i);
      w <<= bitsize - extent;
      w >>= bitsize - extent;
      reinterpret_cast<Word &>(data) |= w << begin;
      return *this;
    }

    BITSTRUCT_CONSTEXPR operator data_type() const noexcept {
      Word mask = static_cast<Word>(0xFFFFFFFFFFFFFFFF);
      mask <<= bitsize - extent - begin;
      mask >>= bitsize - extent - begin;
      mask >>= begin;
      return data_type((static_cast<Word>(data) >> begin) & mask);
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

#undef BITSTRUCT_PUBLIC
#undef BITSTRUCT_CONSTEXPR