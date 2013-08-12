#define CAPTURE_TYPE_GRAY 0
#define CAPTURE_TYPE_BGR 1

void initCapture(void);
void finishCapture(void);
void captureOneFrame(unsigned char* targetBuffer, int type);
