/******************************************************************************/
/*!
 * @file   bifrost_dense_map_handle.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   Strictly typed wrapper around an integer handle for use in the DenseMap<T>.
 *   
 *   Inspired By:
 *     [http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html]
 *
 * @version 0.0.1
 * @date    2019-12-27
 *
 * @copyright Copyright (c) 2019-2020
 */
/******************************************************************************/
#ifndef BF_DENSE_MAP_HANDLE_HPP
#define BF_DENSE_MAP_HANDLE_HPP

#include <cstdint> /* uint32_t, uint16_t */
#include <limits>  /* numeric_limits<T>  */

namespace bf
{
  namespace dense_map
  {
    using IDType    = std::uint32_t;  //!< The type used for an ID in a DenseMap.
    using IndexType = std::uint16_t;  //!< The type used for indexing into a DenseMap.

    static constexpr IndexType INDEX_MASK              = std::numeric_limits<IndexType>::max();
    static constexpr IDType    ONE_PLUS_INDEX_TYPE_MAX = (1 << std::numeric_limits<IndexType>::digits);

    static_assert(sizeof(IDType) >= sizeof(IndexType), "IDType must be able to hold ONE_PLUS_INDEX_TYPE_MAX.");
  }  // namespace dense_map

  template<typename T>
  struct DenseMapHandle final
  {
    dense_map::IDType id_index;  //!< This contains the unique id, the bottom bits contain the index into the m_SparseIndices array.

    DenseMapHandle(const dense_map::IDType id = dense_map::INDEX_MASK) :
      id_index(id)
    {
    }

    [[nodiscard]] bool operator==(const DenseMapHandle<T>& rhs) const { return id_index == rhs.id_index; }
    [[nodiscard]] bool operator!=(const DenseMapHandle<T>& rhs) const { return id_index != rhs.id_index; }
    [[nodiscard]] bool isValid() const { return id_index != dense_map::INDEX_MASK; }
  };
}  // namespace bf

#endif /* BF_DENSE_MAP_HANDLE_HPP */

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
