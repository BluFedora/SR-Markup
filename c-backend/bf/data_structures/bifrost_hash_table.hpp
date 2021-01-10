/******************************************************************************/
/*!
  @file   bifrost_hash_table.hpp
  @author Shareef Abdoul-Raheem
  @brief
      This is a basic unordered hash map that uses linear probing rather than
      a linked list causing references to nodes in the table to be unstable
      so DO NOT store references long after you are given them.

      TODO: A good optimization would be for all functions taking in a key to
            take in a templated key as to not for a constructionof a TKey specifically.
            Ex: A String vs StringRange shoudl be allowed tp be compared as is.
*/
/******************************************************************************/
#ifndef BIFROST_HASH_TABLE_HPP
#define BIFROST_HASH_TABLE_HPP

// TODO(Shareef): Be more Standards compliant and include headers for these.
// uint8_t, size_t, aligned_storage, alignment_of, hash, equal_to

#include <algorithm>        /* copy, min */
#include <initializer_list> /* initializer_list */
// #include <utility>          /* */

namespace bf
{
  static constexpr std::size_t MAX_PROBES = 16u;

  enum class NodeState : std::uint8_t
  {
    DELETED,
    UNUSED,
    OCCUPIED
  };

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  class HashTable;

  namespace detail
  {
    template<typename TKey, typename TValue>
    class HashNode
    {
     private:
      using KeyT  = typename std::aligned_storage<sizeof(TKey), std::alignment_of<TKey>::value>::type;
      using DataT = typename std::aligned_storage<sizeof(TValue), std::alignment_of<TValue>::value>::type;

      template<typename TK, typename TV, std::size_t initial_size, typename Hasher, typename TEqual>
      friend class HashTable;

      public: // TODO: Why does this need to be public?
     // private:
      KeyT      m_Key;
      DataT     m_Value;
      NodeState m_State;


     // private:
      HashNode() :
        m_Key(),
        m_Value(),
        m_State(NodeState::UNUSED)
      {
      }

      void set(const TKey& key, const TValue& value)
      {
        deleteNode();
        new (&this->m_Key) TKey(key);
        new (&this->m_Value) TValue(value);
        this->m_State = NodeState::OCCUPIED;
      }

      void set(const TKey& key, TValue&& value)
      {
        deleteNode();
        new (&this->m_Key) TKey(key);
        new (&this->m_Value) TValue(std::move(value));
        this->m_State = NodeState::OCCUPIED;
      }

      template<typename... Args>
      void emplace(const TKey& key, Args... args)
      {
        deleteNode();
        new (&this->m_Key) TKey(key);
        new (&this->m_Value) TValue(std::forward<decltype(args)...>(args...));
        this->m_State = NodeState::OCCUPIED;
      }

      [[nodiscard]] bool isWritable() const
      {
        // return m_State == NodeState::DELETED || this->m_State == NodeState::UNUSED;
        return m_State != NodeState::OCCUPIED;
      }

      [[nodiscard]] bool isFilled() const
      {
        return m_State == NodeState::OCCUPIED;
      }

      void deleteNode()
      {
        if (m_State == NodeState::OCCUPIED)
        {
          key().~TKey();
          value().~TValue();
          m_State = NodeState::DELETED;
        }
      }

      ~HashNode()
      {
        deleteNode();
      }

     public:
      HashNode(HashNode&& rhs) = delete;
      HashNode& operator=(HashNode&& rhs) = delete;
      HashNode(const HashNode& rhs)       = delete;

      // TODO(Shareef): Add an operator for RValues.
      HashNode& operator=(const HashNode& rhs)
      {
        if (this != &rhs)
        {
          deleteNode();

          if (rhs.isFilled())
          {
            new (&this->m_Key) TKey(rhs.key());
            new (&this->m_Value) TValue(rhs.value());
          }

          m_State = rhs.m_State;
        }

        return *this;
      }

      [[nodiscard]] const TKey& key() const
      {
        return *reinterpret_cast<const TKey*>(&m_Key);
      }

      [[nodiscard]] TValue& value()
      {
        return *reinterpret_cast<TValue*>(&m_Value);
      }

      [[nodiscard]] const TValue& value() const
      {
        return *reinterpret_cast<const TValue*>(&m_Value);
      }
    };
  }  // namespace detail

  template<typename TKey,
           typename TValue,
           std::size_t initial_size = 128,
           typename Hasher          = std::hash<TKey>,
           typename TEqual          = std::equal_to<>>  // std::equal_to<TKey>
  class HashTable
  {
   private:
    using HashT = detail::HashNode<TKey, TValue>;

    class iterator
    {
     public:
      typedef iterator    self_type;
      typedef TValue      value_type;
      typedef value_type& reference;
      typedef value_type* pointer;

     private:
      HashT* m_Position;
      HashT* m_End;

     public:
      iterator(HashT* pos, HashT* end);
      iterator(HashT* end);

      self_type& operator++();  // pre-fix
      self_type  next() const;
      self_type  operator++(int);  // post-fix

      bool operator==(const iterator& rhs) const;
      bool operator!=(const iterator& rhs) const;

      HashT*       operator->();
      HashT&       operator*();
      const HashT* operator->() const;
      const HashT& operator*() const;
    };

   private:
    static Hasher StaticHasher;
    static TEqual StaticEqualer;

   private:
    HashT*      m_Table;
    std::size_t m_Capacity;

   public:
    HashTable();

    [[nodiscard]] std::size_t capacity() const;

    template<typename ConvertKey,
             typename ConvertValue,
             typename = typename std::enable_if<std::is_convertible<ConvertKey, TKey>::value, TKey>::type,
             typename = typename std::enable_if<std::is_convertible<ConvertValue, TValue>::value, TValue>::type>
    HashTable(const std::initializer_list<std::pair<ConvertKey, ConvertValue>>& list);

    HashTable(HashTable&& rhs) noexcept;
    HashTable(const HashTable& rhs);
    HashTable& operator=(const HashTable& rhs);
    HashTable& operator=(HashTable&& rhs) noexcept;

    TValue& operator[](const TKey& key);

    iterator begin();
    iterator find(const TKey& key) const;
    iterator end();
    iterator begin() const;
    iterator end() const;

    iterator     insert(const TKey& key, const TValue& value);
    iterator     insert(const TKey& key, TValue&& value);
    unsigned int count(const TKey& key) const;
    bool         has(const TKey& key) const;

    template<typename F>
    void forEach(F&& callback) const;

    void set(const TKey& key, const TValue& value);

    template<typename... Args>
    bool emplace(const TKey& key, Args&&... args);

    TValue* at(const TKey& key) const;
    TValue* get(const TKey& key) const;
    bool    remove(const TKey& key);
    void    erase(const TKey& key);
    void    clear() const;

    ~HashTable();

   private:
    void        destroy();
    HashT*      getNode(const TKey& key) const;
    HashT*      getFreeNode(const TKey& key);
    void        rehash();
    HashT*      nodeAt(std::size_t index) const;
    std::size_t index(const TKey& key) const;
    std::size_t hash(const TKey& key) const;
  };

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::iterator(HashT* pos, HashT* end) :
    m_Position(pos),
    m_End(end)
  {
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::iterator(HashT* end) :
    m_Position(end),
    m_End(end)
  {
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::self_type& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator++()
  {
    ++this->m_Position;

    while (this->m_Position != this->m_End &&
           this->m_Position->m_State != NodeState::OCCUPIED)
      ++this->m_Position;

    return (*this);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::self_type HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::next() const
  {
    iterator it = (*this);
    ++it;
    return it;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::self_type HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator++(int)
  {
    iterator it = (*this);
    ++(*this);
    return it;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  bool HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator==(const iterator& rhs) const
  {
    return this->m_Position == rhs.m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  bool HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator!=(const iterator& rhs) const
  {
    return m_Position != rhs.m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator->()
  {
    return m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::operator*()
  {
    return *m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  const typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::
  operator->() const
  {
    return m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  const typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator::
  operator*() const
  {
    return *m_Position;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashTable() :
    m_Table(new HashT[initial_size]()),
    m_Capacity(initial_size)
  {
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  std::size_t HashTable<TKey, TValue, initial_size, Hasher, TEqual>::capacity() const
  {
    return m_Capacity;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  template<typename ConvertKey, typename ConvertValue, typename, typename>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashTable(
   const std::initializer_list<std::pair<ConvertKey, ConvertValue>>& list) :
    HashTable()
  {
    for (const auto& it : list)
    {
      this->emplace(it.first, it.second);
    }
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashTable(HashTable&& rhs) noexcept :
    m_Table(rhs.m_Table),
    m_Capacity(rhs.m_Capacity)
  {
    rhs.m_Table    = nullptr;
    rhs.m_Capacity = 0;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashTable(const HashTable& rhs) :
    m_Table(new HashT[rhs.m_Capacity]()),
    m_Capacity(rhs.m_Capacity)
  {
    std::copy(rhs.m_Table, rhs.m_Table + rhs.m_Capacity, m_Table);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::operator=(const HashTable& rhs)
  {
    if (this != &rhs)
    {
      destroy();
      m_Table    = new HashT[rhs.m_Capacity]();
      m_Capacity = rhs.m_Capacity;
      std::copy(rhs.m_Table, rhs.m_Table + rhs.m_Capacity, m_Table);
    }

    return *this;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::operator=(HashTable&& rhs) noexcept
  {
    destroy();
    m_Table        = rhs.m_Table;
    m_Capacity     = rhs.m_Capacity;
    rhs.m_Table    = nullptr;
    rhs.m_Capacity = 0;

    return *this;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  TValue& HashTable<TKey, TValue, initial_size, Hasher, TEqual>::operator[](const TKey& key)
  {
    HashT* node = getNode(key);

    if (!node)
    {
      node = getFreeNode(key);
      node->set(key, TValue());
    }

    return node->value();
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::begin()
  {
    HashT* start = m_Table;

    while (start != (m_Table + m_Capacity) && start->m_State != NodeState::OCCUPIED)
    {
      ++start;
    }

    return iterator(start, this->m_Table + this->m_Capacity);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::find(const TKey& key) const
  {
    auto* node = this->getNode(std::forward<decltype(key)>(key));

    return node ? iterator(node, m_Table + m_Capacity) : end();
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::end()
  {
    return iterator(m_Table + m_Capacity);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::begin() const
  {
    HashT* start = this->m_Table;

    while (start != this->m_Table + this->m_Capacity && start->m_State != NodeState::OCCUPIED)
      ++start;

    return iterator(start, this->m_Table + this->m_Capacity);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::end() const
  {
    return iterator(this->m_Table + this->m_Capacity);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::insert(const TKey& key, const TValue& value)
  {
    auto hashCode = this->index(key);

    for (std::size_t i = 0; i < MAX_PROBES; ++i)
    {
      if (hashCode >= m_Capacity) break;

      HashT* node = nodeAt(hashCode);

      if (node->isWritable())
      {
        node->set(key, value);
        return iterator(node, this->m_Table + this->m_Capacity);
      }

      ++hashCode;
    }

    rehash();
    return insert(key, value);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::iterator HashTable<TKey, TValue, initial_size, Hasher, TEqual>::insert(const TKey& key, TValue&& value)
  {
    auto hashCode = this->index(key);

    for (std::size_t i = 0; i < MAX_PROBES; ++i)
    {
      if (hashCode >= m_Capacity) break;

      HashT* node = nodeAt(hashCode);

      if (node->isWritable())
      {
        node->emplace(key, std::forward<TValue&&>(value));
        return iterator(node, this->m_Table + this->m_Capacity);
      }

      ++hashCode;
    }

    rehash();
    return insert(key, std::forward<TValue&&>(value));
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  unsigned HashTable<TKey, TValue, initial_size, Hasher, TEqual>::count(const TKey& key) const
  {
    return has(std::forward<decltype(key)>(key));
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  bool HashTable<TKey, TValue, initial_size, Hasher, TEqual>::has(const TKey& key) const
  {
    return get(key) != nullptr;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  template<typename F>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::forEach(F&& callback) const
  {
    for (std::size_t i = 0; i < this->m_Capacity; ++i)
    {
      if (m_Table[i].m_State == NodeState::OCCUPIED)
      {
        callback(this->m_Table[i].key(), this->m_Table[i].value());
      }
    }
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::set(const TKey& key, const TValue& value)
  {
    auto hashCode = this->index(key);

    for (std::size_t i = 0; i < MAX_PROBES; ++i)
    {
      if (hashCode >= this->m_Capacity)
      {
        break;
      }

      HashT* node = this->nodeAt(hashCode);

      if (node->isWritable() || HashTable::StaticEqualer(key, node->key()))
      {
        node->set(key, value);
        return;
      }

      ++hashCode;
    }

    if (hashCode >= m_Capacity)
    {
      rehash();
      set(key, value);
    }
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  template<typename... Args>
  bool HashTable<TKey, TValue, initial_size, Hasher, TEqual>::emplace(const TKey& key, Args&&... args)
  {
    HashT* node = this->getFreeNode(key);

    if (node)
    {
      node->emplace(
       std::forward<decltype(key)>(key),
       std::forward<decltype(args)...>(args...));
    }

    return (node != nullptr);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  TValue* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::at(const TKey& key) const
  {
    return get(std::forward<decltype(key)>(key));
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  TValue* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::get(const TKey& key) const
  {
    HashT* const node = getNode(key);

    if (node)
    {
      return &node->value();
    }

    return nullptr;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  bool HashTable<TKey, TValue, initial_size, Hasher, TEqual>::remove(const TKey& key)
  {
    auto   hashCode    = this->index(key);
    auto   hashCodeMax = std::min(hashCode + MAX_PROBES, m_Capacity);
    HashT* node        = nodeAt(hashCode);

    while (node->m_State != NodeState::UNUSED && hashCode < hashCodeMax)
    {
      if (node->isFilled() && HashTable::StaticEqualer(key, node->key()))
      {
        node->deleteNode();
        return true;
      }

      node = nodeAt(++hashCode);
    }

    throw "Shareef messed up the HashTable::remove function.";
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::erase(const TKey& key)
  {
    remove(std::forward<decltype(key)>(key));
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::clear() const
  {
    for (std::size_t i = 0; i < m_Capacity; ++i)
    {
      nodeAt(i)->deleteNode();
    }
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  HashTable<TKey, TValue, initial_size, Hasher, TEqual>::~HashTable()
  {
    destroy();
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::destroy()
  {
    delete[] m_Table;
    m_Table    = nullptr;
    m_Capacity = 0;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::getNode(const TKey& key) const
  {
    auto hashCode    = this->index(key);
    auto hashCodeMax = std::min(hashCode + MAX_PROBES, m_Capacity);

    HashT* node = this->nodeAt(hashCode);

    while (hashCode < hashCodeMax && node->m_State == NodeState::OCCUPIED)
    {
      if (node->isFilled() && HashTable::StaticEqualer(key, node->key()))
      {
        return node;
      }

      node = this->nodeAt(++hashCode);
    }

    return nullptr;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::getFreeNode(const TKey& key)
  {
    auto hashCode = this->index(key);

    for (std::size_t i = 0; i < MAX_PROBES; ++i)
    {
      if (hashCode >= this->m_Capacity) break;

      HashT* node = this->nodeAt(hashCode);

      if (node->isWritable())
      {
        return node;
      }

      if (node->isFilled() && StaticEqualer(node->key(), key))
      {
        return nullptr;
      }

      ++hashCode;
    }

    this->rehash();
    return this->getFreeNode(key);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  void HashTable<TKey, TValue, initial_size, Hasher, TEqual>::rehash()
  {
    const auto old_capacity = this->m_Capacity;

    m_Capacity <<= 1;
    HashT* const newTable = new HashT[m_Capacity]();
    HashT* const oldTable = m_Table;
    HashT*       walker   = oldTable;

    m_Table = newTable;

    // TODO(Shareef): The insert call can recursively call 'rehash'. This can be made into an iterative method.
    for (std::size_t i = 0; i < old_capacity; ++i)
    {
      if (!walker->isWritable())
      {
        insert(walker->key(), std::move(walker->value()));
      }

      ++walker;
    }

    delete[] oldTable;
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  typename HashTable<TKey, TValue, initial_size, Hasher, TEqual>::HashT* HashTable<TKey, TValue, initial_size, Hasher, TEqual>::nodeAt(
   const std::size_t index) const
  {
    return (m_Table + index);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  std::size_t HashTable<TKey, TValue, initial_size, Hasher, TEqual>::index(const TKey& key) const
  {
    if constexpr ((initial_size & (initial_size - 1)) != 0)
    {
      return hash(std::forward<decltype(key)>(key)) % m_Capacity;
    }
    else
    {
      return hash(std::forward<decltype(key)>(key)) & (m_Capacity - 1);
    }
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  std::size_t HashTable<TKey, TValue, initial_size, Hasher, TEqual>::hash(const TKey& key) const
  {
    return HashTable::StaticHasher(key);
  }

  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  Hasher HashTable<TKey, TValue, initial_size, Hasher, TEqual>::StaticHasher = {};
  template<typename TKey, typename TValue, std::size_t initial_size, typename Hasher, typename TEqual>
  TEqual HashTable<TKey, TValue, initial_size, Hasher, TEqual>::StaticEqualer = {};
}  // namespace bifrost

#endif /* BIFROST_HASH_TABLE_HPP */
