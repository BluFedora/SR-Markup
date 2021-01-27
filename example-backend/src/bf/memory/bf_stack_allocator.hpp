/******************************************************************************/
/*!
* @file   bf_stack_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is a designed for allocations where you can guarantee
*   deallocation is in a LIFO (Last in First Out) order in return you get
*   some speed.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BIFROST_STACK_ALLOCATOR_HPP
#define BIFROST_STACK_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* MemoryManager */

namespace bf
{
  class StackAllocator final : public MemoryManager
  {
   private:
    char*       m_StackPtr;
    std::size_t m_MemoryLeft;

   public:
    StackAllocator(char* memory_block, std::size_t memory_size);

    std::size_t usedMemory() const { return size() - m_MemoryLeft; }
    void*       allocate(std::size_t size) override;
    void        deallocate(void* ptr, std::size_t num_bytes) override;

   private:
    class StackHeader
    {
     public:
      std::size_t block_size;
      std::size_t align_size;
    };

   public:
    static constexpr std::size_t header_size = sizeof(StackHeader);
  };
}  // namespace bifrost

#endif /* BIFROST_STACK_ALLOCATOR_HPP */
