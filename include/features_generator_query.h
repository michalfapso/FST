#ifndef _FEATURES_GENERATOR_QUERY_H_
#define _FEATURES_GENERATOR_QUERY_H_

#include "features_generator.h"
#include "statistics.h"

template <class TPath>
class FeaturesGenerator_Query : public FeaturesGenerator
{
	public:
		typedef TPath Path;
		typedef typename Path::Arc Arc;
		typedef ParallelArcs<Arc> PA;

		FeaturesGenerator_Query(const std::string& outputFilename) : 
			FeaturesGenerator(outputFilename)
		{
			// Print table header
			SetPrintFieldNames(true);
			SetPrintFieldValues(false);
			{
				PrintFeatures(Path(-1,-1), NULL, mOss); mOss << endl;
			}
			SetPrintFieldNames(false);
			SetPrintFieldValues(true);
		}

		void Generate(const OverlappingPathGroupList<Path>& pgl) {
			assert(pgl.size() == 1);
			foreach(const OverlappingPathGroup<Path>* pg, pgl) {
				Generate(pg);
			}
		}
		void Generate(const OverlappingPathGroup<Path>* pg) {
			assert(pg);
			assert(pg->size() == 1);
			Generate(pg->GetBestPath(), pg);
//			foreach(const Path* p, *pg) {
//				Generate(p, pg);
//			}
		}
		void Generate(const Path* p, const OverlappingPathGroup<Path>* pg) {
			assert(p);
			PrintFeatures(*p, pg, mOss);
			//mOss << "| " << *p;
			// Various path info:
			mOss << endl;
			//Path::SetPrintType(PRINT_ALL);
			//mOss << "DEBUG: " << *p << endl << endl;
		}

	protected:
		void PrintFeatures(const Path& path, const OverlappingPathGroup<Path>* pPathGroup, std::ostream& oss) const
		{
			OnlineAverage<float>     phonemes_per_parallel_arc_avg;
			typename Arc::Weight     weight_multiplied = Arc::Weight::One();
			Statistics<float>        pa_length_stats;
			Statistics<float>        pa_weight_stats;
			Statistics<unsigned int> pa_phonemes_count_stats;
			Statistics<float>        a_best_length_stats;
			Statistics<float>        a_best_weight_stats;
			unsigned int             a_best_count = 0;
			typename Arc::Weight     a_weight_sum = Arc::Weight::Zero();

			float pa_start_time = path.GetStartTime();
			float pa_weight_avg_weighted_by_arc_length = 0;
			float a_best_weight_avg_weighted_by_arc_length = 0;
			float time_length = -1;

			typename Arc::Weight opg_sum_weight = Arc::Weight::Zero();

			if (msPrintFieldValues) {
				foreach(const PA* pa, path) {
					assert(pa);
					weight_multiplied = fst::Times(weight_multiplied, pa->GetWeight());

					// PA phonemes count
					unsigned int pa_phonemes_count = 0;
					foreach(const Arc* a, *pa) { pa_phonemes_count += a->ilabel == 0 ? 0 : 1; }
					pa_phonemes_count_stats.Add(pa_phonemes_count);

					// PA timing
					assert(pa->GetEndTime() >= 0);
					float pa_length = pa->GetEndTime() - pa_start_time;
					pa_length_stats.Add(pa_length);
					pa_start_time = pa->GetEndTime();

					// PA weight
					if (pa_length > 0) {
						pa_weight_avg_weighted_by_arc_length += exp(-pa->GetWeight().Value()) * pa_length;
					}
					typename Arc::Weight pa_weight_nonepsilon = Arc::Weight::Zero();
					foreach(const Arc* a, *pa) { assert(a);
						if (a->ilabel != 0) {
							pa_weight_nonepsilon = Plus(pa_weight_nonepsilon, a->weight);
						}
					}
					a_weight_sum = Plus(a_weight_sum, pa_weight_nonepsilon);
					pa_weight_stats.Add(exp(-pa_weight_nonepsilon.Value()));

					// PA best arc
					const Arc* a_best = NULL;
					foreach(const Arc* a, *pa) { assert(a);
						if (!a_best || a_best->weight.Value() > a->weight.Value()) {
							a_best = a;
						}
					}
					if (a_best && a_best->ilabel != 0) {
						a_best_length_stats.Add(pa_length);
						a_best_count ++;
						a_best_weight_stats.Add(exp(-a_best->weight.Value()));
						a_best_weight_avg_weighted_by_arc_length += exp(-a_best->weight.Value()) * pa_length;
					}

					//DBG("pa: "<<*pa<<" length="<<pa_length<<" weight="<<pa->GetWeight().Value());
				}
				assert(path.GetEndTime() >= 0);
				time_length = path.GetEndTime() - path.GetStartTime();
				if (time_length <= 0) {
					pa_weight_avg_weighted_by_arc_length = 0;
					a_best_weight_avg_weighted_by_arc_length = 0;
				} else {
					pa_weight_avg_weighted_by_arc_length /= time_length;
					pa_weight_avg_weighted_by_arc_length = -log(pa_weight_avg_weighted_by_arc_length);
					a_best_weight_avg_weighted_by_arc_length /= time_length;
					a_best_weight_avg_weighted_by_arc_length = -log(a_best_weight_avg_weighted_by_arc_length);
				}
			}

			// Standard MLF fields
			oss << std::setprecision(6);
			// Arcs count
			PrintField(oss, "paC",           path.size());
			PrintField(oss, "aBestC",        a_best_count);
			// Time
			PrintField(oss, "lenT",          time_length);
			PrintField(oss, "paLenTMin",     pa_length_stats.GetMin());
			PrintField(oss, "paLenTMax",     pa_length_stats.GetMax());
			PrintField(oss, "paLenTAvg",     pa_length_stats.GetAvg());
			PrintField(oss, "aBestLenTMin",  a_best_length_stats.GetMin());
			PrintField(oss, "aBestLenTMax",  a_best_length_stats.GetMax());
			PrintField(oss, "aBestLenTAvg",  a_best_length_stats.GetAvg());
			// Parallel arcs phonemes count
			PrintField(oss, "paPhnCMin",     pa_phonemes_count_stats.GetMin());
			PrintField(oss, "paPhnCMax",     pa_phonemes_count_stats.GetMax());
			PrintField(oss, "paPhnCAvg",     pa_phonemes_count_stats.GetAvg());
			// Weight
			PrintField(oss, "paWMin",        pa_weight_stats.GetMin());
			PrintField(oss, "paWMax",        pa_weight_stats.GetMax());
			PrintField(oss, "paWAvg",        pa_weight_stats.GetAvg());
			PrintField(oss, "paWAvgWghtd",   pa_weight_avg_weighted_by_arc_length);
			PrintField(oss, "aBestWMin",     a_best_weight_stats.GetMin());
			PrintField(oss, "aBestWMax",     a_best_weight_stats.GetMax());
			PrintField(oss, "aBestWAvg",     a_best_weight_stats.GetAvg());
			PrintField(oss, "aBestWAvgWghtd",a_best_weight_avg_weighted_by_arc_length);
			PrintField(oss, "aWSumLog",      a_weight_sum.Value());
			PrintField(oss, "aWSum",         exp(-a_weight_sum.Value()));

			PrintField(oss, "multW",         weight_multiplied.Value());
			PrintField(oss, "pathInfo",      GetPathInfoString(path));
		}
	protected:
		static const std::string GetPathInfoString(const Path& p) 
		{
			std::ostringstream oss;
			float pa_start_time = p.GetStartTime();
			std::string pa_separator = "";
			foreach(const ParallelArcs<Arc>* pa, p) { assert(pa);
				const Arc* a_best = NULL;
				foreach(const Arc* a, *pa) { assert(a);
					if (!a_best || a_best->weight.Value() > a->weight.Value()) {
						a_best = a;
					}
				}
				if (a_best && a_best->ilabel != 0) {
					oss << pa_separator << Path::GetSymbols()->Find(a_best->ilabel);

					float pa_length = pa->GetEndTime() - pa_start_time;
					pa_start_time = pa->GetEndTime();
					oss << ":" << seconds_to_frames(pa_length);
					pa_separator = "_";
				}
			}
			return oss.str();
		}
};

#endif
