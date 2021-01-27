/******************************************************************************/
/*!
* @file   bifrost_json.hpp
* @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
* @brief
*   C++ Wrapper around the C Json API with conversions to/from a C++ Value.
*
* @version 0.0.1
* @date    2020-03-17
*
* @copyright Copyright (c) 2020
*/
/******************************************************************************/
#ifndef BIFROST_JSON_HPP
#define BIFROST_JSON_HPP

#include "bf/data_structures/bifrost_array.hpp"      /* Array<T>         */
#include "bf/data_structures/bifrost_hash_table.hpp" /* HashTable<K, V>  */
#include "bf/data_structures/bifrost_string.hpp"     /* String           */
#include "bf/data_structures/bifrost_variant.hpp"    /* Variant<Ts...>   */

#include <initializer_list> /* initializer_list */

namespace bf::json
{
  class Value;

  using Pair    = std::pair<String, Value>;
  using Object  = HashTable<String, Value, 16>;
  using Array   = ::bf::Array<Value>;
  using String  = ::bf::String;
  using Number  = double;
  using Boolean = bool;

  namespace detail
  {
    using Value_t           = Variant<Object, Array, String, Number, Boolean>;
    using ObjectInitializer = std::initializer_list<Pair>;
    using ArrayInitializer  = std::initializer_list<Value>;
  }  // namespace detail

  Value fromString(char* source, std::size_t source_length);
  void  toString(const Value& json, String& out);

  class Value final : public detail::Value_t
  {
   public:
    using Base_t = detail::Value_t;

   public:
    // Constructors
    Value() = default;

    template<typename T>
    Value(const T& data_in) noexcept :
      Base_t(data_in)
    {
    }

    template<std::size_t N>
    Value(const char (&data_in)[N]) noexcept :
      Base_t(String(data_in))
    {
    }

    Value(detail::ObjectInitializer&& values);
    Value(detail::ArrayInitializer&& values);
    Value(const char* value);
    Value(int value);
    Value(std::uint64_t value);
    Value(std::int64_t value);

    // Assignment

    Value& operator=(detail::ObjectInitializer&& values);
    Value& operator=(detail::ArrayInitializer&& values);

    // Meta API

    using Base_t::as;
    using Base_t::is;
    using Base_t::type;
    bool isObject() const { return Base_t::is<Object>(); }
    bool isArray() const { return Base_t::is<Array>(); }
    bool isString() const { return Base_t::is<String>(); }
    bool isNumber() const { return Base_t::is<double>(); }
    bool isBoolean() const { return Base_t::is<bool>(); }

    // Cast API

    template<typename T>
    const T& asOr(const T& default_value) const
    {
      if (Base_t::is<T>())
      {
        return Base_t::as<T>();
      }

      return default_value;
    }

    template<typename T, typename... Args>
    T& cast(Args&&... args)
    {
      if (!Base_t::is<T>())
      {
        set<T>(std::forward<Args>(args)...);
      }

      return Base_t::as<T>();
    }

    // Object API

    Value& operator[](const StringRange& key);
    Value& operator[](const char* key);
    Value* at(const StringRange& key) const;

    template<typename T>
    T get(const StringRange& key, const T& default_value = {}) const
    {
      if (isObject())
      {
        Value* const value = at(key);

        return value && value->is<T>() ? value->as<T>() : default_value;
      }

      return default_value;
    }

    // Array API

    Value&      operator[](int index);  // Must be an int as to not conflict with the 'const char* key' overload.
    std::size_t size() const;
    void        push(const Value& item);
    Value&      push();
    void        insert(std::size_t index, const Value& item);
    Value&      back();
    void        pop();

    // Special Operations

    //
    // If isObject() then adds add it as a field
    // If isArray    then it is pushed.
    // Else this object is assigned to value.
    //
    void add(StringRange key, const Value& value);
  };
}  // namespace bf::json

#endif /* BIFROST_JSON_HPP */
