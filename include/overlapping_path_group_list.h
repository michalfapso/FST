#ifndef OVERLAPPING_PATH_GROUP_LIST_H
#define OVERLAPPING_PATH_GROUP_LIST_H

#include <vector>
#include "overlapping_path_group.h"

template <class Path>
struct OverlappingPathGroupList_Base {
	typedef ContainerInterface< std::vector<OverlappingPathGroup<Path>*> > type;
};

template <class Path>
class OverlappingPathGroupList : public OverlappingPathGroupList_Base<Path>::type
{
	private:
		typedef typename OverlappingPathGroupList_Base<Path>::type Base;
	public:
		~OverlappingPathGroupList() {
			for (typename Base::iterator i = this->mContainer.begin(); i != this->mContainer.end(); i++) {
				delete *i;
			}
		}

		void Add(const OverlappingPathGroup<Path>& pg) {
			for (typename OverlappingPathGroup<Path>::const_iterator i = pg.begin(); i!=pg.end(); i++) {
				this->Add(**i);
			}
		}

		void Add(const OverlappingPathGroupList<Path>& pgl) {
			for (typename Base::const_iterator i = pgl.begin(); i != pgl.end(); i++) {
				this->Add(**i);
			}
		}

		void Add(const Path& p) {
			// Start searching from the end
			for (typename Base::Container::const_reverse_iterator i = this->mContainer.rbegin(); i != this->mContainer.rend(); i++) {
				OverlappingPathGroup<Path>* group = *i;
				if (group->IsOverlapping(p.GetStartTime(), p.GetEndTime())) {
					group->Add(p);
					return;
				}
			}
			// If there is no overlapping path group, create one
			this->mContainer.push_back(new OverlappingPathGroup<Path>(p));
		}

		void Print(const std::string& word) const {
			for (typename Base::const_iterator i = this->begin(); i != this->end(); i++) {
				(*i)->Print(word);
				std::cout << std::endl;
			}
		}
};

#endif
