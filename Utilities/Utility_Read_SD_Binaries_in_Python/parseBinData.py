#Python
import os.path
import struct
#os.path.chdir(os.path.gcwd())
filePath = "SampleCounterData.bin"
fileSize = os.path.getsize(filePath)
fileLocation = 0
printOutput = False

with open(filePath,'rb') as binFile:
    with open(filePath[:-3]+"csv",'w') as csvFile:
        csvFile.write("Unix Time,System usec,ID (Hex),DLC,B0,B1,B2,B3,B4,B5,B6,B7\n")
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

                if printOutput:
                    print(timeSeconds,end='.')
                    print("%06d" %timeMicroseconds, end=',')
                    print(systemMicroseconds,end=',')
                    print("%08X" %ID,end=',')
                    print(DLC,end='')
                    for b in payload:
                        print(",%02X" %b,end='')
                    print()
print("Done.")
                
    
