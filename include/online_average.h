#ifndef _ONLINE_AVERAGE_H_
#define _ONLINE_AVERAGE_H_

#include "dbg.h"

template <typename T>
class OnlineAverage {
	public:
		OnlineAverage() : mCount(0), mValue(0) {}
		inline T GetValue() const { return mValue; }
		static T WithValue(T oldVal, unsigned int oldCount, T addVal) {
			return (oldVal * oldCount + addVal) / (oldCount+1);
		}
		static T WithoutValue(T oldVal, unsigned int oldCount, T remVal) {
			return (oldVal * oldCount - remVal) / (oldCount-1);
		}
		void Add(T val) {
			mValue = WithValue(mValue, mCount, val);
			mCount++;
		}
		void Remove(T val) {
			mValue = WithoutValue(mValue, mCount, val);
			mCount--;
		}
	protected:
		unsigned int mCount;
		T mValue;
};

#endif
