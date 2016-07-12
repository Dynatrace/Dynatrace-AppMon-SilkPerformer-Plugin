#ifndef OUTOFRANGEEXCEPTION_H_
#define OUTOFRANGEEXCEPTION_H_

#include <exception>

namespace pnapi {
	class OutOfRangeException : public std::exception {
		const char *msg;
	public:
		OutOfRangeException(const char *message = "Out of range") :msg(message) {};
		virtual ~OutOfRangeException() throw () {};
		const char* what() const throw() { return msg; };
	};
}

#endif // OUTOFRANGEEXCEPTION_H_
