#include <bitstruct.hpp>
#include <cstdint>
#include <iostream>
#include <string>

namespace {
enum struct Type : uint8_t { null, A, B, C };
// [0..3] len [4..5] type [6] a [] b
struct Register : private bit::Bitstruct<8> {
  auto len() noexcept { return get<0, 4>(); }
  auto type() noexcept { return get<4, 2, Type>(); }
  auto a() noexcept { return get<6, 1, bool>(); }
  auto b() noexcept { return get<7, 1, bool>(); }
};
std::string format(Type t) {
  switch (t) {
  case Type::null:
    return "null";
  case Type::A:
    return "A";
  case Type::B:
    return "B";
  case Type::C:
    return "C";
  }
  return "?";
}
} // namespace

int main() {
  Register reg;
  reg.len() = 12;
  reg.type() = Type::C;
  reg.b() = 1;
  std::cout << "len: " << int(reg.len()) << " type: " << format(reg.type())
            << " a: " << reg.a() << " b: " << reg.b() << '\n';
  return 0;
}
