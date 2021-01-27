/******************************************************************************/
/*!
 * @file   bf_non_copy_move.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   Helper Mix-in classes for disabling stupid C++isms.
 *
 * @version 0.0.1
 * @date    2019-12-28
 *
 * @copyright Copyright (c) 2019-2020
 */
/******************************************************************************/
#ifndef BF_NON_COPY_MOVE_HPP
#define BF_NON_COPY_MOVE_HPP

namespace bf
{
  template<typename T>
  class NonCopyable  // NOLINT(hicpp-special-member-functions)
  {
   public:
    NonCopyable(const NonCopyable&) = delete;
    explicit NonCopyable(const T&)  = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    T&           operator=(const T&) = delete;

    //protected:
    NonCopyable()  = default;
    ~NonCopyable() = default;
  };

  template<typename T>
  class NonMoveable  // NOLINT(hicpp-special-member-functions)
  {
   public:
    NonMoveable(NonMoveable&&) = delete;
    explicit NonMoveable(T&&)  = delete;
    NonMoveable& operator=(NonMoveable&&) = delete;
    T&           operator=(T&&) = delete;

    //protected:
    NonMoveable()  = default;
    ~NonMoveable() = default;
  };

  // clang-format off
  template<typename T>
  class NonCopyMoveable : public NonCopyable<T>, public NonMoveable<T>
  // clang-format on
  {
  };
}  // namespace bf

#endif /* BF_NON_COPY_MOVE_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2020 Shareef Abdoul-Raheem

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
/******************************************************************************/
