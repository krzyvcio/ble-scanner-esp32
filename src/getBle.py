import serial
import json
import random
# Open the serial port
ser = serial.Serial('/dev/ttyUSB0', 
                    baudrate=115200,
                    bytesize=serial.EIGHTBITS,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE,
                    timeout=0.1,
                    xonxoff=0,
                    rtscts=0,
                    interCharTimeout=None

                    )

# Define ANSI escape sequences for colors


RESET = '\033[0m'
WHITE = '\033[37m'
BLACK = '\033[30m'

class Colors:
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    ORANGE = '\033[91m'
    PURPLE = '\033[95m'
    GRAY = '\033[90m'
    
def random_color():
    return random.choice([Colors.RED, Colors.GREEN, Colors.YELLOW, Colors.BLUE, Colors.MAGENTA, Colors.CYAN, Colors.ORANGE, Colors.PURPLE, Colors.GRAY])

# creaate sqllite3
import sqlite3
import re
conn = sqlite3.connect('ble.db')
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS ble (id INTEGER PRIMARY KEY, data TEXT, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)''')
conn.commit()
conn.close()

def insert_data(data):
    conn = sqlite3.connect('ble.db')
    c = conn.cursor()
    c.execute("INSERT INTO ble (data) VALUES (?)", (data,))
    conn.commit()
    conn.close()


# Usage example

# Read lines from the serial port
accumulatedString = ""
while True:
    line = ser.readline().decode('utf-8', 'ignore')
    if line:
        

        # find [ and ] in the line
        # sometimes line endds withou ] and can be starttes without [
        # so we need to find the first [ and the last ] making operation adding efeythis to the strin from ocurrences
       
        accumulatedString = accumulatedString + line
        first = accumulatedString.find('[')
        #wait for the first [
        last = accumulatedString.rfind(']')
        if not last:
            continue

        # if we have a complete json string

        if first != -1 and last != -1:
            # extract the json string
            jsonStr = accumulatedString[first:last+1]
            # remove the json string from accumulatedString
            accumulatedString = accumulatedString[last+1:]
            # parse the json string
            try:
                data = jsonStr
                if not data:
                    continue
                if data == "":
                    continue
                    
                # print(data)
                insert_data(data)
                json = json.loads(data)
                print(random_color())
                print(json)
                print(RESET)

            except:
                print("Error parsing json string: " + jsonStr)
                continue

