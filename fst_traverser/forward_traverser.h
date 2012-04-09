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
#include "path_generator_forward.h"
#include "is_phoneme.h"

using namespace fst;
using namespace std;

template <class Arc>
class ForwardTraverser {
	protected:
		//--------------------------------------------------
		// PathTerminatorTermEnd {{{
		class PathTerminatorTermEnd : public PathTerminator<Arc>
		{
			public:
				PathTerminatorTermEnd(const SymbolTable& syms) : PathTerminator<Arc>() 
				{
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
		typedef typename Nodes<Arc>::Node Node;

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

			//Path<Arc>::SetPrintType(PRINT_ALL);
			//Path<Arc>::SetPrintType(PRINT_NODES_ONLY);
			Path<Arc>::SetPrintType(PRINT_PHONEMES_ONLY);
		}
	
		void Traverse()
		{
			DBG("Traverse()");

			// Loop through states
			for (StateIterator<Fst<Arc>> siter(*mpFst); !siter.Done(); siter.Next())
			{
				// Loop through output arcs
				unsigned int state_id = siter.Value();
				Node& n = mNodes[state_id];
				//DBG("state_id: "<<state_id);

				//DBG("");
				//DBG(fixed << setprecision(10) << "alpha "<<state_id<<" "<<n.GetAlpha());

				// Detections for the current state have to be filled in before traversing the arcs
				//WordLinksToDetections(state_id);

				for (ArcIterator<Fst<Arc>> aiter(*mpFst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					const string olabel = mpSyms->Find(arc.olabel);
					const string ilabel = mpSyms->Find(arc.ilabel);
					Node& n_next = mNodes[arc.nextstate];

					//DBG("");
					//DBG(state_id<<" -> "<<arc.nextstate<<" ("<<ilabel<<":"<<olabel<<")");

					n_next.ComputeAlpha(n, arc);
					n_next.SetStartTime(n, arc, olabel);

					if (ilabel == "TERM_START") {
						DBG("TERM_START");
						DBG(n);
						PathTerminatorTermEnd path_term(*mpSyms);
						PathGeneratorForward<Arc> path_gen(*mpFst, mNodes, path_term, PathGenerator<Arc>::FINAL_NODE_IGNORE);
						OverlappingPathGroupList<Arc> paths;
						path_gen.GeneratePaths(arc.nextstate, n.GetStartTime(), &paths);
						DBG("Generated paths:");
						paths.Print("_DETECTION_");
						DBG("Generated paths end");
					} else if (ilabel == "TERM_END") {
					}

					n_next.AddFwdPathsCount(n.GetFwdPathsCount());
					n_next.AddFwdPhonemesCount(n.GetFwdPhonemesCount() + (is_phoneme(olabel) ? n.GetFwdPathsCount() : 0));
				}
			}
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
