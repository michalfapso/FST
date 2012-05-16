#ifndef _PATH_GENERATOR_FORWARD_H
#define _PATH_GENERATOR_FORWARD_H

#include "path_generator.h"
#include "node.h"

template <class Path>
class PathGeneratorForward : public PathGenerator<Path>
{
	protected:
		typedef typename Path::Arc Arc;
		typedef typename Nodes<Arc>::Node Node;
	public:
		PathGeneratorForward(
				const Fst<Arc>& fst, 
				const Nodes<Arc>& nodes, 
				const PathTerminator<Arc>& pathTerminator, 
				typename PathGenerator<Path>::FinalNodePolicy finalNodePolicy) : 
			PathGenerator<Path>(fst, nodes, pathTerminator, finalNodePolicy)
		{
		}

		virtual void GeneratePaths(int startStateId, float startTime, OverlappingPathGroupList<Path>* pPaths)
		{
			typedef Node_BestPath< Path, Node_Base<Arc> > NBP;
			typedef vector< NBP > VNBP;
			VNBP vnbp;
			vnbp.resize(this->mNodes.size());

			//DBG("PathGeneratorForward::GeneratePaths()");

			vnbp[startStateId].SetBestPathStartStateId(startStateId);
			vnbp[startStateId].SetBestPathStartTime(this->mNodes[startStateId].GetStartTime());
			//DBG("vnbp["<<startStateId<<"]:"<<vnbp[startStateId]);
			for (unsigned int state_id = startStateId; state_id < this->mNodes.size(); state_id++)
			{
				if (vnbp[state_id].GetBestPath())
				{
					const Node& n = this->mNodes[state_id];
					// Loop through n.GetParallelArcs()
					for (typename Node::Nextnode2ParallelArcs::const_iterator i = n.GetParallelArcs().begin(); i != n.GetParallelArcs().end(); i++) 
					{
						int nextstate = i->first;
						const ParallelArcs<Arc>& pa = i->second;

						bool include_arc = false;
						bool path_end = this->mfPathTerminator(state_id, pa, &include_arc)
							|| (this->mFinalNodePolicy == PathGenerator<Path>::FINAL_NODE_ADD_PATH && FstProperties::IsFinalState(this->mFst, nextstate));
						if (path_end) {
							const Path* path_to_add = vnbp[state_id].GetBestPath();
							if (path_to_add && path_to_add->IsValid()) {
								if (include_arc) {
									Path p(*path_to_add);
									p.push_back(&pa);
									pPaths->Add(p);
								} else {
									pPaths->Add(*path_to_add);
								}
							}
						} else {
							vnbp[nextstate].ForwardBestPathFromNode(vnbp[state_id], &pa);
							//DBG("ForwardBestPathFromNode() "<<state_id<<" -> "<<nextstate<<": "<<vnbp[nextstate]);
						}
					}
				}
			}
		}
};

#endif
