#ifndef OVERLAPPING_PATH_GROUP_H
#define OVERLAPPING_PATH_GROUP_H

#include <iomanip>
#include <iostream>
#include <list>
#include "path.h"
#include "container_interface.h"

template <class Path>
struct OverlappingPathGroup_Base {
	typedef ContainerInterface< std::list< Path > > type;
};

template <class Path>
class OverlappingPathGroup : public OverlappingPathGroup_Base<Path>::type
{
	protected:
		typedef typename OverlappingPathGroup_Base<Path>::type Base;
	public:
		OverlappingPathGroup(const Path& p) :
			mStartTime(p.GetStartTime()),
			mEndTime(p.GetEndTime()),
			mWeight(p.GetWeight()),
			mBestWeight(p.GetWeight())
//			mPhonemesCount(p.mPhonemesCount),
		{
			this->mContainer.push_back(p);
		}

//		~OverlappingPathGroup() {
//			for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++) {
//				delete *i;
//			}
//		}

		bool IsOverlapping(float startTime, float endTime) {
			return endTime > mStartTime && startTime < mEndTime;
		}

		void Add(const Path& p) {
			//cerr << "OverlappingPathGroup::Add("<<startTime<<", "<<endTime<<", "<<weight<<") ["<<mStartTime<<", "<<mEndTime<<", "<<mWeight<<"]" << endl;

			if (mBestWeight.Value() > p.GetWeight().Value()) {
				mBestWeight = p.GetWeight();
				mStartTime  = p.GetStartTime();
				mEndTime    = p.GetEndTime();
			}

			mWeight = Plus(mWeight, p.GetWeight());

			this->mContainer.push_back(p);
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
				 << endl
				 ;
			int idx = 0;
			for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++, idx++) {
				if ((*i).GetWeight() == mBestWeight) {
					cout << "BEST";
				}
				cout << "  path["<<idx<<"] " << *i << endl;
				cout << endl;
			}
		}

	protected:
		static const char FIELD_SEPARATOR = ' ';
		float mStartTime;
		float mEndTime;
		typename Path::Arc::Weight mWeight;
		typename Path::Arc::Weight mBestWeight;
};

#endif
