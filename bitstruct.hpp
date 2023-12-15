#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#ifndef NDEBUG
#include <stdexcept>
#endif

#if __cplusplus > 201103L
#define BITSTRUCT_CONSTEXPR constexpr
#else
#define BITSTRUCT_CONSTEXPR
#endif

#if __has_cpp_attribute(nodiscard)
#define BITSTRUCT_NODISCARD [[nodiscard]]
#else
#define BITSTRUCT_NODISCARD
#endif

#define BITSTRUCT_FUNCTION BITSTRUCT_NODISCARD BITSTRUCT_CONSTEXPR

#if __cplusplus >= 202002L
#include <span>
#endif

#define BITSTRUCT_FIELD(name, index, extent)                                   \
  Bitref<index % 8, extent> name() noexcept { return get<index, extent>(); }   \
  Bitref<index % 8, extent, const uint8_t> name() const noexcept {             \
    return get<index, extent>();                                               \
  }

#define BITSTRUCT_FIELD_T(name, index, extent, type)                           \
  Bitref<index % 8, extent, type> name() noexcept {                            \
    return get<index, extent, type>();                                         \
  }                                                                            \
  Bitref<index % 8, extent, type const> name() const noexcept {                \
    return get<index, extent, type>();                                         \
  }

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
#ifndef NDEBUG
constexpr bool throwing = true;
#else
constexpr bool throwing = false;
#endif
constexpr size_t div_ceil(size_t a, size_t d) noexcept {
  if (a % d == 0)
    return a / d;
  return a / d + 1;
};
} // namespace impl

template <size_t bitlength> struct Bitstruct {
  template <size_t begin, size_t extent, typename data_type = uint8_t>
  struct Bitref {
    using Word = impl::Bytes<sizeof(data_type)>;

    constexpr Bitref(Word &w) : data(w){};
    Word &data;

    BITSTRUCT_CONSTEXPR Bitref &
    operator=(data_type i) noexcept(!impl::throwing) {
      static_assert(not std::is_const<data_type>::value,
                    "cannot assign to const value");
      auto w = Word(i);
#ifndef NDEBUG
      size_t max = size_t(1) << extent;
      if (w >= max) {
        throw std::runtime_error("Data type i is out of bounds");
      }
#endif
      w = impl::truncate<sizeof(Word) * 8 - extent>(w) << begin;
      auto mask =
          impl::truncate<sizeof(Word) * 8 - extent>(Word(0xFFFFFFFFFFFFFFFF));
      mask <<= begin;
      mask = ~mask;
      data &= mask;
      data |= w;

      return *this;
    }

    BITSTRUCT_FUNCTION operator data_type() const noexcept {
      Word d = data;
      d = impl::truncate<sizeof(Word) * 8 - (extent + begin)>(d);
      d >>= begin;
      return data_type(d);
    }

    static_assert(std::is_trivial<data_type>::value,
                  "Data type must be trivial");
  };

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_FUNCTION Bitref<bit % 8, extent, T> get() noexcept {
    static_assert(sizeof(T) <= sizeof(_data), "Data type is too small");
    static_assert(bit < sizeof(_data) * 8, "Bit index is out of bounds");
    static_assert(bit + extent <= sizeof(_data) * 8,
                  "Bit extent is out of bounds");
    static_assert((bit / 8) % alignof(T) == 0, "Access to T is not aligned");

    using Word = impl::Bytes<sizeof(T)>;
    return Bitref<bit % 8, extent, T>(reinterpret_cast<Word &>(_data[bit / 8]));
  }

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_FUNCTION Bitref<bit % 8, extent, const T> get() const noexcept {
    return const_cast<Bitstruct &>(*this).get<bit, extent, const T>();
  }

  BITSTRUCT_FUNCTION uint8_t *data() noexcept { return _data.data(); }
  BITSTRUCT_FUNCTION const uint8_t *data() const noexcept {
    return _data.data();
  }
  constexpr size_t size() const noexcept { return _data.size(); };

#ifdef __cpp_lib_span

  template <typename Char = uint8_t>
  BITSTRUCT_FUNCTION std::span<Char> data_span() noexcept {
    return std::span<Char>(reinterpret_cast<Char *>(_data.data()),
                           _data.size());
  }
  template <typename Char = uint8_t>
  BITSTRUCT_FUNCTION std::span<const Char> data_span() const noexcept {
    return std::span<const Char>(reinterpret_cast<const Char *>(_data.data()),
                                 _data.size());
  }

#endif

private:
  std::array<uint8_t, bitlength / 8> _data{};
};

} // namespace bit

#undef BITSTRUCT_FUNCTION
#undef BITSTRUCT_NODISCARD
#undef BITSTRUCT_CONSTEXPR