#ifndef LIST_H_
#define LIST_H_

#include <new>
#include <stdexcept>
#include "../pnapi/OutOfRangeException.h"

// TODO move to namespace pnapi? or util::collection?
namespace util {
	class ListNode;
	class GenericListNode;
	class LinkedList;

	class Iterator {
	private:
		LinkedList *list;
		ListNode *current;
		ListNode *head;
		ListNode *tail;
	public:
		Iterator(LinkedList *list);
		bool hasNext();
		void reset();
		ListNode *getNext();
		void remove();
	};

	/**
	 * @Deprecated. Use TypedLinkedList instead
	 */
	class LinkedList {
	private:
		ListNode *head;
		ListNode *tail;
		int count;
	public:
		LinkedList();
		virtual ~LinkedList();
		void addTail(ListNode *node);
		void add(ListNode *node);
		void remove(ListNode *node);
		/** returns a new iterator, which must be deleted */
		ListNode *get(int index);
		Iterator *iterator();
		ListNode *getHead()							{ return head;  }
		ListNode *getTail() 						{ return tail;  }
		int getCount() 								{ return count; }
	};
	
	class ListNode {
	private:
		ListNode *next;
		ListNode *prev;
	public:
		ListNode() : next(0), prev(0)				{}
		virtual ~ListNode() 						{}
		void setNext(ListNode *next)				{ this->next = next; }
		void setPrev(ListNode *prev)				{ this->prev = prev; }
		ListNode *getNext()							{ return next; }
		ListNode *getPrev()							{ return prev; }
	};

	class GenericListNode : public ListNode {
	private:
		void *data;
	public:
		GenericListNode(void *data)	: ListNode(), data(data)	{}
		void *getData()											{ return data; }
	};

	template<class T>
	class TypedLinkedList {
	private:
		// Hide copy ctor and assignment operator
		TypedLinkedList<T>(const TypedLinkedList<T> &);
		TypedLinkedList<T> &operator=(const TypedLinkedList<T> &);
		class Node {
			private:
				Node *next;
				Node *prev;
				T elem;
			public:
				Node(T element) : next(0), prev(0), elem(element) {}
				virtual ~Node() {}
				void setNext(Node *next) { 
					this->next = next; 
				}
				void setPrev(Node *prev) { 
					this->prev = prev; 
				}
				Node *getNext() { 
					return next; 
				}
				Node *getPrev() { 
					return prev; 
				}
				T get() { 
					return elem; 
				}
		}; // class TypedNode
		void remove(Node *node) {
			node->getPrev()->setNext(node->getNext());
			node->getNext()->setPrev(node->getPrev());
			count--;
			delete node;
		}
		Node *getHead() { 
			return head;  
		}
		Node *getTail() { 
			return tail;  
		}
	public:
		class TypedIterator {
		private:
			Node *current;
			Node *head;
			Node *tail;
			TypedLinkedList<T> *list;
		public:
			TypedIterator(TypedLinkedList *list) : current(list->getHead()), 
					head(list->getHead()), tail(list->getTail()), list(list) {}
			bool hasNext() {
				return current->getNext() != tail;
			}
			void reset() {
				current = head;
			}
			T getNext() {
				if (!hasNext()) {
					throw pnapi::OutOfRangeException();
				}
				current = current->getNext();
				return current->get();
			}
			void remove() {
				if (current == head) {
					throw pnapi::OutOfRangeException();
				}
				current = current->getPrev();
				list->remove(current->getNext());
			}
		}; // class TypedIterator
		
		Node *head;
		Node *tail;
		int count;
		TypedLinkedList<T>() {
			count = 0;
			head = new Node(T());
			tail = new Node(T());
			head->setNext(tail);
			tail->setPrev(head);
			head->setPrev(NULL);
			tail->setNext(NULL);
		}
		virtual ~TypedLinkedList<T>() {
			removeAll();
			delete head;
			delete tail;
		}
		void addTail(T element) {
			Node *node = new Node(element);
			node->setNext(tail);
			node->setPrev(tail->getPrev());
			tail->getPrev()->setNext(node);
			tail->setPrev(node);
			count++;
		}
		void add(T element) {
			addTail(element);
		}
		/** returns a new iterator, which must be deleted */
		T get(int index) {
			if (index < 0 || index >= count) {
				throw pnapi::OutOfRangeException();
			}
			Node *n = head;
			int ctr;
			for (n = head->getNext(), ctr = 0; n != tail && ctr < index; n = n->getNext(), ctr++);
			return n->get();
		}
		void remove(int index) {
			if (index < 0 || index >= count) {
				throw pnapi::OutOfRangeException();
			}
			TypedIterator iter(this);
			iter.getNext();
			for (int i = 0; i < index; i++) {
				iter.getNext();
			}
			iter.remove();
		}
		void removeAll() {
			if (count == 0) {
				return;
			}
			Node *n = head->getNext(), *next;
			while (n != tail) {
				next = n->getNext();
				delete n;
				n = next;
			}
			count = 0;
			head->setNext(tail);
			tail->setPrev(head);
		}
		TypedIterator iterator() {
			return TypedIterator(this);
		}
		int getCount() { 
			return count; 
		}
	}; // class TypedLinkedList
} // namespace util
#endif /*LIST_H_*/
