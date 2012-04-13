#ifndef _MIN_MAX_H_
#define _MIN_MAX_H_

#include <limits>

template <typename T>
class Min {
	public:
		Min() : mValue(std::numeric_limits<T>::max()) {}
		inline T GetValue() const { return mValue; }
		void Add(T val) {
			if (mValue > val) {
				mValue = val;
			}
		}
	protected:
		T mValue;
};

template <typename T>
class Max {
	public:
		Max() : mValue(std::numeric_limits<T>::min()) {}
		inline T GetValue() const { return mValue; }
		void Add(T val) {
			if (mValue < val) {
				mValue = val;
			}
		}
	protected:
		T mValue;
};

#endif
