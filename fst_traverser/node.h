#ifndef _NODE_H
#define _NODE_H

#include <boost/unordered_map.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include "string2float.h"

template <class TArc>
class Node_Base {
	public:
		typedef TArc Arc;
		friend std::ostream& operator<<(std::ostream& oss, const Node_Base<Arc>& n) {
			return oss;
		}
};

template <class NodeBase>
class Node_Various : public NodeBase {
	protected:
		typedef Node_Various<NodeBase> Node;
		typedef typename NodeBase::Arc Arc;
	public:
		Node_Various() : 
			NodeBase(),
			mAlpha(Arc::Weight::Zero()),
			mFwdPathsCount(0),
			mFwdPhonemesCount(0),
			mPathPosition(-1)
		{}

		virtual void InitStartNode() {
			mAlpha = Arc::Weight::One();
			mPathPosition = 0;
			mFwdPathsCount = 1;
		}

		void ComputeAlpha(const Node& nFrom, const Arc& arc)
		{
			//DBG(std::fixed << std::setprecision(10) << "  alpha["<<arc.nextstate<<"] = Plus(Times("<<nFrom.mAlpha<<", "<<arc.weight<<"), "<<mAlpha<<")  = Plus("<<Times(nFrom.mAlpha, arc.weight)<<", "<<mAlpha<<") = "<< Plus(Times(nFrom.mAlpha, arc.weight), mAlpha));
			mAlpha = Plus(Times(nFrom.mAlpha, arc.weight), mAlpha);
		}

		friend std::ostream& operator<<(std::ostream& oss, const Node& n) {
			oss << (NodeBase)n;
			oss << "alpha=" << n.mAlpha << " ";
			return oss;
		}

		inline const typename Arc::Weight& GetAlpha() const {return mAlpha;}
		inline void SetAlpha(const typename Arc::Weight& w) {mAlpha = w;}

		inline unsigned int GetFwdPathsCount() const {return mFwdPathsCount;}
		inline void AddFwdPathsCount(unsigned int i) {mFwdPathsCount += i;}

		inline unsigned int GetFwdPhonemesCount() const {return mFwdPhonemesCount;}
		inline void AddFwdPhonemesCount(unsigned int i) {mFwdPhonemesCount += i;}

	protected:
		typename Arc::Weight mAlpha;
		unsigned int mFwdPathsCount;
		unsigned int mFwdPhonemesCount;
		int mPathPosition;
};

template <class NodeBase>
class Node_ParallelArcs : public NodeBase {
	public:
		typedef Node_ParallelArcs<NodeBase> Node;
		typedef typename NodeBase::Arc Arc;
	public:
		typedef boost::unordered_map<int, ParallelArcs<Arc> > Nextnode2ParallelArcs;

		Node_ParallelArcs() : NodeBase() {}

		inline const Nextnode2ParallelArcs& GetParallelArcs() const { return mNextnode2ParallelArcs; }

		ParallelArcs<Arc>* GetParallelArcsForNextNode(int nextNodeId) {
			typename Nextnode2ParallelArcs::iterator i = mNextnode2ParallelArcs.find(nextNodeId);
			if (i == mNextnode2ParallelArcs.end()) {
				ParallelArcs<Arc> pa;
				pair<typename Nextnode2ParallelArcs::iterator,bool> ret;
				ret = mNextnode2ParallelArcs.insert(make_pair(nextNodeId, pa));
				i = ret.first;
			}
			return &(i->second);
		}

		void AddArcToParallelArcs(const Arc& arc) {
			ParallelArcs<Arc>* pa = GetParallelArcsForNextNode(arc.nextstate);
			pa->Add(&arc);
		}

		friend std::ostream& operator<<(std::ostream& oss, const Node& n) {
			oss << (NodeBase)n;
			for (typename Nextnode2ParallelArcs::const_iterator i = n.GetParallelArcs().begin(); i != n.GetParallelArcs().end(); i++) {
				oss << "(" << i->second << " -> " << i->first << ") ";
			}
			return oss;
		}

	protected:
		Nextnode2ParallelArcs mNextnode2ParallelArcs;
};

template <class NodeBase>
class Node_StartTime : public NodeBase {
	protected:
		typedef Node_StartTime<NodeBase> Node;
		typedef typename NodeBase::Arc Arc;
	public:
		typedef std::vector<float> TimeContainer;

		Node_StartTime() : NodeBase() {}

		float GetStartTime() const {
			float sum = 0;
			for (TimeContainer::const_iterator i=mStartTimes.begin(); i!=mStartTimes.end(); i++) {
				sum += *i;
			}
			return mStartTimes.empty() ? 0.0f : sum / mStartTimes.size();
		}

		void SetStartTime(const Node& nFrom, const Arc& arc, const std::string& arcLabel)
		{
			if (arcLabel.substr(0,2) == "t=") {
				mStartTimes.push_back(string2float(arcLabel.substr(2)));
			} else {
				mStartTimes.push_back(nFrom.GetStartTime());
//				for (TimeContainer::const_iterator i=nFrom.mStartTimes.begin(); i!=nFrom.mStartTimes.end(); i++) {
//					mStartTimes.push_back(*i);
//				}
			}
		}

		friend std::ostream& operator<<(std::ostream& oss, const Node& n) {
			oss << (NodeBase)n;
			oss << "StartTimes:";
			for (TimeContainer::const_iterator i=n.mStartTimes.begin(); i!=n.mStartTimes.end(); i++) {
				oss << " " << std::setprecision(2) << *i;
			}
			oss << " GetStartTime=" << std::setprecision(2) << n.GetStartTime();
			return oss;
		}

	protected:
		TimeContainer mStartTimes;
};

template <class Path, class NodeBase>
class Node_BestPath : public NodeBase {
	protected:
		typedef Node_BestPath<Path, NodeBase> Node;
		typedef typename NodeBase::Arc Arc;
		typedef typename Arc::Weight Weight;
		static const int INVALID_PATH_START_STATE_ID;
		static const float INVALID_PATH_START_TIME;
	public:
		//typedef PathAvgWeight<Arc> Path;
		//typedef PathMultWeight<Arc> Path;

		Node_BestPath() : NodeBase(), mBestPath(INVALID_PATH_START_STATE_ID, INVALID_PATH_START_TIME) {}

		inline const Path& GetBestPath() const {return mBestPath;}
		inline const void SetBestPathStartStateId(int id) {mBestPath.SetStartStateId(id);}
		inline const void SetBestPathStartTime(float t) {mBestPath.SetStartTime(t);}
		inline const void SetBestPathWeight(Weight w) {mBestPath.SetWeight(w);}

		void ForwardBestPathFromNode(const Node& nFrom, const ParallelArcs<Arc>* pa)
		{
			if (nFrom.GetBestPath().GetStartStateId() != INVALID_PATH_START_STATE_ID) {
				const Weight w = nFrom.GetBestPath().GetWeightWithArc(*pa);
				if (GetBestPath().empty() || w.Value() < GetBestPath().GetWeight().Value()) {
					mBestPath = nFrom.GetBestPath();
					mBestPath.push_back(pa);
				}
			}
		}

		friend std::ostream& operator<<(std::ostream& oss, const Node& n) {
			oss << (NodeBase)n;
			oss << "BestPath:" << n.mBestPath;
			return oss;
		}

	protected:
		Path mBestPath;
};
template <class Path, class NodeBase> const int Node_BestPath<Path, NodeBase>::INVALID_PATH_START_STATE_ID = -1;
template <class Path, class NodeBase> const float Node_BestPath<Path, NodeBase>::INVALID_PATH_START_TIME = -1;

#endif
