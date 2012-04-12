#ifndef PATH_POOL_H
#define PATH_POOL_H

#include <unordered_map>

#include "overlapping_path_group_list.h"

template <class Path>
class PathPool : public std::unordered_map<std::string, OverlappingPathGroupList<Path>* > {
	public:
		PathPool(unsigned int minPhonemesCountThreshold) : mMinPhonemesCountThreshold(minPhonemesCountThreshold) {}

		~PathPool() {
			for (typename PathPool<Path>::const_iterator iPathPoolItem = this->begin(); iPathPoolItem != this->end(); iPathPoolItem++) {
				delete iPathPoolItem->second;
			}
		}

		void Add(const Path& rPath) 
		{
			Path* p = new Path(rPath); // Create a copy

			if ((float)p.mPhonemesCount / p.mPathsCount < mMinPhonemesCountThreshold) {
				return;
			}

			assert(p.mWord != NO_WORD);
			assert(p.mStartTime != NO_TIME); 
			assert(p.mEndTime != NO_TIME);

			OverlappingPathGroupList<Path>* pPathList = GetOverlappingPathGroupList(p.mWord);
			pPathList->AddPath(p);
//			cerr << "PathPool::AddPath() len="<<pPath->mPhonemesCount<<" lennosil="<<pPath->mPhonemesCount<<" path="<<p<<endl;
		}

		void Print() const 
		{
			for (typename PathPool<Path>::const_iterator iPathPoolItem = this->begin(); iPathPoolItem != this->end(); iPathPoolItem++) {
				iPathPoolItem->second->Print(iPathPoolItem->first);
			}
		}
	protected:
		OverlappingPathGroupList<Path>* GetOverlappingPathGroupList(const std::string& word) 
		{
			typename PathPool<Path>::iterator iPathPoolItem = this->find(word);
			// If the word is not yet in the pool, create it
			if (iPathPoolItem == this->end()) {
				OverlappingPathGroupList<Path>* pPathList = new OverlappingPathGroupList<Path>;
				this->insert(make_pair(word, pPathList));
				return pPathList;
			} else {
				return iPathPoolItem->second;
			}
		}
	protected:
		unsigned int mMinPhonemesCountThreshold;
};

#endif
