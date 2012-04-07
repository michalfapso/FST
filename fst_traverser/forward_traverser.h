#ifndef FORWARD_TRAVERSER_H
#define FORWARD_TRAVERSER_H

#include <fst/fst.h>
#include <fst/topsort.h>
#include <stdexcept>

#include "dbg.h"
#include "detection_end.h"
#include "node.h"
#include "nodes.h"
#include "string2float.h"
#include "fst_backward_arcs.h"
#include "fst_properties.h"
#include "path_generator.h"

using namespace fst;
using namespace std;

template <class Arc>
class ForwardTraverser {
	protected:
		//--------------------------------------------------
		// PathTerminatorTermEnd {{{
		class PathTerminatorTermEnd : public PathGenerator<Arc>::PathTerminator
		{
			public:
				PathTerminatorTermEnd(const SymbolTable& syms) {
					std::string term_end_str = "TERM_END";
					SYMBOL_TERM_END = syms.Find(term_end_str);
					if (SYMBOL_TERM_END == SymbolTable::kNoSymbol) {
						throw runtime_error("PathTerminatorTermEnd: symbol '"+term_end_str+"' not found!");
					}
				}
				virtual bool operator()(const Arc& arc, bool* includeArc) const { 
					includeArc = false;
					return arc.ilabel == SYMBOL_TERM_END;
				}
			protected:
				int SYMBOL_TERM_END;
		};
		// }}}
		//--------------------------------------------------
	public:
		typedef typename Arc::Weight Weight;

		ForwardTraverser(const Fst<Arc>* pFst, const SymbolTable* pSyms, PathPool<Arc>* pPathPool) :  
			mpFst(pFst),
			mpSyms(pSyms),
			mNodes(*pFst),
			mpPathPool(pPathPool),
			mBackwardArcs(*pFst)
		{ 
			if (!FstProperties::IsTopologicallySorted(*mpFst)) {
				THROW("ERROR: ForwardTraverser: Fst is not topologically sorted!");
			}
			Path<Arc>::SetSymbols(mpSyms);
			//mBackwardArcs.Print(*mpSyms);
		}
	
		void Traverse()
		{
			DBG("Traverse()");

			// Loop through states
			for (StateIterator<Fst<Arc>> siter(*mpFst); !siter.Done(); siter.Next())
			{
				// Loop through output arcs
				unsigned int state_id = siter.Value();
				Node<Arc>& n = mNodes[state_id];
//				DBG("state_id: "<<state_id);

				DBG("");
				DBG(fixed << setprecision(10) << "alpha "<<state_id<<" "<<n.GetAlpha());

				// Detections for the current state have to be filled in before traversing the arcs
				//WordLinksToDetections(state_id);

				for (ArcIterator<Fst<Arc>> aiter(*mpFst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					const string olabel = mpSyms->Find(arc.olabel);
					const string ilabel = mpSyms->Find(arc.ilabel);
					Node<Arc>& n_next = mNodes[arc.nextstate];

					DBG("");
					DBG(state_id<<" -> "<<arc.nextstate<<" ("<<ilabel<<":"<<olabel<<")");

					n_next.ComputeAlpha(n, arc);
					n_next.SetStartTime(n, arc, olabel);

					if (ilabel == "TERM_START") {
						DBG("TERM_START");
						DBG(n);
						PathTerminatorTermEnd path_term(*mpSyms);
						PathGenerator<Arc> path_gen(*mpFst, mNodes, path_term, PathGenerator<Arc>::FINAL_NODE_IGNORE);
						OverlappingPathGroupList<Arc> paths;
						path_gen.GeneratePaths(arc.nextstate, n.GetStartTime(), &paths);
						DBG("Generated paths:");
						paths.Print("_DETECTION_");
						DBG("Generated paths end");
					} else if (ilabel == "TERM_END") {
					}

					n_next.AddFwdPathsCount(n.GetFwdPathsCount());
					n_next.AddFwdPhonemesCount(n.GetFwdPhonemesCount() + (IsPhoneme(olabel) ? n.GetFwdPathsCount() : 0));
				}
			}
		}


	protected:

		void AddToWordDetectionsMap(string word, unsigned int stateId, DetectionEnd<Arc> det)
		{
			Node<Arc>& n = mNodes[stateId];
			typename Node<Arc>::Word2DetectionMap::iterator it = n.mWord2WeightMap.find(word);
			if (it != n.mWord2WeightMap.end()) {
				if (it->second.mWeight.Value() > det.mWeight.Value()) {
					DBG("AddToWordDetectionsMap('"<<word<<"', "<<stateId<<", "<<det.mWeight<<"): original weight="<<it->second.mWeight<<" is already in the map ...replacing with the new weight");
					it->second.mWeight = det.mWeight;
				} else {
					DBG("AddToWordDetectionsMap('"<<word<<"', "<<stateId<<", "<<det.mWeight<<"): original weight="<<it->second.mWeight<<" is already in the map ...keeping the old weight");
				}
				if (it->second.mEndTime < det.mEndTime) {
					it->second.mEndTime = det.mEndTime;
				}
			} else {
				DBG("AddToWordDetectionsMap("<<stateId<<"): adding word '"<<word<<"' (weight="<<det.mWeight<<")");
				n.mWord2WeightMap.insert(make_pair(word, det));
			}
		}

		void WordLinksToDetections(unsigned int stateId)
		{
			const Node<Arc>& n = mNodes[stateId];
			for (ArcIterator<Fst<Arc>> aiter(*mpFst, stateId); !aiter.Done(); aiter.Next())
			{
				const Arc& arc = aiter.Value();
				const string olabel = mpSyms->Find(arc.olabel);
				Node<Arc>& n_next = mNodes[arc.nextstate];

				// For word links, take also the following timelinks and put the detections into the current node
				if (olabel.substr(0,2) == "W=") 
				{
					string word = olabel.substr(2);
					//n_next.SetAlpha(ComputeAlpha(stateId, arc));
					// Now all next outgoing links should be timelinks with label 't=...'
					for (ArcIterator<Fst<Arc>> aiter_next(*mpFst, arc.nextstate); !aiter_next.Done(); aiter_next.Next())
					{
						const Arc &arc_next = aiter_next.Value();
						const string olabel_next = mpSyms->Find(arc_next.olabel);

						// This has to be a timelink ending in a final state
						assert(olabel_next.substr(0,2) == "t=");
						assert(IsFinalState(arc_next.nextstate));

						float time = string2float(olabel_next.substr(2));
						typename Arc::Weight weight = Times(Times(n.GetAlpha(), arc.weight), arc_next.weight);
						DBG("Detection weight = alpha["<<stateId<<"] + l["<<stateId<<"->"<<arc.nextstate<<"] + l["<<arc.nextstate<<"->"<<arc_next.nextstate<<"] = "<<n.GetAlpha()<<" + "<<arc.weight<<" + "<<arc_next.weight);

						// For end time links, add the weight to the Word2DetectionMap
						AddToWordDetectionsMap(word, stateId, DetectionEnd<Arc>(weight, time));
					}
				}
			}
		}

		bool IsOnPath(const Node<Arc>& n, Node<Arc>& n_next, unsigned int stateId, const Arc& arc, const vector<string>& allowedPath)
		{
			string olabel = mpSyms->Find(arc.olabel);
			if (n.mPathPosition >= 0)
			{
				if (olabel == "<eps>")
				{
					n_next.mPathPosition = n.mPathPosition;
					return true;
				}
				else 
				if (olabel == allowedPath[n.mPathPosition])
				{
					n_next.mPathPosition = n.mPathPosition + 1;
					return true;
				}
			}
			return false;
		}

		bool IsPhoneme(const string& s) 
		{
			return 
				s != "<eps>" && 
				s != "sil" &&
				s != "sp" &&
				!(s[0] == '_' && s[s.length()] == '_') &&
				s.substr(0,2) != "t=" &&
				s.substr(0,2) != "W=";
		}

	protected:
		static const float SYMBOLS_TIME_STEP = 0.01;

		const Fst<Arc>* mpFst;
		const SymbolTable* mpSyms;
		
		Nodes<Arc> mNodes;
		PathPool<Arc>* mpPathPool;
		FstBackwardArcs<Arc> mBackwardArcs;
};

#endif
