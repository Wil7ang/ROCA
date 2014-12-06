from serial import Serial
import time
import string

import sys

sys.path.insert(0,"/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/")
sys.path.insert(0,"/opt/local/lib")

def tohex(val, nbits):
    hexval =  hex((val + (1 << nbits)) % (1 << nbits))
    hexval = hexval[2:len(hexval)]

    if(len(hexval) == 1):
        hexval = "\\x00\\x0" + hexval
    elif(len(hexval) == 2):
        hexval = "\\x00\\x" + hexval
    elif(len(hexval) == 3):
        hexval = "\\x0" + hexval[0] + "\\x" + hexval[1:3]
    else:
        hexval = "\\x" + hexval[0:2] + "\\x" + hexval[2:4]

    return hexval;

def getMoveCommandString(joint, angle):
    return "\\x" + str(joint) + tohex(angle, 16)

def moveArm(base, shoulder, elbow, wristA, wristB):
    BASE = 81
    SHOULDER = 82
    ELBOW = 83
    WRISTA = 84
    WRISTB = 85

    #try:
    connection = Serial(port=sys.argv[1], baudrate=19200, timeout=1)
    print getMoveCommandString(BASE, int(base))
    connection.write(getMoveCommandString(BASE, int(base)).decode('string_escape'))
    time.sleep(0.05)
    connection.write(getMoveCommandString(SHOULDER, int(shoulder)).decode('string_escape'))
    time.sleep(0.05)
    connection.write(getMoveCommandString(ELBOW, int(elbow)).decode('string_escape'))
    time.sleep(0.05)
    connection.write(getMoveCommandString(WRISTA, int(wristA)).decode('string_escape'))
    time.sleep(0.05)
    connection.write(getMoveCommandString(WRISTB, int(wristB)).decode('string_escape'))
    time.sleep(0.05)

    connection.close()
    # except Exception:
    #     print "Failed to open serial port!"

if(len(sys.argv) == 7):
    moveArm(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6])
else:
    print "Usage: controller.py <serial port> <base angle> <shoulder angle> <elbow angle> <wristA angle> <wristB angle>"
    
