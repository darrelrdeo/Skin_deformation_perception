/*============================================================================*/
/*        National Instruments USB X-Series DAQ Wrapper Functions             */
/*----------------------------------------------------------------------------*/
/*                 Andrew Stanley and Zhan Fan Quek 2013                      */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       NIDAQcommands.h                                               */
/* Purpose:     Include file for NIDAQcommands wrapper functions              */
/*                                                                            */
/*============================================================================*/

#ifndef NIDAQCOMMANDS_H
#define NIDAQCOMMANDS_H

// Define the DAQ in use
#define USB_6341
//#define USB_6343


#include "NIDAQmx.h"

#ifdef USB_6341
	#define SAMPLING_RATE 5000 //Arbitrary (for now) sampling rate (in Hertz, I assume)
	#define ALL_AO_CHANNELS "Dev1/ao0:1" //All 2 analog outputs of the NI USB-6341 DAQ
	#define ALL_AI_CHANNELS "Dev1/ai0:15" //All 16 analog inputs of the NI USB-6341 DAQ
	#define ALL_PORT0_CHANNELS "Dev1/port0" //All of the 8 channels on port0 of the NI USB-6341 DAQ
	#define ALL_PORT1_CHANNELS "Dev1/port1" //All of the 8 channels on port1 of NI USB-6341 DAQ
	#define ALL_PORT2_CHANNELS "Dev1/port2" //All of the 8 channels on port2 of NI USB-6341 DAQ
	#define SAMPLES_TO_ACQUIRE 10 //If you want many samples to filter/average an input reading, make this higher
	#define SINGLE_SAMPLE 1 //Only take one sample
	#define CONSTANT_OUTPUT 1   //When writing a constant output, only need to send one sample
	#define SAMPLES_TO_SEND 1000 //When creating a variable output send array of sample values
	#define TIMEOUT_TIME 10 //Seconds to timeout
	#define AUTO_START_TRUE 1
	#define AUTO_START_FALSE 0
	#define DIGITAL_OUTPUT_CHANNELS 8 //Number of channels on port0 of the NI USB-6341 DAQ
	#define DIGITAL_INPUT_CHANNELS1 8 //Number of channels on port1 of NI USB-6341 DAQ
	#define DIGITAL_INPUT_CHANNELS2 8 //Number of channels on port2 of NI USB-6341 DAQ
	#define ANALOG_OUTPUT_CHANNELS 2 //Number of analog output channels available on the NI USB-6341 DAQ
	#define ANALOG_INPUT_CHANNELS 16 //Number of analog input channels avaialbe on the NI USB-6341 DAQ
	#define VMIN -10 //Lowest output voltage of NI USB-6341 DAQ
	#define VMAX 10 //Highest output voltage of NI USB-6341 DAQ
#endif // USB_6341

#ifdef USB_6343
	#define SAMPLING_RATE 5000 //Arbitrary (for now) sampling rate (in Hertz, I assume)
	#define ALL_AO_CHANNELS "Dev1/ao0:3" //All 4 analog outputs of the NI USB-6343 DAQ
	#define ALL_AI_CHANNELS "Dev1/ai0:31" //All 16 analog inputs of the NI USB-6343 DAQ
	#define ALL_PORT0_CHANNELS "Dev1/port0" //All of the 32 channels on port0 of the NI USB-6343 DAQ
	#define ALL_PORT1_CHANNELS "Dev1/port1" //All of the 8 channels on port1 of NI USB-6343 DAQ
	#define ALL_PORT2_CHANNELS "Dev1/port2" //All of the 8 channels on port2 of NI USB-6343 DAQ
	#define SAMPLES_TO_ACQUIRE 10 //If you want many samples to filter/average an input reading, make this higher
	#define SINGLE_SAMPLE 1 //Only take one sample
	#define CONSTANT_OUTPUT 1   //When writing a constant output, only need to send one sample
	#define SAMPLES_TO_SEND 1000 //When creating a variable output send array of sample values
	#define TIMEOUT_TIME 10 //Seconds to timeout
	#define AUTO_START_TRUE 1
	#define AUTO_START_FALSE 0
	#define DIGITAL_OUTPUT_CHANNELS 32 //Number of channels on port0 of the NI USB-6343 DAQ
	#define DIGITAL_INPUT_CHANNELS1 8 //Number of channels on port1 of NI USB-6343 DAQ
	#define DIGITAL_INPUT_CHANNELS2 8 //Number of channels on port2 of NI USB-6343 DAQ
	#define ANALOG_OUTPUT_CHANNELS 4 //Number of analog output channels available on the NI USB-6343 DAQ
	#define ANALOG_INPUT_CHANNELS 32 //Number of analog input channels avaialbe on the NI USB-6343 DAQ
	#define VMIN -10 //Lowest output voltage of NI USB-6343 DAQ
	#define VMAX 10 //Highest output voltage of NI USB-6343 DAQ
#endif // USB_6343


class NIDAQcommands
{
public:
    NIDAQcommands();
    void writeAnalogOutput(int channelNumber, double analogValue);
    double readAnalogInput(int channelNumber);
    void writeDigitalOutput(int channelNumber, int digitalValue);
    void writeDigitalOutputsPort0(unsigned char digitalArray[]);
    int readDigitalInput(int channelNumber);

private:
    void setupAnalogInputs(char* channelNames);
    TaskHandle taskHandleAI;
    double dataAI[ANALOG_INPUT_CHANNELS*SAMPLES_TO_ACQUIRE];

    void setupAnalogOutputs(char* channelNames);
    TaskHandle taskHandleAO;
    double dataAO[ANALOG_OUTPUT_CHANNELS];

    void setupDigitalInputs(char* channelNames);
    TaskHandle taskHandleDI;
    unsigned char dataDI[SINGLE_SAMPLE]; //DIGITAL_INPUT_CHANNELS1*

    void setupDigitalOutputs(char* channelNames);
    TaskHandle taskHandleDO;
    unsigned char dataDO[DIGITAL_OUTPUT_CHANNELS];

};

#endif // NIDAQCOMMANDS_H
