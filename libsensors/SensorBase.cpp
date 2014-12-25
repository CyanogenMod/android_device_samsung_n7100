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

#include <utils/Log.h>

#include <linux/input.h>

#include "SensorBase.h"

/*****************************************************************************/

SensorBase::SensorBase(
        const char* dev_name,
        const char* data_name)
    : dev_name(dev_name), data_name(data_name),
      dev_fd(-1), data_fd(-1)
{
    if (data_name) {
        data_fd = openInput(data_name);
    }

    if (!data_fd)
    {
        ALOGE("open device %s failed", dev_name);
    }
}

SensorBase::~SensorBase() {
    if (data_fd >= 0) {
        close(data_fd);
    }
    if (dev_fd >= 0) {
        close(dev_fd);
    }
}

int SensorBase::open_device() {
    if (dev_fd<0 && dev_name) {
        dev_fd = open(dev_name, O_RDONLY);
        ALOGE_IF(dev_fd<0, "Couldn't open %s (%s)", dev_name, strerror(errno));
    }
    return 0;
}

int SensorBase::close_device() {
    if (dev_fd >= 0) {
        close(dev_fd);
        dev_fd = -1;
    }
    return 0;
}

int SensorBase::getFd() const {
    if (!data_name) {
        return dev_fd;
    }
    return data_fd;
}

int SensorBase::setDelay(int32_t handle, int64_t ns) {
    return 0;
}

bool SensorBase::hasPendingEvents() const {
    return false;
}

int64_t SensorBase::getTimestamp() {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}

int SensorBase::openInput(const char* inputName) {
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                strcpy(input_name, filename);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    ALOGE_IF(fd<0, "couldn't find '%s' input device", inputName);

    return fd;
}

int SensorBase::batch(int handle, int flags, int64_t period_ns, int64_t timeout)
{
    return 0;
}

int SensorBase::flush(int handle)
{
    return 0;
}

int SensorBase::sspEnable(const char* sensorname, int sensorvalue, int en)
{
    FILE* sspfile;
    int oldvalue = 0;
    int reset = 0;
    int newvalue;
    int fd;

    sspfile = fopen(SSP_DEVICE_ENABLE, "r");
    fscanf(sspfile, "%d", &oldvalue);
    fclose(sspfile);

    if(en) {
        newvalue = oldvalue | sensorvalue;
    } else {
        newvalue = oldvalue & (~sensorvalue);
    }
    ALOGI("%s: name: %s sensor: %i old value: %x  new value: %x ", __func__, sensorname, sensorvalue, oldvalue, newvalue);
    if (sspWrite(newvalue))
	return -1;
    else
        return 0;
}

int SensorBase::sspWrite(int sensorvalue)
{
    char buf[10];
    int fd, ret, err;

    sprintf(buf, "%d", sensorvalue);
    fd = open(SSP_DEVICE_ENABLE, O_RDWR);
    if (fd >= 0) {
        err = write(fd, buf, sizeof(buf));
	ret = 0;
    } else {
        ALOGI("%s: error writing to file", __func__);
	ret = -1;
    }
    
    close(fd);
    return ret;
}
