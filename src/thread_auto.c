#include "thread_auto.h"

#ifdef W_AUTOTHREAD

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// function to get CPU cores
static int get_num_cores(void) {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
#else
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    return (cores > 0) ? (int)cores : 1;
#endif
}

int calc_thread(Image img, int user_override) {
    if (user_override > 0) {
        return user_override;
    }

    int cores = get_num_cores();

    // Tune this threshold for minimal pixels per thread to justify overhead
    const int pixels_per_thread = 250000;

    int total_pixels = img.width * img.height;
    int max_threads_by_size = total_pixels / pixels_per_thread;
    if (max_threads_by_size < 1){
		max_threads_by_size = 1;
	}
    int threads = (cores < max_threads_by_size) ? cores : max_threads_by_size;

    // At least 1
    if (threads < 1){
		threads = 1;
	}
    return threads;
}

#else //NO AUTOSCALING

int calc_thread(Image img,int user_override) {
    int total_size = img.width * img.height;
    // rough optimization
    if (user_override > 0) {
        return user_override;
    }else if(total_size< 500* 1024) {				//5k
        return 1; 
    } else if (total_size < 1000 * 1024) {		//1m
        return 2;
    } else if (total_size < 4000 * 1024) {	    //4m
        return 4;
    } else {
        return 8;
    }
}
#endif