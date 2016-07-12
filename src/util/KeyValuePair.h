#ifndef KEYVALUEPAIR__H__INCLUDED
/********************************************************************************
 *
 * dynaTrace Diagnostics (c) dynaTrace software GmbH 
 * 
 * Filename: Misc.h
 * Created:	 2008/02/18
 *
 *******************************************************************************/
#define KEYVALUEPAIR__H__INCLUDED

namespace util {

/**
 * A standard key value pair class.
 * Note that the template parameters K and V must have a default constructor
 * to allow this template to be instantiated.
 */
template<class K, class V>
class KeyValuePair {
	K k;
	V v;
public:
	KeyValuePair() {
		k = K();
		v = V();
	}
	KeyValuePair(K key, V value) {
		k = key;
		v = value;
	}
	K key() { return k; }
	V value() { return v; }
}; // class KeyValuePair

} // namespace util

#endif // KEYVALUEPAIR__H__INCLUDED
