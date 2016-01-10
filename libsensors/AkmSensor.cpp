/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>

#include <utils/Log.h>
#include "AkmSensor.h"

#define LOGTAG "AkmSensor"

//#define ALOG_NDEBUG 0

/*****************************************************************************/

int (*akm_is_sensor_enabled)(uint32_t sensor_type);
int (*akm_enable_sensor)(uint32_t sensor_type);
int (*akm_disable_sensor)(uint32_t sensor_type);
int (*akm_set_delay)(uint32_t sensor_type, uint64_t delay);

int stub_is_sensor_enabled(uint32_t sensor_type) {
    return 0;
}

int stub_enable_disable_sensor(uint32_t sensor_type) {
    return -ENODEV;
}

int stub_set_delay(uint32_t sensor_type, uint64_t delay) {
    return -ENODEV;
}


AkmSensor::AkmSensor()
: SensorBase(NULL, NULL),
      mEnabled(0),
      mPendingMask(0),
      mInputReader(32)
{
    /* Open the library before opening the input device.  The library
     * creates a uinput device.
     */
    if (loadAKMLibrary() == 0) {
        data_name = "compass_sensor";
        data_fd = openInput("compass_sensor");
    }

    //Incase first time fails
    if(data_fd < 0){
         ALOGI("%s: retrying to open compass sensor", LOGTAG);
         data_fd = openInput("compass_sensor");
    }

    if(data_fd > 0){
         ALOGI("%s: compass sensor successfully opened: %i", LOGTAG, data_fd);
    }else{
         ALOGI("%s: failed to open compass sensor", LOGTAG);
    }

    memset(mPendingEvents, 0, sizeof(mPendingEvents));

    mPendingEvents[MagneticField].version = sizeof(sensors_event_t);
    mPendingEvents[MagneticField].sensor = ID_M;
    mPendingEvents[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvents[MagneticField].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;

    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags = 0;
    if (akm_is_sensor_enabled(SENSOR_TYPE_MAGNETIC_FIELD))  {
        mEnabled |= 1<<MagneticField;
        if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_X), &absinfo)) {
            mPendingEvents[MagneticField].magnetic.x = absinfo.value * CONVERT_M_X;
        }
        if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Y), &absinfo)) {
            mPendingEvents[MagneticField].magnetic.y = absinfo.value * CONVERT_M_Y;
        }
        if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Z), &absinfo)) {
            mPendingEvents[MagneticField].magnetic.z = absinfo.value * CONVERT_M_Z;
        }
    }
}

AkmSensor::~AkmSensor()
{
    if (mLibAKM) {
        unsigned ref = ::dlclose(mLibAKM);
    }
}

int AkmSensor::setInitialState()
{
    return 0;
}

int AkmSensor::enable(int32_t handle, int en)
{
    int what = -1;

    switch (handle) {
        case ID_M: what = MagneticField; break;
        case ID_O: what = Orientation;   break;
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    int newState  = en ? 1 : 0;
    int err = 0;

    if ((uint32_t(newState)<<what) != (mEnabled & (1<<what))) {

        uint32_t sensor_type;

        switch (what) {
            case MagneticField: sensor_type = SENSOR_TYPE_MAGNETIC_FIELD; break;
        }
        short flags = newState;
        if (en){
            err = akm_enable_sensor(sensor_type);
        }else{
            err = akm_disable_sensor(sensor_type);
        }

        err = sspEnable(LOGTAG, SSP_MAG, en);
        setInitialState();

        ALOGE_IF(err, "Could not change sensor state (%s)", strerror(-err));
        if (!err) {
            mEnabled &= ~(1<<what);
            mEnabled |= (uint32_t(flags)<<what);
        }
    }
    return err;
}

int AkmSensor::setDelay(int32_t handle, int64_t ns)
{
    int what = -1;
    int fd;
    uint32_t sensor_type = 0;

    if (ns < 0)
        return -EINVAL;

    switch (handle) {
        case ID_M: sensor_type = SENSOR_TYPE_MAGNETIC_FIELD; break;
    }

    if (sensor_type == 0)
        return -EINVAL;

    fd = open("/sys/class/sensors/ssp_sensor/mag_poll_delay", O_RDWR);
    if (fd >= 0) {
        char buf[80];
        sprintf(buf, "%lld", ns);
        write(fd, buf, strlen(buf)+1);
        close(fd);
     }

    fd = open("/sys/class/sensors/ssp_sensor/ori_poll_delay", O_RDWR);
    if (fd >= 0) {
        char buf[80];
        sprintf(buf, "%lld", ns);
        write(fd, buf, strlen(buf)+1);
        close(fd);
     }

    mDelays[what] = ns;
    return 0;
}

int AkmSensor::loadAKMLibrary()
{
    mLibAKM = dlopen("libakm.so", RTLD_NOW);

    if (!mLibAKM) {
        akm_is_sensor_enabled = stub_is_sensor_enabled;
        akm_enable_sensor = stub_enable_disable_sensor;
        akm_disable_sensor = stub_enable_disable_sensor;
        akm_set_delay = stub_set_delay;
        ALOGE("%s: unable to load AKM Library, %s", LOGTAG, dlerror());
        return -ENOENT;
    }

    *(void **)&akm_is_sensor_enabled = dlsym(mLibAKM, "akm_is_sensor_enabled");
    *(void **)&akm_enable_sensor = dlsym(mLibAKM, "akm_enable_sensor");
    *(void **)&akm_disable_sensor = dlsym(mLibAKM, "akm_disable_sensor");
    *(void **)&akm_set_delay = dlsym(mLibAKM, "akm_set_delay");

    return 0;
}

int AkmSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_REL) {
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_ABS) {
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_SYN) {
            int64_t time = timevalToNano(event->time);
            for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
                if (mPendingMask & (1<<j)) {
                    mPendingMask &= ~(1<<j);
                    mPendingEvents[j].timestamp = time;
                    if (mEnabled & (1<<j)) {
                        *data++ = mPendingEvents[j];
                        count--;
                        numEventReceived++;
                    }
                }
            }
            if (!mPendingMask) {
                mInputReader.next();
            }
        } else {
            ALOGE("%s: unknown event (type=%d, code=%d)", LOGTAG,
                    type, event->code);
            mInputReader.next();
        }
    }
    return numEventReceived;
}

void AkmSensor::processEvent(int code, int value)
{
    switch (code) {
        case EVENT_TYPE_MAGV_X:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.x = value * CONVERT_M_X;
            break;
        case EVENT_TYPE_MAGV_Y:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.y = value * CONVERT_M_Y;
            break;
        case EVENT_TYPE_MAGV_Z:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.z = value * CONVERT_M_Z;
            break;
    }
}
