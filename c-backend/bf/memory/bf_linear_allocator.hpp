/******************************************************************************/
/*!
* @file   bf_linear_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is very good for temporary memory allocations throughout
*   the frame. There is no individual deallocation but a whole clear operation
*   that should happen at the beginning (or end) of each 'frame'.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BIFROST_LINEAR_ALLOCATOR_HPP
#define BIFROST_LINEAR_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* MemoryManager */

namespace bf
{
  class LinearAllocatorScope;

  class LinearAllocator : public MemoryManager
  {
    friend class LinearAllocatorScope;

   public:
    static constexpr std::size_t header_size = 0u;

   private:
    std::size_t m_MemoryOffset;

   public:
    LinearAllocator(char* memory_block, std::size_t memory_block_size);

    size_t usedMemory() const { return m_MemoryOffset; }

    void  clear(void);
    void* allocate(std::size_t size) override final;
    void  deallocate(void* ptr, std::size_t num_bytes) override final;

    virtual ~LinearAllocator() = default;

   private:
    char* currentBlock() const;
  };

  template<std::size_t k_BufferSize>
  class FixedLinearAllocator final : public LinearAllocator
  {
   private:
    char m_FixedBuffer[k_BufferSize];

   public:
    FixedLinearAllocator() :
      LinearAllocator(m_FixedBuffer, k_BufferSize),
      m_FixedBuffer{}
    {
    }
  };

  class LinearAllocatorScope final
  {
   private:
    LinearAllocator* m_Allocator;
    std::size_t      m_OldOffset;

   public:
    LinearAllocatorScope(LinearAllocator& allocator);
    LinearAllocatorScope(LinearAllocatorScope&& rhs) noexcept;

    LinearAllocatorScope(const LinearAllocatorScope& rhs) = delete;
    LinearAllocatorScope& operator=(const LinearAllocatorScope& rhs) = delete;
    LinearAllocatorScope& operator=(LinearAllocatorScope&& rhs) noexcept = delete;

    ~LinearAllocatorScope();
  };
}  // namespace bf

#endif /* BIFROST_LINEAR_ALLOCATOR_HPP */
