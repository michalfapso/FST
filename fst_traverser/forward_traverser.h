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
#include "path.h"

using namespace fst;
using namespace std;

template <class Path>
class ForwardTraverser {
	public:
		typedef typename Path::Arc Arc;
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
					*includeArc = false;
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

		ForwardTraverser(const Fst<Arc>* pFst, const SymbolTable* pSyms) :  
			mpFst(pFst),
			mpSyms(pSyms),
			mNodes(*pFst, pSyms),
			mBackwardArcs(*pFst)
		{ 
			if (!FstProperties::IsTopologicallySorted(*mpFst)) {
				THROW("ERROR: ForwardTraverser: Fst is not topologically sorted!");
			}
			Path::SetSymbols(mpSyms);
			Path::SetPrintType(PRINT_ALL); // PRINT_PHONEMES_ONLY | PRINT_NODES_ONLY | PRINT_ALL
		}
	
		void Traverse(OverlappingPathGroupList<Path>* pPathsOut)
		{
			//DBG("Traverse()");

			PathTerminatorTermEnd path_term(*mpSyms);
			PathGeneratorForward<Path> path_gen(*mpFst, mNodes, path_term, PathGenerator<Path>::FINAL_NODE_IGNORE);

			// Loop through states
			for (StateIterator<Fst<Arc>> siter(*mpFst); !siter.Done(); siter.Next())
			{
				// Loop through output arcs
				unsigned int state_id = siter.Value();
				Node& n = mNodes[state_id];
				//DBG("state_id: "<<state_id);

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
						//DBG("TERM_START");
						//DBG(n);
						//if (arc.nextstate != 83715) continue;
						OverlappingPathGroupList<Path> paths;
						path_gen.GeneratePaths(arc.nextstate, n.GetStartTime(), &paths);
						pPathsOut->Add(paths);
						//DBG("Generated paths:");
						//OverlappingPathGroup<Path>::PrintAllPathsInGroup(true);
						//OverlappingPathGroup<Path>::PrintBestPathInGroup(false);
						//paths.Print("_DETECTION_");
						//DBG("Generated paths end");
					} else if (ilabel == "TERM_END") {
					}

					n_next.AddFwdPathsCount(n.GetFwdPathsCount());
					n_next.AddFwdPhonemesCount(n.GetFwdPhonemesCount() + (is_phoneme(olabel) ? n.GetFwdPathsCount() : 0));
				}
			}
		}


	protected:
		static constexpr float SYMBOLS_TIME_STEP = 0.01;

		const Fst<Arc>* mpFst;
		const SymbolTable* mpSyms;
		
		Nodes<Arc> mNodes;
		FstBackwardArcs<Arc> mBackwardArcs;
};

#endif
