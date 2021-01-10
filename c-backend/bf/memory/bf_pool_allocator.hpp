/******************************************************************************/
/*!
* @file   bf_pool_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is a designed for static (known at compile time)
*   pools of objects. Features O(1) allocation and O(1) deletion.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BF_POOL_ALLOCATOR_HPP
#define BF_POOL_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp"

namespace bf
{
  namespace detail
  {
    template<std::size_t size_of_t, std::size_t alignment>
    static constexpr std::size_t aligned_size()
    {
      return ((size_of_t + alignment - 1) / alignment) * alignment;
    }
  }  // namespace detail

  class PoolAllocatorImpl : public MemoryManager
  {
   protected:
    class PoolHeader
    {
     public:
      PoolHeader* next;
    };

   public:
    static constexpr std::size_t header_size = sizeof(PoolHeader);

   private:
    PoolHeader* m_PoolStart;
    std::size_t m_BlockSize;

   public:
    PoolAllocatorImpl(char* memory_block, std::size_t memory_block_size, std::size_t sizeof_block, std::size_t alignof_block);

    void*       allocate(std::size_t size) override;
    void        deallocate(void* ptr, std::size_t num_bytes) override;
    std::size_t indexOf(const void* ptr) const;
    void*       fromIndex(std::size_t index);  // The index must have been from 'indexOf'
    void        reset();

    std::size_t capacity() const
    {
      return size() / m_BlockSize;
    }
  };

  template<typename T, std::size_t num_elements>
  class PoolAllocator : public PoolAllocatorImpl
  {
   private:
    template<size_t arg1, size_t... others>
    struct static_max;

    template<size_t arg>
    struct static_max<arg>
    {
      static constexpr size_t value = arg;
    };

    template<size_t arg1, size_t arg2, size_t... others>
    struct static_max<arg1, arg2, others...>
    {
      static constexpr size_t value = arg1 >= arg2 ? static_max<arg1, others...>::value : static_max<arg2, others...>::value;
    };

   public:
    static constexpr std::size_t header_size       = PoolAllocatorImpl::header_size;
    static constexpr std::size_t alignment_req     = static_max<alignof(T), alignof(PoolHeader)>::value;
    static constexpr std::size_t allocation_size   = static_max<sizeof(T), header_size>::value;
    static constexpr std::size_t pool_stride       = detail::aligned_size<allocation_size, alignment_req>();
    static constexpr std::size_t memory_block_size = pool_stride * num_elements;

   private:
    char m_AllocBlock[memory_block_size];

   public:
    // Not initialized by design since the PoolAllocatorImpl ctor does some setup.
    // ReSharper disable once CppPossiblyUninitializedMember
    PoolAllocator() :
      PoolAllocatorImpl{m_AllocBlock, memory_block_size, sizeof(T), alignof(T)}
    {
    }
  };
}  // namespace bf

#endif /* BF_POOL_ALLOCATOR_HPP */
