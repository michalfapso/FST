#ifndef _PARALLEL_ARCS_H
#define _PARALLEL_ARCS_H

#include <iostream>
#include <list>
#include <fst/fst.h>
#include "container_interface.h"
#include "print_type.h"
#include "foreach.h"
#include "exception.h"

template <class Arc>
struct ParallelArcs_Base {
	typedef ContainerInterface< std::list<const Arc*> > type;
};

template <class Arc>
class ParallelArcs : public ParallelArcs_Base<Arc>::type {
	protected:
		typedef typename ParallelArcs_Base<Arc>::type Base;
	public:
		ParallelArcs() : mWeight(Arc::Weight::Zero()) {}

		void Add(const Arc* arc) {
			assert(this->empty() || (*this->begin())->nextstate == arc->nextstate);
			this->mContainer.push_back(arc);
			mWeight = Plus(mWeight, arc->weight);
		}

		inline typename Arc::Weight GetWeight() const { return mWeight; }
		inline int GetNextState() const { return this->empty() ? -1 : (*this->begin())->nextstate; }
		bool IsEpsilon() const {
			bool res = false;
			foreach(const Arc* a, *this) {
				res |= a->ilabel == 0;
			}
			if (res && this->size() > 1) {
				THROW("ERROR: ParallelArcs::IsEpsilon(): epsilon should not have parallel arcs! ("<<*this<<")");
			}
			return res;
		}
		bool ContainsPhoneme() const {
			if (!mspSyms) { THROW("ERROR: ParallelArcs::ContainsPhoneme(): symbols are not set!"); }
			bool res = false;
			foreach(const Arc* a, *this) {
				res |= is_phoneme(mspSyms->Find(a->ilabel));
			}
			return res;
		}
		float GetEndTime() const {
			if (!mspSyms) { THROW("ERROR: ParallelArcs::GetEndTime(): symbols are not set!"); }
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

		//--------------------------------------------------
		// PRINTING
		//--------------------------------------------------
		std::ostream& PrintNodesOnly(std::ostream& oss) const {
			if (this->empty()) {
				return oss << "[EMPTY]";
			} else {
				return oss << GetNextState();
			}
		}

		std::ostream& PrintAllInfo(std::ostream& oss) const {
			if (this->empty()) {
				return oss << "[EMPTY]";
			} else {
				oss << "[";
				string separator = "";
				foreach(const Arc* a, *this) {
					oss << separator;
					if (mspSyms) {oss << mspSyms->Find(a->ilabel);} else {oss << a->ilabel;}
					oss << ":";
					if (mspSyms) {oss << mspSyms->Find(a->olabel);} else {oss << a->olabel;}
					oss << "/" << a->weight;
					separator = " ";
				}
				oss << "] -> " << (*this->begin())->nextstate;
				oss << " endTime=" << GetEndTime();
				return oss;
			}
		}

		std::ostream& PrintPhonemesOnly(std::ostream& oss) const {
			if (this->empty()) {
				return oss << "[EMPTY]";
			} else {
				oss << "[";
				string separator = "";
				foreach(const Arc* a, *this) {
					oss << separator;
					if (mspSyms) {oss << mspSyms->Find(a->ilabel);} else {oss << a->ilabel;}
					separator = " ";
				}
				oss << "] ";
				return oss;
			}
		}

		friend std::ostream& operator<<(std::ostream& oss, const ParallelArcs<Arc>& pa) {
			switch (msPrintType) {
				case PRINT_ALL: return pa.PrintAllInfo(oss);
				case PRINT_NODES_ONLY: return pa.PrintNodesOnly(oss);
				case PRINT_PHONEMES_ONLY: return pa.PrintPhonemesOnly(oss);
				default: throw std::runtime_error("ERROR: Unknown print type of Path!");
			}
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		static void SetSymbols(const fst::SymbolTable* syms) { mspSyms = syms; }
		static void SetPrintType(PrintType pt) { msPrintType = pt; }
		static PrintType GetPrintType() { return msPrintType; }

	protected:
		typename Arc::Weight mWeight;
		static const fst::SymbolTable* mspSyms;
		static PrintType msPrintType;
};

template <class Arc> const fst::SymbolTable* ParallelArcs<Arc>::mspSyms = NULL;
template <class Arc> PrintType ParallelArcs<Arc>::msPrintType = PRINT_ALL;

#endif
