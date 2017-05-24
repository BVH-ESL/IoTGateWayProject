# python script for subscriber test datas
# loop testing 10 times
import sys, os, time, datetime, argparse, threading
import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import numpy

# init pin for change state
GPIO.setmode(GPIO.BCM)
btnPin = 17
GPIO.setup(btnPin, GPIO.IN)
trigPin = 27
GPIO.setup(trigPin, GPIO.OUT, initial=1)
GPIO.setwarnings(False)


# argument for script
ap = argparse.ArgumentParser()
ap.add_argument(
    "-H", "--host", default="192.168.1.9",
    help="MQTT host to connect to"
)
ap.add_argument(
    "-Q", "--qos", type=int, default=0,
    help="QoS Level"
)
ap.add_argument(
    "-C", "--connrate", type=int, default=10,
    help="connect rate for testing"
)
ap.add_argument(
    "-P", "--payload", type=int, default=32,
    help="connect rate for testing"
)
args = vars(ap.parse_args())

host        = args["host"]
port        = 1883
qos         = args["qos"]
connrate    = args["connrate"]
payload     = args["payload"]
countTest   = 0
totalTest   = 10
countMsg    = 0
totalMsg    = 1000
state       = 0
startTestTime   = 0
lastTestTime    = 0
lastMsgTimeList     = []

startTestTimeList   = []
lastTestTimeList    = [[]]

latencyList     = []
throughputList  = []
errRateList     = []


def on_connect(client, userdata, flags, rc):
    global qos
    print("CONNACK received with code %d." % (rc))
    client.subscribe('#',qos=qos)

def on_publish(client, userdata, mid):
    # print "sendding"
    pass

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_message(client, userdata, msg):
    global countMsg
    global lastTestTime
    global lastMsgTimeList
    lastMsgTimeList.append(time.time() - lastTestTime)
    lastTestTime = time.time()
    countMsg += 1

directory   = "result"

# check directory is had
if not os.path.exists(directory):
    os.makedirs(directory)

# create MQTT client
client = mqtt.Client(client_id="", clean_session=True, protocol=mqtt.MQTTv311)

# set callback functions
client.on_connect   = on_connect
client.on_publish   = on_publish
client.on_message   = on_message
client.on_subscribe = on_subscribe

# connect MQTT broker
client.connect(host, 1883, 60)
client.loop_start()

def saveResultFactorFile():
    global lastTestTime
    global startTestTime
    global countMsg
    global lastMsgTimeList
    global directory
    factorFileName = directory + "/Factor_" + str(connrate) + ".txt"
    bufferString = "TestNumber,\tLatency,\tThrough Put,\terror Rate\n"
    bufferString += "total Test Time " + str(lastTestTime - startTestTime) + '\n'
    bufferString += "total msg " + str(countMsg) + '\n'
    bufferString += "minTime " + str(numpy.amin(lastMsgTimeList)) + " maxTime " + str(numpy.amax(lastMsgTimeList)) + " avgTime " + str(numpy.average(lastMsgTimeList))
    bufferString += '\n'
    file = open(txtFileName, "a")
    for i in range(len(lastMsgTimeList)):
        bufferString += str(lastMsgTimeList[i])
        bufferString += '\n'
    file.write(bufferString)
    file.close()

def saveResultFile():
    global factorFileName
    bufferString = "avg,\t"+str(numpy.average(latencyList))+",\t"+str(numpy.average(throughputList))+",\t"+str(numpy.average(errorRate))+"\n"
    file = open(factorFileName, "a")
    file.write(bufferString)
    file.close()
    print "finish test"

factorFileName = directory + "/Factor_" + str(connrate) + "_" + str(payload) + ".txt"
bufferString = "TestNumber,\tLatency,\tThrough Put,\tnumPacket\n"
file = open(factorFileName, "a")
file.write(bufferString)
file.close()

try:
    while 1:
        # pass
        if countTest == totalTest:
            break
        if state == 0:
            # wait for user push button
            if GPIO.input(btnPin) == 0 or countTest != 0 and countTest < totalTest :
                print "Start Testing"
                lastMsgTimeList = []
                startTestTime = time.time()
                lastTestTime = time.time()
                GPIO.output(trigPin, 0)
                state = 1
        elif state == 1:
            GPIO.output(trigPin, 1)
            if countMsg >= totalMsg or time.time() - lastTestTime > 10 :
                # GPIO.output(trigPin, 1)
                print "finish test at " + str(countTest+1)
                latency = lastTestTime - startTestTime
                throughput  = (countMsg)/latency
                errorRate   = countMsg * 100 / totalMsg

                print "saving result to file "
                bufferString = str(countTest+1)+",\t"+str(latency)+",\t"+str(throughput)+",\t"+str(countMsg)+"\n"
                file = open(factorFileName, "a")
                file.write(bufferString)
                file.close()
                print "done"

                latencyList.append(latency)
                throughputList.append(throughput)
                errRateList.append(errorRate)
                # startTestTimeList.append(startTestTime)
                # lastTestTimeList.appednd(lastMsgTimeList)
                state = 0
                countMsg = 0
                countTest += 1
                time.sleep(30)

    saveResultFile()
except KeyboardInterrupt as ex:
    print 'Terminated...'
finally:
    client.loop_stop()
    client.disconnect()
    sys.exit(0)
