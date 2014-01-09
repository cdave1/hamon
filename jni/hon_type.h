// hon_type.h

#ifndef HON_TYPE_H_
#define HON_TYPE_H_

#define TRUE 1
#define FALSE 0

// APadの最低値、それいかにあまり変化がない（時間差にとって）
// #define BUFFER_SIZE 256

// Galaxy Sの最低限
//#define BUFFER_SIZE 128 // 0.72562358276643990929705215419501ms
//#define BUFFER_SIZE 1024 // 5.8049886621315192743764172335601ms
#define BUFFER_SIZE 512 // 5.8049886621315192743764172335601ms
#define BUFFER_SIZE_SHORT (BUFFER_SIZE / 2)

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, __VA_ARGS__)
#define LOGDw(...) __android_log_write(ANDROID_LOG_DEBUG, __VA_ARGS__)

#define SEC_IN_NS 1000000000
#define SEC_IN_US 1000000

#define PI 3.14159265358979

typedef void (*func_ptr)();

#endif /* HON_TYPE_H_ */
