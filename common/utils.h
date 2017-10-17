//-----------------------------------------------------------------------------
/*

Common Utility Functions

*/
//-----------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

//-----------------------------------------------------------------------------

static inline float clamp(float x, float a, float b) {
	if (x < a) {
		return a;
	}
	if (x > b) {
		return b;
	}
	return x;
}

//-----------------------------------------------------------------------------

#endif				// UTILS_H

//-----------------------------------------------------------------------------