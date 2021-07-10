import logging
import threading
import random
import serial
import sys
from tkinter import *

# ToDo: Documentation
# Todo: Aufräumen
# ToDo: Button presses


class Sensor:
    def __init__(self, name, description, value, unit):
        self.description = description
        self.name = name
        self.value = value
        self.unit = unit
        self.label = Label(name=self.name, text="{}: {}{}".format(self.description, self.value, self.unit), fg="#000000")

    def updateValue(self, value):
        self.value = value
        return "{}: {}{}".format(self.description, self.value, self.unit)

class Btn():
    def __init__(self, name, description, number,):
        self.name = name
        self.description = description
        self.number = number
        self.on_color = "#16DC0F"
        self.off_color = "#DC0F16"
        self.error_color= "#000000"
        self.color = self.off_color
        self.name = name
        self.text = description
        self.number = number
        self.is_on = False

    def updateState(self, value):
        logging.info("updateState: {} of type {}".format(value, type(value)))
        if (int(value) == 0):
            self.is_on = False
            self.color = self.off_color    
        elif (int(value) == 1):
            self.is_on = True
            self.color = self.on_color
        else:
            self.color =  self.error_color
        return self.color
        

def pressButton(serial, number):
        logging.debug("pressButton {}".format(number))
        serial.write("{}\n".format(number).encode("utf-8"))


waageWidth = 200
waageHeight = 200
waageCenterX = waageWidth/2
waageCenterY = waageHeight/2
waageInnerRadius = 20
multiplikatorX = (waageCenterX-waageInnerRadius)/10
multiplikatorY = (waageCenterY-waageInnerRadius)/10
waageLineWidth = 5

sensor_labels = {}

serial_buttons = {}

    
sensors = {
    "temp": Sensor("temp", "Temperatur",   0.0, "°C"),
    "humi": Sensor("humi", "Luftfeuchte",  0.0, "%"),
    "pres": Sensor("pres", "Druck",        0.0, "hPa"),
    "aalt": Sensor("aalt", "Höhe über NN", 0.0, "m"),
}

buttons = {
    "out1_hi": Btn("out1_hi", "Wechselrichter", 1),
    "out2_hi": Btn("out2_hi", "Wechselrichter", 2),
    "out3_hi": Btn("out3_hi", "Wechselrichter", 3),
}
entries = {
            'accx': '0',
            'accy': '0',
            'accz': '8.99',
        }

canvas = ""
bubble = ""

def updateList(line):
    try:
        newEntry = line.split(":")
        entryName = newEntry[0]
        entryValue = newEntry[1].strip()
        if ((entryName in sensor_labels) & (entryName in sensors)):
            logging.debug("updateList: sensor: {}".format(line))
            sensor_labels[entryName].config(text=sensors[entryName].updateValue(entryValue))
        elif ((entryName in serial_buttons) & (entryName in buttons)):
            logging.debug("updateList: button: {}".format(line))
            serial_buttons[entryName].config(background=buttons[entryName].updateState(entryValue))
        elif (entryName in entries):
            old_accx = entries["accx"]
            old_accy = entries["accy"]
            entries[entryName] = entryValue
            if ((canvas != "") and (bubble != "") and (entryName != 'accz')):
                changex = float(old_accx) - float(entries["accx"])
                changey = float(old_accy) - float(entries["accy"])
                updateCanvas(canvas, bubble, multiplikatorX*changex, multiplikatorY*changey,)
                logging.debug("updateList: qyro: {}".format(line))
        else:
            logging.error("updateList: not recognized: {}".format(line))
        
    except Exception as e:
        logging.error("updateList: {} - {}".format(e, line))

class Reader():
    def __init__(self, serial_instance, debug=False):
        self.serial = serial_instance
        self._write_lock = threading.Lock()
        self.log = logging.getLogger('reader')



    def start(self):
        """connect the serial port to the TCP port by copying everything
           from one side to the other"""
        logging.info("Reader: starting")
        self.alive = True
        self.thread_read = threading.Thread(target=self.read_data)
        self.thread_read.daemon = True
        self.thread_read.name = 'serial->socket'
        self.thread_read.start()


    def read_data(self):
        logging.info("Thread read_data: starting")
        line = ""
        while self.alive:
            try:
                data = self.serial.read(1)
                if data:
                    if data == b'\r' or data == b'\n':
                        if (len(line)>0):
                            updateList(line)
                        line = ""
                    else:
                        line = "{}{}".format(line, data.decode())
            except Exception as e:
                logging.error('Thread read_data: Error: {}'.format(e))
                # probably got disconnected
                break
        self.alive = False
        logging.debug('Thread read_data: terminated')
    
    def stop(self):
        if self.alive:
            self.alive = False
            self.thread_read.join()

def escape(root):
        root.geometry("200x200")

def fullscreen(root):
        width, height = root.winfo_screenwidth(), root.winfo_screenheight()
        root.geometry("%dx%d+0+0" % (width, height))


def updateCanvas(canvas, item, x, y):
    canvas.move(item, x, y)

if __name__ == "__main__":
    format = "%(asctime)s: %(message)s"
    logging.basicConfig(format=format, level=logging.INFO,
                        datefmt="%H:%M:%S")

    logging.info("Main    : create Serial")
    ser = serial.serial_for_url("/dev/ttyACM0", do_not_open=True)
    try:
        logging.info("Main    : open Serial")
        ser.open()
    except serial.SerialException as e:
        logging.error("Could not open serial port {}: {}".format(ser.name, e))
        sys.exit(1)
    logging.info("Main    : opened Serial succesfully")
    r = Reader(ser)
    try:
        logging.info("Main    : start Reader")
        r.start()
    except Exception as e:
        logging.error("Main    : Error: {}".format(ser.name, e))

    root = Tk()
    width, height = root.winfo_screenwidth(), root.winfo_screenheight()
    
    root.geometry("%dx%d+0+0" % (width, height))
    root.bind("<Escape>", lambda a :escape(root))
    root.bind("<F1>", lambda b: fullscreen(root))
    
    sensor_frame = Frame(root)
    button_frame = Frame(root)
    waage_frame = Frame(root)



    waage = Canvas(height=waageHeight, width=waageWidth, master=waage_frame)
    waage.pack()
    waage.create_oval(  waageLineWidth,
                        waageLineWidth,
                        waageWidth-waageLineWidth,
                        waageHeight-waageLineWidth,
                        width=5, fill="#476042")
    waage.create_oval(  waageCenterX-75,
                        waageCenterY-75,
                        waageCenterX+75,
                        waageCenterY+75,
                        width=5, fill="#476042")
    waage.create_oval(  waageCenterX-50,
                        waageCenterY-50,
                        waageCenterX+50,
                        waageCenterY+50,
                        width=5, fill="#476042")
    waage.create_oval(  waageCenterX-25,
                        waageCenterY-25,
                        waageCenterX+25,
                        waageCenterY+25,
                        width=5, fill="#476042")

    canvas = waage
    bubble = waage.create_oval( 
        waageCenterX+multiplikatorX*float(entries["accx"])-waageInnerRadius,
        waageCenterY+waageCenterY*float(entries["accy"])-waageInnerRadius,
        waageCenterX+multiplikatorX*float(entries["accx"])+waageInnerRadius,
        waageCenterY+waageCenterY*float(entries["accy"])+waageInnerRadius,
        width=2, fill="white")
    
    

    label_temp = Label(text=sensors["temp"].updateValue(sensors["temp"].value), fg="#0A116B", master=sensor_frame)
    label_temp.pack(side=LEFT)
    sensor_labels[sensors["temp"].name] = label_temp

    label_humi = Label(text=sensors["humi"].updateValue(sensors["humi"].value), fg="#0A116B", master=sensor_frame)
    label_humi.pack(side=LEFT)
    sensor_labels[sensors["humi"].name] = label_humi

    label_pres = Label(text=sensors["pres"].updateValue(sensors["pres"].value), fg="#0A116B", master=sensor_frame)
    label_pres.pack(side=LEFT)
    sensor_labels[sensors["pres"].name] = label_pres

    label_aalt = Label(text=sensors["aalt"].updateValue(sensors["aalt"].value), fg="#0A116B", master=sensor_frame)
    label_aalt.pack(side=LEFT)
    sensor_labels[sensors["aalt"].name] = label_aalt


    button = buttons["out1_hi"]
    btn1 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=1), bg="#DC0F16", width=20, height=10, master=button_frame)
    btn1.pack(side=BOTTOM)
    serial_buttons[button.name] = btn1

    button = buttons["out2_hi"]
    btn2 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=2), bg="#DC0F16", width=20, height=10, master=button_frame)
    btn2.pack(side=BOTTOM)
    serial_buttons[button.name] = btn2

    button = buttons["out3_hi"]
    btn3 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=3), bg="#DC0F16", width=20, height=5, master=button_frame)
    btn3.pack(side=BOTTOM)
    serial_buttons[button.name] = btn3


    sensor_frame.pack(side=TOP)
    button_frame.pack(side=LEFT)
    waage_frame.pack(side=RIGHT)
    

    root.mainloop()
