#ifndef DBG_H
#define DBG_H

#include <iostream>
#include <iomanip>

//#define DBG(str) std::cerr << std::dec << std::setprecision(13) << std::setw(20) << std::left << __FILE__ << ':' << std::setw(5) << std::right << __LINE__ << "> " << str << std::endl << std::flush
#define DBG(str) std::cerr << std::dec << std::setw(20) << std::left << __FILE__ << ':' << std::setw(5) << std::right << __LINE__ << "> " << str << std::endl << std::flush
//#define DBG(str)

#endif
