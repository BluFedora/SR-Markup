// Inspired By [https://stackoverflow.com/questions/34134886/how-to-implement-an-intrusive-linked-list-that-avoids-undefined-behavior]

#ifndef BIFROST_INTRUSIVE_LIST_HPP
#define BIFROST_INTRUSIVE_LIST_HPP

#include "bf/bf_non_copy_move.hpp" /* bfNonCopyable<T>, bfNonCopyMoveable<t> */

namespace bf::intrusive
{
  template<typename T>
  class ListView;

  template<typename T>
  struct Node // : private NonCopyable<Node<T>>
  {
    Node<T>* prev;
    T*       next;

    explicit Node(Node<T>* p = nullptr, T* n = nullptr) :
      prev{p},
      next{n}
    {
    }
  };

  template<typename T>
  using ListNode = Node<T>;

  template<typename T>
  class ListIterator final
  {
    friend class ListView<T>;

   public:
    typedef ListIterator<T>        self_type;
    typedef T                      value_type;
    typedef value_type&            reference;
    typedef value_type*            pointer;
    using MemberAccessor = Node<T> T::*;

   private:
    Node<T>*       m_Current;
    MemberAccessor m_Link;

   public:
    explicit ListIterator(Node<T>* node, Node<T> T::*link) :
      m_Current{node},
      m_Link{link}
    {
    }

    [[nodiscard]] self_type next() const
    {
      self_type it = (*this);
      ++it;
      return it;
    }

    self_type operator++()  // Pre-fix
    {
      m_Current = &(m_Current->next->*m_Link);
      return *this;
    }

    self_type operator++(int)  // Post-fix
    {
      self_type it = (*this);
      ++(*this);

      return it;
    }

    self_type operator--()  // Pre-fix
    {
      m_Current = m_Current->prev;
      return *this;
    }

    self_type operator--(int)  // Post-fix
    {
      self_type it = (*this);
      --(*this);
      return it;
    }

    bool operator==(const self_type& rhs) const
    {
      return m_Current == rhs.m_Current;
    }

    bool operator!=(const self_type& rhs) const
    {
      return m_Current != rhs.m_Current;
    }

    pointer operator->()
    {
      return m_Current->next;
    }

    reference operator*()
    {
      return *m_Current->next;
    }

    const value_type* operator->() const
    {
      return m_Current->next;
    }

    const value_type& operator*() const
    {
      return *m_Current->next;
    }

    [[nodiscard]] pointer value() const
    {
      return m_Current->next;
    }
  };

  // This list does not own the memory.
  template<typename T>
  class ListView final
  {
    using MemberAccessor = Node<T> T::*;

   private:
    mutable Node<T> m_Head; // TODO(SR): This is for the const versions begin and end, but maybe they should be actually const??
    MemberAccessor  m_Link;

   public:
    explicit ListView(MemberAccessor link) :
      m_Head{&m_Head, nullptr},
      m_Link{link}
    {
    }

    ListIterator<T>    begin() { return ListIterator<T>(&m_Head, m_Link); }
    ListIterator<T>    end() { return ListIterator<T>(m_Head.prev, m_Link); }
    ListIterator<T>    begin() const { return ListIterator<T>(&m_Head, m_Link); }
    ListIterator<T>    end() const { return ListIterator<T>(m_Head.prev, m_Link); }
    T&                 front() { return *m_Head.next; }
    const T&           front() const { return *m_Head.next; }
    T&                 back() { return *(m_Head.prev->prev->next); }
    const T&           back() const { return *(m_Head.prev->prev->next); }
    [[nodiscard]] bool isEmpty() const { return &m_Head == m_Head.prev; }

    void pushBack(T& node)
    {
      insert(end(), node);
    }

    void pushFront(T& node)
    {
      insert(begin(), node);
    }

    void popBack()
    {
      erase(--end());
    }

    void popFront()
    {
      erase(begin());
    }

    void insert(ListIterator<T> pos, T& node)
    {
      (node.*m_Link).next                                                       = pos.m_Current->next;
      ((node.*m_Link).next ? (pos.m_Current->next->*m_Link).prev : m_Head.prev) = &(node.*m_Link);
      (node.*m_Link).prev                                                       = pos.m_Current;
      pos.m_Current->next                                                       = &node;
    }

    ListIterator<T> makeIterator(T& node) const
    {
      return ListIterator<T>((node.*m_Link).prev, m_Link);
    }

    ListIterator<T> erase(T& node)
    {
      return erase(makeIterator(node));
    }

    ListIterator<T> erase(ListIterator<T> it)
    {
      it.m_Current->next                                                      = (it.m_Current->next->*m_Link).next;
      (it.m_Current->next ? (it.m_Current->next->*m_Link).prev : m_Head.prev) = it.m_Current;
      return it;
    }

    void clear()
    {
      while (!isEmpty())
      {
        erase(begin());
      }
    }

    ~ListView()
    {
      clear();
    }
  };
}  // namespace bf::intrusive

// TODO: Make a new file for this

#include "bf/IMemoryManager.hpp"

namespace bf
{
  // Memory Owning List
  template<typename T>
  class List : private NonCopyMoveable<List<T>>
  {
   private:
    struct Node;

   public:
    class iterator final
    {
      friend class List<T>;

     public:
      typedef iterator    self_type;
      typedef T           value_type;
      typedef value_type& reference;
      typedef value_type* pointer;

     private:
      intrusive::ListIterator<Node> m_Current;

     public:
      explicit iterator(intrusive::ListIterator<Node> current) :
        m_Current{current}
      {
      }

      [[nodiscard]] self_type next() const
      {
        self_type it = (*this);
        ++it;

        return it;
      }

      self_type operator++()  // Pre-fix
      {
        ++m_Current;

        return *this;
      }

      self_type operator++(int)  // Post-fix
      {
        self_type it = (*this);
        ++(*this);

        return it;
      }

      self_type operator--()  // Pre-fix
      {
        --m_Current;

        return *this;
      }

      self_type operator--(int)  // Post-fix
      {
        self_type it = (*this);
        --(*this);

        return it;
      }

      [[nodiscard]] bool operator==(const self_type& rhs) const
      {
        return m_Current == rhs.m_Current;
      }

      [[nodiscard]] bool operator!=(const self_type& rhs) const
      {
        return m_Current != rhs.m_Current;
      }

      [[nodiscard]] pointer operator->()
      {
        return m_Current->cast();
      }

      [[nodiscard]] reference operator*()
      {
        return *m_Current->cast();
      }

      [[nodiscard]] pointer value()
      {
        return operator->();
      }
    };

   private:
    struct Node final
    {
      std::aligned_storage_t<sizeof(T), alignof(T)> data_storage;  //!< Must be the first member since this will be treated as a 'T'.
      intrusive::Node<Node>                         list;          //!< The set of pointers for the ListView.

      Node() :
        data_storage{},
        list{}
      {
      }

      T* cast()
      {
        return reinterpret_cast<T*>(&data_storage);
      }

      const T* cast() const
      {
        return reinterpret_cast<const T*>(&data_storage);
      }
    };

   private:
    IMemoryManager&           m_Memory;
    intrusive::ListView<Node> m_InternalList;

   public:
    explicit List(IMemoryManager& memory) :
      m_Memory{memory},
      m_InternalList{&Node::list}
    {
    }

    iterator           begin() { return iterator(m_InternalList.begin()); }
    iterator           end() { return iterator(m_InternalList.end()); }
    T&                 front() { return *m_InternalList.front().cast(); }
    const T&           front() const { return *m_InternalList.front().cast(); }
    T&                 back() { return *m_InternalList.back().cast(); }
    const T&           back() const { return *m_InternalList.back().cast(); }
    [[nodiscard]] bool isEmpty() const { return m_InternalList.isEmpty(); }
    IMemoryManager&    memory() const { return m_Memory; }

    template<typename... Args>
    T& emplaceBack(Args&&... args)
    {
      return insert(end(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    T& emplaceFront(Args&&... args)
    {
      return insert(begin(), std::forward<Args>(args)...);
    }

    iterator makeIterator(T& element) const
    {
      return iterator(m_InternalList.makeIterator(nodeFromElement(element)));
    }

    template<typename... Args>
    T& insert(iterator pos, Args&&... args)
    {
      Node* const node      = m_Memory.allocateT<Node>();
      T* const    node_data = new (&node->data_storage) T(std::forward<Args>(args)...);

      m_InternalList.insert(pos.m_Current, *node);

      return *node_data;
    }

    iterator erase(iterator pos)
    {
      Node* const node = &*pos.m_Current;

      const auto it = iterator(m_InternalList.erase(pos.m_Current));

      node->cast()->~T();
      m_Memory.deallocateT(node);

      return it;
    }

    void popBack()
    {
      erase(--end());
    }

    void popFront()
    {
      erase(begin());
    }

    void clear()
    {
      while (!isEmpty())
      {
        erase(begin());
      }
    }

    ~List()
    {
      clear();
    }

   private:
    static Node& nodeFromElement(T& element)
    {
      return reinterpret_cast<Node&>(element);
    }
  };
}  // namespace bf

#endif /* BIFROST_INTRUSIVE_LIST_HPP */
