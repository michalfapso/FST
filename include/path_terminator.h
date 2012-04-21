#ifndef _PATH_TERMINATOR_H_
#define _PATH_TERMINATOR_H_

#include "exception.h"

template<class Arc>
class PathTerminator {
	public:
		// return value: TRUE = terminate path, FALSE = continue
		// includeArc: TRUE = include the given arc in the path
		virtual bool operator()(const Arc& arc, bool* includeArc) const {
			*includeArc = false;
			return true; 
		}
		// For ParallelArcs:
		virtual bool operator()(int stateId, const ParallelArcs<Arc>& pa, bool* pIncludeArc) const
		{
			bool add_path = false;
			for (typename ParallelArcs<Arc>::const_iterator i = pa.begin(); i != pa.end(); i++) {
				const Arc& arc = **i;
				bool add_path_current_arc = (*this)(arc, pIncludeArc);
				if (add_path && !add_path_current_arc) {
					THROW("ERROR: Among parallel arcs, some are path terminators, but others are not! ("<<stateId<<" -> "<<arc.nextstate<<")");
				}
				add_path |= add_path_current_arc;
			}
			return add_path;
		}
};


#endif
