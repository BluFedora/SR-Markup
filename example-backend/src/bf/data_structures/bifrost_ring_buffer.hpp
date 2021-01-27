/******************************************************************************/
/*!
 * @file bifrost_ring_buffer.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 * @version 0.0.1
 * @date 2019-12-22
 *
 * @copyright Copyright (c) 2019
 */
/******************************************************************************/
#ifndef BIFROST_RING_BUFFER_HPP
#define BIFROST_RING_BUFFER_HPP

#include "bifrost/memory/bifrost_imemory_manager.hpp" /* IMemoryManager       */
#include "bifrost/utility/bifrost_non_copy_move.hpp"  /* bfNonCopyMoveable<T> */

#include <type_traits> /* aligned_storage */

namespace bifrost
{
  namespace detail
  {
    template<typename T, typename FWrap>
    struct RingBufferImpl
    {
      T*          buffer;
      std::size_t head;
      std::size_t tail;
      FWrap       wrap; // TODO(SR): This can benefit from a zero sized base class trick.

      RingBufferImpl(T* storage) :
        buffer{storage},
        head{0},
        tail{0},
        wrap{}
      {
      }

      [[nodiscard]] std::size_t size() const { return tail - head; }
      [[nodiscard]] bool        isEmpty() const { return head == tail; }
      [[nodiscard]] bool        isFull() const { return wrap(tail + 1) == head; }

      void push(const T& element)
      {
        // TODO(SR): Throw a real exception and also disable this for release (it is expected of subclasses to handle the full case).
        if (isFull())
        {
          throw "";
        }

        new (buffer + tail) T(element);
        tail = wrap(tail + 1);
      }

      template<typename F>
      void forEach(F&& fn)
      {
        for (size_t i = head; i != tail; i = wrap(i + 1))
        {
          fn(i, buffer[i]);
        }
      }

      T pop()
      {
        if (isEmpty())
        {
          // TODO(Shareef): Throw a 'real' exception.
          throw "";
        }

        T element = std::move(buffer[head]);
        buffer[head].~T();
        head = wrap(head + 1);
        return element;
      }

      ~RingBufferImpl()
      {
        forEach([](size_t, T& element) {
          element.~T();
        });
      }
    };

    template<std::size_t N>
    struct WrapFixed final
    {
      std::size_t operator()(std::size_t number) const
      {
        if constexpr ((N & N - 1) == 0)
        {
          return number & (N - 1);
        }
        else
        {
          return number % N;
        }
      }
    };

    struct WrapDynamic final
    {
      std::size_t capacity;

      std::size_t operator()(std::size_t number) const
      {
        return number % capacity;
      }
    };
  }  // namespace detail

  template<typename T, std::size_t N>
  class FixedRingBuffer final : private detail::RingBufferImpl<T, detail::WrapFixed<N>>
  {
    static_assert(N != 0, "A buffer of 0 size is not valid.");

    using storage_t = std::aligned_storage_t<sizeof(T) * N, alignof(T)>;
    using base_t    = detail::RingBufferImpl<T, detail::WrapFixed<N>>;

   public:
    [[nodiscard]] static std::size_t capacity() { return N; }

   private:
    storage_t m_Storage;

   public:
    FixedRingBuffer() :
      base_t(reinterpret_cast<T*>(&m_Storage)),
      m_Storage{}
    {
    }

    using base_t::isEmpty;
    using base_t::isFull;
    using base_t::pop;
    using base_t::size;

    bool push(const T& element)
    {
      if (isFull())
      {
        // NOTE(Shareef):
        //   Since this is used for the event system having the latest event
        //   is a better policy than keeping the old events.
        pop();
      }

      base_t::push(element);
      return true;
    }
  };

  // clang-format off
  template<typename T>
  class RingBuffer : private detail::RingBufferImpl<T, detail::WrapDynamic>, private bfNonCopyMoveable<RingBuffer<T>>
  // clang-format on
  {
    using base_t = detail::RingBufferImpl<T, detail::WrapDynamic>;

   private:
    IMemoryManager& m_Memory;

   public:
    RingBuffer(IMemoryManager& memory, size_t initial_size) :
      base_t(reinterpret_cast<T*>(m_Memory.allocateAligned(sizeof(T) * initial_size, alignof(T)))),
      m_Memory{memory}
    {
      base_t::wrap.capacity = initial_size;
    }

    using base_t::isEmpty;
    using base_t::isFull;
    using base_t::pop;
    using base_t::size;
    [[nodiscard]] std::size_t capacity() const { return base_t::wrap.capacity; }

    void push(const T& element)
    {
      if (base_t::isFull())
      {
        const std::size_t new_capacity = capacity() * 2;
        auto              new_buffer   = m_Memory.allocateAligned(sizeof(T) * new_capacity, alignof(T));
        base_t::forEach(
         [new_buffer](const size_t i, T& element) mutable {
           new (new_buffer + i) T(std::move(element));
           element.~T();
         });
        m_Memory.deallocateAligned(base_t::buffer);
        base_t::buffer        = new_buffer;
        base_t::wrap.capacity = new_capacity;
        base_t::head          = 0;
        base_t::tail          = size();
      }

      base_t::push(element);
    }

    ~RingBuffer()
    {
      m_Memory.deallocateAligned(base_t::buffer);
    }
  };
}  // namespace bifrost

#endif /* BIFROST_RING_BUFFER_HPP */
