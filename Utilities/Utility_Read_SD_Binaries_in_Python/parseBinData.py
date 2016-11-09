#Python
import os.path
import struct
import matplotlib.pyplot as plt

filePath = "TU02_001.bin"
fileSize = os.path.getsize(filePath)
fileLocation = 0
printOutput = False
startTime = None
with open(filePath,'rb') as binFile:
    SPN190_time=[]
    SPN190_value=[]
    with open(filePath[:-3]+"csv",'w') as csvFile:
        csvFile.write("Unix Time,System usec,ID (Hex),DLC,B0,B1,B2,B3,B4,B5,B6,B7\n")
        with open(filePath[:-4] + "_signal.csv", 'w') as signalFile:
            signalFile.write("Time,PGN,SPN,Value\n")
            while (fileLocation < fileSize):
                block = binFile.read(512)
                fileLocation+=512
                binFile.seek(fileLocation)
                #print(".",end='')
                for recordNum in range(21):
                    record = block[4+recordNum*24:4+(recordNum+1)*24]
                    #print(record)
                    timeSeconds = struct.unpack("<L",record[0:4])[0]
                    timeMicrosecondsAndDLC = struct.unpack("<L",record[8:12])[0]
                    timeMicroseconds = timeMicrosecondsAndDLC & 0x00FFFFFF
                    DLC = (timeMicrosecondsAndDLC & 0xFF000000) >> 24
                    systemMicroseconds = struct.unpack("<L",record[4:8])[0]
                    ID = struct.unpack("<L",record[12:16])[0]
                    payload = struct.unpack("BBBBBBBB",record[16:24])

                    csvFile.write("%d." %timeSeconds)
                    csvFile.write("%06d," %timeMicroseconds)
                    csvFile.write("%d," %systemMicroseconds)
                    csvFile.write("%08X," %ID)
                    csvFile.write("%d" %DLC)
                    for b in payload:
                        csvFile.write(",%02X" %b)
                    csvFile.write("\n")

                    PF = (0x00FF0000 & ID) >> 16
                    DA = (0x0000FF00 & ID) >> 8
                    if PF >= 240: #PDU2 format
                      PGN = PF*256+DA
                      DA = 0xFF
                    else:
                      PGN = PF*256

                    if startTime == None:
                        startTime=timeSeconds + timeMicroseconds * 0.000001
                      

                    if PGN == 61444: #EEC1
                        SPN190 = (payload[4]*256 + payload[3])*0.125 #0.125 RPM per bit
                        signalFile.write("%d." %timeSeconds)
                        signalFile.write("%06d," %timeMicroseconds)
                        signalFile.write("EEC1,%d," %PGN)
                        signalFile.write("Engine Speed,%0.3f\n" %SPN190)

                        SPN190_time.append(timeSeconds + timeMicroseconds * 0.000001 -startTime)
                        SPN190_value.append(SPN190)
                        
                    if printOutput:
                        print(timeSeconds,end='.')
                        print("%06d" %timeMicroseconds, end=',')
                        print(systemMicroseconds,end=',')
                        print("%08X" %ID,end=',')
                        print(DLC,end='')
                        for b in payload:
                            print(",%02X" %b,end='')
                        print()
plt.plot(SPN190_time,SPN190_value,".")
plt.title("Data from "+filePath)
plt.xlabel("Time [sec]")
plt.ylabel("Engine Speed [RPM]")
plt.grid()
plt.savefig(filePath[:-4]+"_signals.png",rasterized=True)
plt.axis([210,220,600,1400])
plt.savefig(filePath[:-4]+"_signals_zoomed.png")
print("Done.")
                
    
