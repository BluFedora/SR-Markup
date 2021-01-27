/******************************************************************************/
/*!
* @file   bf_proxy_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   This allocator is not really an allocator at all but allows for more
*   debugging opportunities when the occasions arise.
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BIFROST_PROXY_ALLOCATOR_HPP
#define BIFROST_PROXY_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* IMemoryManager */

namespace bf
{
  class ProxyAllocator final : public IMemoryManager
  {
   private:
    IMemoryManager& m_Impl;

   public:
    ProxyAllocator(IMemoryManager& real_allocator);

   public:
    void* allocate(std::size_t size) override;
    void  deallocate(void* ptr, std::size_t num_bytes) override;

   public:
    static constexpr std::size_t header_size = 0u;
  };
}  // namespace bf

#endif /* BIFROST_PROXY_ALLOCATOR_HPP */
