#include "stdio.h"
#include "windows.h"
//#include "nr.h"

//#define DEFAULT_MATBUFFER_LENGTH 50 // 1000
//#define SLICE_BUF_SIZE 50
//#define CONTROL_DIM 2

double	readTime(bool resetFlag);
//class MatBuffer;
//
//class MatBuffer {
//private:
//	Mat_DP*  buf;
//	int pos, length, fullFlag;
//public:
//	MatBuffer(int len=DEFAULT_MATBUFFER_LENGTH);
//	~MatBuffer(void);
//	MatBuffer& operator++();
//	Mat_DP& operator[](int n);
//	Mat_DP getBuffer();
//	int GetFlag();
//};