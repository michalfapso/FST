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

#endif