#ifndef _FST_ARC_PRINTER_H_
#define _FST_ARC_PRINTER_H_

#include <fst/fst.h>
#include <ostream>

template <class FstWeight>
std::ostream& operator<<(std::ostream& oss, const fst::ArcTpl<FstWeight>& arc) {
	return oss << arc.ilabel << ":" << arc.olabel << "/" << arc.weight << "->" << arc.nextstate;
}

#endif
