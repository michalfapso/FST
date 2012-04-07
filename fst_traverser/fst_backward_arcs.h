#ifndef _FST_BACKWARD_ARCS_H
#define _FST_BACKWARD_ARCS_H

#include <fst/fst.h>
#include "container_interface.h"

template <class Arc>
struct BackwardArc
{
	const Arc* mpArc;
	unsigned int mPrevState;
	BackwardArc(const Arc* a, unsigned int prevstate) : mpArc(a), mPrevState(prevstate) {}
};

template <class Arc>
class NodeBackwardArcs : public ContainerInterface < std::vector<BackwardArc<Arc> > >
{
	public:
		void Add(BackwardArc<Arc>& ba) {
			this->mContainer.push_back(ba);
		}
};

template <class Arc>
class FstBackwardArcs : public IndexedContainerInterface < std::vector<NodeBackwardArcs<Arc> > >
{
	public:
		
		FstBackwardArcs(const fst::Fst<Arc>& fst)
		{
			using namespace fst;

			unsigned int nstates = 0;
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next()) {
				++nstates;
			}

			this->mContainer.resize(nstates);
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next())
			{
				// Loop through output arcs
				unsigned int state_id = siter.Value();
				for (ArcIterator< Fst<Arc> > aiter(fst, state_id); !aiter.Done(); aiter.Next())
				{
					BackwardArc<Arc> barc(&aiter.Value(), state_id);
					this->mContainer[aiter.Value().nextstate].Add(barc);
				}
			}
		}

		void Print(const fst::SymbolTable& syms)
		{
			cout << "--------------------------------------------------" << endl;
			cout << "FstBackwardArcs::Print()" << endl;
			cout << "--------------------------------------------------" << endl;
			for (size_t n=0; n<this->size(); n++) {
				const NodeBackwardArcs<Arc> &node_back_arcs = this->mContainer[n];
				cout << n << endl;
				for (typename NodeBackwardArcs<Arc>::const_iterator i=node_back_arcs.begin(); i!=node_back_arcs.end(); i++) {
					const BackwardArc<Arc>& barc = *i;
					cout << "  " << syms.Find(barc.mpArc->ilabel) << ":" << syms.Find(barc.mpArc->olabel) << "/" << barc.mpArc->weight << " -> " << barc.mPrevState << endl;
				}
			}
			cout << "--------------------------------------------------" << endl;
		}
};

#endif
