#ifndef FST_PROPERTIES_H
#define FST_PROPERTIES_H

#include <fst/fst.h>
#include <fst/topsort.h>
#include <list>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include <limits>
#include <numeric>
#include <bitset>
#include <iostream>
#include <iomanip>

using namespace fst;
using namespace std;

class FstProperties {
	public:
		enum LabelType {
			LABEL_INPUT,
			LABEL_OUTPUT,
			LABEL_INPUT_OUTPUT
		};

		template <class Arc>
		static bool IsTopologicallySorted(const Fst<Arc>& fst)
		{
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next()) 
			{
				// Loop through outgoing arcs
				int state_id = siter.Value();
				for (ArcIterator<Fst<Arc>> aiter(fst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					if (arc.nextstate < state_id) {
						return false;
					}
				}
			}
			return true;
		}

		template <class Arc>
		static bool IsFinalState(const Fst<Arc>& fst, unsigned int stateId) {
			//return fst.Final(stateId) != FloatLimits<float>::kPosInfinity;
			return fst.Final(stateId) != Arc::Weight::Zero();
		}

		template <class Arc>
		static bool StateHasNoOutgoingArcs(const Fst<Arc>& fst, unsigned int stateId) {
			ArcIterator<Fst<Arc>> aiter(fst, stateId); 
			return aiter.Done();
		}

		template <class Arc>
		static bool HasOnlyArcsWithPrefix(const Fst<Arc>& fst, const SymbolTable& syms, unsigned int stateId, const string& prefix, LabelType labelType) 
		{
			bool has_arcs = false;
			for (ArcIterator< Fst<Arc> > aiter(fst, stateId); !aiter.Done(); aiter.Next())
			{
				has_arcs = true;
				const Arc &arc = aiter.Value();
				if (labelType == LABEL_INPUT || labelType == LABEL_INPUT_OUTPUT) {
					const string& ilabel = syms.Find(arc.ilabel);
					if (ilabel.substr(0,prefix.length()) != prefix) {
						return false;
					}
				}
				if (labelType == LABEL_OUTPUT || labelType == LABEL_INPUT_OUTPUT) {
					const string& olabel = syms.Find(arc.olabel);
					if (olabel.substr(0,prefix.length()) != prefix) {
						return false;
					}
				}
			}
			return has_arcs;
		}

		template <class Arc>
		static unsigned int StatesCount(const Fst<Arc>& fst) 
		{
			unsigned int nstates = 0;
			for (StateIterator<Fst<Arc>> siter(fst); !siter.Done(); siter.Next()) {
				++nstates;
			}
			return nstates;
		}
	
};

#endif
