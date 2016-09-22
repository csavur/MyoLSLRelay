#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H


// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.

// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <myo/myo.hpp>

#define COLUMN_SIZE (9)


//------------------------------------------------------------------------
// Callback function pointer.
typedef void (*CallbackFunctionPtr)(void*, int);

class DataCollector : public myo::DeviceListener {
public:
    DataCollector();

    virtual void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
    virtual void onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
    virtual void onDisconnect(myo::Myo *myo, uint64_t timestamp);
    virtual void onUnpair(myo::Myo* myo, uint64_t timestamp);
    virtual void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose);
    virtual void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& rotation);
    virtual void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg);

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print();

    // The values of this array is set by onEmgData() above.
    std::array<int8_t, COLUMN_SIZE> emgSamples;
    myo::Quaternion<float> rotation;
    myo::Pose pose;

    std::vector<int> getData();

    void resetCounter();

    // Clients can connect their callback with this.  They can provide
    // an extra pointer value which will be included when they are called.
    void registerEMGCallback(CallbackFunctionPtr cb, void *p);
    void registerGYROCallback(CallbackFunctionPtr cb, void *p);
    void registerPoseCallback(CallbackFunctionPtr cb, void *p);

    // Test the callback to make sure it works.
    void onEMGCallback(int index);
    void onGYROCallback(int index);
    void onPoseCallback(int index);


    myo::Quaternion<float> getRotation() const;
    void setRotation(const myo::Quaternion<float> &value);

    myo::Pose getPose() const;
    void setPose(const myo::Pose &value);


    // This is a utility function implemented for this sample that maps a myo::Myo* to a unique ID starting at 1.
    // It does so by looking for the Myo pointer in knownMyos, which onPair() adds each Myo into as it is paired.
    size_t identifyMyo(myo::Myo* myo);

    int howManyMyo();

private:
    int counter;

    // The callback provided by the client via connectCallback().
    CallbackFunctionPtr m_EMGcb;
    // The additional pointer they provided (it's "this").
    void *m_EMGp;

    // The callback provided by the client via connectCallback().
    CallbackFunctionPtr m_GYROcb;
    // The additional pointer they provided (it's "this").
    void *m_GYROp;

    // The callback provided by the client via connectCallback().
    CallbackFunctionPtr m_Posecb;
    // The additional pointer they provided (it's "this").
    void *m_Posep;

    // We store each Myo pointer that we pair with in this list, so that we can keep track of the order we've seen
    // each Myo and give it a unique short identifier (see onPair() and identifyMyo() above).
    std::vector<myo::Myo*> knownMyos;
};

#endif // DATACOLLECTOR_H
