#ifndef _NODES_H
#define _NODES_H

#include <fst/fst.h>
#include <vector>
#include "container_interface.h"
#include "node.h"

template <class Arc>
class Nodes : public IndexedContainerInterface< std::vector< Node<Arc> > >
{
	public:
		Nodes(const fst::Fst<Arc>& fst) {
			using namespace fst;
			size_t nstates = 0;
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next()) {
				++nstates;
			}
			this->mContainer.resize(nstates);
			InitializeParallelArcs(fst);
			(*this)[0].InitStartNode();
		}

		void InitializeParallelArcs(const fst::Fst<Arc>& fst) 
		{
			using namespace fst;
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next())
			{
				unsigned int state_id = siter.Value();
				Node<Arc>& n = (*this)[state_id];
				for (ArcIterator<Fst<Arc>> aiter(fst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					n.AddArcToParallelArcs(arc);
				}
			}
		}

};

#endif
