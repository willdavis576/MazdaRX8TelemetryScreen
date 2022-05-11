import serial

ser = serial.Serial('COM3', 115200)

pedal = 0
engineTemp = 0

while True:
    try:
        # data = str(ser.readline())
        
        data = ser.readline()[2:].decode("utf-8").strip()

        if '201' in data[:2]:
            datasplit = data.split(",")
            pedal = int(eval(datasplit[8]))/2
            # print(pedal)
            
        if '420' in data[:5]:
            datasplit = data.split(",")
            print(datasplit)
            engineTemp = int(eval(datasplit[2]))
    
    except KeyboardInterrupt:
        break



#513	8	13	212	255	255	39	16	0	255		201
