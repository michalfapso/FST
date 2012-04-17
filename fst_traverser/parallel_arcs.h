#ifndef _PARALLEL_ARCS_H
#define _PARALLEL_ARCS_H

#include <iostream>
#include <list>
#include <fst/fst.h>
#include "container_interface.h"
#include "print_type.h"
#include "foreach.h"
#include "exception.h"
#include "fst_arc_printer.h"

template <class Arc>
struct ParallelArcs_Base {
	typedef ContainerInterface< std::list<const Arc*> > type;
};

template <class Arc>
class ParallelArcs : public ParallelArcs_Base<Arc>::type {
	protected:
		typedef typename ParallelArcs_Base<Arc>::type Base;
	public:
		ParallelArcs() : mWeight(Arc::Weight::Zero()), mIsEpsilon(false) {}

		void Add(const Arc* arc) {
			assert(arc);
			if (!this->empty()) {
				assert((*this->begin())->nextstate == arc->nextstate);
				if (!mIsEpsilon && arc->ilabel == 0) {
					THROW("ERROR: ParallelArcs::Add(): epsilon arcs should not have parallel arcs! (pa:"<<*this<<" new_arc:"<<*arc<<")");
				}
			}
			mIsEpsilon |= arc->ilabel == 0;
			this->mContainer.push_back(arc);
			mWeight = Plus(mWeight, arc->weight);
			mEndTime = RecomputeEndTime();
		}

		inline typename Arc::Weight GetWeight() const { return mWeight; }
		inline int GetNextState() const { return this->empty() ? -1 : (*this->begin())->nextstate; }
		bool IsEpsilon() const {return mIsEpsilon;}
		float GetEndTime() const {return mEndTime;}
//		bool ContainsPhoneme() const {
//			if (!mspSyms) { THROW("ERROR: ParallelArcs::ContainsPhoneme(): symbols are not set!"); }
//			bool res = false;
//			foreach(const Arc* a, *this) {
//				res |= is_phoneme(mspSyms->Find(a->ilabel));
//			}
//			return res;
//		}

		//--------------------------------------------------
		// PRINTING
		//--------------------------------------------------
		friend std::ostream& operator<<(std::ostream& oss, const ParallelArcs<Arc>& pa) {
			if (pa.empty()) {
				return oss << "[EMPTY]";
			}
			switch (msPrintType) {
				case PRINT_ALL: 
				{
					oss << "[";
					string separator = "";
					foreach(const Arc* a, pa) {
						oss << separator;
						if (mspSyms) {oss << mspSyms->Find(a->ilabel);} else {oss << a->ilabel;}
						oss << ":";
						if (mspSyms) {oss << mspSyms->Find(a->olabel);} else {oss << a->olabel;}
						oss << "/" << a->weight;
						separator = " ";
					}
					oss << "]/"<<pa.GetWeight()<<" -> " << (*pa.begin())->nextstate;
					oss << " endTime=" << pa.GetEndTime();
					oss.flush();
					return oss;
				}
				case PRINT_NODES_ONLY: 
				{
					return oss << pa.GetNextState();
				}
				case PRINT_PHONEMES_ONLY: 
				{
					oss << "[";
					string separator = "";
					foreach(const Arc* a, pa) {
						oss << separator;
						if (mspSyms) {oss << mspSyms->Find(a->ilabel);} else {oss << a->ilabel;}
						separator = " ";
					}
					oss << "] ";
					oss.flush();
					return oss;
				}
				default: throw std::runtime_error("ERROR: Unknown print type of Path!");
			}
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		static void SetSymbols(const fst::SymbolTable* syms) { mspSyms = syms; }
		static void SetPrintType(PrintType pt) { msPrintType = pt; }
		static PrintType GetPrintType() { return msPrintType; }

	protected:
		float RecomputeEndTime() const {
			if (!mspSyms) { THROW("ERROR: ParallelArcs::RecomputeEndTime(): symbols are not set!"); }
			float end_time = 0;
			bool time_arcs_found = false;
			foreach(const Arc* a, *this) {
				assert(a);
				const std::string& olabel = mspSyms->Find(a->olabel);
				if (olabel.substr(0,2) == "t=") {
					time_arcs_found = true;
					float t = string2float(olabel.substr(2));
					end_time += t * ( exp(-a->weight.Value()) / exp(-GetWeight().Value()) ); // Time is weighted by the normalized probability of the arc 
				}
			}
			return time_arcs_found ? end_time : -1;
		}

		typename Arc::Weight mWeight;
		bool mIsEpsilon;
		float mEndTime;
		static const fst::SymbolTable* mspSyms;
		static PrintType msPrintType;
};

template <class Arc> const fst::SymbolTable* ParallelArcs<Arc>::mspSyms = NULL;
template <class Arc> PrintType ParallelArcs<Arc>::msPrintType = PRINT_ALL;

#endif
