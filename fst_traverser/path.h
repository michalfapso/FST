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

template <class TArc>
struct Path_Base {
	typedef ContainerInterface< std::list< const ParallelArcs<TArc>* > > type;
};

template <class TArc>
class Path : public Path_Base<TArc>::type
{
	public:
		typedef TArc Arc;
	protected:
		typedef typename Path_Base<Arc>::type Base;
		typedef typename Arc::Weight Weight;

		int mStartStateId;
		Weight mWeight;
		float mStartTime;
		float mEndTime;
		unsigned int mPhonemesCount;

		static const fst::SymbolTable* mspSyms;
		static PrintType msPrintType;
	public:

		Path(int startStateId, float startTime) :
			mStartStateId(startStateId),
			mWeight(Weight::One()),
			mStartTime(startTime),
			mEndTime(NO_TIME),
			mPhonemesCount(0)
		{}

		Path(const Path& p) {
			this->mStartStateId  = p.mStartStateId;
			this->mWeight        = p.mWeight;
			this->mStartTime     = p.mStartTime;
			this->mEndTime       = p.mEndTime;
			this->mPhonemesCount = p.mPhonemesCount;
			this->mContainer     = p.mContainer;
		}

		virtual Weight GetWeightWithArc(const ParallelArcs<Arc>& pa) const = 0;
		virtual Weight GetWeightWithoutArc(const ParallelArcs<Arc>& pa) const = 0;

		void push_back(const ParallelArcs<Arc>* pa) { 
			this->mWeight = GetWeightWithArc(*pa);
			this->mContainer.push_back(pa); 
		}
		void pop_back() {
			this->mWeight = GetWeightWithoutArc(*this->mContainer.back());
			this->mContainer.pop_back();
		}

		static bool compare(const Path<Arc> *p1, const Path<Arc> *p2) {
			return p1->GetWeight().Value() < p2->GetWeight().Value();
		}

		inline void   SetStartStateId(int id) { mStartStateId = id; }
		inline void   SetStartTime(float t)   { mStartTime = t; }
		inline void   SetEndTime(float t)     { mEndTime = t; }
		inline void   SetWeight(Weight w)     { mWeight = w; }

		inline Weight GetWeight()       const { return mWeight; }
		inline int    GetStartStateId() const { return mStartStateId; }
		inline float  GetStartTime()    const { return mStartTime; }
//		inline float  GetEndTime()      const { return mEndTime; }
		inline float  GetEndTime()      const 
		{
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
			oss << endl;

			PrintType pt = ParallelArcs<Arc>::GetPrintType();
			ParallelArcs<Arc>::SetPrintType(PRINT_PHONEMES_ONLY);
			oss << "phonemes: ";
			PrintPhonemesOnly(oss);
			ParallelArcs<Arc>::SetPrintType(pt);
			return oss;
		}

		std::ostream& PrintPhonemesOnly(std::ostream& oss) const {
			oss << "phonemes: ";
			for (typename Base::const_iterator i=this->begin(); i!=this->end(); i++) {
				const ParallelArcs<Arc>& pa = **i;
				if (pa.ContainsPhoneme(mspSyms)) {
					oss << pa << " ";
				}
			}
			return oss;
		}

		friend std::ostream& operator<<(std::ostream& oss, const Path<Arc>& p) {
			switch (msPrintType) {
				case PRINT_ALL: return p.PrintAllInfo(oss);
				case PRINT_NODES_ONLY: return p.PrintNodesOnly(oss);
				case PRINT_PHONEMES_ONLY: return p.PrintPhonemesOnly(oss);
				default: throw std::runtime_error("ERROR: Unknown print type of Path!");
			}
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
};

template <class TArc> const fst::SymbolTable* Path<TArc>::mspSyms = NULL;
template <class TArc> PrintType Path<TArc>::msPrintType = PRINT_ALL;

template <class TArc>
class PathAvgWeight : public Path<TArc>
{
	public:
		typedef TArc Arc;
		typedef typename Arc::Weight Weight;

		PathAvgWeight(int startStateId, float startTime) : Path<Arc>(startStateId, startTime) {}

		Weight GetWeightWithArc(const ParallelArcs<Arc>& pa) const {
			int c = this->size();
			return (this->mWeight.Value() * c + pa.GetWeight().Value()) / (c+1);
		}
		Weight GetWeightWithoutArc(const ParallelArcs<Arc>& pa) const {
			int c = this->size();
			return (this->mWeight.Value() * c - pa.GetWeight().Value()) / (c-1);
		}
};

template <class TArc>
class PathMultWeight : public Path<TArc>
{
	public:
		typedef TArc Arc;
		typedef typename Arc::Weight Weight;

		PathMultWeight(int startStateId, float startTime) : Path<Arc>(startStateId, startTime) {}

		Weight GetWeightWithArc(const ParallelArcs<Arc>& pa) const {
			return this->empty() ? 
				pa.GetWeight() : 
				fst::Times(this->mWeight, pa.GetWeight());
		}
		Weight GetWeightWithoutArc(const ParallelArcs<Arc>& pa) const {
			return fst::Divide(this->mWeight, pa.GetWeight());
		}
};

#endif
