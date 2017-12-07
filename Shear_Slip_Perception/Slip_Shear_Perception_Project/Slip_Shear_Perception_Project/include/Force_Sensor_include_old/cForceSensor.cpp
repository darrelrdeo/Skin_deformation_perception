#include "cForceSensor.h"

// Previously used as default, but won't work if using multiple DAQs
//#define FS_DEVICE_NAME "Dev1/ai0:5"

cForceSensor::cForceSensor(void)
{
    /*
    m_Force.set(0, 0, 0);
    */
}

cForceSensor::~cForceSensor(void)
{

}

// Function to acquire a sample of the force/torque data from the sensor
int cForceSensor::AcquireFTData()
{
    // Read in the force/torque data from the sensor
    static int Status;

    Status = FTSensor->ReadSingleFTRecord(m_FTData);


    if (Status != 0)
    {
        return Status;
    }
	else
	{
		return 0;
	}
}

// Function to get the force vector
void cForceSensor::GetForceReading(double* a_readBuffer)
{
    for (int i = 0; i < 3; i++)
	{
		a_readBuffer[i] = m_FTData[i];
	}
}

// Function to get the torque vector
void cForceSensor::GetTorqueReading(double* a_readBuffer)
{
    for (int i = 0; i < 3; i++)
	{
		a_readBuffer[i] = m_FTData[i+3];
	}
}

// Function that initialize the force sensor and the DAQ hardware
int cForceSensor::Initialize_Force_Sensor(std::string a_Device)
{
    FTSensor = new cATIForceSensor();

    m_Frequency = 10000;
    m_AveragingSize = 10;

    // Load the calibration file before start force sensor
    int loadCalFileStatus = FTSensor->LoadCalibrationFile(m_CalibrationFileLocation, 1);

    if (loadCalFileStatus != 0)
    {
        // Fail to load calibration file
		printf("Fail to load calibration file!\n");
        return -1;
    }

	int startStatus = FTSensor->StartSingleSampleAcquisition(a_Device, m_Frequency, m_AveragingSize, 0, false);
    // int startStatus = FTSensor->StartSingleSampleAcquisition(FS_DEVICE_NAME, m_Frequency, m_AveragingSize, 0, false);

    if (startStatus != 0)
    {
        // Start acquisition has failed
		printf("Fail to start acquisition!\n");
        return -2;
    }

    // Setting SI units
    int setForceUnitsStatus = FTSensor->SetForceUnits("N");
    if (setForceUnitsStatus != 0)
    {
        // Set force unit has failed
		printf("Fail to set force unit!\n");
        return -3;
    }

    int setTorqueUnitsStatus = FTSensor->SetTorqueUnits("Nm");
    if (setTorqueUnitsStatus != 0)
    {
        // Set torque unit has failed
		printf("Fail to set torque unit!\n");
        return -4;
    }

    return 0;
}

// Function to stop the force sensor acquisition of data
int cForceSensor::Stop_Force_Sensor()
{
    return FTSensor->StopAcquisition();

    return 0;
}



// Function to zero the bias of the force sensor
int cForceSensor::Zero_Force_Sensor(void)
{
    int returnValue = FTSensor->BiasCurrentLoad();

    return returnValue;

}

// Function to set the filename of the calibration file
void cForceSensor::Set_Calibration_File_Loc(std::string a_Location)
{
    m_CalibrationFileLocation = a_Location;
}

// Function to get the unit for the force vector
std::string cForceSensor::GetForceUnit(void)
{
    return FTSensor->GetForceUnits();
}

// Function to get the unit for the torque vector
std::string cForceSensor::GetTorqueUnit(void)
{
    return FTSensor->GetTorqueUnits();
}
