#ifndef _STRING_2_FLOAT_H
#define _STRING_2_FLOAT_H

#include <sstream>

float string2float(string s) {
	float f = 0;
	std::istringstream in(s);
	in >> f;
	return f;
}

#endif
