#include <bitstruct.hpp>
#include <cassert>
#include <cstdint>
#include <memory>

namespace {
template <typename To, typename From> To implicit_cast(From f) { return f; }

enum struct Type : uint8_t { null, A, B, C };
// [0..3] len [4..5] type [6] a [7] b
struct Register : bit::Bitstruct<8> {
  auto len() noexcept { return get<0, 4>(); }
  auto type() noexcept { return get<4, 2, Type>(); }
  auto a() const noexcept { return get<6, 1, bool>(); }
  auto b() noexcept { return get<7, 1, bool>(); }
};

void test_reg() {
  Register reg;
  reg.len() = 12;
  reg.type() = Type::C;
  // const correctness!
  // reg.a() = 2; cannot be written to
  reg.b() = true;
  assert(reg.len() == 12);
  assert(reg.type() == Type::C);
  assert(reg.a() == false);
  assert(reg.b() == true);
  reg.b() = false;
  assert(reg.b() == false);
  assert(reg.size() == 1);
  // 0     0     11 1100
  // false false C  12
  assert(*reg.data() == 0b0'0'11'1100);
}

enum struct DataType : uint8_t { null, integer, floating };
struct PackedPtr : bit::Bitstruct<64> {
  BITSTRUCT_FIELD_T(ptr, 0, 48, void *);
  BITSTRUCT_FIELD(len, 48, 8);
  BITSTRUCT_FIELD_T(type, 56, 8, DataType);
};

void test_ptr() {
  auto arr = std::make_unique<int[]>(12);
  PackedPtr ptr{};
  ptr.ptr() = arr.get();
  ptr.len() = 12;
  ptr.type() = DataType::integer;

  assert(ptr.ptr() == arr.get());
  assert(ptr.len() = 12);
  assert(ptr.type() == DataType::integer);
}

} // namespace

int main() {
  test_reg();
  test_ptr();
}
