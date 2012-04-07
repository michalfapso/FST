#ifndef PATH_POOL_H
#define PATH_POOL_H

#include <unordered_map>

#include "overlapping_path_group_list.h"

template <class Arc>
class PathPool : public std::unordered_map<std::string, OverlappingPathGroupList<Arc>* > {
	public:
		PathPool(unsigned int minPhonemesCountThreshold) : mMinPhonemesCountThreshold(minPhonemesCountThreshold) {}

		~PathPool() {
			for (typename PathPool<Arc>::const_iterator iPathPoolItem = this->begin(); iPathPoolItem != this->end(); iPathPoolItem++) {
				delete iPathPoolItem->second;
			}
		}

		void Add(const Path<Arc>& rPath) 
		{
			Path<Arc>* p = new Path<Arc>(rPath); // Create a copy

			if ((float)p.mPhonemesCount / p.mPathsCount < mMinPhonemesCountThreshold) {
				return;
			}

			assert(p.mWord != NO_WORD);
			assert(p.mStartTime != NO_TIME); 
			assert(p.mEndTime != NO_TIME);

			OverlappingPathGroupList<Arc>* pPathList = GetOverlappingPathGroupList(p.mWord);
			pPathList->AddPath(p);
//			cerr << "PathPool::AddPath() len="<<pPath->mPhonemesCount<<" lennosil="<<pPath->mPhonemesCount<<" path="<<p<<endl;
		}

		void Print() const 
		{
			for (typename PathPool<Arc>::const_iterator iPathPoolItem = this->begin(); iPathPoolItem != this->end(); iPathPoolItem++) {
				iPathPoolItem->second->Print(iPathPoolItem->first);
			}
		}
	protected:
		OverlappingPathGroupList<Arc>* GetOverlappingPathGroupList(const std::string& word) 
		{
			typename PathPool<Arc>::iterator iPathPoolItem = this->find(word);
			// If the word is not yet in the pool, create it
			if (iPathPoolItem == this->end()) {
				OverlappingPathGroupList<Arc>* pPathList = new OverlappingPathGroupList<Arc>;
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
