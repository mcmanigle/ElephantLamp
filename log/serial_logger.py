import serial
import datetime

ts = lambda: datetime.datetime.now().timestamp()

while True:
    try:
        with serial.Serial('/dev/tty.usbmodem1101', 115200) as ser:
            with open('log.'+str(ts()), 'w') as file:
                while True:
                    s = ser.readline().decode('utf8')
                    file.write(str(ts()) + ' - ' + s)
                    print(s, end='', flush=True)
    except:
       pass
