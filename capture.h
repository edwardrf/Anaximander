#define CAPTURE_TYPE_GRAY 0
#define CAPTURE_TYPE_RGB 1

static void captureStart(void);
static void captureStop(void);
static void captureOneFrame(unsigned char* targetBuffer, int type);