
#include "UDP_BG.h"
#include <iostream>
#include <stdlib.h>

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
string recvudp(int sock, const int size, sockaddr_in& SenderAddr, int& SenderAddrSize)
{
	// TODO: use std::vector<char> here instead of char array
	char* buf = 0;
	buf = new char[size];

	int retsize = recvfrom(sock, buf, size, 0, (sockaddr*)&SenderAddr, &SenderAddrSize);
	//printf("LENGTH OF DATAGRAM: %d\n",retsize); //361 length datagram

	// snag bytes 198 - 201 as bytes for Xvel, 202 - 205
	char* Xvel = 0;
	char* Yvel = 0;
	Xvel = new char[4]; // Char size is 1 byte
	Yvel = new char[4]; 

	Xvel[0] = buf[198 - 1];
	Xvel[1] = buf[199 - 1];
	Xvel[2] = buf[200 - 1];
	Xvel[3] = buf[201 - 1];

	Yvel[0] = buf[202 - 1];
	Yvel[1] = buf[203 - 1];
	Yvel[2] = buf[204 - 1];
	Yvel[3] = buf[205 - 1];
	float xf;
	xf = stof(Xvel);
	float yf;
	yf = stof(Yvel);

	printf("\rX VEL : %f                                        Y Vel : %f", xf, yf);
	if (retsize == -1)
	{
		cout << "\nRecv Error : " << WSAGetLastError();

		if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == 0)
		{
			return "";
		}

		return "\0";
	}
	else if (retsize < size)
	{
		buf[retsize] = NULL;
	}

	string str(Xvel);
	delete[] buf;

	return str;
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




