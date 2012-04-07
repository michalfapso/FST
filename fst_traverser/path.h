#ifndef PATHINFO_H
#define PATHINFO_H

#include <fst/fst.h>
#include <stdexcept>
#include <limits>
#include <list>
#include "container_interface.h"
#include "const.h"
#include "string2float.h"
#include "parallel_arcs.h"
#include "print_type.h"

template <class Arc>
struct Path_Base {
	typedef ContainerInterface< std::list< const ParallelArcs<Arc>* > > type;
};

template <class Arc>
class Path : public Path_Base<Arc>::type
{
	protected:
		typedef typename Path_Base<Arc>::type Base;
	public:

		Path(int startStateId, float startTime) :
			mStartStateId(startStateId),
			mWeight(Arc::Weight::One()),
			mStartTime(startTime),
			mEndTime(NO_TIME),
			mPhonemesCount(0)
		{}

		Path(const Path& p) {
			this->mWeight        = p.mWeight;
			this->mStartTime     = p.mStartTime;
			this->mEndTime       = p.mEndTime;
			this->mPhonemesCount = p.mPhonemesCount;
			this->mContainer     = p.mContainer;
		}

		virtual void push_back(const ParallelArcs<Arc>* pa) { 
			// Weight is computed in derived classes
			this->mContainer.push_back(pa); 
		}
		virtual void pop_back() { 
			// Weight is computed in derived classes
			this->mContainer.pop_back();
		}

		static bool compare(const Path<Arc> *p1, const Path<Arc> *p2) {
			return p1->GetWeight().Value() < p2->GetWeight().Value();
		}

		inline typename Arc::Weight GetWeight()       const { return mWeight; }
		inline int                  GetStartStateId() const { return mStartStateId; }
		inline float                GetStartTime()    const { return mStartTime; }
//		inline float                GetEndTime()      const { return mEndTime; }
		inline float                GetEndTime()      const { 
			assert(mspSyms);
			float end_time = -1;
			for (typename Base::Container::const_reverse_iterator i=this->mContainer.rbegin(); i!=this->mContainer.rend(); i++) {
				const ParallelArcs<Arc>& pa = **i;
				for (typename ParallelArcs<Arc>::const_iterator i_arc = pa.begin(); i_arc != pa.end(); i_arc++)
				{
					const std::string& olabel = mspSyms->Find((*i_arc)->olabel);
					if (olabel.substr(0,2) == "t=") {
						float t = string2float(olabel.substr(2));
						if (end_time < t) {
							end_time = t;
						}
					}
				}
				//if (end_time > -1) { return end_time; }
			}
			return end_time;
			//throw std::runtime_error("ERROR: Path does not contain any time arc!");
		}
		inline void                 SetEndTime(float t)     { mEndTime = t; }

		static void SetSymbols(const fst::SymbolTable* syms) { mspSyms = syms; ParallelArcs<Arc>::SetSymbols(syms); }
		static void SetPrintType(PrintType pt) { msPrintType = pt; ParallelArcs<Arc>::SetPrintType(pt); }

		//--------------------------------------------------
		// PRINTING
		//--------------------------------------------------
		std::ostream& PrintNodesOnly(std::ostream& oss) const {
			string separator = "";
			for (typename Base::const_iterator i=this->begin(); i!=this->end(); i++) {
				const ParallelArcs<Arc>& pa = **i;
				oss << separator << pa;
				separator = " ";
			}
			return oss;
		}

		std::ostream& PrintAllInfo(std::ostream& oss) const {
			oss << std::setprecision(2) << "t[" << GetStartTime() << ".." << GetEndTime() << "] ";
			oss << std::setprecision(4) << "w:" << GetWeight() << " ";
			oss << mStartStateId;
			for (typename Base::const_iterator i=this->begin(); i!=this->end(); i++) {
				const ParallelArcs<Arc>& pa = **i;
				oss << " -- " << pa;
			}
			return oss;
		}

		friend std::ostream& operator<<(std::ostream& oss, const Path<Arc>& p) {
			if (msPrintType == PRINT_ALL) {
				return p.PrintAllInfo(oss);
			} else if (msPrintType == PRINT_NODES_ONLY) {
				return p.PrintNodesOnly(oss);
			} else {
				throw std::runtime_error("ERROR: Unknown print type of Path!");
			}
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	protected:
		int mStartStateId;
		typename Arc::Weight mWeight;
		float mStartTime;
		float mEndTime;
		unsigned int mPhonemesCount;

		static const fst::SymbolTable* mspSyms;
		static PrintType msPrintType;
};

template <class Arc> const fst::SymbolTable* Path<Arc>::mspSyms = NULL;
template <class Arc> PrintType Path<Arc>::msPrintType = PRINT_ALL;

template <class Arc>
class PathAvgWeight : public Path<Arc>
{
	public:
		PathAvgWeight(int startStateId, float startTime) : Path<Arc>(startStateId, startTime) {}
		virtual void push_back(const ParallelArcs<Arc>* pa) { 
			int c = this->size();
			this->mWeight = (this->mWeight.Value() * c + pa->GetWeight().Value()) / (c+1);
			Path<Arc>::push_back(pa);
		}
		virtual void pop_back() { 
			int c = this->size();
			this->mWeight = (this->mWeight.Value() * c - this->mContainer.back()->GetWeight().Value()) / (c-1);
			Path<Arc>::pop_back();
		}
};

template <class Arc>
class PathMultWeight : public Path<Arc>
{
	public:
		PathMultWeight(int startStateId, float startTime) : Path<Arc>(startStateId, startTime) {}
		virtual void push_back(const ParallelArcs<Arc>* pa) { 
			this->mWeight = fst::Times(this->mWeight, pa->GetWeight());
			Path<Arc>::push_back(pa);
		}
		virtual void pop_back() {
			this->mWeight = fst::Divide(this->mWeight, this->mContainer.back()->GetWeight());
			Path<Arc>::pop_back();
		}
};

#endif
