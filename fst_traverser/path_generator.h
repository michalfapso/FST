#ifndef _PATH_GENERATOR_H
#define _PATH_GENERATOR_H

#include "nodes.h"
#include "parallel_arcs.h"
#include "exception.h"
#include "path.h"
#include "fst_properties.h"
#include "path_terminator.h"

template <class Arc>
class PathGenerator
{
	public:
		enum FinalNodePolicy {
			FINAL_NODE_ADD_PATH,
			FINAL_NODE_IGNORE
		};

		PathGenerator(
				const Fst<Arc>& fst, 
				const Nodes<Arc>& nodes, 
				const PathTerminator<Arc>& pathTerminator, 
				FinalNodePolicy finalNodePolicy) : 
			mFst(fst), 
			mNodes(nodes), 
			mfPathTerminator(pathTerminator), 
			mFinalNodePolicy(finalNodePolicy) 
		{
			if (!FstProperties::IsTopologicallySorted(mFst)) {
				THROW("ERROR: PathGenerator: Fst is not topologically sorted!");
			}
		}

		virtual void GeneratePaths(int startStateId, float startTime, OverlappingPathGroupList<Arc>* pPaths) = 0;

	protected:
		const Fst<Arc>& mFst;
		const Nodes<Arc>& mNodes;
		const PathTerminator<Arc>& mfPathTerminator;
		const FinalNodePolicy mFinalNodePolicy;
};

#endif
