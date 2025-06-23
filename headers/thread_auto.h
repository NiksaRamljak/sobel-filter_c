#ifndef THREAD_AUTO_H
#define THREAD_AUTO_H

#include "image.h"

// Gives optimal thread count, or user_override if > 0
int calc_thread(Image img, int user_override);

#endif
