#ifndef _FEATURES_GENERATOR_H_
#define _FEATURES_GENERATOR_H_

#include <iomanip>
#include <sstream>
#include "exception.h"
#include "foreach.h"
#include "path.h"
#include "dbg.h"
#include "online_average.h"
#include "min_max.h"
#include "seconds_to_mlf_time.h"

#include "mlf.h"
#include "hypotheses.h"

class FeaturesGenerator
{
	public:
		FeaturesGenerator(const std::string& outputFilename)
		{
			mOss.open(outputFilename);
			if (!mOss.good()) {
				THROW("ERROR: FeaturesGenerator: Can not open file "<<outputFilename<<" for writing!");
			}
		}
		virtual ~FeaturesGenerator() {
			mOss.close();
		}
		
		static void SetPrintFieldNames(bool p) {msPrintFieldNames = p;}
		static void SetPrintFieldValues(bool p) {msPrintFieldValues = p;}
	protected:
		static bool msPrintFieldNames;
		static bool msPrintFieldValues;
		std::ofstream mOss;

		template <typename T>
		static void PrintField(std::ostream& oss, const std::string& name, const T& val) {
			if (msPrintFieldNames) { oss << name; }
			if (msPrintFieldNames && msPrintFieldValues) { oss << "="; }
			if (msPrintFieldValues) { oss << val; }
			oss << " ";
		}

		static bool IsHit(float startTimeSeconds, float endTimeSeconds, const mlf::MlfRecords<ReferenceMlfRecord>& recsRef) {
			//float best_overlapping_ratio = 0;
			float p_start_frame = seconds_to_frames(startTimeSeconds);
			float p_end_frame = seconds_to_frames(endTimeSeconds);
			foreach(const ReferenceMlfRecord* ref_rec, recsRef) {
				// if they overlap
				if (p_start_frame < ref_rec->GetEndTime() && p_end_frame > ref_rec->GetStartTime()) {
					return true;
				}
			}
			return false;
		}
};
bool FeaturesGenerator::msPrintFieldNames = false;
bool FeaturesGenerator::msPrintFieldValues = true;


template <class TPath>
class FeaturesGenerator_Path : public FeaturesGenerator
{
	public:
		typedef TPath Path;
		typedef typename Path::Arc Arc;
		typedef ParallelArcs<Arc> PA;

		FeaturesGenerator_Path(const std::string& outputFilename, const std::string& term, const mlf::MlfRecords<ReferenceMlfRecord>& recsRef) : 
			FeaturesGenerator(outputFilename),
			mTerm(term), 
			mRecsRef(recsRef)
		{
			// Print table header
			SetPrintFieldNames(true);
			SetPrintFieldValues(false);
			{
				PrintFeatures(Path(-1,-1), mTerm, mRecsRef, NULL, mOss); mOss << endl;
			}
			SetPrintFieldNames(false);
			SetPrintFieldValues(true);
		}

		void Generate(const OverlappingPathGroupList<Path>& pgl) {
			foreach(const OverlappingPathGroup<Path>* pg, pgl) {
				Generate(pg);
			}
		}
		void Generate(const OverlappingPathGroup<Path>* pg) {
			assert(pg);
			Generate(pg->GetBestPath(), pg);
//			foreach(const Path* p, *pg) {
//				Generate(p, pg);
//			}
		}
		void Generate(const Path* p, const OverlappingPathGroup<Path>* pg) {
			assert(p);
			PrintFeatures(*p, mTerm, mRecsRef, pg, mOss);
			//mOss << "| " << *p;
			// Various path info:
			mOss << endl;
			//Path::SetPrintType(PRINT_ALL);
			//mOss << "DEBUG: " << *p << endl << endl;
		}

	protected:
		static void PrintFeatures(const Path& path, const std::string& term, const mlf::MlfRecords<ReferenceMlfRecord>& recsRef, const OverlappingPathGroup<Path>* pPathGroup, std::ostream& oss)
		{
			unsigned int epsilons_count = 0;
			OnlineAverage<float> phonemes_per_parallel_arc_avg;
			OnlineAverage<float> weight_epsilons_avg;
			OnlineAverage<float> weight_nonepsilons_avg;
			OnlineAverage<float> weight_avg;
			OnlineAverage<float> weight_avg_log;
			typename Arc::Weight weight_multiplied = Arc::Weight::One();
			Min<float>           parallel_arcs_length_min;
			Max<float>           parallel_arcs_length_max;
			OnlineAverage<float> parallel_arcs_length_avg;

			float pa_start_time = path.GetStartTime();
			float weight_nonepsilons_avg_weighted_by_arc_length = 0;
			float time_length = -1;

			typename Arc::Weight opg_sum_weight = Arc::Weight::Zero();

			if (msPrintFieldValues) {
				foreach(const PA* pa, path) {
					assert(pa);
					bool epsilon_found = false;
					weight_multiplied = fst::Times(weight_multiplied, pa->GetWeight());
					weight_avg_log.Add(pa->GetWeight().Value());
					weight_avg.Add(exp(-pa->GetWeight().Value()));
					foreach(const Arc* a, *pa) {
						epsilon_found |= a->ilabel == 0;
					}
					if (epsilon_found && pa->size() > 1) {
						THROW("ERROR: epsilon arc should not have more parallel arcs! ("<<*pa<<")");
					}
					epsilons_count += epsilon_found;
					if (epsilon_found) {
						weight_epsilons_avg.Add(pa->GetWeight().Value());
					} else {
						weight_nonepsilons_avg.Add(pa->GetWeight().Value());
						phonemes_per_parallel_arc_avg.Add(pa->size());
						assert(pa->GetEndTime() >= 0);
						float pa_length = pa->GetEndTime() - pa_start_time;
						parallel_arcs_length_min.Add(pa_length);
						parallel_arcs_length_max.Add(pa_length);
						parallel_arcs_length_avg.Add(pa_length);
						if (pa_length > 0) {
							weight_nonepsilons_avg_weighted_by_arc_length += exp(-pa->GetWeight().Value()) * pa_length;
						}
						pa_start_time = pa->GetEndTime();
						//DBG("pa: "<<*pa<<" length="<<pa_length<<" weight="<<pa->GetWeight().Value());
					}
				}
				assert(path.GetEndTime() >= 0);
				time_length = path.GetEndTime() - path.GetStartTime();
				if (time_length <= 0) {
					weight_nonepsilons_avg_weighted_by_arc_length = 0;
				} else {
					weight_nonepsilons_avg_weighted_by_arc_length /= time_length;
					weight_nonepsilons_avg_weighted_by_arc_length = -log(weight_nonepsilons_avg_weighted_by_arc_length);
				}

				if (pPathGroup) {
					foreach (Path* p, *pPathGroup) {
						opg_sum_weight = Plus(opg_sum_weight, p->GetWeight());
					}
				}
			}

			// Standard MLF fields
			PrintField(oss, "isHit",         IsHit(path.GetStartTime(), path.GetEndTime(), recsRef));
			PrintField(oss, "startT",        SecondsToMlfTime(path.GetStartTime()));
			PrintField(oss, "endT",          SecondsToMlfTime(path.GetEndTime()));
			PrintField(oss, "term",          term);
			oss << std::setprecision(6);
			PrintField(oss, "score",         -path.GetWeight().Value());
			// Arcs count
			PrintField(oss, "epsC",          epsilons_count);
			PrintField(oss, "phnC",          path.size() - epsilons_count);
			// Time
			PrintField(oss, "lenT",          time_length);
			PrintField(oss, "lenTMin",       parallel_arcs_length_min.GetValue());
			PrintField(oss, "lenTMax",       parallel_arcs_length_max.GetValue());
			PrintField(oss, "lenTAvg",       parallel_arcs_length_avg.GetValue());
			// Parallel arcs phonemes count
			PrintField(oss, "paCAvg",        phonemes_per_parallel_arc_avg.GetValue());
			// Weight
			PrintField(oss, "epsWAvg",       weight_epsilons_avg.GetValue());
			PrintField(oss, "nepsWAvg",      weight_nonepsilons_avg.GetValue());
			PrintField(oss, "nepsWAvgWghtd", weight_nonepsilons_avg_weighted_by_arc_length);
			PrintField(oss, "avgW",          log(weight_avg.GetValue()));
			PrintField(oss, "avgWLog",       weight_avg_log.GetValue());
			PrintField(oss, "multW",         weight_multiplied.Value());
			PrintField(oss, "opgSize",       pPathGroup ? pPathGroup->size() : 0);
			PrintField(oss, "opgSumW",       opg_sum_weight.Value());
			PrintField(oss, "pathInfo",      GetPathInfoString(path));
		}
	protected:
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
