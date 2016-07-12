#include "List.h"
#include "../pnapi/Global.h"

namespace util {
	Iterator::Iterator(LinkedList *list) {
		this->list = list;
		head = list->getHead();
		tail = list->getTail();
		current = head;
	}
	
	bool Iterator::hasNext() {
		return current->getNext() != tail;
	}
	
	void Iterator::reset() {
		current = head;
	}
	
	ListNode *Iterator::getNext() {
		current = current->getNext();
		return current;
	}
	
	void Iterator::remove() {
		current = current->getPrev();
		list->remove(current->getNext());
	}

	LinkedList::LinkedList() : count(0) {
		head = new ListNode();
		tail = new ListNode();
		head->setNext(tail);
		tail->setPrev(head);
	}
	
	LinkedList::~LinkedList() {
		ListNode *n = head, *next;
		while (n) {
			next = n->getNext();
			delete n;
			n = next;
		}
	}
	
	ListNode *LinkedList::get(int index) {
		if (count <= index)
			return NULL;	// TODO throw exception
		if (index < 0)
			return NULL;	// TODO throw exception
		ListNode *node = head;
		for (int i = 0; i <= index; i++)
			node = node->getNext();
		return node;
	}

	Iterator *LinkedList::iterator() {
		Iterator *iter = new Iterator(this);
		return iter;
	}
	
	void LinkedList::addTail(ListNode *node) {
		node->setNext(tail);
		node->setPrev(tail->getPrev());
		tail->getPrev()->setNext(node);
		tail->setPrev(node);
		count++;
	}

	void LinkedList::add(ListNode *node) {
		addTail(node);
	}
	
	void LinkedList::remove(ListNode *node) {
		node->getPrev()->setNext(node->getNext());
		node->getNext()->setPrev(node->getPrev());
		count--;
	}
}
