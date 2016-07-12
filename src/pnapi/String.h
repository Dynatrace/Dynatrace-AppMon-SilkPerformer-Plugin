// ----------------------------------------------------------------------------
// file:                        String.h
// ----------------------------------------------------------------------------


#ifndef _String_h_
#define _String_h_

/*
	pnapi::String has the following limitations:
		- pessimistic maximum string length is MAXSHORT (2^31)
		- Unicode not specifically handled, but should work fine
*/

#include <apr.h>
#include <apr_portable.h>
#include <apr_strings.h>

/** This is the name space for the Portable Native API. */
namespace pnapi {

/**
  String represents a Zero ('\\0') terminated array of bytes (corresponding to
  the C type 'char'). It provides the basic functionality for handling ASCII
  character strings. It's signature is designed after the Java class \c java.lang.String \n
  But it allows also for modifications of the content (a long the lines of \c java.lang.StringBuffer).

  Where possible the C++/C Runtime Library API calls are wrapped with the
  Apache Runtime.

  \warning {WARNING! pnapi::String is not thoroughly TESTED ... the code is not 100% mature!!!}

  Sample usage in a C++ program:

  \code

    #include "String.h"

    pnapi::String str("abc");
    pnapi::String s2;

    s2 = str + ".txt";
    if (s2.substring(0,3).equalsIgnoreCase("AbC")) {
       s2.sprintf("==> \"%s\" checked ok!\n", s2.getBytes());
       fprintf(stderr, "%s", s2.getBytes());
    }
  \endcode

 */

// MeJ 25-Aug-2006 ... introduced for the EBCDIC <-> ASCII headache on MVS / OS/390 / z/OS
#ifdef __MVS__
typedef enum {
	FROM_EBCDIC_2_ASCII = 1174,
	FROM_ASCII_2_EBCDIC = 4711,
} eStrCharConversionMode;
#define OsCompliantToAscii(e)	(pnapi::String((e),pnapi::FROM_EBCDIC_2_ASCII).getBytes())
#define AsciiToOsCompliant(a)	(pnapi::String((a),pnapi::FROM_ASCII_2_EBCDIC).getBytes())
#else
// on all NON-EBCDIC platforms these macros shall not anything!
#define OsCompliantToAscii(s)	(s)
#define AsciiToOsCompliant(s)	(s)
#endif

#define ASCII_A (0x41)
#define ASCII_B (0x42)
#define ASCII_C (0x43)
#define ASCII_D (0x44)
#define ASCII_E (0x45)
#define ASCII_F (0x46)
#define ASCII_G (0x47)
#define ASCII_H (0x48)
#define ASCII_I (0x49)
#define ASCII_J (0x4A)
#define ASCII_K (0x4B)
#define ASCII_L (0x4C)
#define ASCII_M (0x4D)
#define ASCII_N (0x4E)
#define ASCII_O (0x4F)
#define ASCII_P (0x50)
#define ASCII_Q (0x51)
#define ASCII_R (0x52)
#define ASCII_S (0x53)
#define ASCII_T (0x54)
#define ASCII_U (0x55)
#define ASCII_V (0x56)
#define ASCII_W (0x57)
#define ASCII_X (0x58)
#define ASCII_Y (0x59)
#define ASCII_Z (0x5A)
#define ASCII_SQUARE_BRACKET_OPEN (0x5B)

class String {

  private:

    /** The buffer for the string characters (bytes). Gets allocated (freed) with
        new char[] (delete []); an empty (or null) string holds at least the zero byte ('\\0').
     */
    char *msByteBuf;

    /** Maximum count of bytes (including the trailing '\\0') which can be stored in msByteBuf */
    unsigned int muByteBufLen;

    /** The current string length (excluding the trailing '\\0'). */
    unsigned int muStrLen;

    void DoMakeString(const char *cstr);

	void DoMakeString(const char *cstr, int length);

    /** Utility function; used internally only. Asserts 0 <= idx < muStrLen.
      \warning *EXITS* the program abnormally if the assertion fails.
     */
    void AssertIndexInRange(int idx) const;

	/** Utility function; used internally only. Asserts 0 <= idx < muStrLen.
      Returns false if assertion not true.
	*/
	bool isValidIndex(int idx) const;

  public:

    /** Allocates or extends the buffer. In the case of extension (buffer exists already)
        the bytes representing the string are preserved. No effect if newByteBufLen is less
        or equal than \a muByteBufLen.
      \warning *EXITS* the program abnormally if the memory allocation fails
      \sa msByteBuf, muByteBufLen
     */
    void ReAlloc(unsigned int newByteBufLen);

    /** Initializes a newly created String object so that it represents an empty string. */
    String();

    /** Constructs a new String object from the incoming array of char
      \param ascii this Zero terminated array of char initializes the new object
    */
    String(const char *ascii);

	/** Constructs a new String object from the incoming array of char
      \param ascii this non Zero terminated array of char initializes the new object
	  \param length specifies the length of the string
    */
    String(const char *ascii, int length);

    /** Initializes a newly created String object from the incoming String object (Copy Constructor).
      \param original a String object
    */
    String(const String &original);

// MeJ 25-Aug-2006 ... introduced for the EBCDIC <-> ASCII headache on MVS / OS/390 / z/OS
#ifdef __MVS__
    /** Construct a new String object from the incoming array of char converted according to
        the mode specified
      \param bytes Zero terminated array of characters
      \param convMode specifies the conversion mode (from ASCII to EBCDIC or vice versa)
    */
    String(const char *bytes, eStrCharConversionMode convMode);
#endif

    /** Destroys this object and cleans up the allocated buffer. */
    ~String();


    /* alphabetically ordered method listing */

    /** Returns the char at index position idx (beginning at 0) */
    char charAt(int idx) const;

    /** Returns 0 if equal; 1 if this string is (naturally) greater than str; -1 if less than str */
    int compareTo(const String &anotherString) const;

    /** Same functionality as compareTo() but ignoring case */
    int compareToIgnoreCase(const String &str) const;


    String& concat(const String &str);

    /** Returns a non zero value if the test evaluates to true, 0 otherwise */
    int endsWith(const String &suffix) const;


    /** Test for equality. Returns a non zero value if the test evaluates to true, 0 otherwise */
    int equals(const String &anotherString) const;

    /** Test for equality. Returns a non zero value if the test evaluates to true, 0 otherwise */
    int equals(const char *cstr) const;

    /** Test for equality ignoring case.
      \sa equals()
     */
    int equalsIgnoreCase(const String &anotherString) const;

    /** Test for equality ignoring case.
      \sa equals()
     */
    int equalsIgnoreCase(const char *cstr) const;


    /** Returns the address of the internal byte buffer. */
    const char *getBytes() const { return msByteBuf; }


    int indexOf(int ch) const;
    int indexOf(int ch, int fromIndex) const;
    int indexOf(const String &str) const;
    int indexOf(const String &str, int fromIndex) const;

    int lastIndexOf(int ch) const;
    int lastIndexOf(int ch, int fromIndex) const;
    int lastIndexOf(const String &str) const;
    int lastIndexOf(const String &str, int fromIndex) const;

    int length() const { return muStrLen; }

    // TODO int matches(const String &regex);

    /** Replaces (inplace) all occurrences of oldChar with newChar. */
    String& replace(int oldChar, int newChar);

    /** Replaces (inplace) all occurrences of target with replacement. 
    	Caution: fails if target and replacement have different length.
    */
    String& replace(const String& target, const String& replacement);
    
	/** Replaces (inplace) all occurrences of orig with replacement. 
		Works also if pattern and replacement have different length. 
	*/
	String& replaceAll(const String& pattern, const String& replacement);

    // TODO String[] split(const String &regex, int limit);

    /** Re-format this string using the specified format string and arguments.
        Consult the C function snprintf() documentation for more details on the
        format semantics.

        \remark
        This method uses apr_vformatter() from the Apache Runtime; there are
        some extensions (see the APR docs for detailed info!). '%p' does
        not work here! You will get a 'bogus \%p' in your output! Use '%pp' instead.\n
        Excerpt from the APR 1.0.0 docs on apr_vformatter():
        <PRE>
        The extensions are:
         \%pA  takes a struct in_addr *, and prints it as a.b.c.d
         \%pI  takes an apr_sockaddr_t * and prints it as a.b.c.d:port
         \%pT takes an apr_os_thread_t * and prints it in decimal
             ('0' is printed if !APR_HAS_THREADS)
         \%pp takes a void * and outputs it in hex
        </PRE>

        \warning The size this String object can grow to is limited to 512.
     */
    String& sprintf(const char *format, ...);

    int startsWith(const String &prefix) const;
    int startsWith(const String &prefix, int toffset) const;

    /** Returns a new String holding the bytes msByteBuf[beginIndex .. muStrLen-1] */
    String substring(int beginIndex) const;

    /** Returns a new String holding the bytes msByteBuf[beginIndex .. endIndex-1] */
    String substring(int beginIndex, int endIndex) const;

    /** Changes (inplace) to lower case. */
    String& toLowerCase();

    /** Changes (inplace) to upper case */
    String& toUpperCase();

    /** Removes (inplace) the leading and trailing whitespace(s). */
    String& trim();

    /** clears the string buffer by writing zeros */
    void clear();

	/** creates a 0 terminated string, filled with the symbol fillChar */
	void fill(char fillChar, int length);

    // allowing byte buffer access with checked index
    char operator [](int idx) const;  // reading

    // C++ allows for operator overloading
    const String& operator =  (const String &rval);
    const String& operator += (const String &rval);
    const String& operator += (const int ch);

    int operator == (const String &rval) const;
    int operator != (const String &rval) const;

    friend String  operator + (const String op1, const String &op2);
};

inline String operator + ( String op1, const String &op2 ) { return op1.concat(op2); }

} // namespace pnapi

#endif // _String_h_
