#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <QObject>

#include <myo/myo.hpp>
#include <include/lsl_cpp.h>

#define COLUMN_SIZE (9)

class DataCollector : public QObject, public myo::DeviceListener {
    Q_OBJECT

public:
    DataCollector(QObject *parent = nullptr);

    virtual void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
    virtual void onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
    virtual void onDisconnect(myo::Myo *myo, uint64_t timestamp);
    virtual void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose);
    virtual void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& rotation);
    virtual void onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& accel);
    virtual void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg);

    // This is a utility function implemented for this sample that maps a myo::Myo* to a unique ID starting at 1.
    // It does so by looking for the Myo pointer in knownMyos, which onPair() adds each Myo into as it is paired.
    size_t identifyMyo(myo::Myo* myo);

    int howManyMyo();
    myo::Vector3<float> getAccelData() const;

    void createLSLStreams();

signals:
    void connectionLost();

private:
    // We store each Myo pointer that we pair with in this list, so that we can keep track of the order we've seen
    // each Myo and give it a unique short identifier (see onPair() and identifyMyo() above).
    std::vector<myo::Myo*> knownMyos;

    lsl::stream_outlet *outletEMG;
    lsl::stream_outlet *outletPose;
    lsl::stream_outlet *outletAccel;
    lsl::stream_outlet *outletOrient;
};

#endif // DATACOLLECTOR_H
