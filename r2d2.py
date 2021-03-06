import logging
import threading
import serial
import sys
from tkinter import *
from models import *

# ToDo: Documentation
# Todo: Aufräumen
# ToDo: Button presses
ser_port = "/dev/ttyACM0"

waageWidth = 200
waageHeight = 200
waageCenterX = waageWidth/2
waageCenterY = waageHeight/2
waageInnerRadius = 20
multiplikatorX = (waageCenterX-waageInnerRadius)/10
multiplikatorY = (waageCenterY-waageInnerRadius)/10
waageLineWidth = 5

canvas = ""
bubble = ""
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
            newColor = buttons[entryName].updateState(entryValue)
            serial_buttons[entryName].config(background=newColor, activebackground=newColor)
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


def pressButton(serial, number):
        logging.debug("pressButton {}".format(number))
        serial.write("{}\n".format(number).encode("utf-8"))

def escape(root):
        root.geometry("200x200")

def fullscreen(root):
        width, height = root.winfo_screenwidth(), root.winfo_screenheight()
        root.geometry("%dx%d+0+0" % (width, height))


def updateCanvas(canvas, item, x, y):
    canvas.move(item, x, y)

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

if __name__ == "__main__":
    format = "%(asctime)s: %(message)s"
    logging.basicConfig(format=format, level=logging.INFO,
                        datefmt="%H:%M:%S")

    logging.info("Main    : create Serial")
    # ToDO define Serial from shell
    ser = serial.serial_for_url(ser_port, do_not_open=True)
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
    btn1 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=1), bg="#DC0F16", width=20, height=10, master=button_frame, activebackground="#DC0F16")
    btn1.pack(side=BOTTOM)
    serial_buttons[button.name] = btn1

    button = buttons["out2_hi"]
    btn2 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=2), bg="#DC0F16", width=20, height=10, master=button_frame, activebackground="#DC0F16")
    btn2.pack(side=BOTTOM)
    serial_buttons[button.name] = btn2

    button = buttons["out3_hi"]
    btn3 = Button(name=button.name, text=button.description, command=lambda: pressButton(serial=ser, number=3), bg="#DC0F16", width=20, height=5, master=button_frame, activebackground="#DC0F16")
    btn3.pack(side=BOTTOM)
    serial_buttons[button.name] = btn3


    sensor_frame.pack(side=TOP)
    button_frame.pack(side=LEFT)
    waage_frame.pack(side=RIGHT)
    

    root.mainloop()
