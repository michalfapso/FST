#ifndef _FEATURES_GENERATOR_PATH_DETECTION_H_
#define _FEATURES_GENERATOR_PATH_DETECTION_H_

#include "features_generator_path.h"
#include "statistics.h"

template <class TPath>
class FeaturesGenerator_PathDetection : public FeaturesGenerator_Path<TPath>
{
	public:
		typedef TPath Path;
		typedef typename Path::Arc Arc;
		typedef ParallelArcs<Arc> PA;

		FeaturesGenerator_PathDetection(const std::string& outputFilename, const std::string& term, const mlf::MlfRecords<ReferenceMlfRecord>& recsRef) : 
			FeaturesGenerator_Path<TPath>(outputFilename),
			mTerm(term), 
			mRecsRef(recsRef)
		{
			this->mFieldNamePrefix = "d";
		}

	protected:
		virtual void PrintFeatures(const Path& path, const OverlappingPathGroup<Path>* pPathGroup, std::ostream& oss)
		{
			FeaturesGenerator_Path<TPath>::PrintFeatures(path, pPathGroup, oss);

			unsigned int epsilons_count = 0;
			Statistics<float> weight_epsilons_stats;

			float time_length = -1;

			typename Arc::Weight opg_sum_weight = Arc::Weight::Zero();

			if (this->msPrintFieldValues) {
				foreach(const PA* pa, path) {
					assert(pa);
					if (pa->IsEpsilon() && pa->size() > 1) {
						THROW("ERROR: epsilon arc should not have more parallel arcs! ("<<*pa<<")");
					}
					if (pa->IsEpsilon()) {
						epsilons_count ++;
						weight_epsilons_stats.Add(exp(-pa->GetWeight().Value()));
					}
				}
				assert(path.GetEndTime() >= 0);
				time_length = path.GetEndTime() - path.GetStartTime();

				if (pPathGroup) {
					foreach (Path* p, *pPathGroup) {
						opg_sum_weight = Plus(opg_sum_weight, p->GetWeight());
					}
				}
			}

			// Standard MLF fields
			this->PrintField(oss, "IsHit",         IsHit(path.GetStartTime(), path.GetEndTime(), mRecsRef));
			this->PrintField(oss, "StartT",        SecondsToMlfTime(path.GetStartTime()));
			this->PrintField(oss, "EndT",          SecondsToMlfTime(path.GetEndTime()));
			this->PrintField(oss, "Term",          mTerm);
			oss << std::setprecision(6);
			this->PrintField(oss, "Score",         -path.GetWeight().Value());
			// Arcs count
			this->PrintField(oss, "EpsC",          epsilons_count);
			this->PrintField(oss, "PhnC",          path.size() - epsilons_count);
			// Weight
			this->PrintField(oss, "EpsWMin",       weight_epsilons_stats.GetMin());
			this->PrintField(oss, "EpsWMax",       weight_epsilons_stats.GetMax());
			this->PrintField(oss, "EpsWAvg",       weight_epsilons_stats.GetAvg());

			this->PrintField(oss, "OpgSize",       pPathGroup ? pPathGroup->size() : 0);
			this->PrintField(oss, "OpgSumW",       opg_sum_weight.Value());
			this->PrintField(oss, "PathInfo",      GetPathInfoString(path));
		}

		static const std::string GetPathInfoString(const Path& p) 
		{
			std::ostringstream oss;
			float pa_start_time = p.GetStartTime();
			std::string pa_separator = "";
			foreach(const ParallelArcs<Arc>* pa, p) { assert(pa);
				if (!pa->IsEpsilon()) {
					oss << pa_separator << "[";
					std::string a_separator = "";
					foreach(const Arc* a, *pa) { assert(a);
						oss << a_separator << Path::GetSymbols()->Find(a->ilabel);
						a_separator = "+";
					}
					float pa_length = pa->GetEndTime() - pa_start_time;
					pa_start_time = pa->GetEndTime();
					oss << "]:" << seconds_to_frames(pa_length) << ":" << std::setprecision(2) << exp(-pa->GetWeight().Value());
					pa_separator = "_";
				}
			}
			return oss.str();
		}

	protected:
		const std::string mTerm;
		const mlf::MlfRecords<ReferenceMlfRecord>& mRecsRef;
};

#endif
