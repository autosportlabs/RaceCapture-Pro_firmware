import json


CHANNEL_SYSTEM_CHANNEL_FLAG    = 0

CHANNEL_TYPE_UNKNOWN           = 0
CHANNEL_TYPE_ANALOG            = 1
CHANNEL_TYPE_FREQ              = 2
CHANNEL_TYPE_GPIO              = 3
CHANNEL_TYPE_PWM               = 4
CHANNEL_TYPE_IMU               = 5
CHANNEL_TYPE_GPS               = 6
CHANNEL_TYPE_STATISTICS        = 7

channels = json.load(open('system_channels.json'))

sys_channels = channels['channels']

channel_len = len(sys_channels)

print '#ifndef SYSTEMCHANNELS_H_'
print '#define SYSTEMCHANNELS_H_'
print ''
print 'typedef enum{'


for i in range(channel_len):
    channel = sys_channels[i]
    name = channel['nm']
    print 'CHANNEL_' + name + ' = ' + str(i),
    print ',' if i < channel_len - 1 else ''
print '} ChannelIds;'

print ''
print '#define DEFAULT_CHANNEL_META { \\'
print str(channel_len) + ', \\'
print '{ \\'
for i in range(channel_len):
    channel = sys_channels[i]
    name = channel['nm']
    units = channel['ut']
    precision = channel['prec']
    sys = channel['sys']
    
    type = channel['type']
    if type == 'analog':
        channelType = CHANNEL_TYPE_ANALOG
    elif type == 'freq':
        channelType = CHANNEL_TYPE_FREQ
    elif type == 'gpio':
        channelType = CHANNEL_TYPE_GPIO
    elif type == 'pwm':
        channelType = CHANNEL_TYPE_PWM
    elif type == 'imu':
        channelType = CHANNEL_TYPE_IMU
    elif type == 'gps':
        channelType = CHANNEL_TYPE_GPS
    elif type == 'stat':
        channelType = CHANNEL_TYPE_STATISTICS
    else:
        channelType = CHANNEL_TYPE_UNKNOWN
    
    flags = 0
    if sys:
        flags = flags | (1 << CHANNEL_SYSTEM_CHANNEL_FLAG)
        
    flags = flags | (channelType << 1)
    
    min = channel['min']
    max = channel['max']
    
    print '{"' + name + '", "' + units + '", ' + str(precision) + ', ' + str(flags) + ', ' + str(min) + ', ' + str(max) + '}',
    print ', \\' if i < channel_len - 1 else ' \\'
print '} \\'
print '}'
print '#endif'

