import gspread
import paho.mqtt.client as mqtt
from twilio.rest import Client
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime

# Create a client to interact with the Twilio REST API
account_sid = "AC4e84a781b6ba01e8a0e8e7ab728c9339"
auth_token  = "c55ddd26fb0987332340619710480544"
twilio_client = Client(account_sid, auth_token)

# Create a client to interact with the Google Drive API
scope = ['https://spreadsheets.google.com/feeds']
creds = ServiceAccountCredentials.from_json_keyfile_name('client_secret.json', scope)
google_client = gspread.authorize(creds)

# Find a workbook by name and open the first sheet
sheet = google_client.open("IOT Lock").sheet1
row = 2

# The callback for when the MQTT client receives a CONNACK response
# from the server
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code: " + str(rc) + "\n")

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("IOTLock/SendCode")
    client.subscribe("IOTLock/Success")
    client.subscribe("IOTLock/Failure")

# The callback for when a PUBLISH message is received from the server
# Either sends the randomly generated passcode or posts logging information
# to the Google Spreadsheet
def on_message(client, userdata, msg):
    global row

    if msg.topic == "IOTLock/SendCode":
        print(msg.topic + ": " + str(msg.payload))
        message = twilio_client.messages.create(
            to="+12032462176",
            from_="+16319941036",
            body=str(msg.payload))
        print("Message SID: " + message.sid + "\n")
    else:
        print(msg.topic + ": " + str(msg.payload) + "\n")
        log = datetime.now().strftime("%Y-%m-%d %H:%M:%S").split()
        if msg.topic == "IOTLock/Success":
            log.append("SUCCESS")
        elif msg.topic == "IOTLock/Failure":
            log.append("FAILURE")
        sheet.insert_row(log, row)
        row += 1

# Create a client to interact with the MQTT broker
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect("m2m.eclipse.org", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqtt_client.loop_forever()
