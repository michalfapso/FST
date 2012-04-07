#ifndef _FST_HELPER_METHODS_H
#define _FST_HELPER_METHODS_H

class FstHelperMethods {
	public:

		bool HasOnlyForwardTimeLinks(unsigned int stateId) 
		{
			for (ArcIterator<Fst<Arc>> aiter(*mpFst, stateId); !aiter.Done(); aiter.Next())
			{
				const Arc &arc = aiter.Value();
				const string olabel = mpSyms->Find(arc.olabel);

				if (olabel.substr(0,2) != "t=") {
					return false;
				}
			}
			return true;
		}

		bool HasOnlyWordArcs(unsigned int stateId) 
		{
			bool has_arcs = false;
			for (ArcIterator<Fst<Arc>> aiter(*mpFst, stateId); !aiter.Done(); aiter.Next())
			{
				has_arcs = true;
				const Arc &arc = aiter.Value();
				const string& olabel = mpSyms->Find(arc.olabel);
				if (olabel.substr(0,2) != "W=") {
					return false;
				}
			}
			return has_arcs;
		}

		bool IsFinalState(unsigned int stateId) {
			ArcIterator<Fst<Arc>> aiter(*mpFst, stateId); 
			return aiter.Done();
		}

		bool IsTopologicallySorted()
		{
			for (StateIterator<Fst<Arc>> siter(*mpFst); !siter.Done(); siter.Next()) 
			{
				// Loop through output arcs
				int state_id = siter.Value();
				for (ArcIterator<Fst<Arc>> aiter(*mpFst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					if (arc.nextstate < state_id) {
						return false;
					}
				}
			}
			return true;
		}

};

#endif
