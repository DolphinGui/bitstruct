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

#if __cplusplus > 201103L
#define BITSTRUCT_CONSTEXPR constexpr
#if __cplusplus >= 202002L
#include <span>
#endif
#else
#define BITSTRUCT_CONSTEXPR
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
  Bitref<index % 8, extent, const type> name() const noexcept {                \
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
} // namespace impl

template <size_t bitlength> struct Bitstruct {
  template <size_t begin, size_t extent, typename data_type = uint8_t>
  struct Bitref {
    using Word = impl::Bytes<sizeof(data_type)>;

    constexpr Bitref(Word &w) : data(w){};
    Word &data;

    BITSTRUCT_CONSTEXPR Bitref &operator=(data_type i) noexcept(!THROWING) {
      static_assert(not std::is_const<data_type>::value,
                    "cannot assign to const value");
      auto w = Word(i);
#ifndef NDEBUG
      size_t max = size_t(1) << extent;
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

    static_assert(std::is_trivial<data_type>::value,
                  "Data type must be trivial");
  };

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_CONSTEXPR Bitref<bit % 8, extent, T> get() noexcept {
    static_assert(sizeof(T) <= sizeof(_data), "Data type is too small");
    static_assert(bit < sizeof(_data) * 8, "Bit index is out of bounds");
    static_assert(bit + extent <= sizeof(_data) * 8,
                  "Bit extent is out of bounds");
    using Word = impl::Bytes<sizeof(T)>;
    return Bitref<bit % 8, extent, T>(reinterpret_cast<Word &>(_data[bit / 8]));
  }

  template <typename T>
  using NonPtr = typename std::enable_if<!std::is_pointer<T>::value, T>::type;
  template <typename T>
  using Ptr = typename std::enable_if<std::is_pointer<T>::value, T>::type;

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_CONSTEXPR Bitref<bit % 8, extent, const NonPtr<T>>
  get() const noexcept {
    return const_cast<Bitstruct &>(*this).get<bit, extent, const T>();
  }

  template <size_t bit, size_t extent = 1, typename T = uint8_t>
  BITSTRUCT_CONSTEXPR Bitref<bit % 8, extent,
                             const typename std::remove_pointer<Ptr<T>>::type *>
  get() const noexcept {
    return const_cast<Bitstruct &>(*this)
        .get<bit, extent, const typename std::remove_pointer<T>::type *>();
  }

  BITSTRUCT_CONSTEXPR uint8_t *data() noexcept { return _data.data(); }
  BITSTRUCT_CONSTEXPR const uint8_t *data() const noexcept {
    return _data.data();
  }
  constexpr size_t size() const noexcept { return _data.size(); };

#ifdef __cpp_lib_span

  template <typename Char = uint8_t>
  BITSTRUCT_CONSTEXPR std::span<Char> data_span() noexcept {
    return std::span<Char>(reinterpret_cast<Char *>(_data.data()),
                           _data.size());
  }
  template <typename Char = uint8_t>
  BITSTRUCT_CONSTEXPR std::span<const Char> data_span() const noexcept {
    return std::span<const Char>(reinterpret_cast<const Char *>(_data.data()),
                                 _data.size());
  }

#endif

private:
  std::array<uint8_t, bitlength / 8> _data{};
};

} // namespace bit

#undef BITSTRUCT_CONSTEXPR