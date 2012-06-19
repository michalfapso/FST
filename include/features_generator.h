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
#include "ostream_file_stdout.h"

#include "mlf.h"
#include "hypotheses.h"

class FeaturesGenerator
{
	public:
		FeaturesGenerator(const std::string& outputFilename) :
			mOss(outputFilename),
			mFieldNamePrefix("")
		{}

		static void SetPrintFieldNames(bool p) {msPrintFieldNames = p;}
		static void SetPrintFieldValues(bool p) {msPrintFieldValues = p;}
	protected:
		static bool msPrintFieldNames;
		static bool msPrintFieldValues;
		OstreamFileStdout mOss;
		bool mFieldSeparator;
		std::string mFieldNamePrefix;

		void PrintField_begin(std::ostream& oss) {
			mFieldSeparator = false;
		}

		template <typename T>
		void PrintField(std::ostream& oss, const std::string& name, const T& val) {
			if (mFieldSeparator) { oss << " "; } else {mFieldSeparator = true;}
			if (msPrintFieldNames) { oss << mFieldNamePrefix << name; }
			if (msPrintFieldNames && msPrintFieldValues) { oss << "="; }
			if (msPrintFieldValues) { oss << val; }
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

#endif
