#ifndef BIFROST_OBJECT_HASH_CACHE_HPP
#define BIFROST_OBJECT_HASH_CACHE_HPP

#include "bf/bf_non_copy_move.hpp" /* NonCopyMoveable<T> */

#include <cassert>     /* assert                */
#include <cstddef>     /* size_t                */
#include <cstdint>     /* uint64_t              */
#include <cstring>     /* memset                */
#include <type_traits> /* is_trivially_copyable */

namespace bf
{
  template<typename T>
  struct MemCompare
  {
    bool operator()(const T& a, const T& b) const
    {
      return std::memcmp(&a, &b, sizeof(T)) == 0;
    }
  };

  // clang-format off

  //
  // Non owning data structure. Just used for managing a hash-based cache of objects.
  //
  template<typename T, typename TConfig, typename Compare = MemCompare<TConfig>>
  class ObjectHashCache final : private NonCopyMoveable<ObjectHashCache<T, TConfig, Compare>>, private Compare
  // clang-format on
  {
    struct Node final
    {
      T*            value     = nullptr;
      std::uint64_t hash_code = 0x0;
      TConfig       config_data;
    };

   private:
    Node*       m_Nodes;
    std::size_t m_NodesCapacity;
    std::size_t m_MaxLoad;

   public:
    explicit ObjectHashCache(std::size_t initial_size = 32) :
      m_Nodes{new Node[initial_size]},
      m_NodesCapacity{initial_size},
      m_MaxLoad{5}
    {
      assert(initial_size && !(initial_size & initial_size - 1) && "Initial size of a ObjectHashCache must be a non 0 power of two.");
      clear();
    }

    // This method will update an old slot
    // with the new value if a collision occurs.
    void insert(std::uint64_t key, T* value, const TConfig& config_data)
    {
      if (!internalInsert(key, value, config_data))
      {
        grow();
        insert(key, value, config_data);
      }
    }

    [[nodiscard]] T* find(std::uint64_t key, const TConfig& config_data) const
    {
      const std::size_t hash_mask = m_NodesCapacity - 1;
      std::size_t       idx       = key & hash_mask;

      for (std::size_t i = 0; i < m_MaxLoad; ++i)
      {
        const auto& node = m_Nodes[idx];

        if (node.hash_code == key)
        {
          if (Compare::operator()(node.config_data, config_data))
          {
            return node.value;
          }
        }

        idx = idx + 1 & hash_mask;
      }

      return nullptr;
    }

    bool remove(std::uint64_t key, T* value)
    {
      const std::size_t hash_mask = m_NodesCapacity - 1;
      std::size_t       idx       = key & hash_mask;

      for (std::size_t i = 0; i < m_MaxLoad; ++i)
      {
        auto& node = m_Nodes[idx];

        if (node.value == value && node.hash_code == key)
        {
          node.value     = nullptr;
          node.hash_code = key;

          return true;
        }

        idx = idx + 1 & hash_mask;
      }

      return false;
    }

    void clear()
    {
      static_assert(std::is_trivially_copyable_v<Node>, "To memset nodes I need to make sure they are trivial types.");

      if (m_Nodes)
      {
        std::memset(m_Nodes, 0x0, sizeof(Node) * m_NodesCapacity);
      }
    }

    ~ObjectHashCache()
    {
      delete[] m_Nodes;
    }

    template<typename F>
    void forEach(F&& callback)
    {
      for (std::size_t i = 0; i < m_NodesCapacity; ++i)
      {
        if (m_Nodes[i].value)
        {
          callback(m_Nodes[i].value, m_Nodes[i].config_data);
        }
      }
    }

   private:
    bool internalInsert(std::uint64_t key, T* value, const TConfig& config_data)
    {
      assert(value && "nullptr is used as an indicator of an empty slot.");

      const std::size_t hash_mask = m_NodesCapacity - 1;
      std::size_t       idx       = key & hash_mask;

      for (std::size_t i = 0; i < m_MaxLoad; ++i)
      {
        auto& node = m_Nodes[idx];

        if (!node.value /*|| node.hash_code == key*/)
        {
          node.value       = value;
          node.hash_code   = key;
          node.config_data = config_data;
          return true;
        }

        idx = (idx + 1) & hash_mask;
      }

      return false;
    }

    void grow()
    {
      const std::size_t old_size  = m_NodesCapacity;
      Node* const       old_nodes = m_Nodes;
      bool              was_success;

      do
      {
        const std::size_t new_size  = m_NodesCapacity * 2;
        Node* const       new_nodes = new Node[new_size]();
        m_Nodes                     = new_nodes;
        m_NodesCapacity             = new_size;
        ++m_MaxLoad;

        was_success = true;
        for (std::size_t i = 0; i < old_size; ++i)
        {
          const Node& old_node = old_nodes[i];

          if (old_node.value && !internalInsert(old_node.hash_code, old_node.value, old_node.config_data))
          {
            was_success = false;
            break;
          }
        }

        if (!was_success)
        {
          delete[] new_nodes;
        }

      } while (!was_success);

      delete[] old_nodes;
    }
  };
}  // namespace bifrost

#endif /* BIFROST_OBJECT_HASH_CACHE_HPP */
