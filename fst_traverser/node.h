#ifndef _NODE_H
#define _NODE_H

#include <boost/unordered_map.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include "string2float.h"

template <class Arc>
class Node {
	public:
		typedef boost::unordered_map<int, ParallelArcs<Arc> > Nextnode2ParallelArcs;
		typedef std::vector<float> TimeContainer;
		typedef std::map<string, DetectionEnd<Arc> > Word2DetectionMap;

		Node() :
			mAlpha(Arc::Weight::Zero()),
			mFwdPathsCount(0),
			mFwdPhonemesCount(0),
			mPathPosition(-1),
			mStartTime(1000000000),
			mEndTime(0)
		{}

		void InitStartNode() {
			mAlpha = Arc::Weight::One();
			mPathPosition = 0;
			mFwdPathsCount = 1;
		}

		float GetStartTime() const {
			float sum = 0;
			for (TimeContainer::const_iterator i=mStartTimes.begin(); i!=mStartTimes.end(); i++) {
				sum += *i;
			}
			return mStartTimes.empty() ? 0.0f : sum / mStartTimes.size();
		}

		void SetStartTime(const Node<Arc>& nFrom, const Arc& arc, const std::string& arcLabel)
		{
			if (arcLabel.substr(0,2) == "t=") {
				mStartTimes.push_back(string2float(arcLabel.substr(2)));
			} else {
				for (TimeContainer::const_iterator i=nFrom.mStartTimes.begin(); i!=nFrom.mStartTimes.end(); i++) {
					mStartTimes.push_back(*i);
				}
			}
		}

		void ComputeAlpha(const Node<Arc>& nFrom, const Arc& arc)
		{
			DBG(std::fixed << std::setprecision(10) << "  alpha["<<arc.nextstate<<"] = Plus(Times("<<nFrom.mAlpha<<", "<<arc.weight<<"), "<<mAlpha<<")  = Plus("<<Times(nFrom.mAlpha, arc.weight)<<", "<<mAlpha<<") = "<< Plus(Times(nFrom.mAlpha, arc.weight), mAlpha));
			mAlpha = Plus(Times(nFrom.mAlpha, arc.weight), mAlpha);
		}

		friend std::ostream& operator<<(std::ostream& oss, const Node<Arc>& n) {
			oss << "Start Times:";
			for (TimeContainer::const_iterator i=n.mStartTimes.begin(); i!=n.mStartTimes.end(); i++) {
				oss << " " << std::setprecision(2) << *i;
			}
			oss << " GetStartTime=" << std::setprecision(2) << n.GetStartTime() << endl;
			return oss;
		}

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
		float mStartTime;
		float mEndTime;
		TimeContainer mStartTimes;
		Nextnode2ParallelArcs mNextnode2ParallelArcs;
};

#endif
