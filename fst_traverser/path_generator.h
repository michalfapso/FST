#ifndef _PATH_GENERATOR_H
#define _PATH_GENERATOR_H

#include "nodes.h"
#include "parallel_arcs.h"
#include "exception.h"
#include "path.h"

template <class Arc>
class PathGenerator
{
	public:
		struct PathTerminator {
			// return value: TRUE = terminate path, FALSE = continue
			// includeArc: TRUE = include the given arc in the path
			virtual bool operator()(const Arc& arc, bool* includeArc) const { 
				includeArc = false;
				return true; 
			}
		};

		enum FinalNodePolicy {
			FINAL_NODE_ADD_PATH,
			FINAL_NODE_IGNORE
		};

		PathGenerator(const Fst<Arc>& fst, const Nodes<Arc>& nodes, const PathTerminator& pathTerminator, FinalNodePolicy finalNodePolicy) : 
			mFst(fst), 
			mNodes(nodes), 
			mfPathTerminator(pathTerminator), 
			mFinalNodePolicy(finalNodePolicy) 
		{}

		void GeneratePaths(int startStateId, float startTime, OverlappingPathGroupList<Arc>* pPaths)
		{
			DBG("PathGenerator::GeneratePaths()");
			PathAvgWeight<Arc> init_path(startStateId, startTime);
//			PathMultWeight<Arc> init_path(startStateId, startTime);
			Path<Arc>::SetPrintType(PRINT_ALL);
			//Path<Arc>::SetPrintType(PRINT_NODES_ONLY);
			GeneratePaths_recursive(startStateId, init_path, pPaths);
		}
	protected:
		bool PathTerminatorForParallelArcs(int stateId, const ParallelArcs<Arc>& pa, bool* pIncludeArc)
		{
			bool add_path = false;
			for (typename ParallelArcs<Arc>::const_iterator i = pa.begin(); i != pa.end(); i++) {
				const Arc& arc = **i;
				bool add_path_current_arc = mfPathTerminator(arc, pIncludeArc);
				if (add_path_current_arc) {
					DBG("add_path_current_arc");
					exit(1);
				}
				if (arc.olabel == 100217) {
					DBG("TERM_END found!");
					exit(1);
				}
				if (add_path && !add_path_current_arc) {
					THROW("ERROR: Among parallel arcs, some are path terminators, but others are not! ("<<stateId<<" -> "<<arc.nextstate<<")");
				}
				add_path |= add_path_current_arc;
			}
			return add_path;
		}

		void GeneratePaths_recursive(typename Arc::StateId stateId, Path<Arc>& path, OverlappingPathGroupList<Arc> *pPaths)
		{
			DBG("GeneratePaths_recursive() path:"<<path);
			ArcIterator< Fst<Arc> > iarc(mFst, stateId);
			// FINAL STATE REACHED
			if (iarc.Done())
			{
				DBG("Final state reached!");
				exit(1);
				// Skip dead-ends (with zero probability).
				if (mFinalNodePolicy == FINAL_NODE_ADD_PATH && mFst.Final(stateId) != Arc::Weight::Zero()) {
					cout << "Adding path: " << path << endl;
					pPaths->Add(path);
				}
			}
			// THERE ARE SOME LINKS FROM THE CURRENT STATE
			else
			{
				const Node<Arc>& n = mNodes[stateId];
				// Loop through n.GetParallelArcs()
				for (typename Node<Arc>::Nextnode2ParallelArcs::const_iterator i = n.GetParallelArcs().begin(); i != n.GetParallelArcs().end(); i++) 
				{
					int nextstate = i->first;
					const ParallelArcs<Arc>& pa = i->second;
					DBG("pa: "<<pa);
					bool include_arc = false;
					bool add_path = PathTerminatorForParallelArcs(stateId, pa, &include_arc);

					if (add_path) {
						DBG("ADD_PATH!!!");
						exit(1);
					}
					if (add_path && !include_arc) { cout << "Adding path: " << path << endl; pPaths->Add(path); }
					path.push_back(&pa);
					if (add_path && include_arc) { cout << "Adding path: " << path << endl; pPaths->Add(path); }
					// dive into the link only when it is not a "terminal" link
					if (!add_path) {
						GeneratePaths_recursive(nextstate, path, pPaths);
					}
					path.pop_back();
				}
			}
		}

	protected:
		const Fst<Arc>& mFst;
		const Nodes<Arc>& mNodes;
		const PathTerminator& mfPathTerminator;
		const FinalNodePolicy mFinalNodePolicy;
};

#endif
