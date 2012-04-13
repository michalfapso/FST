#ifndef _SECONDS_TO_MLF_TIME_H_
#define _SECONDS_TO_MLF_TIME_H_

#include "seconds_to_frames.h"

class SecondsToMlfTime {
	public:
		SecondsToMlfTime(float seconds) : mSeconds(seconds) {}
		friend std::ostream& operator<<(std::ostream& oss, const SecondsToMlfTime& s) {
			unsigned int frames = seconds_to_frames(s.mSeconds);
			if (frames == 0) {
				oss << "0";
			} else {
				oss << frames << "00000";
			}
			return oss;
		}
	protected:
		float mSeconds;
};

#endif
