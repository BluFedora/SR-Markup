/******************************************************************************/
/*!
 * @file   bifrost_dense_map.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   The DenseMap is used for fast addition and removal of
 *   elements while keeping a cache local array of objects.
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
#ifndef BF_DENSE_MAP_HPP
#define BF_DENSE_MAP_HPP

#include "bifrost_array.hpp"            /* Array<T>          */
#include "bifrost_dense_map_handle.hpp" /* DenseMapHandle<T> */

#include <cassert> /* assert */
#include <utility> /* move   */

namespace bf
{
  namespace dense_map
  {
    /*!
     * @brief
     *    The Index used to manage the Indices in the DenseMap.
     *
     *    This is a freelist node embedded in an array.
     */
    struct Index
    {
     public:
      IDType    id;     //!< Used to check if the passed in _unique_ ID is correct.
      IndexType index;  //!< The actual index of the object in the DenseMap.
      IndexType next;   //!< The next free index in the m_SparseIndices array.

     public:
      Index(IDType id, IndexType i) :
        id(id),
        index(i),
        next(INDEX_MASK)
      {
      }
    };
  }  // namespace dense_map

  /*!
   * @brief
   *   The DenseMap is used for fast addition and removal of
   *   elements while keeping a cache local array of objects.
   *
   *   Made for faster (frequent) Insert(s) and Remove(s) relative to Vector
   *   While keeping a dense array with good cache locality.
   */
  template<typename TObject>
  class DenseMap final
  {
   public:
    struct Proxy
    {
      TObject           data;
      dense_map::IDType id;

      template<typename... Args>
      explicit Proxy(dense_map::IDType id, Args&&... args) :
        data(std::forward<decltype(args)>(args)...),
        id(id)
      {
      }

      TObject*       operator->() { return &data; }
      const TObject* operator->() const { return &data; }
      TObject&       operator*() { return data; }
      const TObject& operator*() const { return data; }
      operator TObject&() { return data; }
      operator const TObject&() const { return data; }
    };

    class iterator
    {
     public:
      using self_type  = iterator;
      using value_type = TObject;
      using reference  = value_type&;
      using pointer    = value_type*;

     private:
      Proxy* m_Position;

     public:
      explicit iterator(Proxy* pos) :
        m_Position{pos}
      {
      }

      [[nodiscard]] self_type next() const
      {
        self_type it = (*this);
        ++it;

        return it;
      }

      self_type operator++(int)  // Post-fix
      {
        self_type it = (*this);
        ++(*this);

        return it;
      }

      self_type& operator++()  // Pre-fix
      {
        ++m_Position;

        return *this;
      }

      friend iterator       operator+(const iterator& lhs, std::size_t offset) { return iterator(lhs.m_Position + offset); }
      bool                  operator==(const iterator& rhs) const { return m_Position == rhs.m_Position; }
      bool                  operator==(pointer rhs) const { return m_Position == rhs; }
      bool                  operator!=(const iterator& rhs) const { return m_Position != rhs.m_Position; }
      bool                  operator!=(pointer rhs) const { return m_Position != rhs; }
      pointer               operator->() { return &**m_Position; }
      reference             operator*() { return *m_Position; }
      const value_type*     operator->() const { return &*m_Position; }
      const value_type&     operator*() const { return *m_Position; }
      [[nodiscard]] pointer value() const { return m_Position; }
    };

    using ObjectArray    = Array<Proxy>;
    using IndexArray     = Array<dense_map::Index>;
    using const_iterator = iterator;

   private:
    ObjectArray          m_DenseArray;     //!< The actual dense array of objects
    IndexArray           m_SparseIndices;  //!< Used to manage the indices of the next free index.
    dense_map::IndexType m_NextSparse;     //!< Keeps track of the next free index and whether or not to grow, a value of `dense_map::INDEX_MASK` indicates there are no free index slots.

   public:
    /*!
     * @brief
     *   Constructs a new empty DenseMap.
     *
     * @param memory
     *   The allocator to use for the objects and sparse mapping array.
    */
    explicit DenseMap(IMemoryManager& memory) :
      m_DenseArray(memory),
      m_SparseIndices(memory),
      m_NextSparse(dense_map::INDEX_MASK)
    {
    }

    /*!
     * @brief
     *   Reserved memory in the internal arrays so that adding objects will
     *   not allocate at random times.
     *
     * @param size
     *   The new size you want ot make sure the arrays are at least.
    */
    void reserve(std::size_t size)
    {
      assert(size < dense_map::INDEX_MASK && "A size bigger than `dense_map::INDEX_MASK` will not help you.");

      m_DenseArray.reserve(size);
      m_SparseIndices.reserve(size);
    }

    /*!
     * @brief
     *   Adds an object to this slot map by constructing it in place.
     *
     * @tparam Args
     *   The Types of the arguments for TObject's constructor.
     *
     * @param args
     * The arguments for TObject's constructor.
     *
     * @return
     *   The id to the newly created object.
    */
    template<typename... Args>
    DenseMapHandle<TObject> add(Args&&... args)
    {
      assert(m_DenseArray.size() < dense_map::INDEX_MASK && "Too many objects created (how did this happen?? The max is 0xFFFF).");

      dense_map::Index& in = getNextIndex();

      // Each time an object gets created change the ID to be unique.
      in.id += dense_map::ONE_PLUS_INDEX_TYPE_MAX;
      in.index = dense_map::IndexType(m_DenseArray.size());

      m_DenseArray.emplace(in.id, std::forward<Args>(args)...);

      return in.id;
    }

    /*!
     * @brief
     *    Check if the passed in ID is valid in this DenseMap.
     *
     * @param id
     *    The id to be checking against.
     *
     * @return
     *  true  - if the element can be found in this DenseMap
     *  false - if the ID is invalid and should not be used to get / remove an object.
     */
    bool has(DenseMapHandle<TObject> id) const
    {
      const dense_map::IndexType index = id.id_index & dense_map::INDEX_MASK;

      if (index < m_SparseIndices.size())
      {
        const dense_map::Index& in = m_SparseIndices[index];

        return in.id == id.id_index && in.index != dense_map::INDEX_MASK;
      }

      return false;
    }

    /*!
     * @brief
     *    Finds the object from the associated ID.
     *    This function does no bounds checking so be sure to call
     *    DenseMap<T>::has to guarantee safety.
     *
     *    This is the limitation of a decision I made to return a reference.
     *    Otherwise this function would return a pointer with nullptr indicating failure.
     *
     * @param id
     *    The id of the object to be found.
     *
     * @return
     *  The Object associated with that particular ID.
     */
    TObject& find(const DenseMapHandle<TObject> id)
    {
      assert(has(id) && "Only valid IDs are allowed to be passed into DenseMap::find.");

      const dense_map::Index& in = m_SparseIndices[id.id_index & dense_map::INDEX_MASK];

      return m_DenseArray[in.index].data;
    }

    /*!
     * @brief
     *  Removes the object of the specified ID from this DenseMap.
     *  This function will cause a move of the last elements.
     *  Complexity: O(1)
     *
     * @param id
     *  The ID of the object to be removed.
     */
    void remove(DenseMapHandle<TObject> id)
    {
      assert(has(id) && "Only valid IDs are allowed to be passed into DenseMap::remove.");

      const dense_map::IndexType index     = id.id_index & dense_map::INDEX_MASK;
      dense_map::Index&          in           = m_SparseIndices[index];
      Proxy&                     obj          = m_DenseArray[in.index];
      Proxy&                     back_element = m_DenseArray.back();

      if (&obj != &back_element)
      {
        obj = std::move(back_element);

        // Remap the newly moved back_element's index it's new slot.
        m_SparseIndices[obj.id & dense_map::INDEX_MASK].index = in.index;
      }

      m_DenseArray.pop();

      // The current bucket is now invalid.
      in.index = dense_map::INDEX_MASK;

      // Add this bucket to the list of free slots.
      in.next      = m_NextSparse;
      m_NextSparse = index;
    }

    /*!
     * @brief
     *   Invalidates all IDs handed out and clears any internal state.
    */
    void clear()
    {
      m_DenseArray.clear();
      m_SparseIndices.clear();
      m_NextSparse = dense_map::INDEX_MASK;
    }

    // NOTE(Shareef): STL Standard Container Functions

    iterator                  begin() { return iterator(m_DenseArray.data()); }
    const_iterator            begin() const { return iterator(m_DenseArray.data()); }
    TObject&                  at(const std::size_t index) { return m_DenseArray.at(index); }
    const TObject&            at(const std::size_t index) const { return m_DenseArray.at(index); }
    [[nodiscard]] std::size_t size() const { return m_DenseArray.size(); }
    TObject&                  operator[](const std::size_t index) { return m_DenseArray[index]; }
    const TObject&            operator[](const std::size_t index) const { return m_DenseArray[index]; }
    iterator                  end() { return iterator(m_DenseArray.data() + size()); }
    const_iterator            end() const { return iterator(m_DenseArray.data() + size()); }
    Proxy*                    data() { return m_DenseArray.data(); }
    const Proxy*              data() const { return m_DenseArray.data(); }

    // TODO(SR): Maybe there should be a function to shrink memory usage as this will just grow and grow.

   private:
    dense_map::Index& getNextIndex()
    {
      using namespace dense_map;

      // If we have a free sparse slot available.
      if (m_NextSparse != INDEX_MASK)
      {
        Index& current_index = m_SparseIndices[m_NextSparse];
        m_NextSparse         = current_index.next;

        return current_index;
      }

      return m_SparseIndices.emplace(IDType(m_SparseIndices.length()), INDEX_MASK);
    }
  };
}  // namespace bf

#endif /* BF_DENSE_MAP_HPP */

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
