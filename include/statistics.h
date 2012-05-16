#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include "min_max.h"
#include "online_average.h"

template <typename T>
class Statistics {
	public:
		void Add(T val) {
			mMin.Add(val);
			mMax.Add(val);
			mAvg.Add(val);
		}
		T GetMin() { return mMin.GetValue(); }
		T GetMax() { return mMax.GetValue(); }
		float GetAvg() { return mAvg.GetValue(); }
	protected:
		Min<T> mMin;
		Max<T> mMax;
		OnlineAverage<float> mAvg;
};

#endif
