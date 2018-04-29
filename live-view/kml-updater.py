import simplekml
import serial
import io
from time import sleep

altconst = 62.0

file = "../dump.csv"
file = open(file, 'r')


def description_generator(list):
    string = \
        "PacketID " + str(list[6]) + "\n" +\
        "Latitude " + str(list[7]) + "\n" +\
        "Longitude " + str(list[8]) + "\n" +\
        "Altitude " + str(float(list[9])+altconst) + "\n" +\
        "Temperature " + str(list[13]) + "\n" +\
        "Humidity " + str(list[14]) + "\n" +\
        "Pressure " + str(list[15]) + "\n" +\
        "Sky temperature " + str(list[16]) + "\n" +\
        "RSSI " + str(list[17])
    return string


def follow():
    while True:
        line = file.readline()
        if not line:
            sleep(0.1)
            continue
        yield line

coords = []
kml = simplekml.Kml()
kml.save("live.kml")
currentPoint = kml.newpoint(name="Current Position")
currentPoint.style.iconstyle.icon.href = 'icon.png'
currentPoint.style.iconstyle.scale = 2
currentPoint.altitudemode = 'absolute'
linestring = kml.newlinestring(name="Trail")
linestring.altitudemode = simplekml.AltitudeMode.absolute
iterator = 0
lines = follow()
for line in lines:
    try:
        print ("doing stuff")
        list = line.split(",")
        coords.append([float(list[8]), float(list[7]), float(list[9])+altconst])
        if iterator % 5 == 0:
            new = kml.newpoint()
            new.name = ""
            new.coords = [(float(list[8]), float(list[7]), float(list[9])+altconst)]
            new.description = description_generator(list)
            linestring.coords = coords
            currentPoint.coords = [(float(list[8]), float(list[7]), float(list[9])+altconst)]
            currentPoint.description = description_generator(list)
            kml.save("live.kml")
        print(line)
        iterator += 1
    except Exception as e:
        print("Exception raised:")
        print(e)
        pass

