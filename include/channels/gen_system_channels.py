import json

channels = json.load(open('system_channels.json'))

sys_channels = channels['system_channels']

channel_len = len(sys_channels)

print '#ifndef SYSTEMCHANNELS_H_'
print '#define SYSTEMCHANNELS_H_'
print ''
print 'typedef enum{'


for i in range(channel_len):
    channel = sys_channels[i]
    name = channel['name']
    print 'CHANNEL_' + name + ' = ' + str(i),
    print ',' if i < channel_len - 1 else ''
print '} ChannelIds;'

print ''
print '#define DEFAULT_CHANNEL_META { \\'
print 'CHANNEL_COUNT, \\'
print '{ \\'
for i in range(channel_len):
    channel = sys_channels[i]
    name = channel['name']
    units = channel['units']
    precision = channel['precision']
    print '{"' + name + '", "' + units + '", ' + str(precision) + '}',
    print ', \\' if i < channel_len - 1 else ' \\'
print '} \\'
print '}'
print '#endif'

