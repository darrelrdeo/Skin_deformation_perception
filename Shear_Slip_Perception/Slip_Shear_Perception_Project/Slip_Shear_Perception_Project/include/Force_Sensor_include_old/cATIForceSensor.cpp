#include "cATIForceSensor.h"

#define FIRST_TORQUE_INDEX 3            // the index of the first torque reading in
                                        // the standard output list order (fx, fy, fz, tx, ty, tz)
#define NUM_FT_AXES 6                   // the number of force/torque axes
#define NUM_STRAIN_GAUGES 6             // the number of strain gauges
#define NUM_MATRIX_ELEMENTS 36          // the total number of elements in a calibration matrix
#define GAUGE_SATURATION_LEVEL 0.995    // gauges are considered saturated when they reach 99.5% of their maximum load

using namespace DAQFTCLIBRARY;

cATIForceSensor::cATIForceSensor() :
    m_hiHardware( new cDaqHardwareInterface ), m_Calibration ( NULL ),
    m_iMaxVoltage( 10 ), m_iMinVoltage( -10 )
{
    m_dUpperSaturationVoltage = m_iMaxVoltage * GAUGE_SATURATION_LEVEL;
    m_dLowerSaturationVoltage = m_iMinVoltage * GAUGE_SATURATION_LEVEL;
}


cATIForceSensor::~cATIForceSensor()
{
    destroyCalibration( m_Calibration );
}


int cATIForceSensor::ReadSingleGaugePoint( double a_GaugeValues[] )
{
    if ( m_hiHardware->ReadSingleSample( a_GaugeValues ) )
    {
        m_sErrorInfo = m_hiHardware->GetErrorInfo();
        return -1;
    }

    // precondition: gaugeValues has the most recent gauge readings
    // postcondition: will have returned 2 if any gauge is saturated. otherwise, i = number of gauges
    for(unsigned int i = 0; i < m_hiHardware->GetNumChannels(); i++ )
    {
        if ( ( a_GaugeValues[i] > m_dUpperSaturationVoltage ) || ( a_GaugeValues[i] < m_dLowerSaturationVoltage ) )
        {
            m_sErrorInfo = std::string("Gauge Saturation");
            return 2;
        }
    }
    return 0;
}


int cATIForceSensor::StartSingleSampleAcquisition( std::string a_DeviceName,
                                                              double a_SampleFrequency,
                                                              int a_AveragingSize,
                                                              int a_FirstChannel,
                                                              bool a_UseTempComp )
{
    int numChannels;

    if (a_UseTempComp == 1)
    {
        numChannels = NUM_STRAIN_GAUGES + 1;
    }
    else
    {
        numChannels = NUM_STRAIN_GAUGES;
    }

    int status; // status of starting the hardware acquisition

    status = m_hiHardware->ConfigSingleSampleTask( a_SampleFrequency,
                                                   a_AveragingSize,
                                                   a_DeviceName,
                                                   a_FirstChannel,
                                                   numChannels,
                                                   m_iMinVoltage,
                                                   m_iMaxVoltage );
    if ( status )
    {
        if ( status < 0 ) // hardware error
        {
            m_sErrorInfo = m_hiHardware->GetErrorInfo();
        }
        else // hardware warning
        {
            m_sErrorInfo = m_hiHardware->GetErrorCodeDescription( status );
        }

        return -1;
    }

    // set calibration's use of temp comp
    if ( NULL != m_Calibration )
    {
        m_Calibration->cfg.TempCompEnabled = a_UseTempComp;
    }

    return 0;
}


int cATIForceSensor::StartBufferedAcquisition( std::string a_DeviceName, double a_SampleFrequency, int a_AveragingSize,
                                                      int a_FirstChannel, bool a_UseTempComp, int a_BufferSize )
{
    int numChannels;

    if (a_UseTempComp == 1)
    {
        numChannels = NUM_STRAIN_GAUGES + 1;
    }
    else
    {
        numChannels = NUM_STRAIN_GAUGES;
    }

    int32n status = m_hiHardware->ConfigBufferTask( a_SampleFrequency, a_AveragingSize, a_DeviceName, a_FirstChannel, numChannels,
                                                   m_iMinVoltage, m_iMaxVoltage, a_BufferSize );
    if ( status )
    {
        m_sErrorInfo = m_hiHardware->GetErrorCodeDescription( status );
        return -1;
    }

    // set calibration's use of temp comp
    if ( NULL != m_Calibration )
    {
        m_Calibration->cfg.TempCompEnabled = a_UseTempComp;
    }

    return 0;
}

int cATIForceSensor::StopAcquisition()
{
    if (m_hiHardware->StopCollection())
    {
        return -1;
    }

    return 0;
}

std::string cATIForceSensor::GetErrorInfo()
{
    return m_sErrorInfo;
}


std::string cATIForceSensor::GetDeviceName()
{
    return m_hiHardware->GetDeviceName();
}

double cATIForceSensor::GetSampleFrequency()
{
    return m_hiHardware->GetSampleFrequency();
}

int cATIForceSensor::GetAveragingSize()
{
    return m_hiHardware->GetAveragingSamples();
}

int cATIForceSensor::GetFirstChannel()
{
    return m_hiHardware->GetFirstChannel();
}

int cATIForceSensor::LoadCalibrationFile( std::string a_CalFile, int a_CalibrationIndex )
{
    // Unload any currently loaded calibration
    if( NULL != m_Calibration )
    {
        destroyCalibration(m_Calibration);
    }


    m_Calibration = createCalibration( (char*)a_CalFile.c_str(), a_CalibrationIndex );

    if ( NULL == m_Calibration )
    {
        m_sErrorInfo = std::string( "Could not load calibration file successfully" );
        return -1;
    }

    // determine max and min voltages and saturation levels
    m_iMinVoltage = 0;
    m_iMaxVoltage = m_Calibration->VoltageRange;

    if ( m_Calibration->BiPolar )
    {
        m_iMinVoltage -= ( m_Calibration->VoltageRange / 2 );
        m_dLowerSaturationVoltage = m_iMinVoltage * GAUGE_SATURATION_LEVEL;
        m_iMaxVoltage -= ( m_Calibration->VoltageRange / 2 );
        m_dUpperSaturationVoltage = m_iMaxVoltage * GAUGE_SATURATION_LEVEL;
    }

    return 0;
}

std::string cATIForceSensor::GetSerialNumber()
{
    if ( NULL == m_Calibration )
    {
        return std::string("");
    }
    return std::string(m_Calibration->Serial);
}

std::string cATIForceSensor::GetCalibrationDate()
{
    if ( NULL == m_Calibration )
    {
        return std::string( "" );
    }
    return std::string( m_Calibration->CalDate );
}

double cATIForceSensor::GetMaxLoad( int a_AxisIndex )
{
    if ( NULL == m_Calibration )
    {
        return 0;
    }
    float retVal;

    // find the maximum load in the current output units, not necessarily the same as the calibration units
    retVal = m_Calibration->MaxLoads[a_AxisIndex]; // this is the max load in the calibration units

    if ( a_AxisIndex < FIRST_TORQUE_INDEX ) // this is a force axis, convert to output force units
    {
        retVal *= ForceConv( m_Calibration->cfg.ForceUnits ) / ForceConv( m_Calibration->ForceUnits );
    } else // this is a torque axis, convert to output torque units
    {
        retVal *= TorqueConv( m_Calibration->cfg.TorqueUnits ) / TorqueConv( m_Calibration->TorqueUnits );
    }

    return retVal;
}


int cATIForceSensor::SetForceUnits(std::string a_ForceUnits )
{
    return DAQFTCLIBRARY::SetForceUnits( m_Calibration, (char*)a_ForceUnits.c_str() );
}

std::string cATIForceSensor::GetForceUnits()
{
    if ( NULL == m_Calibration )
        return std::string("");
    return std::string( m_Calibration->cfg.ForceUnits );
}

int cATIForceSensor::SetTorqueUnits(std::string a_TorqueUnits )
{
    return DAQFTCLIBRARY::SetTorqueUnits( m_Calibration, (char*)a_TorqueUnits.c_str() );
}

std::string cATIForceSensor::GetTorqueUnits()
{
    if ( NULL == m_Calibration )
        return std::string( "" );
    return std::string( m_Calibration->cfg.TorqueUnits );
}


bool cATIForceSensor::GetTempCompAvailable()
{
    if ( NULL == m_Calibration )
    {
        return false;
    }

    return ( 0 != m_Calibration->TempCompAvailable );
}

bool cATIForceSensor::GetTempCompEnabled()
{
    if ( NULL == m_Calibration )
    {
        return false;
    }

    return ( 0 != m_Calibration->cfg.TempCompEnabled );
}

int cATIForceSensor::ToolTransform(double a_TransformVector[], std::string a_DistanceUnits, std::string a_AngleUnits )
{
    float tempTransforms[6]; // unmanaged array of transformation values

    // precondition: transformVector has the transformation values
    // postcondition: tempTransforms has a copy of transformVector, i = NUM_FT_AXES

    for (int i = 0; i < NUM_FT_AXES; i++ )
    {
        tempTransforms[i] = (float)a_TransformVector[i];
    }

    return DAQFTCLIBRARY::SetToolTransform( m_Calibration, tempTransforms, (char*)a_DistanceUnits.c_str(), (char*)a_AngleUnits.c_str());
}


int cATIForceSensor::GetTransformVector(double a_TransformVector[])
{
    if ( NULL == m_Calibration )
    {
        return 1; // calibration not initialized
    }

    int i; // generic loop/array index

    for ( i = 0; i < NUM_FT_AXES; i++ )
    {
        a_TransformVector[i] = m_Calibration->cfg.UserTransform.TT[i];
    }
    return 0;
}

std::string cATIForceSensor::GetTransformDistanceUnits()
{
    if ( NULL == m_Calibration )
    {
        return std::string( "" );
    }
    return std::string( m_Calibration->cfg.UserTransform.DistUnits );
}

std::string cATIForceSensor::GetTransformAngleUnits()
{
    if ( NULL == m_Calibration )
    {
        return std::string( "" );
    }
    return std::string( m_Calibration->cfg.UserTransform.AngleUnits );
}

std::string cATIForceSensor::GetBodyStyle()
{
    if ( NULL == m_Calibration )
    {
        return std::string("");
    }
    return std::string( m_Calibration->BodyStyle );
}

std::string cATIForceSensor::GetCalibrationType()
{
    if ( NULL == m_Calibration )
    {
        return std::string("");
    }

    return std::string( m_Calibration->PartNumber );
}

int cATIForceSensor::GetWorkingMatrix(double a_Matrix[6][6])
{
    if ( NULL == m_Calibration )
    {
        return 1;
    }

    for (int i = 0; i < NUM_FT_AXES; i++ )
    {
        for(int j = 0; j < NUM_STRAIN_GAUGES; j++ )
        {
            a_Matrix[i][j] = m_Calibration->rt.working_matrix[i][j];
        }
    }

    return 0;
}

int cATIForceSensor::ReadSingleFTRecord(double a_Readings[] )
{

    if ( NULL == m_Calibration )
    {
		printf("Error 1Calibration: cATIForceSensor\n");
        return 1;
    }

    int retVal = 0;

    double *gcVoltages = new double[NUM_STRAIN_GAUGES + 1]; // allow an extra reading for the thermistor
    float nogcVoltages[NUM_STRAIN_GAUGES + 1];
    float tempResult [NUM_FT_AXES];
    retVal = ReadSingleGaugePoint( gcVoltages );

	
    if ( retVal ) // check for error when reading from hardware
    {
        if ( 2 == retVal )
        {
			printf("Error 2: cATIForceSensor");
            return 2; // saturation
			
        }
		printf("Error 1: cATIForceSensor");
        return 1;  // other error
    }


    // precondition: gcVoltages has the voltages from the DAQ card
    // postcondition: nogcVoltages has a copy of the data in gcVoltages, i = NUM_STRAIN_GAUGES + 1

    for (int i = 0; i <= NUM_STRAIN_GAUGES; i++ )
    {
        nogcVoltages[i] = (float)gcVoltages[i];
    }

    DAQFTCLIBRARY::ConvertToFT( m_Calibration, nogcVoltages, tempResult );


    // precondition: tempResult has f/t values
    // postcondition: readings has copy of f/t values, i = NUM_FT_AXES

    for (int i = 0; i < NUM_FT_AXES; i++ )
    {
        a_Readings[i] = tempResult[i];
    }

	delete gcVoltages;

    return retVal;
}


int cATIForceSensor::ReadBufferedFTRecords( int a_NumRecords, double a_Readings[] )
{
    if ( NULL == m_Calibration ) // invalid calibration
    {
        return 1;
    }

    long status; // the status of hardware reads

    int numGauges;

    if (GetTempCompEnabled())
    {
        numGauges = NUM_STRAIN_GAUGES + 1;
    }
    else
    {
        numGauges = NUM_STRAIN_GAUGES;
    }

    unsigned int numGaugeValues = a_NumRecords * numGauges;   // the number of individual gauge readings
    double *gaugeValues = new double[numGaugeValues];       // the gauge readings which are fed to the c library

    float currentGaugeReadings[ NUM_STRAIN_GAUGES + 1 ]; // current gauge reading
    float currentFTValues[ NUM_FT_AXES ]; // current f/t reading

    status = m_hiHardware->ReadBufferedSamples( a_NumRecords, gaugeValues );

    if ( status )
    {
        m_sErrorInfo = m_hiHardware->GetErrorCodeDescription( status );
        return (int) status;
    }

    // precondition: gaugeValues has the buffered gauge readings, numGauges has the
    //               number of active gauges (6 or 7)
    // postcondition: currentGaugeReadings contains the last gauge reading,
    //                currentFTValues contains the last ft value, readings contains all ft values,
    //                i = numRecords, j = NUM_FT_AXES, or the function will have already returned due
    //                to gauge saturation.

    for (int i = 0; i < a_NumRecords; i++ )
    {

        // precondition: i is the number of the f/t record we are currently calculating.
        // postcondition: currentGaugeReadings contains the i'th gauge readings,
        //                j = numGauges, or the function will have returned due to gauge saturation
        for (int j = 0; j < numGauges; j++ )
        {
            currentGaugeReadings[j] = (float)gaugeValues[ j + ( i * numGauges ) ];
            // check for saturation
            if ( m_dUpperSaturationVoltage < currentGaugeReadings[j] )
            {
                return 2;
            }
            if ( m_dLowerSaturationVoltage > currentGaugeReadings[j] )
            {
                return 2;
            }
        }

        DAQFTCLIBRARY::ConvertToFT( m_Calibration, currentGaugeReadings, currentFTValues );

        // precondition: currentFTValues has the i'th ft reading from the buffer
        // postcondition: j = NUM_FT_AXES, the i'th f/t record in readings contains the values
        //                from currentFTValues

        for (int j = 0; j < NUM_FT_AXES; j++ )
        {
            a_Readings[ j + ( i * NUM_FT_AXES ) ] = currentFTValues[j];
        }
    }

    return 0;
}


int cATIForceSensor::ReadBufferedGaugeRecords(int a_NumRecords, double a_Readings[])
{
    long status; // The status of hardware reads

    int numGauges;

    // The number of gauges in the scan list
    if (GetTempCompEnabled())
    {
        numGauges = NUM_STRAIN_GAUGES + 1;
    }
    else
    {
        numGauges = NUM_STRAIN_GAUGES;
    }

    status = m_hiHardware->ReadBufferedSamples( a_NumRecords, a_Readings );
    if ( status )
    {
        m_sErrorInfo = m_hiHardware->GetErrorCodeDescription( status );
        return (int)status;
    }
    if ( CheckForGaugeSaturation( a_Readings ) )
    {
        status = 2;
    }

    return status;
}

int cATIForceSensor::BiasKnownLoad(double a_BiasVoltages[])
{
    if ( NULL == m_Calibration )
    {
        return 1;
    }

    float nogcVoltages[NUM_STRAIN_GAUGES];

    // precondition: biasVoltages has the known bias voltages
    // postcondition: nogcVoltages is a copy of biasVoltages, i = NUM_STRAIN_GAUGES

    for (int i = 0; i < NUM_STRAIN_GAUGES; i++ )
    {
        nogcVoltages[i] = (float)a_BiasVoltages[i];
    }

    DAQFTCLIBRARY::Bias( m_Calibration, nogcVoltages );
    return 0;
}

int cATIForceSensor::BiasCurrentLoad()
{
    if ( NULL == m_Calibration )
    {
        return 1; // calibration not initialized
    }
    double *curVoltages = new double[NUM_STRAIN_GAUGES + 1]; // the current strain gauge load
    float nogcVoltages[NUM_STRAIN_GAUGES]; // voltages that can be passsed to unmanaged c library code
    int retVal;
    retVal = ReadSingleGaugePoint( curVoltages );
    if ( retVal )
    {
        return retVal; // error reading from hardware
    }

    // precondition: curVoltages has current reading from hardware
    // postcondition: nogcVoltages has a copy of the reading

    for (int i = 0; i < NUM_STRAIN_GAUGES; i++ )
    {
        nogcVoltages[i] = (float)curVoltages[i];
    }
    DAQFTCLIBRARY::Bias( m_Calibration, nogcVoltages );
    return retVal;
}

int cATIForceSensor::GetMaxVoltage()
{
    return m_iMaxVoltage;
}

int cATIForceSensor::GetMinVoltage()
{
    return m_iMinVoltage;
}

int cATIForceSensor::GetBiasVector( double a_BiasVector[] )
{
    if ( NULL == m_Calibration )
    {
        return 1;
    }


    // precondition: m_Calibration is a valid calibration
    // postcondition: i = NUM_STRAIN_GAUGES, biasVector contains a copy of the calibration's bias vector

    for (int i = 0; i < NUM_STRAIN_GAUGES; i++ )
    {
        a_BiasVector[i] = m_Calibration->rt.bias_vector[i];
    }

    return 0;
}

bool cATIForceSensor::CheckForGaugeSaturation(double readings[])
{
    int i; // Index into gauge readings.
    // Precondition: readings has the gauge readings.
    // Postcondition: function has returned if any gauge reading is saturated.

    int length = sizeof(readings)/sizeof(double)
;
    for( i = 0; i < length; i++ )
    {
        if ( ( m_dUpperSaturationVoltage < readings[i] ) ||
             ( m_dLowerSaturationVoltage > readings[i] ) )
        {
            return true;
        }
    }
    return false;
}

double cATIForceSensor::GetThermistorValue()
{
    if ( NULL == m_Calibration )
    {
        return 0;
    }

    return m_Calibration->rt.thermistor;
}

double cATIForceSensor::GetBiasSlope( int index )
{
    if ( NULL == m_Calibration )
    {
        return 0;
    }

    return m_Calibration->rt.bias_slopes[index];
}

double cATIForceSensor::GetGainSlope( int index )
{
    if ( NULL == m_Calibration )
    {
        return 0;
    }

    return m_Calibration->rt.gain_slopes[index];
}

void cATIForceSensor::SetConnectionMode( ConnectionType connType )
{
    m_hiHardware->SetConnectionMode( connType );
}


ConnectionType cATIForceSensor::GetConnectionMode( )
{
    return (ConnectionType)m_hiHardware->GetConnectionMode();
}


bool cATIForceSensor::GetHardwareTempComp( )
{
    if ( m_Calibration->HWTempComp )
    {
        return true;
    }

    return false;
}
