#ifndef OVERLAPPING_PATH_GROUP_H
#define OVERLAPPING_PATH_GROUP_H

#include <iomanip>
#include <iostream>
#include <list>
#include "path.h"
#include "container_interface.h"

template <class Arc>
struct OverlappingPathGroup_Base {
	typedef ContainerInterface< std::list< Path<Arc>* > > type;
};

template <class Arc>
class OverlappingPathGroup : public OverlappingPathGroup_Base<Arc>::type
{
	protected:
		typedef typename OverlappingPathGroup_Base<Arc>::type Base;
	public:
		OverlappingPathGroup(const Path<Arc>& p) :
			mStartTime(p.GetStartTime()),
			mEndTime(p.GetEndTime()),
			mWeight(p.GetWeight()),
			mBestWeight(p.GetWeight()),
//			mPhonemesCount(p.mPhonemesCount),
			mPathsCount(1)
		{
			this->mContainer.push_back(new Path<Arc>(p));
		}

		~OverlappingPathGroup() {
			for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++) {
				delete *i;
			}
		}

		bool IsOverlapping(float startTime, float endTime) {
			return endTime > mStartTime && startTime < mEndTime;
		}

		void Add(const Path<Arc>& pRef) {
			//cerr << "OverlappingPathGroup::Add("<<startTime<<", "<<endTime<<", "<<weight<<") ["<<mStartTime<<", "<<mEndTime<<", "<<mWeight<<"]" << endl;
			Path<Arc>* p = new Path<Arc>(pRef); // Create a copy
			this->mContainer.push_back(p);

			if (mBestWeight.Value() > p->GetWeight().Value()) {
				mBestWeight = p->GetWeight();
				mStartTime  = p->GetStartTime();
				mEndTime    = p->GetEndTime();
			}

			mPathsCount ++;
//			mPhonemesCount += p->mPhonemesCount;
			mWeight = Plus(mWeight, p->GetWeight());

			//mAvgPhonemeCount = ((float)(mPathsCount-1) / mPathsCount) * mAvgPhonemeCount + p.mPhonemesCount/mPathsCount; // on-the-fly average
		}

		void Print(const std::string& word) const {
			using namespace std;
			cout << "Overlapping Path Group: "
//				 << (unsigned int)(mStartTime * 100) << "00000" << FIELD_SEPARATOR 
//				 << (unsigned int)(mEndTime   * 100) << "00000" << FIELD_SEPARATOR 
				 << setprecision(2) << mStartTime << FIELD_SEPARATOR 
				 << setprecision(2) << mEndTime   << FIELD_SEPARATOR 
				 << word << FIELD_SEPARATOR 
				 << setprecision(10) 
				 << fixed
				 << -mWeight.Value() << FIELD_SEPARATOR
				 << -mBestWeight.Value() << FIELD_SEPARATOR
//				 << (float)mPhonemesCount / mPathsCount << FIELD_SEPARATOR
				 << endl
				 ;
			int idx = 0;
			for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++, idx++) {
				if ((**i).GetWeight() == mBestWeight) {
					cout << "BEST";
				}
				cout << "  path["<<idx<<"] " << **i << endl;
				cout << endl;
			}
		}

	protected:
		static const char FIELD_SEPARATOR = ' ';
		float mStartTime;
		float mEndTime;
		typename Arc::Weight mWeight;
		typename Arc::Weight mBestWeight;
//		unsigned int mPhonemesCount;
		unsigned int mPathsCount;
};

#endif
