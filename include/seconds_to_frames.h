#ifndef _SECONDS_TO_FRAMES_H_
#define _SECONDS_TO_FRAMES_H_

unsigned int seconds_to_frames(float sec) {
	return (int)(floor(sec * 100 + 0.5));
}

#endif
