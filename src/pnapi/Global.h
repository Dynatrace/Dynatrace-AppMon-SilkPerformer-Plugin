#ifndef GLOBAL__H__INCLUDED
/********************************************************************************
 *
 * dynaTrace Diagnostics (c) dynaTrace software GmbH
 *
 * Filename: Global.h
 * Created:	 2008/02/18
 *
 * This file contains all global definitions, e.g. typedefs or macros used
 * in Java and .NET.
 *
 *******************************************************************************/
#define GLOBAL__H__INCLUDED

#include <stdlib.h>
#include <apr_pools.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define malloc dtdMalloc
#define realloc dtdRealloc
#define calloc dtdCalloc
#define strdup dtdStrdup

#define apr_pool_create_ex dtdApr_pool_create_ex

#ifdef _WIN32
#define CoTaskMemAlloc dtdCoTaskMemAlloc
LPVOID dtdCoTaskMemAlloc(size_t cp);
#endif // _WIN32

void *dtdMalloc(size_t size);
void *dtdRealloc(void *old, size_t size);
void *dtdCalloc(size_t num, size_t size);
char *dtdStrdup(const char *old);
apr_status_t dtdApr_pool_create_ex(apr_pool_t **newpool, apr_pool_t *parent, apr_abortfunc_t abort_fn, apr_allocator_t *allocator);

#include "../util/List.h"
#include "../util/KeyValuePair.h"

namespace pnapi {

/**
 * A smart pointer container for holding raw data (not objects).
 * NOTE: NEVER add the same pointer more than once! Doing so would cause
 * a double-free -> application crash.
 */
template<typename T = char *>
class LocalPtr {
private:
	util::TypedLinkedList< util::KeyValuePair<T, bool> > vars;
public:
	LocalPtr() {}

	virtual ~LocalPtr() {
		while (vars.getCount() > 0) {
			util::KeyValuePair<T, bool> kvp = vars.get(0);
			T p = kvp.key();
			bool isArray = kvp.value();
			vars.remove(0);
			if (p != NULL) {
				if (isArray)
					delete[] p;
				else
					delete p;
			}
		}
	}

	T add(T var, bool isArray = false) {
		if (var == NULL)
			return var;
		vars.add(util::KeyValuePair<T, bool>(var, isArray));
		return var;
	}

	T release(T var) {
		typename util::TypedLinkedList<
			util::KeyValuePair<T, bool> >::TypedIterator i = vars.iterator();
		for (; i.hasNext();) {
			if (i.getNext().key() == var) {
				i.remove();
				break;
			}
		}
		return var;
	}

	void releaseAll() {
		vars.removeAll();
	}
}; // LocalPtr

template<class T>
class AutoPtr {
private:
	T *ptr;

	/* We hide the copy constructor and the operator=
	 * because using this is very error prone (and boost
	 * does this, too ;-)
	 */
	AutoPtr<T>(const AutoPtr<T> &other);
	AutoPtr<T> &operator=(const AutoPtr<T> &other);
public:
	AutoPtr<T>(T *ptr = 0) : ptr(ptr) {}
	~AutoPtr<T>() {
		if (ptr) {
			delete ptr;
		}
	}
	AutoPtr<T> &operator=(T *rhs) {
		if (ptr) {
			delete ptr;
		}
		ptr = rhs;
		return *this;
	}
	T *release() {
		T *tmp = ptr;
		ptr = NULL;
		return tmp;
	}
	operator T*() const {
		return ptr;
	}
	operator T&() const {
		return *ptr;
	}
	T &operator*() const {
		return *ptr;
	}
	T *&get() {
		return ptr;
	}
	operator T**() {
		return &ptr;
	}
	T *operator->() const {
		return ptr;
	}
}; // class AutoPtr

template<class T>
class ArrayAutoPtr {
private:
	T *ptr;

	/* We hide the copy constructor and the operator=
	 * because using this is very error prone (and boost
	 * does this, too ;-)
	 */
	ArrayAutoPtr<T>(const ArrayAutoPtr<T> &other);
	ArrayAutoPtr<T> &operator=(const ArrayAutoPtr<T> &other);
public:
	ArrayAutoPtr<T>(T *ptr = 0) : ptr(ptr) {}
	~ArrayAutoPtr<T>() {
		if (ptr) {
			delete[] ptr;
		}
	}
	ArrayAutoPtr<T> &operator=(T *rhs) {
		if (ptr) {
			delete[] ptr;
		}
		ptr = rhs;
		return *this;
	}
	T *release() {
		T *tmp = ptr;
		ptr = NULL;
		return tmp;
	}
	operator T*() const {
		return ptr;
	}
	operator T&() const {
		return *ptr;
	}
	T &operator*() const {
		return *ptr;
	}
	T *get() const {
		return ptr;
	}
	//operator T**() const {
	//	return const_cast<T**>(&ptr);
	//}
	T *operator->() const {
		return ptr;
	}
}; // class ArrayAutoPtr

/**
 * Calls a specified function, when the object goes out of scope
 */
template<class T>
class AutoFreePtr {
private:
	T *ptr;

	/* We hide the copy constructor and the operator=
	 * because using this is very error prone (and boost
	 * does this, too ;-)
	 */
	AutoFreePtr<T>(const AutoFreePtr<T> &other);
	AutoFreePtr<T> &operator=(const AutoFreePtr<T> &other);
public:
	AutoFreePtr<T>(T *ptr = 0) : ptr(ptr) {}
	~AutoFreePtr<T>() {
		if (ptr) {
			free(ptr);
		}
	}
	AutoFreePtr<T> &operator=(T *rhs) {
		if (ptr) {
			free(ptr);
		}
		ptr = rhs;
		return *this;
	}
	T *release() {
		T *tmp = ptr;
		ptr = NULL;
		return tmp;
	}
	operator T*() const {
		return ptr;
	}
	operator T&() const {
		return *ptr;
	}
	T &operator*() const {
		return *ptr;
	}
	T *get() const {
		return ptr;
	}
	operator T**() const {
		return const_cast<T**>(&ptr);
	}
	T *operator->() const {
		return ptr;
	}
}; // class AutoFreePtr

/**
 * Global initialization function: Initializes the APR etc.
 */
void initGlobal();

/**
 * Global de-initialization function: shuts down APR etc.
 */
void shutdownGlobal();

} // namespace pnapi

#define RETHROW(type, exception) { char __buf[1024]; char __buf2[10]; sprintf(__buf2, "%i", __LINE__); strncpy(__buf, exception.what(), 1024); strncat(__buf, "\nat " __FILE__ " - ", 1024); strncat(__buf,  __buf2, 1024); throw type(__buf); }

#endif // GLOBAL__H__INCLUDED

