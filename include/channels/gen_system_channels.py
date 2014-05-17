import json

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
    flags = 0
    min = channel['min']
    max = channel['max']
    
    print '{"' + name + '", "' + units + '", ' + str(precision) + ', ' + str(flags) + ', ' + str(min) + ', ' + str(max) + '}',
    print ', \\' if i < channel_len - 1 else ' \\'
print '} \\'
print '}'
print '#endif'

