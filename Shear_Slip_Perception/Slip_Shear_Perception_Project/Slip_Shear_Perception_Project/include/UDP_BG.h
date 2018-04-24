
#ifndef UDP_BG_H
#define UDP_BG_H
#define WIN32_LEAN_AND_MEAN
#include <winSock2.h>
#include <windows.h>
#include <string>

// library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include "UdpSocket.h"
//#include "OSC_Listener.h"
#include "chai3d.h"
#include "shared_Data.h"

void linkSharedDataToUDP_BG(shared_data& sharedData);
void initUDP_BG(void);
void updateUDP_BG(void);


int udpsock(int port, const char* addr);
int recvudp(int sock, const int size, sockaddr_in& SenderAddr, int& SenderAddrSize);
int sendudp(std::string str, sockaddr_in dest, int sock);
int packFloat(char *buf, float x);
#endif  // UDP_BG_H
