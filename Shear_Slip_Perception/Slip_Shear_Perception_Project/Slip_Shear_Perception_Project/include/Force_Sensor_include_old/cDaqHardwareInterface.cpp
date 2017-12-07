#include "cDaqHardwareInterface.h"
#include "NIDAQmx.h"



cDaqHardwareInterface::cDaqHardwareInterface()
{
    this->m_thDAQTask = new TaskHandle;
    SetConnectionMode( DAQmx_Val_Diff );
}


cDaqHardwareInterface::~cDaqHardwareInterface()
{
    /*stop and clear tasks, without regard to error*/
    DAQmxClearTask( *m_thDAQTask );
    /*delete unmanaged pointers*/
    delete m_thDAQTask;
}

int32n cDaqHardwareInterface::ConfigSingleSampleTask(double a_SampleRate, int a_AveragingSize, std::string a_DeviceName, int a_FirstChannel, int a_NumChannels, int a_MinVoltage, int a_MaxVoltage)
{
	
    int32n retVal;

    // set up member data
    m_f64SamplingFrequency = a_SampleRate;

    if (a_AveragingSize > 0)
    {
        m_uiAveragingSize = a_AveragingSize;
    }
    else
    {
        m_uiAveragingSize = 1;
    }

    m_sDeviceName = a_DeviceName;
    m_uiFirstChannel = a_FirstChannel;
    m_uiNumChannels = a_NumChannels;
    m_iMinVoltage = a_MinVoltage;
    m_iMaxVoltage = a_MaxVoltage;

    unsigned int numSamplesPerChannel = m_uiAveragingSize; // the number of samples per channel that daqmx is configured with

    if ( MIN_SAMPLES_PER_CHANNEL > numSamplesPerChannel )
    {
        numSamplesPerChannel = MIN_SAMPLES_PER_CHANNEL;
    }

    StopCollection(); // stop currently running task

    // if any function fails (returns non-zero), don't execute any more daqmx functions
    // create the daqmx task
    if( !( retVal = DAQmxCreateTask( "", m_thDAQTask ) ) )
    {
        // add the analog input channels to the task
        if( !( retVal = DAQmxCreateAIVoltageChan( *m_thDAQTask, m_sDeviceName.c_str(), "", m_iConnectionMode,
                                                  m_iMinVoltage, m_iMaxVoltage, DAQmx_Val_Volts, NULL ) ) )
        {
			printf("Sampling frequency = %f\n", m_f64SamplingFrequency);
            // set up timing for the task
            if( !( retVal = DAQmxCfgSampClkTiming( *m_thDAQTask, "", m_f64SamplingFrequency,
                                                   DAQmx_Val_Rising, DAQmx_Val_ContSamps, numSamplesPerChannel*a_NumChannels ) ) )
            {

				#ifdef CONTINUOUS_SAMPLING
				printf("Clocking is set\n");
				if( !( retVal = DAQmxRegisterEveryNSamplesEvent(*m_thDAQTask, DAQmx_Val_Acquired_Into_Buffer, numSamplesPerChannel, 0, EveryNCallback, data)))
				{
					printf("Callback DAQmxRegisterEveryNSamplesEvent registered\n");
					if( !( retVal = DAQmxRegisterDoneEvent(*m_thDAQTask, 0, DoneCallback, NULL)))
					{
						retVal = DAQmxStartTask( *m_thDAQTask );
						printf("Callback DAQmxRegisterDoneEvent registered. Starting task\n");
					}
				}
				
				#else
                // set read position relative to next sample to be read
                if( !( retVal = DAQmxSetReadRelativeTo( *m_thDAQTask, DAQmx_Val_MostRecentSamp ) ) )
                {
                    // offset of -1 from the next sample, meaning we read the most recent sample
                    if( !( retVal = DAQmxSetReadOffset( *m_thDAQTask, 0 ) ) )
                    {
                        // start the task
                        retVal = DAQmxStartTask( *m_thDAQTask );
                    }
                }
				#endif
            }
        }
    }
	return retVal;
}


int32n cDaqHardwareInterface::StopCollection()
{
    int32n retVal;
    retVal = DAQmxClearTask( *m_thDAQTask );
    return retVal;
}



int32n cDaqHardwareInterface::ReadSingleSample(double buffer[])
{
    int32n retVal;
    unsigned long numRawSamples = m_uiNumChannels * m_uiAveragingSize;  // the number of raw gauge value samples
    double * rawBuffer = new double[numRawSamples];                     // the buffer which holds the raw, unaveraged gauge values
    double timeOut = ( numRawSamples / m_f64SamplingFrequency ) + 1;    // allow a full second for Windows timing inaccuracies

    int32n read; // number samples read

	#ifdef CONTINUOUS_SAMPLING
		for (int i=0; i<numRawSamples ; i++)
		{
			rawBuffer[i] = data[i];
		}
	#else
		retVal = DAQmxReadAnalogF64( *m_thDAQTask, m_uiAveragingSize, timeOut, DAQmx_Val_GroupByScanNumber,
                                 rawBuffer, numRawSamples, &read, NULL );
	#endif

    for (unsigned int i = 0; i < m_uiNumChannels; i++ )
    {
        // sum the raw values, storing the sum in the first raw data point
        for (unsigned int j = 1; j < m_uiAveragingSize; j++ )
        {
            rawBuffer[i] += rawBuffer[ i + ( j * m_uiNumChannels ) ];
        }

        // store the average values in the output buffer
        buffer[i] = rawBuffer[i] / m_uiAveragingSize;
    }

    delete []rawBuffer;

    return 0;
}


std::string cDaqHardwareInterface::GetErrorInfo()
{
    char errorBuffer[2048];
    DAQmxGetExtendedErrorInfo( errorBuffer, 2048 );

    std::string temp_string(errorBuffer);
    m_sErrorInfo = temp_string;

    return m_sErrorInfo;
}

int cDaqHardwareInterface::GetMinVoltage()
{
    return m_iMinVoltage;
}

int cDaqHardwareInterface::GetMaxVoltage()
{
    return m_iMaxVoltage;
}

unsigned int cDaqHardwareInterface::GetFirstChannel()
{
    return m_uiFirstChannel;
}

std::string cDaqHardwareInterface::GetDeviceName()
{
    return m_sDeviceName;
}

unsigned int cDaqHardwareInterface::GetNumChannels()
{
    return m_uiNumChannels;
}

double cDaqHardwareInterface::GetSampleFrequency()
{
    return m_f64SamplingFrequency;
}

unsigned int cDaqHardwareInterface::GetAveragingSamples()
{
    return m_uiAveragingSize;
}

std::string cDaqHardwareInterface::GetErrorCodeDescription( long errCode )
{
    char errorBuffer [CHRACTER_SIZE];
    DAQmxGetErrorString( errCode, errorBuffer, CHRACTER_SIZE );

    std::string temp_string(errorBuffer);
    return temp_string;
}

int32n cDaqHardwareInterface::ConfigBufferTask( double a_SampleRate, int a_AveragingSize, std::string a_DeviceName, int a_FirstChannel,
        int a_NumChannels, int a_MinVoltage, int a_MaxVoltage, int a_BufferSize )
{
    int32n retVal;

    /*set up member data*/
    m_f64SamplingFrequency = a_SampleRate;

    if (a_AveragingSize > 0)
    {
        m_uiAveragingSize = a_AveragingSize;
    }
    else
    {
        m_uiAveragingSize = 1;
    }

    m_sDeviceName = a_DeviceName;
    m_uiFirstChannel = a_FirstChannel;
    m_uiNumChannels = a_NumChannels;
    m_iMinVoltage = a_MinVoltage;
    m_iMaxVoltage = a_MaxVoltage;
    m_ulBufferedSize = a_BufferSize;

    unsigned int numSamplesPerChannel = m_uiAveragingSize * m_ulBufferedSize; // the number of samples per channel tha daqmx is configured with

    // NI-DAQmx requires a minimum number of samples per channel
    if ( MIN_SAMPLES_PER_CHANNEL > numSamplesPerChannel )
    {
        numSamplesPerChannel = MIN_SAMPLES_PER_CHANNEL;
    }

    StopCollection(); // stop any currently running task

    // if any function fails (returns non-zero), don't execute any more daqmx functions
    // create the daqmx task
    if( !( retVal = DAQmxCreateTask( "", m_thDAQTask ) ) )
    {
        // add the analog input channels to the task
        if( !( retVal = DAQmxCreateAIVoltageChan( *m_thDAQTask, m_sDeviceName.c_str(), "", m_iConnectionMode,
                    m_iMinVoltage, m_iMaxVoltage, DAQmx_Val_Volts, NULL ) ) )
        {
            // set up timing for the task
            if( !( retVal = DAQmxCfgSampClkTiming( *m_thDAQTask, NULL, m_f64SamplingFrequency,
                        DAQmx_Val_Rising, DAQmx_Val_ContSamps, numSamplesPerChannel ) ) )
            {
                // start the task
                retVal = DAQmxStartTask( *m_thDAQTask );
            }
        }
    }

    return retVal;
}


int32n cDaqHardwareInterface::ReadBufferedSamples( int a_NumSamples, double a_Buffer[])
{
    int32n retVal;
    int32n sampsPerChannel = m_uiAveragingSize * a_NumSamples; // samples per channel

    unsigned int numRawSamples = sampsPerChannel * m_uiNumChannels; // the total number of individual gauge readings to be read

    double timeOut = ( sampsPerChannel / m_f64SamplingFrequency ) + 1; // timeout value. allows a full second of variance

    double* rawBuffer = new double[numRawSamples];

    int32n read; // number of samples read

    unsigned int rawSetSize = m_uiNumChannels * m_uiAveragingSize; // the number of raw data sets per one output set

    retVal = DAQmxReadAnalogF64( *m_thDAQTask, sampsPerChannel, timeOut, DAQmx_Val_GroupByScanNumber,
                                 rawBuffer, numRawSamples, &read, NULL );

    // precondition: rawBuffer has the raw samples from the DAQ hardware.  rawSetSize is the number of
    //	             data points in one output reading (one raw data point is a single reading of all 6 or 7 gauges).
    // postcondition: buffer has the output (averaged) data points.  the first data point in each raw 'set' has the
    //	              sum of all the readings in that set. i = numSamples, j = m_uiNumChannels, k = m_uiAveragingSize

    for (int i = 0; i < a_NumSamples; i++ )
    {
        unsigned int firstDataPointInSetIndex = i * rawSetSize; // the position of the first element
                                                                // in the first data point in the raw
                                                                // set we're currently averaging

        for (int j = 0; (unsigned int)j < m_uiNumChannels; j++ )
        {
            unsigned int sumIndex = firstDataPointInSetIndex + j; // the index where we're storing
                                                                  // the sum of the gauge we're averaging
                                                                  // the raw values for

            for (int k = 1; (unsigned int)k < m_uiAveragingSize; k++ )
            {
                // put the sum of this set into the first reading in the set*/
                rawBuffer[ sumIndex ] += rawBuffer[ sumIndex + ( k * m_uiNumChannels ) ];
            }

            // put the averages into the output buffer
            a_Buffer[ ( i * m_uiNumChannels ) + j ] = rawBuffer[ sumIndex ] / m_uiAveragingSize;
        }
    }

    delete [] rawBuffer; // keep up with those unmanaged pointers
    return retVal;
}

void cDaqHardwareInterface::SetConnectionMode( int DAQConnMode )
{
    m_iConnectionMode = DAQConnMode;
}

int cDaqHardwareInterface::GetConnectionMode( )
{
    return m_iConnectionMode;
}

long CVICALLBACK EveryNCallback(TaskHandle taskHandle1, long everyNsamplesEventType, unsigned long nSamples, void *callbackData)
{
	long       error=0;
	char       errBuff[2048]={'\0'};
	static int totalRead=0;
	long       read=0;
	double * data;

	data = (double *)callbackData;

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle1,nSamples,10.0,DAQmx_Val_GroupByScanNumber,data,nSamples*6,&read,NULL));
	if( read>0 ) {
		fflush(stdout);
	}

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle1);
		DAQmxClearTask(taskHandle1);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}


long CVICALLBACK DoneCallback(TaskHandle taskHandle, int32n status, void *callbackData)
{
	int32n   error=0;
	char    errBuff[2048]={'\0'};

	return 0;
}
