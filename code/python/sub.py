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

# argument for script
ap = argparse.ArgumentParser()
ap.add_argument(
    "-H", "--host", default="192.168.1.9",
    help="MQTT host to connect to")
ap.add_argument(
    "-Q", "--qos", type=int, default=0,
    help="QoS Level")
args = vars(ap.parse_args())

host        = args["host"]
port        = 1883
qos         = args["qos"]
countMsg    = 0
totalMsg    = 2000
state       = 0
startTestTime   = 0
lastTestTime    = 0
lastMsgTimeList = []

def on_connect(client, userdata, flags, rc):
    global qos
    print("CONNACK received with code %d." % (rc))
    client.subscribe('#',qos=qos)

def on_publish(client, userdata, mid):
    # print "sendding"
    pass

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))
    # pass
    # global state
    # print("Subscribed: " + str(mid) + " " + str(granted_qos))
    # # if state != 0:
    # print "send connect"
    # time.sleep(2)
    # (result,mid)=client.publish('/RPi3/connect/'+str(rpi_id), "",qos=qos)

def on_message(client, userdata, msg):
    global countMsg
    global lastTestTime
    global lastMsgTimeList
    # print("Received msg:" + msg.topic+" "+str(msg.qos)+" "+str(msg.payload) + "at " + str(time.time()))
    # print("at " + time.time())
    lastMsgTimeList.append(time.time() - lastTestTime)
    lastTestTime = time.time()
    # lastMsgTimeList.append(lastTestTime - startTestTime)
    countMsg += 1

directory      = "result"

# check directory is had
if not os.path.exists(directory):
    os.makedirs(directory)

# create MQTT client
client = mqtt.Client(client_id="", clean_session=True, protocol=mqtt.MQTTv311)

# set callback functions
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message   = on_message
client.on_subscribe = on_subscribe

# connect MQTT broker
client.connect(host, 1883, 60)
client.loop_start()

def saveResultFile():
    global lastTestTime
    global startTestTime
    global countMsg
    global lastMsgTimeList
    global directory
    txtFileName = directory+"/time_100msA"+str(time.time())+".txt"
    bufferString = ""
    bufferString += "total Test Time " + str(lastTestTime - startTestTime) + '\n'
    bufferString += "total msg " + str(countMsg) + '\n'
    bufferString += "minTime " + str(numpy.amin(lastMsgTimeList)) + " maxTime " + str(numpy.amax(lastMsgTimeList)) + " avgTime " + str(numpy.average(lastMsgTimeList))
    bufferString += '\n'
    file = open(txtFileName, "w")
    for i in range(len(lastMsgTimeList)):
        bufferString += str(lastMsgTimeList[i])
        bufferString += '\n'
    file.write(bufferString)
    file.close()

try:
    while 1:
        if state == 0:
            # wait for user push button
            if GPIO.input(statePin) == 1:
                lastMsgTimeList = []
                startTestTime = time.time()
                lastTestTime = time.time()
                print "Start Testing"
                state = 1
        elif state == 1:
            if countMsg >= totalMsg or time.time() - lastTestTime > 10 :
                print "finish test"
                print "total Test Time " + str(lastTestTime - startTestTime)
                print "total msg " + str(countMsg)
                print "minTime " + str(numpy.amin(lastMsgTimeList)) + " maxTime " + str(numpy.amax(lastMsgTimeList)) + " avgTime " + str(numpy.average(lastMsgTimeList)) + '\n'
                print "saving result to file "
                saveResultFile()
                print "done"
                state = 0
                countMsg = 0
except KeyboardInterrupt as ex:
    print 'Terminated...'
finally:
    client.loop_stop()
    client.disconnect()
    sys.exit(0)
