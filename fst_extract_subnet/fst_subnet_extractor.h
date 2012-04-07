#ifndef _FST_SUBNET_EXTRACTOR_H_
#define _FST_SUBNET_EXTRACTOR_H_

#include <fst/fst.h>
#include <vector>
#include <list>
#include <utility>
#include "fst_properties.h"
#include "exception.h"

template <class Arc>
class FstSubnetExtractor 
{
	protected:
		struct Node {
			typedef std::list< std::pair<int, const Arc*> > BackwardArcs; // pair<startStateId, arc>

			Node() : mOnForwardPath(false), mOnBackwardPath(false), mNewStateId(-1) {}
			bool mOnForwardPath;
			bool mOnBackwardPath;
			BackwardArcs mBackwardArcs;
			int mNewStateId;
		};
		typedef std::vector<Node> Nodes;
	public:
		FstSubnetExtractor(const Fst<Arc>& fst) : mFst(fst) 
		{
			if (!FstProperties::IsTopologicallySorted(mFst)) {
				THROW("ERROR: FstSubnetExtractor: Fst is not topologically sorted!");
			}
		}
		void Extract(int startState, int endState, MutableFst<Arc>* pFstOut) 
		{
			mNodes.clear();
			mNodes.resize(FstProperties::StatesCount(mFst));

			Forward(startState);
			Backward(endState);
			if (!IsIncluded(startState) || !IsIncluded(endState)) {
				THROW("ERROR: FstSubnetExtractor: There is no path between start and end state!");
			}
			CreateSubnet(startState, endState, pFstOut);
		}
	protected:
		void Forward(unsigned int startState) 
		{
			mNodes[startState].mOnForwardPath = true;
			for (unsigned int state_id = startState; state_id < mNodes.size(); state_id++)
			{
				for (ArcIterator<Fst<Arc>> aiter(mFst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					mNodes[arc.nextstate].mOnForwardPath |= mNodes[state_id].mOnForwardPath;
					mNodes[arc.nextstate].mBackwardArcs.push_back(std::make_pair(state_id, &arc));
				}
			}
		}
		void Backward(int endState) 
		{
			mNodes[endState].mOnBackwardPath = true;
			for (int state_id = endState; state_id >= 0; state_id--)
			{
				for (typename Node::BackwardArcs::iterator i = mNodes[state_id].mBackwardArcs.begin(); i != mNodes[state_id].mBackwardArcs.end(); i++)
				{
					int start_state_id = i->first;
					//const Arc* arc = i->second;
					mNodes[start_state_id].mOnBackwardPath |= mNodes[state_id].mOnBackwardPath;
				}
			}
		}
		void CreateSubnet(int startState, int endState, MutableFst<Arc>* pFstOut) 
		{
			mNodes[startState].mNewStateId = pFstOut->AddState();
			pFstOut->SetStart(mNodes[startState].mNewStateId);
			for (int state_id = startState; state_id <= endState; state_id++)
			{
				for (ArcIterator<Fst<Arc>> aiter(mFst, state_id); !aiter.Done(); aiter.Next())
				{
					const Arc &arc = aiter.Value();
					if (IsIncluded(arc.nextstate)) {
						if (mNodes[arc.nextstate].mNewStateId < 0) {
							mNodes[arc.nextstate].mNewStateId = pFstOut->AddState();
						}
						pFstOut->AddArc(mNodes[state_id].mNewStateId, Arc(arc.ilabel, arc.olabel, arc.weight, mNodes[arc.nextstate].mNewStateId));
					}
				}
			}
			pFstOut->SetFinal(mNodes[endState].mNewStateId, Arc::Weight::One());
		}
		bool IsIncluded(int stateId) {
			Node& n = mNodes[stateId];
			return n.mOnForwardPath && n.mOnBackwardPath;
		}

		const Fst<Arc>& mFst;
		Nodes mNodes;
};

#endif
