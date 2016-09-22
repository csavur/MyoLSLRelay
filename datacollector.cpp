#include "datacollector.h"
#include <iostream>

using namespace std;

DataCollector::DataCollector(): emgSamples()
{
    emgSamples.fill(0);
    counter = 0;
}

void DataCollector::onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{

    cout << "Firmware version : " << firmwareVersion.firmwareVersionMajor << "." << firmwareVersion.firmwareVersionMinor << "." << firmwareVersion.firmwareVersionPatch << endl;
    DeviceListener::onConnect(myo, timestamp, firmwareVersion);
}

void DataCollector::onUnpair(myo::Myo *myo, uint64_t timestamp)
{
    // We've lost a Myo.
    // Let's clean up some leftover state.
    emgSamples.fill(0);
}

void DataCollector::onPose(myo::Myo *myo, uint64_t timestamp, myo::Pose pose)
{
//    //cout << "onPose" << endl;
//    switch (pose.type()) {
//    case myo::Pose::rest:
//        cout << "rest" << endl;
//        break;
//    case myo::Pose::fist:
//        cout << "rest" << endl;
//        break;
//    case myo::Pose::waveIn:
//        cout << "wave in" << endl;
//        break;
//    case myo::Pose::waveOut:
//        cout << "wave out" << endl;
//        break;
//    case myo::Pose::fingersSpread:
//        cout << "finger Spread" << endl;
//        break;
//    default:
//        cout << "Unknown gesture" << endl;
//        break;
//    }

    this->pose = pose;
    onPoseCallback(counter);
}

void DataCollector::onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion<float> &rotation)
{
    //cout << "x :" << rotation.x() << "y :" << rotation.y() << "z :" << rotation.z() << "w :" << rotation.w() << endl;
    this->rotation = rotation;
    onGYROCallback(counter);
}

void DataCollector::onEmgData(myo::Myo *myo, uint64_t timestamp, const int8_t *emg)
{
    counter++;
    //cout << "time stamp : " << timestamp << endl;
    for (int i = 0; i < 8; i++) {
        emgSamples[i] = emg[i];
    }

    // there is a new reading
    onEMGCallback(counter);
}

// Clients can connect their callback with this.  They can provide
// an extra pointer value which will be included when they are called.
void DataCollector::registerEMGCallback(CallbackFunctionPtr cb, void *p)
{
    m_EMGcb = cb;
    m_EMGp = p;
}

void DataCollector::registerGYROCallback(CallbackFunctionPtr cb, void *p)
{
    m_GYROcb = cb;
    m_GYROp = p;
}

void DataCollector::registerPoseCallback(CallbackFunctionPtr cb, void *p)
{
    m_Posecb = cb;
    m_Posep = p;
}

void DataCollector::onEMGCallback(int index)
{
    m_EMGcb(m_EMGp, index);
}

void DataCollector::onGYROCallback(int index)
{
    m_GYROcb(m_GYROp, index);
}

void DataCollector::onPoseCallback(int index)
{
    m_Posecb(m_Posep, index);
}

void DataCollector::print()
{
    // Clear the current line
    std::cout << '\r';

    // Print out the EMG data.
    for (size_t i = 0; i < emgSamples.size(); i++) {

        std::ostringstream oss;
        oss << static_cast<int>(emgSamples[i]);
        std::string emgString = oss.str();

        //std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
        std::cout << emgString << std::string(4 - emgString.size(), ' ');
        std::cout << ",";
    }

    std::cout << counter;
    std::cout << std::flush;
}
myo::Pose DataCollector::getPose() const
{
    return pose;
}

void DataCollector::setPose(const myo::Pose &value)
{
    pose = value;
}

myo::Quaternion<float> DataCollector::getRotation() const
{
    return rotation;
}

void DataCollector::setRotation(const myo::Quaternion<float> &value)
{
    rotation = value;
}


std::vector<int> DataCollector::getData()
{
    vector<int> rawData;

    // Print out the EMG data.
    for (size_t i = 0; i < emgSamples.size(); i++) {
        rawData.push_back(static_cast<int>(emgSamples[i]));
    }

    return rawData;
}

void DataCollector::resetCounter()
{
    counter = 0;
}


