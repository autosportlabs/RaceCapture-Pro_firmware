#include "imu.h"
#include "imu_device.h"
#include "loggerConfig.h"
#include "filter.h"
#include "stddef.h"
#include "printk.h"

//Channel Filters
static Filter g_imu_filter[CONFIG_IMU_CHANNELS];

static void init_filters(LoggerConfig *loggerConfig)
{
    ImuConfig *config  = loggerConfig->ImuConfigs;
    for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        float alpha = (config + i)->filterAlpha;
        init_filter(&g_imu_filter[i], alpha);
    }
}

void imu_sample_all()
{
    for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        update_filter(&g_imu_filter[i], imu_read(i));
    }
}

float imu_read_value(unsigned char imuChannel, ImuConfig *ac)
{
    size_t physicalChannel = ac->physicalChannel;
    int raw = g_imu_filter[physicalChannel].current_value;
    int zeroValue = ac->zeroValue;
    float countsPerUnit = imu_device_counts_per_unit(imuChannel);
    float scaledValue = ((float)(raw - zeroValue) / countsPerUnit);

    //now invert based on configuration
    switch (ac->mode) {
    case MODE_IMU_NORMAL:
        break;
    case MODE_IMU_INVERTED:
        scaledValue = -scaledValue;
        break;
    case MODE_IMU_DISABLED:
    default:
        scaledValue = 0;
        break;
    }
    return scaledValue;
}

static void imu_flush_filter(size_t physicalChannel)
{
    for (size_t i = 0; i < 1000; i++) {
        update_filter(&g_imu_filter[physicalChannel], imu_read(physicalChannel));
    }
}

void imu_calibrate_zero()
{
    for (size_t logicalChannel = 0; logicalChannel < CONFIG_IMU_CHANNELS; logicalChannel++) {

        /* compass can't be calibrated in the conventional sense*/
    	if (logicalChannel == IMU_CHANNEL_COMPASS)
    		continue;

        ImuConfig * c = getImuConfigChannel(logicalChannel);
        size_t physicalChannel = c->physicalChannel;
        imu_flush_filter(physicalChannel);
        int zeroValue = g_imu_filter[physicalChannel].current_value;
        float countsPerUnit = imu_device_counts_per_unit(physicalChannel);
        if (logicalChannel == IMU_CHANNEL_Z) { //adjust for gravity
            if (c->mode == MODE_IMU_INVERTED) {
                countsPerUnit = -countsPerUnit;
            }
            zeroValue -= countsPerUnit;
        }
        c->zeroValue = zeroValue;
    }
}

int imu_init(LoggerConfig *loggerConfig)
{
    imu_device_init();
    init_filters(loggerConfig);
    return 1;
}

int imu_soft_init(LoggerConfig *loggerConfig)
{
    init_filters(loggerConfig);
    return 1;
}



int imu_read(unsigned int channel)
{
    int read = imu_device_read(channel);
    return read;
}

