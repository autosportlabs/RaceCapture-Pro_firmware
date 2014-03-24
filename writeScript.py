import serial
import io

def decodeScript(s):
    s = s[9:]
    s = s[:-4]
    return s.replace('\\n','\n').replace('\_',' ').replace('\\r','\r').replace('\\"','"')

def encodeScript(s):
    return s.replace('\n','\\n').replace(' ', '\_').replace('\r', '\\r').replace('"', '\\"')

def writeScriptPage(ser, script, page):
    cmd = 'writeScriptPage ' + str(page) + ' ' + encodeScript(script) + '\r'
    print(cmd)
    ser.write(cmd)
    line = ser.readline()
    line = ser.readline()
    print(line)
    if 'result="ok"' in line:
        return 1
    else:
        return 0

def readScriptPage(ser, page):
    cmd = 'readScriptPage ' + str(page) + '\r'
    print("page: " + cmd)
    ser.write(cmd)
    line = ser.readline()
    line = ser.readline()
    print(line)
    line = decodeScript(line)
    return line


def readScript(ser):
    i = 0
    lua = ''
    while True:
        script = readScriptPage(ser, i)
        if script:
            lua = lua + script
            i+=1
        else:
            break
    return lua

def writeScript(ser, script):
    i = 0
    res = 0
    while True:
        if len(script) >= 256:
            scr = script[:255]
            script = script[255:]
            res = writeScriptPage(ser, scr, i)
            if res == 0:
                print 'Error: ' + str(i)
                break
            i = i + 1
        else:
            print('fooo')
            writeScriptPage(ser, script, i)
            print('xxxx')
            writeScriptPage(ser, '', i + 1)
            res = 1
            break
    return res

data = open("script.lua").read()

ser = serial.Serial('/dev/ttyACM0', timeout = 1000)

res = writeScript(ser, data)
print(str(res))

lua = readScript(ser)
print('#' + str(lua) + '#\r\n')

outFile = open("script_out.lua","w")
outFile.write(lua)
outFile.close()

ser.close()


