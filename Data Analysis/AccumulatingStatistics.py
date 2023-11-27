import serial
import numpy as np
from sklearn.linear_model import SGDClassifier

class Connection:
    def __init__(self, ser):
        self.ser = ser

    def read(self, size):
        return self.ser.read(size)

    def read_n(self):
        return self.ser.readline()

    def write(self, text):
        self.ser.write(text)

    def set_T1(self, T1):
        self.ser.write("T1" + str(T1))

    def set_T2(self, T2):
        self.ser.write("T2" + str(T2))


port = "COM5"
baudrate = 9600
con = Connection(serial.Serial(port, baudrate=baudrate))

data_list = []
while True:
    data = con.read_n()
    if data:
        data_list.append(int(data.strip()))
        print("Received data:", data)
    if len(data_list) >= 100:
        break

data_array = np.array(data_list)[:, np.newaxis]

labels = np.where(data_array > np.mean(data_array), 1, -1)

classifier = SGDClassifier(random_state=42)
classifier.fit(data_array, labels)

while True:
    new_data = con.read_n()
    if new_data:
        new_data = np.array([int(new_data.strip())])
        prediction = classifier.predict(new_data)
        print("Received data:", new_data)
        print("Prediction:", prediction)