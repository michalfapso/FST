#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <sstream>
#include <stdexcept>

#define THROW(msg) \
	{ \
		std::ostringstream ss; \
		ss << msg; \
		throw std::runtime_error(ss.str()); \
	}


#endif
