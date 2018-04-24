
#include "UDP_BG.h"
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <bitset>


using namespace std;


static shared_data* p_sharedData;  // structure for sharing data between threads


								   // point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToUDP_BG(shared_data& sharedData) {

	p_sharedData = &sharedData;

}

// reset cognitive powers
void initUDP_BG(void) {


}

// plug in the socket to start listening to Emotiv
void updateUDP_BG(void) {

//	UdpListeningReceiveSocket socket(IpEndpointName(IP, PORT), &(p_sharedData->listener));
	//socket.RunUntilSigInt();

}





int udpsock(int port, const char* addr)
{

	SOCKET handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (handle < 1) {
		printf("Handle : %d\n\n", handle);
		return -1;
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	if (addr == INADDR_ANY) {
		address.sin_addr.s_addr = INADDR_ANY;
		printf("ANY ADDRESS");
	}
	else
		address.sin_addr.s_addr = inet_addr(addr);
	address.sin_port = htons((unsigned short)port);

	if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{ 
		printf("BIND FAIL\n");
		return -1;
	}
	
	return handle;
}

// function should return sender address info (for the code the server)
int recvudp(int sock, const int size, sockaddr_in& SenderAddr, int& SenderAddrSize)
{
	// TODO: use std::vector<char> here instead of char array
	char* buf = 0;
	buf = new char[size];
	//char buf[MAX_MSG];

	int retsize = recvfrom(sock, buf, size, 0, (sockaddr*)&SenderAddr, &SenderAddrSize);

	int i = 197;
	
		printf("Buf: 0x%x 0x%x 0x%x 0x%x\n", (unsigned)(unsigned char)buf[i], (unsigned)(unsigned char)buf[i + 1], (unsigned)(unsigned char)buf[i + 2], (unsigned)(unsigned char)buf[i + 3]);
	//printf("LENGTH OF DATAGRAM: %d\n",retsize); //361 length datagram
	//printf("\r%s", buf);
	// snag bytes 198 - 201 as bytes for Xvel, 202 - 205
		char * Xvel_mem = 0;
		char * Yvel_mem = 0;
		Xvel_mem = new char[4];
		Yvel_mem = new char[4];
		memcpy(Xvel_mem, buf + 197, 4);
		memcpy(Yvel_mem, buf + 201, 4);

	//	std::vector<int8_t> Xvel;
	//	std::vector<int8_t> Yvel;

	//	Xvel.push_back(buf[197]);
		//Xvel.push_back(buf[198]);
		//Xvel.push_back(buf[199]);
		//Xvel.push_back(buf[200]);

//		Yvel.push_back(buf[201]);
	//	Yvel.push_back(buf[202]);
	//	Yvel.push_back(buf[203]);
	//	Yvel.push_back(buf[204]);



	//memcpy(Xvel, buf + 197, 4);
	//memcpy(&Yvel[0], &buf[201, sizeof(Yvel));


//		Xvel_f.push_back(Xvel);
		//Xvel_f = (float)Xvel;// reinterpret_cast<float*>(&Xvel);
		//Yvel_f =// reinterpret_cast<float*>(&Yvel);
		
		//Xvel_f = (float)Xvel;
		//Yvel_f = (float)Yvel;
		float *  Xvel_f_ptr = (float *) Xvel_mem;
		//= (float)*Xvel_mem;
		//float  Yvel_f_ptr = (float) *Yvel_mem;

//	printf("Xvel 0x%x 0x%x 0x%x 0x%x\n", (unsigned)(unsigned char)Xvel[0], (unsigned)(unsigned char)Xvel[1], (unsigned)(unsigned char)Xvel[2], (unsigned)(unsigned char)Xvel[3]);
	printf("Xmem 0x%x 0x%x 0x%x 0x%x\n", (unsigned)(unsigned char)Xvel_mem[0], (unsigned)(unsigned char)Xvel_mem[1], (unsigned)(unsigned char)Xvel_mem[2], (unsigned)(unsigned char)Xvel_mem[3]);
	printf("\rx VEL : %.*f                   ",10, *Xvel_f_ptr);
	
	if (retsize == -1)
	{
		cout << "\nRecv Error : " << WSAGetLastError();

		if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == 0)
		{
			return -1;
		}

		return 0;
	}
	else if (retsize < size)
	{
		buf[retsize] = NULL;
	}


//	string str = "HI";
	delete[] buf;

	return( 1);
		
}

// On the client side, prepare dest like this:
//  sockaddr_in dest;
//  dest.sin_family = AF_INET;
//  dest.sin_addr.s_addr = inet_addr(ip.c_str());
//  dest.sin_port = htons(port);
int sendudp(string str, sockaddr_in dest, int sock)
{
	int ret = sendto(sock, str.c_str(), str.size(), 0, (sockaddr*)&dest, sizeof(dest));

	if (ret == -1)
	{
		cout << "\nSend Error Code : " << WSAGetLastError();
	}

	return ret;
}





// pack method for storing data in network,
//   big endian, byte order (MSB first)
// returns number of bytes packed
// usage:
//   float x, y, z;
//   int i = 0;
//   i += packFloat(&buffer[i], x);
//   i += packFloat(&buffer[i], y);
//   i += packFloat(&buffer[i], z);
int packFloat(char *buf, float x) {
	unsigned char *b = (unsigned char *)buf;
	unsigned char *p = (unsigned char *)&x;
#if defined (_M_IX86) || (defined (CPU_FAMILY) && (CPU_FAMILY == I80X86))
	b[0] = p[3];
	b[1] = p[2];
	b[2] = p[1];
	b[3] = p[0];
#else
	b[0] = p[0];
	b[1] = p[1];
	b[2] = p[2];
	b[3] = p[3];
#endif
	return 4;
}