#ifndef DETECTION_END_H
#define DETECTION_END_H

template <class Arc>
struct DetectionEnd {
	DetectionEnd(typename Arc::Weight w, float t) : mWeight(w), mEndTime(t) {}

	typename Arc::Weight mWeight;
	float mEndTime;
};

#endif
