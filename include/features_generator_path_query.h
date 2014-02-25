#ifndef _FEATURES_GENERATOR_PATH_QUERY_H_
#define _FEATURES_GENERATOR_PATH_QUERY_H_

#include "features_generator_path.h"
#include "statistics.h"

template <class TPath>
class FeaturesGenerator_PathQuery : public FeaturesGenerator_Path<TPath>
{
	public:
		typedef TPath Path;
		typedef typename Path::Arc Arc;
		typedef ParallelArcs<Arc> PA;

		FeaturesGenerator_PathQuery(const std::string& outputFilename) : 
			FeaturesGenerator_Path<TPath>(outputFilename)
		{
			this->mFieldNamePrefix = "q";
		}

		void Generate(const OverlappingPathGroupList<Path>& pgl) {
			assert(pgl.size() == 1);
			FeaturesGenerator_Path<TPath>::Generate(pgl);
		}
		void Generate(const OverlappingPathGroup<Path>* pg) {
			assert(pg && pg->size() == 1);
			FeaturesGenerator_Path<TPath>::Generate(pg);
		}

	protected:
		virtual void PrintFeatures(const Path& path, const OverlappingPathGroup<Path>* pPathGroup, std::ostream& oss)
		{
			FeaturesGenerator_Path<TPath>::PrintFeatures(path, pPathGroup, oss);

			PrintField(oss, "PathInfo",      GetPathInfoString(path));
		}

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
