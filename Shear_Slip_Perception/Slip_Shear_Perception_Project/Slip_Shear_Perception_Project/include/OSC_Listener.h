#pragma once
#include "OscPacketListener.h"
class OSC_Listener :
	public osc::OscPacketListener
{
public:
	OSC_Listener(void);
	void queryState(float& PosX, float& PosY, float& Gain);
	~OSC_Listener(void);

protected:
	 virtual void ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint );
};

