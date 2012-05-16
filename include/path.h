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
#include "foreach.h"
#include "online_average.h"

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
		unsigned int mNonepsilonParallelArcsCount;

		static const fst::SymbolTable* mspSyms;
		static PrintType msPrintType;
	public:

		Path(int startStateId, float startTime) :
			mStartStateId(startStateId),
			mWeight(Weight::One()),
			mStartTime(startTime),
			mEndTime(NO_TIME),
			mNonepsilonParallelArcsCount(0)
		{}

		virtual ~Path() {}

		virtual Weight GetWeightWithArc(const ParallelArcs<Arc>& pa) const = 0;
		virtual Weight GetWeightWithoutArc(const ParallelArcs<Arc>& pa) const = 0;

		void push_back(const ParallelArcs<Arc>* pa) { 
			assert(pa);
			this->mWeight = GetWeightWithArc(*pa);
			if (!pa->IsEpsilon()) {
				this->mNonepsilonParallelArcsCount ++;
				this->mEndTime = pa->GetEndTime();
			}
			this->mContainer.push_back(pa); 
		}
		void pop_back() {
			const ParallelArcs<Arc>* pa = this->mContainer.back();
			assert(pa);
			this->mWeight = GetWeightWithoutArc(*pa);
			this->mContainer.pop_back();
			if (!pa->IsEpsilon()) {
				this->mNonepsilonParallelArcsCount --;
				this->mEndTime = RecomputeEndTimeByReverseTraversingPath();
			}
		}

		static bool compare(const Path<Arc> *p1, const Path<Arc> *p2) {
			return p1->GetWeight().Value() < p2->GetWeight().Value();
		}

		bool IsValid() const {
			return mNonepsilonParallelArcsCount >= 2 // at least 2 non epsilon arcs on the path
				&& GetStartTime() >= 0
				&& GetEndTime() > 0
				&& GetStartTime() != GetEndTime();
		}

		inline void   SetStartStateId(int id) { mStartStateId = id; }
		inline void   SetStartTime(float t)   { mStartTime = t; }
//		inline void   SetEndTime(float t)     { mEndTime = t; }
		inline void   SetWeight(Weight w)     { mWeight = w; }

		inline Weight GetWeight()       const { return mWeight; }
		inline int    GetStartStateId() const { return mStartStateId; }
		inline float  GetStartTime()    const { return mStartTime; }
		inline float  GetEndTime()      const { return mEndTime; }

		static void SetSymbols(const fst::SymbolTable* syms) { mspSyms = syms; ParallelArcs<Arc>::SetSymbols(syms); }
		static const fst::SymbolTable* GetSymbols() { return mspSyms; }
		static void SetPrintType(PrintType pt) { msPrintType = pt; ParallelArcs<Arc>::SetPrintType(pt); }
		static PrintType GetPrintType() { return msPrintType; }

		//--------------------------------------------------
		// PRINTING
		//--------------------------------------------------
		friend std::ostream& operator<<(std::ostream& oss, const Path<Arc>& p) {
			switch (msPrintType) {
				case PRINT_ALL: 
				{
					oss << std::setprecision(2) << "t[" << p.GetStartTime() << ".." << p.GetEndTime() << "] ";
					oss << std::setprecision(4) << "w:" << p.GetWeight() << " ";
					oss << "length:" << p.size() << " ";
					oss << "mNonepsilonParallelArcsCount:" << p.mNonepsilonParallelArcsCount << " ";
					oss << p.mStartStateId;
					foreach(const ParallelArcs<Arc>* pa, p) {
						assert(pa);
						oss << " -- " << *pa;
						oss << "(isEpsilon:" << pa->IsEpsilon() << ")";
					}
					oss << endl;

					PrintType pt = GetPrintType();
					SetPrintType(PRINT_PHONEMES_ONLY);
					oss << "phonemes: ";
					oss << p;
					SetPrintType(pt);
					return oss;
				}
				case PRINT_NODES_ONLY:
				{
					string separator = "";
					foreach(const ParallelArcs<Arc>* pa, p) {
						assert(pa);
						oss << separator << *pa;
						separator = " ";
					}
					return oss;
				}
				case PRINT_PHONEMES_ONLY:
				{
					foreach(const ParallelArcs<Arc>* pa, p) {
						assert(pa);
						if (!pa->IsEpsilon()) {
							oss << *pa << " ";
						}
					}
					return oss;
				}
				default: throw std::runtime_error("ERROR: Unknown print type of Path!");
			}
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	protected:
		float RecomputeEndTimeByReverseTraversingPath() const {
			float end_time = -1;
			reverse_foreach(const ParallelArcs<Arc>* pa, *this) {
				assert(pa);
				end_time = pa->GetEndTime();
				if (end_time >= 0) { break; }
			}
			return end_time;
		}
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
			return -log(OnlineAverage<float>::WithValue(exp(-this->mWeight.Value()), this->size(), exp(-pa.GetWeight().Value())));
			//return OnlineAverage<float>::WithValue(this->mWeight.Value(), this->size(), pa.GetWeight().Value());
		}
		Weight GetWeightWithoutArc(const ParallelArcs<Arc>& pa) const {
			return -log(OnlineAverage<float>::WithoutValue(exp(-this->mWeight.Value()), this->size(), exp(-pa.GetWeight().Value())));
			//return OnlineAverage<float>::WithoutValue(this->mWeight.Value(), this->size(), pa.GetWeight().Value());
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
