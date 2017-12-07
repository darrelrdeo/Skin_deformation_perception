/*============================================================================*/
/*        National Instruments USB X-Series DAQ Wrapper Functions             */
/*----------------------------------------------------------------------------*/
/*                 Andrew Stanley and Zhan Fan Quek 2013                      */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       NIDAQcommands.cpp                                             */
/* Purpose:     Make it easier to use the NI X-Series DAQs with C++           */
/*                                                                            */
/*============================================================================*/

#include "NIDAQcommands.h"
#include <stdio.h>

NIDAQcommands::NIDAQcommands()
{
	

    //Initialize variables to zero
    taskHandleAI = 0;
    taskHandleAO = 0;
    taskHandleDI = 0;
    taskHandleDO = 0;

    //Initialize output data to 0 for digital and analog
    for(int i = 0; i<DIGITAL_OUTPUT_CHANNELS; i++){
        dataDO[i] = 0;
    }
    for(int i = 0; i<ANALOG_OUTPUT_CHANNELS; i++){
        dataAO[i] = 0;
    }

    //Set up desired ports
    setupDigitalOutputs(ALL_PORT0_CHANNELS);
    setupDigitalInputs(ALL_PORT1_CHANNELS);
    setupAnalogOutputs(ALL_AO_CHANNELS);
   // setupAnalogInputs(ALL_AI_CHANNELS);
}

void NIDAQcommands::writeAnalogOutput(int channelNumber, double analogValue)
{
    long written;
    dataAO[channelNumber] = analogValue;
    DAQmxWriteAnalogF64(taskHandleAO, CONSTANT_OUTPUT, AUTO_START_TRUE, TIMEOUT_TIME, DAQmx_Val_GroupByChannel, dataAO, &written, NULL);
}

double NIDAQcommands::readAnalogInput(int channelNumber)
{
    long samplesExpected = (long)SAMPLES_TO_ACQUIRE*(long)ANALOG_INPUT_CHANNELS;
    long samplesRead; //To store the actual number of samples the DAQ recorded

    DAQmxReadAnalogF64(taskHandleAI,SAMPLES_TO_ACQUIRE,TIMEOUT_TIME,DAQmx_Val_GroupByChannel,dataAI,samplesExpected,&samplesRead,NULL);

    //For now, just look at the first sample taken
    return dataAI[channelNumber*SAMPLES_TO_ACQUIRE];
}

void NIDAQcommands::writeDigitalOutput(int channelNumber, int digitalValue)
{
    //Sets output high if digitalValue is 1, low if digitalValue is 0
    dataDO[channelNumber] = digitalValue;
    writeDigitalOutputsPort0(dataDO);
}

void NIDAQcommands::writeDigitalOutputsPort0(unsigned char digitalArray[])
{
    //Write all 32 channels of port0 at once
    DAQmxWriteDigitalLines(taskHandleDO,CONSTANT_OUTPUT,AUTO_START_TRUE,TIMEOUT_TIME,DAQmx_Val_GroupByChannel,digitalArray,NULL,NULL);
}

int NIDAQcommands::readDigitalInput(int channelNumber)
{
    //Returns 1 if input is high, 0 if input is low
    long samplesExpected = (long)SINGLE_SAMPLE;
    long samplesRead; //To store the actual number of samples the DAQ recorded

    DAQmxReadDigitalU8(taskHandleDI,SINGLE_SAMPLE,TIMEOUT_TIME,DAQmx_Val_GroupByChannel,dataDI,samplesExpected,&samplesRead,NULL);

    //For now assume only one sample is written, use that sample for output
    return (dataDI[0] >> channelNumber) & 1; //Check the n'th bit of dataDI by shifting right n times, n is channelNumber
}

void NIDAQcommands::setupAnalogInputs(char* channelNames)
{
    DAQmxCreateTask("",&taskHandleAI);
    //Sample voltage differential between pin+ and ground (change DAQmx_Val_RSE to DAQmx_Val_DIFF to read differential beween pin+ and pin-)
    DAQmxCreateAIVoltageChan(taskHandleAI, channelNames,"",DAQmx_Val_RSE,VMIN,VMAX,DAQmx_Val_Volts,NULL);
    DAQmxCfgSampClkTiming(taskHandleAI,"",SAMPLING_RATE,DAQmx_Val_Rising,DAQmx_Val_ContSamps,SAMPLES_TO_ACQUIRE);
    DAQmxStartTask(taskHandleAI);
}

void NIDAQcommands::setupAnalogOutputs(char* channelNames)
{
    DAQmxCreateTask("",&taskHandleAO);
    DAQmxCreateAOVoltageChan(taskHandleAO,channelNames,"",VMIN,VMAX,DAQmx_Val_Volts,NULL);
    DAQmxStartTask(taskHandleAO);
}

void NIDAQcommands::setupDigitalInputs(char* channelNames)
{
    DAQmxCreateTask("",&taskHandleDI);
    DAQmxCreateDIChan(taskHandleDI,channelNames,"",DAQmx_Val_ChanForAllLines);
    DAQmxStartTask(taskHandleDI);
}

void NIDAQcommands::setupDigitalOutputs(char* channelNames)
{
    DAQmxCreateTask("",&taskHandleDO);
    DAQmxCreateDOChan(taskHandleDO,channelNames,"",DAQmx_Val_ChanForAllLines);
    DAQmxStartTask(taskHandleDO);
}
