#ifndef OVERLAPPING_PATH_GROUP_LIST_H
#define OVERLAPPING_PATH_GROUP_LIST_H

#include <vector>
#include "overlapping_path_group.h"

template <class Arc>
struct OverlappingPathGroupList_Base {
	typedef ContainerInterface< std::vector<OverlappingPathGroup<Arc>*> > type;
};

template <class Arc>
class OverlappingPathGroupList : public OverlappingPathGroupList_Base<Arc>::type
{
	private:
		typedef typename OverlappingPathGroupList_Base<Arc>::type Base;
	public:
		~OverlappingPathGroupList() {
			for (typename Base::iterator i = this->mContainer.begin(); i != this->mContainer.end(); i++) {
				delete *i;
			}
		}

		void Add(const OverlappingPathGroup<Arc>& pg) {
			for (typename OverlappingPathGroup<Arc>::const_iterator i = pg.begin(); i!=pg.end(); i++) {
				this->Add(**i);
			}
		}

		void Add(const OverlappingPathGroupList<Arc>& pgl) {
			for (typename Base::const_iterator i = pgl.begin(); i != pgl.end(); i++) {
				this->Add(**i);
			}
		}

		void Add(const Path<Arc>& p) {
			// Start searching from the end
			for (typename Base::Container::const_reverse_iterator i = this->mContainer.rbegin(); i != this->mContainer.rend(); i++) {
				OverlappingPathGroup<Arc>* group = *i;
				if (group->IsOverlapping(p.GetStartTime(), p.GetEndTime())) {
					group->Add(p);
					return;
				}
			}
			// If there is no overlapping path group, create one
			this->mContainer.push_back(new OverlappingPathGroup<Arc>(p));
		}

		void Print(const std::string& word) const {
			for (typename Base::const_iterator i = this->begin(); i != this->end(); i++) {
				(*i)->Print(word);
				std::cout << std::endl;
			}
		}
};

#endif
