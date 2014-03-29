import serial
import io

class RcpSerial:

    def __init__(self, **kwargs):
        super(RcpSerial, self).__init__(**kwargs)
        self.ser = None

    def getSerial():
        if not self.serialPort:
            self.serialPort = open()
        return self.serialPort
            
    def open():
        return serial.Serial('/dev/ttyACM0', timeout = 1000)

    def decodeScript(s):
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
        line = line[9:]
        line = line[:-4]
        deScript(line)
        return line


    def readScript():
        i = 0
        lua = ''
        ser = getSerial()
        while True:
            script = readScriptPage(ser, i)
            if script:
                lua = lua + script
                i+=1
            else:
                break
        return lua

    def writeScript(script):
        i = 0
        res = 0
        ser = getSerial()
        while True:
            if len(script) >= 256:
                scr = script[:256]
                script = script[256:]
                res = writeScriptPage(ser, scr, i)
                if res == 0:
                    print 'Error: ' + str(i)
                    break
                i = i + 1
            else:
                writeScriptPage(ser, script, i)
                writeScriptPage(ser, '', i + 1)
                res = 1
                break
        return res


