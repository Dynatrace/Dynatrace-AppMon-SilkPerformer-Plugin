#include "String.h"
#include "OutOfRangeException.h"

#include <stdlib.h>

#ifdef __MVS__
#include <unistd.h>  // 25-Aug-2006 MeJ for  __etoa() / __atoe() on MVS / OS/390 / z/OS
#endif

#include <string.h>
#include <assert.h>

#include <apr_lib.h>

// Global.h always has to be the _last_ include
#include "../pnapi/Global.h"

namespace pnapi {

static const char gTheEmptyAsciiStr[] = { '\0' };

//#define SHOW_STRING_BYTE_STATS
#ifdef SHOW_STRING_BYTE_STATS
static int gStringBytesInUse = 0;
#endif

//#define _ALLOC_DEBUG

/* this is the only function to modify muByteBufLen */
void String::ReAlloc(unsigned int newByteBufLen) {
	if (newByteBufLen <= muByteBufLen) return;

	char *oldBuf = msByteBuf;

	msByteBuf = new char[newByteBufLen];
#ifdef _ALLOC_DEBUG
	fprintf(stderr, "realloc: old:  %p   new: %p \n", oldBuf, msByteBuf);
#endif

#ifdef SHOW_STRING_BYTE_STATS
	gStringBytesInUse += (newByteBufLen - muByteBufLen);
	fprintf(stderr, "[StringBytes: (-%d+%d)%d]", muByteBufLen, newByteBufLen, gStringBytesInUse);
#endif
	muByteBufLen = newByteBufLen;
	if (oldBuf != NULL) {
		apr_cpystrn(msByteBuf, oldBuf, muByteBufLen);
		delete [] oldBuf;
	}
}

void String::clear() {
	memset(msByteBuf, 0, muByteBufLen);
	muStrLen = 0;
}

void String::fill(char fillChar, int length) {
	ReAlloc(length + 1);
	clear();
	memset(msByteBuf, fillChar, length);
	muStrLen = length;
}

inline void String::AssertIndexInRange(int idx) const {
	if (!isValidIndex(idx)) {
		throw OutOfRangeException();
	}
}

inline bool String::isValidIndex(int idx) const {
	return (idx >= 0 && idx < (int)muStrLen);
}

void String::DoMakeString(const char *cstr) {
	msByteBuf = NULL;
	muByteBufLen = 0;
	muStrLen = 0;

	if (cstr == NULL)
		cstr = gTheEmptyAsciiStr;
	muStrLen = static_cast<int>(strlen(cstr));

	ReAlloc(muStrLen + 1);  // add another byte for the trailing Zero byte
	apr_cpystrn(msByteBuf, cstr, muByteBufLen);
}

void String::DoMakeString(const char *cstr, int length) {
	msByteBuf = NULL;
	muByteBufLen = 0;
	muStrLen = 0;

	if (cstr == NULL)
		cstr = gTheEmptyAsciiStr;
	muStrLen = length;
	ReAlloc(muStrLen + 1);  // add another byte for the trailing Zero byte
	apr_cpystrn(msByteBuf, cstr, muByteBufLen);
}

String::String() {
	DoMakeString(gTheEmptyAsciiStr, 0);
}

String::String(const char *ascii) {
	DoMakeString(ascii);
}

String::String(const char *ascii, int length) {
	DoMakeString(ascii, length);
}

String::String(const String &original) {
	DoMakeString(original.getBytes());
}

// MeJ 25-Aug-2006 ... introduced for the EBCDIC <-> ASCII headache on MVS / OS/390 / z/OS
#ifdef __MVS__
String::String(const char *bytes,  eStrCharConversionMode convMode) {
	DoMakeString(bytes);
	switch(convMode) {
		case FROM_EBCDIC_2_ASCII:
			__etoa(msByteBuf);
			break;
		case FROM_ASCII_2_EBCDIC:
			__atoe(msByteBuf);
			break;
		default:
			assert(false);
			break;
	}
}
#endif

String::~String() {
#ifdef SHOW_STRING_BYTE_STATS
	gStringBytesInUse -= muByteBufLen;
	fprintf(stderr, "[StringByte: (-%d)%d]", muByteBufLen, gStringBytesInUse);
#endif
	if (msByteBuf != NULL) delete [] msByteBuf;
}


char String::operator[](int idx) const {
	return charAt(idx);
}

const String& String::operator = (const String &rval) {
	if (this == &rval) return *this;
	if (rval.muStrLen > muStrLen) ReAlloc(rval.muStrLen + 1);
	muStrLen = rval.muStrLen;
	apr_cpystrn(msByteBuf, rval.msByteBuf, muByteBufLen);
	return *this;
}

const String& String::operator += (const String &rval) {
	ReAlloc(muStrLen + rval.muStrLen + 1);
	apr_cpystrn(msByteBuf + muStrLen, rval.msByteBuf, muByteBufLen - muStrLen);
	muStrLen += rval.muStrLen;
	return *this;
}

const String& String::operator += (const int ch) {
	if (muStrLen + 1 + 1 >= muByteBufLen) ReAlloc(2 * muByteBufLen + 1);
	msByteBuf[muStrLen++] = (char) ch;
	msByteBuf[muStrLen] = '\0';
	return *this;
}

int String::operator == (const String &rval) const {
	return equals(rval);
}

int String::operator != (const String &rval) const {
	return !equals(rval);
}

char String::charAt(int idx) const {
	AssertIndexInRange(idx);
	return msByteBuf[idx];
}

int String::compareTo(const String &anotherString) const {
	return (apr_strnatcmp(msByteBuf, anotherString.msByteBuf));
}

int String::compareToIgnoreCase(const String &anotherString) const {
	return (apr_strnatcasecmp(msByteBuf, anotherString.msByteBuf));
}

String& String::concat(const String &str) {
	(*this) += str;
	return *this;
}

int String::endsWith(const String &suffix) const {
	if (muStrLen < suffix.muStrLen) return FALSE;
	return (apr_strnatcmp(msByteBuf+muStrLen-suffix.muStrLen, suffix.msByteBuf) == 0);
}

int String::equals(const String &anotherString) const {
	if (muStrLen != anotherString.muStrLen) return FALSE;
	return (apr_strnatcmp(msByteBuf, anotherString.msByteBuf) == 0);
}

int String::equalsIgnoreCase(const String &anotherString) const {
	if (muStrLen != anotherString.muStrLen) return FALSE;
	return (apr_strnatcasecmp(msByteBuf, anotherString.msByteBuf) == 0);
}

int String::equals(const char *cstr) const {
	if (!cstr) return FALSE;
	unsigned int len2 = static_cast<int>(strlen(cstr));
	return (muStrLen == len2) && (apr_strnatcmp(msByteBuf, cstr) == 0);
}

int String::equalsIgnoreCase(const char *cstr) const {
	if (!cstr) return FALSE;
	unsigned int len2 = static_cast<int>(strlen(cstr));
	return (muStrLen == len2) && (apr_strnatcasecmp(msByteBuf, cstr) == 0);
}

int String::indexOf(int ch) const {
	char *cp = strchr(msByteBuf, ch);
	return (cp == NULL) ? -1 : static_cast<int>(cp - msByteBuf);
}

int String::indexOf(int ch, int fromIndex) const {
	if (!isValidIndex(fromIndex)) {
		return -1;
	}

	char *cp = strchr(msByteBuf+fromIndex, ch);
	return (cp == NULL) ? -1 : static_cast<int>(cp - msByteBuf);
}

int String::indexOf(const String &str) const {
	return indexOf(str, 0);
}

int String::indexOf(const String &str, int fromIndex) const {
	if (!isValidIndex(fromIndex)) {
		return -1;
	}

	char *cp = strstr(msByteBuf+fromIndex, str.getBytes());
	return (cp == NULL) ? -1 : (int)(cp - msByteBuf);
}

int String::lastIndexOf(int ch) const {
	char *cp = strrchr(msByteBuf, ch);
	return (cp == NULL) ? -1 : (int)(cp - msByteBuf);
}

int String::lastIndexOf(int ch, int fromIndex) const {
	if (!isValidIndex(fromIndex)) {
		return -1;
	}
	char saveCh = msByteBuf[fromIndex];
	msByteBuf[fromIndex] = '\0';
	char *cp = strrchr(msByteBuf, ch);
	msByteBuf[fromIndex] = saveCh;
	return (cp == NULL) ? -1 : static_cast<int>(cp - msByteBuf);
}

int String::lastIndexOf(const String &str) const {
	return lastIndexOf(str, muStrLen - str.muStrLen);
}

int String::lastIndexOf(const String &str, int fromIndex) const {
	if (!isValidIndex(fromIndex)) {
		return -1;
	}
	if (str.muStrLen == 0) return -1;

	char primus = str[0];
	int i;

	for (i = fromIndex; i >= 0; i-- ) {
		if (msByteBuf[i] == primus && (strncmp(str.msByteBuf, msByteBuf+i, str.muStrLen) == 0))
			return i;
	}
	return -1;
}

String& String::replace(int oldChar, int newChar) {
	char *cp = msByteBuf;
	while (*cp) {
		if (*cp == oldChar) *cp = (char)newChar;
		cp++;
	}
	return (*this); //BG-20041029 inserted
}

String& String::replace(const String& target, const String& replacement) {
	assert(target.muStrLen == replacement.muStrLen);
	int pos, fromPos = 0;
	while ((pos = indexOf(target, fromPos)) != -1) {
		(*this) = substring(0, pos) + replacement + substring(pos + target.muStrLen);
		fromPos = pos + target.muStrLen;
	}
	return (*this);
}

String& String::replaceAll(const String& pattern, const String& replacement) {
	assert(pattern.muStrLen > 0);
	unsigned int fromIndex = 0;
	unsigned int toIndex = 0;
	while (indexOf(pattern,fromIndex) != -1) {
		fromIndex = indexOf(pattern, fromIndex);
		toIndex = fromIndex + pattern.muStrLen - 1;
		(*this) = (fromIndex > 0 ? this->substring(0, fromIndex) : String())
				   + replacement
				   + (toIndex < this->muStrLen -1 ? this->substring(toIndex + 1): String());
		fromIndex = fromIndex + replacement.muStrLen;
	}
	return (*this);
}

String& String::sprintf(const char *format, ...) {
	ReAlloc(512+1);
	va_list ap;
	va_start(ap, format);
	apr_vsnprintf(msByteBuf, muByteBufLen, format, ap);
	va_end(ap);
	muStrLen = static_cast<int>(strlen(msByteBuf));
	return *this;
}

int String::startsWith(const String &prefix) const {
	if (muStrLen < prefix.muStrLen) return 0;
	return startsWith(prefix, 0);
}

int String::startsWith(const String &prefix, int toffset) const {
	AssertIndexInRange(toffset);
	if(toffset >= static_cast<int>(muStrLen)) toffset = muStrLen - 1;
	if (muStrLen - toffset < prefix.muStrLen) return 0;
	return strncmp(prefix.msByteBuf, msByteBuf+toffset, prefix.muStrLen) == 0;
}

/** Returns a new String holding the bytes msByteBuf[beginIndex .. muStrLen-1] */
String String::substring(int beginIndex) const {
	return substring(beginIndex, muStrLen);
}

/** Returns a new String holding the bytes msByteBuf[beginIndex .. endIndex-1] */
String String::substring(int beginIndex, int endIndex) const {
	if (beginIndex < 0)						throw OutOfRangeException();
	if ((unsigned)beginIndex > muStrLen)	throw OutOfRangeException();
	if (endIndex < 0)						throw OutOfRangeException();
	if ((unsigned)endIndex > muStrLen)		throw OutOfRangeException();
	if (beginIndex > endIndex)				throw OutOfRangeException();

	String result;
	result.muStrLen = endIndex - beginIndex;
	result.ReAlloc(result.muStrLen + 1);
	apr_cpystrn(result.msByteBuf, msByteBuf + beginIndex, result.muStrLen + 1);
	return result;
}

/** Changes (inplace) to lower case. */
String& String::toLowerCase() {
	char *cp = msByteBuf;
	while (*cp) { *cp = static_cast<char>(apr_tolower(*cp)); cp++; }
	return (*this);
}

/** Changes (inplace) to upper case */
String& String::toUpperCase() {
	char *cp = msByteBuf;
	while (*cp) { *cp = static_cast<char>(apr_toupper(*cp)); cp++; }
	return (*this);
}

/** Removes (inplace) the leading and trailing whitespace(s). */
String& String::trim() {
	char *cp = msByteBuf;

	int leadingSpaces = 0;
	while (*cp && apr_isspace(*cp)) { cp++; leadingSpaces++; }

	int t, j, lastPrintable = muStrLen - 1;
	while (lastPrintable >= 0 && apr_isspace(msByteBuf[lastPrintable])) {
		lastPrintable--;
	}

	t = 0;
	for (j = leadingSpaces; j <= lastPrintable; j++) {
		msByteBuf[t++] = msByteBuf[j];
	}
	msByteBuf[t] = '\0';
	muStrLen = t;

	return (*this);
}

} // namespace pnapi

