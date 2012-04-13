#ifndef OVERLAPPING_PATH_GROUP_H
#define OVERLAPPING_PATH_GROUP_H

#include <iomanip>
#include <iostream>
#include <list>
#include "path.h"
#include "container_interface.h"

template <class Path>
struct OverlappingPathGroup_Base {
	typedef ContainerInterface< std::list< Path* > > type;
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
			mpBestPath(new Path(p))
//			mPhonemesCount(p.mPhonemesCount),
		{
			this->mContainer.push_back(mpBestPath);
		}

		~OverlappingPathGroup() {
			for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++) {
				delete *i;
			}
		}

		bool IsOverlapping(float startTime, float endTime) {
			return endTime > mStartTime && startTime < mEndTime;
		}

		inline const Path* GetBestPath() const {return mpBestPath;}

		void Add(const Path& pRef) {
			//cerr << "OverlappingPathGroup::Add("<<startTime<<", "<<endTime<<", "<<weight<<") ["<<mStartTime<<", "<<mEndTime<<"]" << endl;
			Path* p = new Path(pRef);
			this->mContainer.push_back(p);

			if (mpBestPath->GetWeight().Value() > p->GetWeight().Value()) {
				mpBestPath  = p;
				mStartTime  = p->GetStartTime();
				mEndTime    = p->GetEndTime();
			}
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
				 << -mpBestPath->GetWeight().Value() << FIELD_SEPARATOR
				 << endl
				 ;
			if (msPrintBestPathInGroup) {
				cout << "BestPath: " << *mpBestPath << endl;
			}
			if (msPrintAllPathsInGroup) {
				int idx = 0;
				for (typename Base::const_iterator i=this->mContainer.begin(); i!=this->mContainer.end(); i++, idx++) {
					if (*i == mpBestPath) {
						cout << "BEST";
					}
					cout << "  path["<<idx<<"] " << **i << endl;
					cout << endl;
				}
			}
		}
		static void PrintAllPathsInGroup(bool b) {msPrintAllPathsInGroup = b;}
		static void PrintBestPathInGroup(bool b) {msPrintBestPathInGroup = b;}

	protected:
		static const char FIELD_SEPARATOR = ' ';
		float mStartTime;
		float mEndTime;
		typename Path::Arc::Weight mWeight;
		Path* mpBestPath;
		static bool msPrintAllPathsInGroup;
		static bool msPrintBestPathInGroup;
};
template <class Path> bool OverlappingPathGroup<Path>::msPrintAllPathsInGroup = false;
template <class Path> bool OverlappingPathGroup<Path>::msPrintBestPathInGroup = false;

#endif
