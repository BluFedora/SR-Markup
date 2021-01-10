#ifndef BF_CORE_H
#define BF_CORE_H

#include <stddef.h> /* size_t             */
#include <stdint.h> /* uint32_t, uint64_t */

#if _MSC_VER
/*!
 * @brief
 *  NOTE(Shareef):
 *    This is a compiler intrinsic that tells the compiler that
 *    the default case cannot be reached thus letting the optimizer
 *    do less checks to see if the expression is not in the
 *    set of valid cases.
 */
#define bfInvalidDefaultCase() \
  default:                     \
    __debugbreak();            \
    __assume(0)

/*!
 * @brief
 *  NOTE(Shareef):
 *    This is a compiler intrinsic that tells the compiler that
 *    the this class will never be instantiated directly and thus vtable
 *    initialization does not need to happen in this ctor.
 */
#define bfPureInterface(T) __declspec(novtable) T

// TODO(SR):
//   [https://stackoverflow.com/questions/44054078/how-to-guide-gcc-optimizations-based-on-assertions-without-runtime-cost]
#else
#define bfInvalidDefaultCase() \
  default:                     \
    break

#define bfPureInterface(T) T
#endif

#if __cplusplus
#undef bfCArraySize
#undef bfSizeOfField
#undef bfBit

template<typename T, size_t N>
static constexpr size_t bfCArraySize(const T((&)[N]))
{
  return N;
}

#define bfSizeOfField(T, member) (sizeof(T::member))

template<typename T>
static constexpr T bfBit(T bit_idx)
{
  return T(1) << bit_idx;
}

#else
#define bfCArraySize(arr) ((sizeof(arr) / sizeof(0 [arr])) / ((size_t)(!(sizeof(arr) % sizeof(0 [arr])))))  //(sizeof((arr)) / sizeof((arr)[0]))
#define bfSizeOfField(T, member) (sizeof(((T*)0)->member))
#define bfBit(index) (1ULL << (index))
#endif

/*!
 * @brief 
 *   This will retrieve the conatianing object from one of it's members.
*/
#define bfObjectFromMember(ptr, type, member) ((type*)((char*)(ptr)-offsetof(type, member)))

#if __cplusplus
extern "C" {
#endif

typedef unsigned char bfByte;
typedef uint16_t      bfBool16;
typedef uint32_t      bfBool32;
typedef float         bfFloat32;
typedef double        bfFloat64;
#define bfTrue 1
#define bfFalse 0

/*!
 * @brief
 *   A non-owning reference to a string.
*/
typedef struct bfStringRange_t
{
  const char* bgn;
  const char* end;

} bfStringRange;

#if __cplusplus
constexpr
#endif
 static inline bfStringRange
 bfMakeStringRangeLen(const char* bgn, size_t length)
{
#if __cplusplus
  return {
   bgn,
   bgn + length,
  };
#else
  return (bfStringRange){
   .bgn = bgn,
   .end = bgn + length,
  };
#endif
}

#if __cplusplus
constexpr
#endif
 static inline bfStringRange
 bfMakeStringRangeC(const char* str)
{
  const char* end = str;

  while (end[0])
  {
    ++end;
  }

  return bfMakeStringRangeLen(str, end - str);
}

// This is used to clearly mark flexible-sized arrays that appear at the end of
// some dynamically-allocated structs, known as the "struct hack".
#if __STDC_VERSION__ >= 199901L
// In C99, a flexible array member is just "[]".
#define bfStructHack
#else
// Elsewhere, use a zero-sized array. It's technically undefined behavior,
// but works reliably in most known compilers.
#define bfStructHack 0
#endif
#if __cplusplus
}
#endif

#endif /* BF_CORE_H */
