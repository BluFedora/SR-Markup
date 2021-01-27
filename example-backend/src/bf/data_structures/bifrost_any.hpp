/******************************************************************************/
/*!
 * @file   bf_any.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *    This class can hold any C++ type with a small buffer optimization
 *    for built-in small types.
 *    Anything other class is on the heap.
 *
 * @version 0.0.1
 * @date    2019-12-27
 *
 * @copyright Copyright (c) 2019
 */
/******************************************************************************/
#ifndef BF_ANY_HPP
#define BF_ANY_HPP

#include <algorithm> /* swap, max                                                                */
#include <cstdint>   /* int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t */
#include <cstring>   /* memcpy                                                                   */
#include <utility>   /* exchange                                                                 */

namespace bifrost
{
  class Any;

  namespace detail
  {
    class BadAnyCast
    {
    };

    class EmptyAny
    {
    };

    class BaseAnyPolicy
    {
     public:
      virtual void  static_delete(void** x)                       = 0;
      virtual void  copy_from_value(void const* src, void** dest) = 0;
      virtual void  clone(void* const* src, void** dest)          = 0;
      virtual void  move(void* const* src, void** dest)           = 0;
      virtual void* get_value(void** src)                         = 0;

     protected:
      ~BaseAnyPolicy() = default;
    };

    template<typename T>
    class SmallAnyPolicy final : public BaseAnyPolicy
    {
     public:
      void static_delete(void** x) override
      {
        // All the small types have a trivial destructor.
        (void)x;
      }

      void copy_from_value(void const* src, void** dest) override
      {
        new (dest) T(*reinterpret_cast<T const*>(src));
      }

      void clone(void* const* src, void** dest) override
      {
        *dest = *src;
      }

      void move(void* const* src, void** dest) override
      {
        *dest = *src;
      }

      void* get_value(void** src) override
      {
        return reinterpret_cast<void*>(src);
      }
    };

    template<typename T>
    class BigAnyPolicy final : public BaseAnyPolicy
    {
     public:
      void static_delete(void** x) override
      {
        delete (*reinterpret_cast<T**>(x));
        *x = nullptr;
      }

      void copy_from_value(void const* src, void** dest) override
      {
        *dest = new T(*reinterpret_cast<T const*>(src));
      }

      void clone(void* const* src, void** dest) override
      {
        *dest = new T(**reinterpret_cast<T* const*>(src));
      }

      void move(void* const* src, void** dest) override
      {
        T** t_star_star = reinterpret_cast<T**>(dest);

        (*t_star_star)->~T();
        **t_star_star = **reinterpret_cast<T* const*>(src);
      }

      void* get_value(void** src) override
      {
        return *src;
      }
    };

    // clang-format off

    template<typename T>
    struct ChoosePolicy
    {
      typedef BigAnyPolicy<T> type;
    };

    template<typename T>
    struct ChoosePolicy<T*>
    {
      typedef SmallAnyPolicy<T*> type;
    };

    /// Choosing the policy for an any type is illegal, but should never happen.
    /// This is designed to throw a compiler error.
    template<> struct ChoosePolicy<Any>         { typedef void                        type; };

    /// Specializations for small types.
    template<> struct ChoosePolicy<EmptyAny>    { typedef SmallAnyPolicy<EmptyAny>    type; };
    template<> struct ChoosePolicy<std::byte>   { typedef SmallAnyPolicy<std::byte>   type; };
    template<> struct ChoosePolicy<bool>        { typedef SmallAnyPolicy<bool>        type; };
    template<> struct ChoosePolicy<char>        { typedef SmallAnyPolicy<char>        type; };
    template<> struct ChoosePolicy<int8_t>      { typedef SmallAnyPolicy<int8_t>      type; };
    template<> struct ChoosePolicy<uint8_t>     { typedef SmallAnyPolicy<uint8_t>     type; };
    template<> struct ChoosePolicy<int16_t>     { typedef SmallAnyPolicy<int16_t>     type; };
    template<> struct ChoosePolicy<uint16_t>    { typedef SmallAnyPolicy<uint16_t>    type; };
    template<> struct ChoosePolicy<int32_t>     { typedef SmallAnyPolicy<int32_t>     type; };
    template<> struct ChoosePolicy<uint32_t>    { typedef SmallAnyPolicy<uint32_t>    type; };
    template<> struct ChoosePolicy<int64_t>     { typedef SmallAnyPolicy<int64_t>     type; };
    template<> struct ChoosePolicy<uint64_t>    { typedef SmallAnyPolicy<uint64_t>    type; };
    template<> struct ChoosePolicy<float>       { typedef SmallAnyPolicy<float>       type; };
    template<> struct ChoosePolicy<double>      { typedef SmallAnyPolicy<double>      type; };
    template<> struct ChoosePolicy<long double> { typedef SmallAnyPolicy<long double> type; };
    template<> struct ChoosePolicy<void*>       { typedef SmallAnyPolicy<void*>       type; };

    // clang-format on

    /// This function will return a different policy for each type.
    template<typename T>
    BaseAnyPolicy* getPolicy()
    {
      static typename ChoosePolicy<T>::type policy;
      return &policy;
    }
  }  // namespace detail

  class Any final
  {
   private:
    mutable char           m_Object[std::max(sizeof(long double), sizeof(void*))];
    detail::BaseAnyPolicy* m_Policy;

   public:
    /// Empty constructor.
    Any() :
      m_Object{},
      m_Policy(detail::getPolicy<detail::EmptyAny>())
    {
    }

    /// Initializing constructor.
    template<typename T>
    Any(const T& x) :
      Any()
    {
      assign(x);
    }

    template<typename T>
    Any(T& x) :
      Any()
    {
      if constexpr (std::is_pointer_v<T>)
      {
        assign(x);
      }
      else
      {
        assign(std::addressof(x));
      }
    }

    /// Special initializing constructor for string literals.
    explicit Any(const char* x) :
      Any()
    {
      assign(x);
    }

    /// Copy constructor.
    Any(const Any& x) :
      Any()
    {
      assign(x);
    }

    Any(Any& x) :
      Any()
    {
      assign(x);
    }

    /// Move constructor.
    Any(Any&& x) noexcept :
      m_Object{},
      m_Policy(std::exchange(x.m_Policy, detail::getPolicy<detail::EmptyAny>()))
    {
      std::memcpy(m_Object, x.m_Object, sizeof(m_Object));
    }

    /// Destructor.
    ~Any()
    {
      m_Policy->static_delete(asRawPointer());
    }

    /// Assignment function from another Any.
    Any& assign(const Any& x)
    {
      if (this != &x)
      {
        reset();
        m_Policy = x.m_Policy;
        m_Policy->clone(reinterpret_cast<void* const*>(&x.m_Object), asRawPointer());
      }

      return *this;
    }

    /// Assignment function.
    template<typename T>
    Any& assign(const T& x)
    {
      reset();
      m_Policy = detail::getPolicy<std::remove_cv_t<T>>();
      m_Policy->copy_from_value(&x, asRawPointer());
      return *this;
    }

    /// Copy Assignment operator.
    Any& operator=(const Any& x)
    {
      return assign(x);
    }

    /// Move Assignment operator.
    Any& operator=(Any&& x) noexcept
    {
      std::memcpy(m_Object, x.m_Object, sizeof(m_Object));
      m_Policy = std::exchange(x.m_Policy, detail::getPolicy<detail::EmptyAny>());

      return *this;
    }

    /// Assignment operator.
    template<typename T>
    Any& operator=(const T& x)
    {
      return assign(x);
    }

    /// Assignment operator, specialed for literal strings.
    /// They have types like const char [6] which don't work as expected.
    Any& operator=(const char* x)
    {
      return assign(x);
    }

    /// Utility functions
    Any& swap(Any& x) noexcept
    {
      std::swap(m_Policy, x.m_Policy);
      std::swap(m_Object, x.m_Object);
      return *this;
    }

    template<typename T>
    struct StripPointer
    {
      using ref = T&;
    };

    template<typename T>
    struct StripPointer<T*>
    {
      using ref = std::decay_t<T>*;
    };

    /// It's like cast be more linient
    template<typename T>
    typename StripPointer<T>::ref castSimilar()
    {
      using RawT = std::decay_t<T>;

      if (!isSimilar<T>())
        throw detail::BadAnyCast();

      if constexpr (std::is_pointer_v<T>)
      {
        if (is<std::nullptr_t>())
        {
          return nullptr;
        }
      }

      if (is<RawT*>())
      {
        RawT** r = static_cast<RawT**>(m_Policy->get_value(asRawPointer()));
        return **r;
      }

      RawT* r = static_cast<RawT*>(m_Policy->get_value(asRawPointer()));
      return (*r);
    }

    /// Cast operator. You can only cast to the original type.
    template<typename T>
    T& cast()
    {
      // TODO: This doesn't work with inheritance...
      // if (!is<T>())
      // throw detail::BadAnyCast();

      T* r = static_cast<T*>(m_Policy->get_value(asRawPointer()));
      return (*r);
    }

    template<typename T>
    const T& cast() const
    {
      // TODO: This doesn't work with inheritance...
      // if (!is<T>())
      //   throw detail::BadAnyCast();

      T* r = static_cast<T*>(m_Policy->get_value(asRawPointer()));
      return (*r);
    }

    template<typename T>
    T& as()
    {
      return cast<T>();
    }

    template<typename T>
    const T& as() const
    {
      return cast<T>();
    }

    template<typename T>
    operator T&()
    {
      return castSimilar<T>();
    }

    template<typename T>
    operator T*()
    {
      return castSimilar<T*>();
    }

    /// Frees any allocated memory, and sets the value to NULL.
    void reset()
    {
      m_Policy->static_delete(asRawPointer());
      m_Policy = detail::getPolicy<detail::EmptyAny>();
    }

    template<typename T>
    [[nodiscard]] bool isSimilar() const
    {
      using RawT = std::decay_t<T>;

      if constexpr (std::is_reference_v<T>)
      {
        return m_Policy == detail::getPolicy<RawT*>();
      }
      else if constexpr (std::is_pointer_v<T>)
      {
        return m_Policy == detail::getPolicy<RawT>() || m_Policy == detail::getPolicy<RawT*>() || m_Policy == detail::getPolicy<std::nullptr_t>();
      }
      else
      {
        return m_Policy == detail::getPolicy<RawT>() || m_Policy == detail::getPolicy<RawT*>();
      }
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
      return m_Policy == detail::getPolicy<T>();
    }

    [[nodiscard]] bool isEmpty() const
    {
      return m_Policy == detail::getPolicy<detail::EmptyAny>();
    }

    [[nodiscard]] bool isSameType(const Any& x) const
    {
      return m_Policy == x.m_Policy;
    }

   private:
    void** asRawPointer() const
    {
      return reinterpret_cast<void**>(&m_Object);
    }
  };
}  // namespace bifrost

#endif /* BF_ANY_HPP */
