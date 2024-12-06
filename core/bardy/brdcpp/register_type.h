///
/// \file register_type.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 15.05.2018
///
/// \copyright InSys Copyright (c) 2018
///
///

#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>

namespace InSys {

#define REGISTER_BITMASK_BIT(bit) (1UL << bit)
#define REGISTER_BITMASK_BITS(bits, offset) (((1UL << bits) - 1) << offset)
#define REGISTER_BITMASK_DEFAULT_TRUE(bit) (1UL << bit)
#define REGISTER_BITMASK_DEFAULT_FALSE(bit) (0UL << bit)
#define REGISTER_BITMASK_DEFAULT(bit, value) (value << bit)

using Register32BitType = uint32_t;
using Register16BitType = uint16_t;
using Register8BitType = uint8_t;

namespace Detail {
template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType, AddrType AddrValue>
class RegisterTypeAbstract {
  union DataType;

  DataType m_Data;
  const AddrType m_Address{AddrValue};

 public:
  using ValueType = RegisterType;
  using AddressType = AddrType;
  using BitMaskType = BitMask;
  using BitMaskDefaultType = BitMaskDefault;

  RegisterTypeAbstract() : m_Data{} {}
  virtual ~RegisterTypeAbstract() noexcept = default;
  explicit RegisterTypeAbstract(const RegisterType& value) : m_Data{value} {}
  explicit RegisterTypeAbstract(const std::string& value)
      : m_Data{static_cast<RegisterType>(std::stoi(value))} {}
  void setValue(const RegisterType& value) noexcept {
    m_Data.m_Value = (value & typename std::underlying_type<BitMask>::type(
                                  BitMask::AllBits));
  }
  RegisterType getValue() const noexcept { return m_Data.m_Value; }
  AddrType getAddress() const noexcept { return m_Address; }
  std::string toString() const { return std::to_string(m_Data.m_Value); }
  DataType* operator->() noexcept { return &m_Data; }

 private:
  union DataType {
   public:
    friend RegisterTypeAbstract;
    BitsType Bits;
    DataType()
        : m_Value{typename std::underlying_type<BitMaskDefault>::type(
              BitMaskDefault::AllBits)} {}
    explicit DataType(const RegisterType& value)
        : m_Value{
              RegisterType(value & typename std::underlying_type<BitMask>::type(
                                       BitMask::AllBits))} {}
    DataType& operator=(const DataType& other) {
      if (this == &other) return *this;
      m_Value = other.m_Value;
      return *this;
    }
    void setAllDefaults() noexcept {
      m_Value = typename std::underlying_type<BitMaskDefault>::type(
          BitMaskDefault::AllBits);
    }
    void setAllBits() noexcept {
      m_Value = typename std::underlying_type<BitMask>::type(BitMask::AllBits);
    }
    void clearAllBits() noexcept { m_Value = 0; }
    static_assert(std::is_same<RegisterType, Register8BitType>::value ||
                      std::is_same<RegisterType, Register16BitType>::value ||
                      std::is_same<RegisterType, Register32BitType>::value,
                  "RegisterType is not equal Register8BitType, "
                  "Register16BitType or Register32BitType!");
    static_assert(std::is_class<BitsType>::value, "Wrong type of BitsType!");
    static_assert(std::is_enum<BitMask>::value, "Wrong type of BitMask!");
    static_assert(sizeof(BitsType) == sizeof(RegisterType),
                  "BitsType is not equal RegisterType!");
    static_assert(std::is_same<typename std::underlying_type<BitMask>::type,
                               RegisterType>::value,
                  "BitMask is not underlying type of RegisterType!");

   private:
    RegisterType m_Value;
  };
};

template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType, AddrType AddrValue>
std::istream& operator>>(
    std::istream& lhs,
    RegisterTypeAbstract<RegisterType, BitMask, BitMaskDefault, BitsType,
                         AddrType, AddrValue>& rhs) {
  RegisterType reg;
  lhs >> reg;
  rhs.setValue(reg);
  return lhs;
}

template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType, AddrType AddrValue>
std::ostream& operator<<(
    std::ostream& lhs,
    RegisterTypeAbstract<RegisterType, BitMask, BitMaskDefault, BitsType,
                         AddrType, AddrValue>
        rhs) {
  RegisterType reg = rhs.getValue();
  lhs << reg;
  return lhs;
}

template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType = uint32_t,
          AddrType AddrValue = 0>
struct SpdRegisterTypeAbstract
    : public RegisterTypeAbstract<RegisterType, BitMask, BitMaskDefault,
                                  BitsType, AddrType, AddrValue> {};

template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType = uint32_t,
          AddrType AddrValue = 0>
struct DirectRegisterTypeAbstract
    : public RegisterTypeAbstract<RegisterType, BitMask, BitMaskDefault,
                                  BitsType, AddrType, AddrValue> {};
template <typename RegisterType, typename BitMask, typename BitMaskDefault,
          typename BitsType, typename AddrType = uint32_t,
          AddrType AddrValue = 0>
struct IndirectRegisterTypeAbstract
    : public RegisterTypeAbstract<RegisterType, BitMask, BitMaskDefault,
                                  BitsType, AddrType, AddrValue> {};

}  // namespace Detail

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using Register8BitTypeAbstract =
    Detail::RegisterTypeAbstract<Register8BitType, BitMask, BitMaskDefault,
                                 BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using Register16BitTypeAbstract =
    Detail::RegisterTypeAbstract<Register16BitType, BitMask, BitMaskDefault,
                                 BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using Register32BitTypeAbstract =
    Detail::RegisterTypeAbstract<Register32BitType, BitMask, BitMaskDefault,
                                 BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using SpdRegister8BitTypeAbstract =
    Detail::SpdRegisterTypeAbstract<Register8BitType, BitMask, BitMaskDefault,
                                    BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using SpdRegister16BitTypeAbstract =
    Detail::SpdRegisterTypeAbstract<Register16BitType, BitMask, BitMaskDefault,
                                    BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using SpdRegister32BitTypeAbstract =
    Detail::SpdRegisterTypeAbstract<Register32BitType, BitMask, BitMaskDefault,
                                    BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using DirectRegister8BitTypeAbstract = Detail::DirectRegisterTypeAbstract<
    Register8BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using DirectRegister16BitTypeAbstract = Detail::DirectRegisterTypeAbstract<
    Register16BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using DirectRegister32BitTypeAbstract = Detail::DirectRegisterTypeAbstract<
    Register32BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using IndirectRegister8BitTypeAbstract = Detail::IndirectRegisterTypeAbstract<
    Register8BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using IndirectRegister16BitTypeAbstract = Detail::IndirectRegisterTypeAbstract<
    Register16BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

template <typename BitMask, typename BitMaskDefault, typename BitsType,
          typename AddrType = uint32_t, AddrType AddrValue = 0>
using IndirectRegister32BitTypeAbstract = Detail::IndirectRegisterTypeAbstract<
    Register32BitType, BitMask, BitMaskDefault, BitsType, AddrType, AddrValue>;

}  // namespace InSys
