/******************************************************************************/
/*!
* @file   bf_crt_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*  This allocator is a wrapper around the built in memory allocator.
*  Implemented using "malloc / calloc" and "free".
*  TODO Look Into Alignment: [https://johanmabille.github.io/blog/2014/12/06/aligned-memory-allocator/]
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BF_CRT_ALLOCATOR_HPP
#define BF_CRT_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp"

namespace bf
{
  class CAllocator final : public IMemoryManager
  {
   public:
    CAllocator();

   public:
    void* allocate(std::size_t size) override;
    void  deallocate(void* ptr, std::size_t num_bytes) override;

    static constexpr std::size_t header_size = 0u;
  };

  using CRTAllocator = CAllocator;
}  // namespace bifrost

#endif  /* BF_C_ALLOCATOR_HPP */
