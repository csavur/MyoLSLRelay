#include "datacollector.h"
#include <iostream>

using namespace std;

DataCollector::DataCollector(): emgSamples()
{
    emgSamples.fill(0);
    counter = 0;
}

void DataCollector::onPair(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{
    // Print out the MAC address of the armband we paired with.

    // The pointer address we get for a Myo is unique - in other words, it's safe to compare two Myo pointers to
    // see if they're referring to the same Myo.

    // Add the Myo pointer to our list of known Myo devices. This list is used to implement identifyMyo() below so
    // that we can give each Myo a nice short identifier.
    knownMyos.push_back(myo);

    myo->setStreamEmg(myo::Myo::streamEmgEnabled);

    // Now that we've added it to our list, get our short ID for it and print it out.
    std::cout << "Paired with " << identifyMyo(myo) << "." << std::endl;
}

void DataCollector::onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{
    std::cout << "Myo " << identifyMyo(myo) << " has connected." << std::endl;
}

void DataCollector::onDisconnect(myo::Myo* myo, uint64_t timestamp)
{
    std::cout << "Myo " << identifyMyo(myo) << " has disconnected." << std::endl;
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

//    std::cout << "Myo " << identifyMyo(myo) << " switched to pose " << pose.toString() << "." << std::endl;

    this->pose = pose;
    onPoseCallback(counter);
}

void DataCollector::onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion<float> &rotation)
{
    //cout << "x :" << rotation.x() << "y :" << rotation.y() << "z :" << rotation.z() << "w :" << rotation.w() << endl;

    this->rotation = rotation;
    onGYROCallback(identifyMyo(myo));
}

void DataCollector::onEmgData(myo::Myo *myo, uint64_t timestamp, const int8_t *emg)
{
    counter++;
    //std::cout << "Myo " << identifyMyo(myo) << " : " << endl;

    //! First column indicate device id
    emgSamples[0] = identifyMyo(myo);
    for (int i = 0; i < 8; i++) {
        emgSamples[i + 1] = emg[i];
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

size_t DataCollector::identifyMyo(myo::Myo *myo) {
    // Walk through the list of Myo devices that we've seen pairing events for.
    for (size_t i = 0; i < knownMyos.size(); ++i) {
        // If two Myo pointers compare equal, they refer to the same Myo device.
        if (knownMyos[i] == myo) {
            return i + 1;
        }
    }

    return 0;
}

int DataCollector::howManyMyo()
{
    return knownMyos.size();
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


