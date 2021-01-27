/******************************************************************************/
/*!
 * @file   bf_hash.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   Some hashing utilities for various data types.
 *
 * @version 0.0.1
 * @date 2019-12-28
 *
 * @copyright Copyright (c) 2019
 */
/******************************************************************************/
#ifndef BF_HASH_HPP
#define BF_HASH_HPP

#include <cstddef> /* size_t   */
#include <cstdint> /* uint64_t */

namespace bf::hash
{
  using Hash_t = std::uint64_t;

  /*!
   * @brief
   *   Hashes a pointer to a specified bit size.
   *
   *   This hash is not designed to be good just fast
   *
   * @tparam T
   *   The type you want the pointer to fit into.
   *
   * @param ptr
   *   The pointer you want to reduce.
   *
   * @return
   *   The hashed pointer value.
  */
  template<typename T>
  T reducePointer(const void* ptr) = delete;

  // clang-format off
  template<> std::uint8_t  reducePointer<std::uint8_t >(const void* ptr);
  template<> std::uint16_t reducePointer<std::uint16_t>(const void* ptr);
  template<> std::uint32_t reducePointer<std::uint32_t>(const void* ptr);
  template<> std::uint64_t reducePointer<std::uint64_t>(const void* ptr);
  // clang-format on

  Hash_t simple(const char* p, std::size_t size);
  Hash_t simple(const char* p);
  Hash_t combine(Hash_t lhs, Hash_t hashed_value);

  Hash_t addString(Hash_t self, const char* p);
  Hash_t addString(Hash_t self, const char* p, std::size_t size);
  Hash_t addBytes(Hash_t self, const char* p, std::size_t size);
  Hash_t addU32(Hash_t self, std::uint32_t u32);
  Hash_t addS32(Hash_t self, std::int32_t s32);
  Hash_t addU64(Hash_t self, std::uint64_t u64);
  Hash_t addS64(Hash_t self, std::int64_t s64);
  Hash_t addF32(Hash_t self, float f32);
  Hash_t addPointer(Hash_t self, const void* ptr);
}  // namespace bf::hash

#endif /* BF_HASH_HPP */
