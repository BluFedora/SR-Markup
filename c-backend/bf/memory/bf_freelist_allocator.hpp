/******************************************************************************/
/*!
* @file   bf_freelist_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is a the most generic custom allocator with the heaviest
*   overhead. This has the largest header size of any allocator but can be
*   used as a direct replacement for "malloc/new" and "free/delete".
*
*   For allocating a first fit policy is used.
*   For deallocating an address-ordered pilicy is used.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BIFROST_FREELIST_ALLOCATOR_HPP
#define BIFROST_FREELIST_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* MemoryManager */

namespace bf
{
  class FreeListAllocator final : public MemoryManager
  {
   private:
    struct FreeListNode;
    FreeListNode* m_FreeList;
    std::size_t   m_UsedBytes;

   public:
    FreeListAllocator(char* memory_block, std::size_t memory_block_size);

    std::size_t usedMemory() const { return m_UsedBytes; }

   public:
    void* allocate(std::size_t size) override;
    void  deallocate(void* ptr, std::size_t num_bytes) override;

   private:
    struct AllocationHeader
    {
      std::size_t size;  //!< size does not include the size of the header itself, rather it is the size of the writable region of memory.
    };

    struct FreeListNode final : public AllocationHeader
    {
      //
      // When size is used here it includes the memory taken up by FreeListNode::next.
      //

      FreeListNode* next;  //!< Next free block.

      unsigned char* begin() const { return reinterpret_cast<unsigned char*>(const_cast<FreeListNode*>(this)); }
      unsigned char* end() const { return begin() + header_size + size; }
    };

   public:
    static constexpr std::size_t header_size = sizeof(AllocationHeader);
  };
}  // namespace bf

#endif /* BIFROST_FREELIST_ALLOCATOR_HPP */
