/******************************************************************************/
/*!
* @file   bf_stl_allocator.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   > This allocator is a designed for use with stl containers.             \n
*   > This must only be used in C++11 and later.                            \n
*   > This is because C++03 required all allocators of a certain type to be \n
*     compatible but since this allocator scheme is stateful                \n
*     that is not be guaranteed.                                            \n
*
*  References:
*    [https://howardhinnant.github.io/allocator_boilerplate.html]
*
* @version 0.0.1
* @date    2019-12-26
*
* @copyright Copyright (c) 2019-2020
*/
/******************************************************************************/
#ifndef BF_STL_ALLOCATOR_HPP
#define BF_STL_ALLOCATOR_HPP

#include "bf_imemory_manager.hpp" /* IMemoryManager */

#include <limits> /* numeric_limits<T> */

namespace bf
{
  //
  // C++11 Allocator 'Concept'
  //
  // Traits:
  //   value_type                               T
  //   pointer                                  T*
  //   const_pointer                            const T*
  //   reference                                T&
  //   const_reference                          const T&
  //   size_type                                std::size_t
  //   difference_type                          std::ptrdiff_t
  //   propagate_on_container_move_assignment   std::true_ty
  //   rebind                                   template< class U > struct rebind { typedef allocator<U> other; };
  //   is_always_equal                          std::true_type
  //
  // Methods:
  //   ctor / dtor
  //   address
  //   allocate
  //   deallocate
  //   max_size
  //   construct
  //   destroy
  //
  // Operators:
  //   operator==
  //   operator!=

  //
  // C++17 Allocator 'Concept'
  //
  // Traits:
  //   value_type                               T
  //   propagate_on_container_move_assignment   std::true_type
  //   is_always_equal                          std::true_type
  //
  // Methods:
  //   ctor / dtor
  //   allocate
  //   deallocate
  //
  // Operators:
  //   operator==
  //   operator!=

  namespace detail
  {
    class StlAllocatorBase
    {
     protected:
      IMemoryManager &m_MemoryBackend;

     protected:
      explicit StlAllocatorBase(IMemoryManager &backend) noexcept :
        m_MemoryBackend{backend}
      {
      }

      explicit StlAllocatorBase(const StlAllocatorBase &rhs) noexcept = default;
    };
  }  // namespace detail

  template<class T>
  class StlAllocator final : public detail::StlAllocatorBase
  {
   public:
    using pointer                                = T *;
    using const_pointer                          = const T *;
    using void_pointer                           = void *;
    using const_void_pointer                     = const void *;
    using difference_type                        = std::ptrdiff_t;
    using size_type                              = std::size_t;  // std::make_unsigned_t<difference_type>
    using reference                              = T &;
    using const_reference                        = const T &;
    using value_type                             = T;
    using type                                   = T;
    using is_always_equal                        = std::false_type;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap            = std::false_type;

    template<class U>
    struct rebind
    {
      using other = StlAllocator<U>;
    };

   public:
    StlAllocator(IMemoryManager &backend) noexcept :
      StlAllocatorBase{backend}
    {
    }

    template<class U>
    StlAllocator(const StlAllocator<U> &rhs) noexcept :
      StlAllocatorBase{rhs}
    {
    }

    pointer          address(reference x) const noexcept { return &x; }
    const_pointer    address(const_reference x) const noexcept { return &x; }
    pointer          allocate(size_type s) { return s ? reinterpret_cast<pointer>(m_MemoryBackend.allocate(s * sizeof(T))) : nullptr; }
    void             deallocate(pointer p, size_type s) { m_MemoryBackend.deallocate(p, s * sizeof(T)); }
    static size_type max_size() noexcept { return std::numeric_limits<size_t>::max() / sizeof(value_type); }

    template<class U, class... Args>
    void construct(U *p, Args &&... args)
    {
      ::new (p) U(std::forward<Args>(args)...);
    }

    template<class U>
    void destroy(U *p)
    {
      p->~U();
    }

    StlAllocator select_on_container_copy_construction() const noexcept
    {
      return *this;
    }

    bool operator==(const StlAllocator<T> &rhs) const noexcept
    {
      return &m_MemoryBackend == &rhs.m_MemoryBackend;
    }

    bool operator!=(const StlAllocator<T> &rhs) const noexcept
    {
      return &m_MemoryBackend != &rhs.m_MemoryBackend;
    }
  };
}  // namespace bf

#endif /* BF_STL_ALLOCATOR_HPP */
