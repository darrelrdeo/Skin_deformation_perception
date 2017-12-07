#include "utilsFT.h"

double readTime(bool resetFlag)
{
	LARGE_INTEGER sample1;
	static LARGE_INTEGER epoch;
	static LARGE_INTEGER frequency;

	double time = 0.0;
	double actual_time = 0;
	
	if (resetFlag) 
	{
		QueryPerformanceCounter(&epoch);
		QueryPerformanceFrequency(&frequency);
	}
	else 
	{
		QueryPerformanceCounter(&sample1);
		actual_time = sample1.QuadPart - epoch.QuadPart;
		actual_time /= frequency.QuadPart;
	}

	return actual_time;
}

//MatBuffer::MatBuffer(int len)
//{
//	if (len<1) {
//		len = DEFAULT_MATBUFFER_LENGTH;
//	}
//	buf = new Mat_DP[len];
//	length = len;
//	pos = 0;
//	fullFlag = 0;
//}
//
//MatBuffer::~MatBuffer(void)
//{
//	if (buf)
//		delete[] buf;
//}
//
//MatBuffer& MatBuffer::operator++()
//{
//	pos++;
//	if (pos>=length) {
//		pos = 0;
//		fullFlag = 1;
//	}
//	return *this;
//}
//
//Mat_DP& MatBuffer::operator[](int n)
//{
//	int index = pos-n;
//	while (index<0) {
//		index += length;
//	}
//	while (index>=length) {
//		index -= length;
//	}
//	return buf[index];
//}
//
//Mat_DP MatBuffer::getBuffer()
//{
//	int index = pos+1;
//	int ncols = (*this)[0].ncols();
//	ncols = (ncols<1)? 1:ncols;
//	Mat_DP res(-999,length,ncols);
//	if (fullFlag) {
//		for (int i=0; i<length; i++) {
//			if (index>=length)
//				index -= length;
//			for (int j=0; j<ncols; j++) {
//				res[i][j] = buf[index][0][j];
//			}
//			index++;
//		}
//	}
//	return res;
//}
//
//int MatBuffer::GetFlag()
//{
//	int Out = 0;
//	Out = fullFlag;
//	return Out;
//
//}
