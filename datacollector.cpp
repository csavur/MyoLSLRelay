#include "datacollector.h"
#include <iostream>

using namespace std;

DataCollector::DataCollector(QObject *parent): QObject(parent)
{
    createLSLStreams();
}

void DataCollector::createLSLStreams()
{
    /// [0 EMG]
    const char *channels[] = {"ID","CH0","CH1","CH2","CH3","CH4","CH5","CH6","CH7"};
    const char *units[] = {"int","mV","mV","mV","mV","mV","mV","mV","mV"};
    const char *types[] = {"DeviceID","EMG","EMG","EMG","EMG","EMG","EMG","EMG","EMG"};
    // make a new EMG stream_info (200 Hz)
    lsl::stream_info info("MyoEMG", "EMG", 9, 200, lsl::cf_int16, "id123456");
    // add some description fields
    info.desc().append_child_value("ThamicLab", "MyoArmBand");
    lsl::xml_element chns = info.desc().append_child("channels");

    for (int i=0; i<9; i++) {
        chns.append_child("channel")
        .append_child_value("label", channels[i])
        .append_child_value("unit", units[i])
        .append_child_value("type", types[i]);
    }
    // make a new outlet
    outletEMG = new lsl::stream_outlet(info);
    /// [EMG]

    /// [POSE]
    lsl::stream_info infoPose("MyoPose", "Markers", 1, lsl::IRREGULAR_RATE, lsl::cf_string, "id23443");
    outletPose = new lsl::stream_outlet(infoPose);

    /// Accelerometer
    const char *channelsAc[] = {"ID","roll","pitch","yaw"};
    lsl::stream_info infoAccel("MyoAccel", "RAW", 4, 50, lsl::cf_float32, "id23444");
    infoAccel.desc().append_child_value("ThamicLab", "MyoArmBand");
    lsl::xml_element chnsAcel = infoAccel.desc().append_child("channels");
    for (int i=0; i<4; i++) {
        chnsAcel.append_child("channel")
        .append_child_value("label", channelsAc[i]);
    }
    outletAccel = new lsl::stream_outlet(infoAccel);

    /// Orientation
    const char *channelsOri[] = {"ID","x","y","z"};
    lsl::stream_info infoOri("MyoOrient", "RAW", 4, 50, lsl::cf_float32, "id23445");
    infoOri.desc().append_child_value("ThamicLab", "MyoArmBand");
    lsl::xml_element chnsOri = infoOri.desc().append_child("channels");
    for (int i=0; i<4; i++) {
        chnsOri.append_child("channel")
        .append_child_value("label", channelsOri[i]);
    }
    outletOrient = new lsl::stream_outlet(infoOri);
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
    emit connectionLost();
}

void DataCollector::onPose(myo::Myo *myo, uint64_t timestamp, myo::Pose pose)
{
    Q_UNUSED(myo);
    std::string marker;
    switch (pose.type()) {
    case myo::Pose::rest:
        marker = "rest";
        break;
    case myo::Pose::fist:
        marker = "fist";
        break;
    case myo::Pose::waveIn:
        marker = "wave_in";
        break;
    case myo::Pose::waveOut:
        marker = "wave_out";
        break;
    case myo::Pose::fingersSpread:
        marker = "finger_spread";
        break;
    case myo::Pose::doubleTap:
        marker = "double_tab";
        break;
    default:
        marker = "unknown_gesture";
        break;
    }
    outletPose->push_sample(&marker, timestamp);
}

void DataCollector::onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion<float> &rotation)
{
    //std::cout << "Orientation" << endl;
    std::vector<float> oriData;
    oriData.push_back((float)static_cast<int>(identifyMyo(myo)));

    using std::atan2;
    using std::asin;
    using std::sqrt;
    using std::max;
    using std::min;

    // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
    float roll = atan2(2.0f * (rotation.w() * rotation.x() + rotation.y() * rotation.z()),
     1.0f - 2.0f * (rotation.x() * rotation.x() + rotation.y() * rotation.y()));
    float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (rotation.w() * rotation.y() - rotation.z() * rotation.x()))));
    float yaw = atan2(2.0f * (rotation.w() * rotation.z() + rotation.x() * rotation.y()),
     1.0f - 2.0f * (rotation.y() * rotation.y() + rotation.z() * rotation.z()));

    oriData.push_back(roll);
    oriData.push_back(pitch);
    oriData.push_back(yaw);

    outletOrient->push_sample(oriData, timestamp);
}

void DataCollector::onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float> &accel)
{
    //std::cout << "Accelerometer" << endl;
    std::vector<float> accelData;
    accelData.push_back((float)static_cast<int>(identifyMyo(myo)));
    accelData.push_back(accel.x());
    accelData.push_back(accel.y());
    accelData.push_back(accel.z());
    outletAccel->push_sample(accelData, timestamp);
}

void DataCollector::onEmgData(myo::Myo *myo, uint64_t timestamp, const int8_t *emg)
{
    //std::cout << "EMG" << endl;
    std::vector<int> emgVal;
    emgVal.push_back(identifyMyo(myo));
    for (int i = 0; i < 8; ++i) {
        emgVal.push_back(emg[i]);
    }
    outletEMG->push_sample(emgVal, timestamp);
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

