#ifndef _FEATURES_GENERATOR_H_
#define _FEATURES_GENERATOR_H_

#include <iomanip>
#include "exception.h"
#include "foreach.h"
#include "path.h"
#include "dbg.h"
#include "online_average.h"
#include "min_max.h"
#include "seconds_to_mlf_time.h"

template <class TPath>
class FeaturesGenerator_Path
{
	public:
		typedef TPath Path;
		typedef typename Path::Arc Arc;
		typedef ParallelArcs<Arc> PA;

		static void PrintFeatures(const Path& path, const std::string& term, std::ostream& oss)
		{
			unsigned int epsilons_count = 0;
			OnlineAverage<float> phonemes_per_parallel_arc_avg;
			OnlineAverage<float> weight_epsilons_avg;
			OnlineAverage<float> weight_nonepsilons_avg;
			Min<float>           parallel_arcs_length_min;
			Max<float>           parallel_arcs_length_max;
			OnlineAverage<float> parallel_arcs_length_avg;

			float pa_start_time = path.GetStartTime();
			foreach(const PA* pa, path) {
				assert(pa);
				bool epsilon_found = false;
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
					pa_start_time = pa->GetEndTime();
					parallel_arcs_length_min.Add(pa_length);
					parallel_arcs_length_max.Add(pa_length);
					parallel_arcs_length_avg.Add(pa_length);
				}
			}
			unsigned int phoneme_parallel_arcs_count = path.size() - epsilons_count;
			assert(path.GetEndTime() >= 0);
			float time_length = path.GetEndTime() - path.GetStartTime();

			oss 
				// Standard MLF fields
				<< SecondsToMlfTime(path.GetStartTime()) << " "
				<< SecondsToMlfTime(path.GetEndTime()) << " "
				<< term << " "
				<< path.GetWeight() << " "
				// Arcs count
				<< epsilons_count << " "
				<< phoneme_parallel_arcs_count << " "
				// Time
				<< time_length << " "
				<< parallel_arcs_length_min.GetValue() << " "
				<< parallel_arcs_length_max.GetValue() << " "
				<< parallel_arcs_length_avg.GetValue() << " "
				// Parallel arcs phonemes count
				<< phonemes_per_parallel_arc_avg.GetValue() << " "
				// Weight
				<< weight_epsilons_avg.GetValue() << " "
				<< weight_nonepsilons_avg.GetValue() << " "
				;
		}
};

template <class TPath>
class FeaturesGenerator
{
	public:
		typedef TPath Path;
		FeaturesGenerator(const std::string& term, const std::string& outputFilename) : mTerm(term)
		{
			mOss.open("features.txt");
		}
		~FeaturesGenerator() {
			mOss.close();
		}
		void Generate(const OverlappingPathGroupList<Path>& pgl) {
			foreach(const OverlappingPathGroup<Path>* pg, pgl) {
				Generate(pg);
			}
		}
		void Generate(const OverlappingPathGroup<Path>* pg) {
			assert(pg);
			Generate(pg->GetBestPath());
//			foreach(const Path* p, *pg) {
//				Generate(p);
//			}
		}
		void Generate(const Path* p) {
			assert(p);
			FeaturesGenerator_Path<Path>::PrintFeatures(*p, mTerm, mOss);
			mOss << "| " << *p;
			mOss << endl;
		}
	protected:
		const std::string mTerm;
		std::ofstream mOss;
};
#endif
