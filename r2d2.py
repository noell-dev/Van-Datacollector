import logging
import threading
import serial
import sys
from tkinter import *


entries = {
            'temp': '0',
            'pres (hPa)': '0',
            'aalt (m)': '0',
            'hum (%)': '0',
            'accx': '-0.31',
            'accy': '0.23',
            'accz': '8.99',
            'out1_hi': '0',
            'out2_hi': '0',
            'out3_hi': '0',
        }

def updateList(line):
    try:
        newEntry = line.split(":")
        if (entries.__contains__(newEntry[0])):
            entries[newEntry[0]] = newEntry[1].strip()
            if (newEntry[0] == "temp"):
                label_temp.config(text="Temperatur: {}°C".format( newEntry[1]))
            if (newEntry[0] == "pres (hPa)"):
                label_pres.config(text="Druck: {}hPa".format( newEntry[1]))
            if (newEntry[0] == "aalt (m)"):
                label_aalt.config(text="Höhe über NN: {}m".format( newEntry[1]))
            if (newEntry[0] == "hum (%)"):
                label_humi.config(text="Luftfeuchte: {}%".format( newEntry[1]))
    except Exception as e:
        logging.error("updateList: {}".format(e))

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
        # threading.Thread(target=ui_thread, args=(ser), daemon=True)
    except Exception as e:
        logging.error("Main    : Error: {}".format(ser.name, e))

    root = Tk()

    def one() :
        ser.write(int(1).to_bytes(4, "little"))
        return
    def two() :
        ser.write(int(2).to_bytes(4, "little"))
        return
    def three() :
        ser.write(int(3).to_bytes(4, "little"))
        return
    root.title("Arduino im Test")

    label_temp = Label(text="Temperatur: {}°C".format( entries["temp"]), fg="#0A116B")
    label_temp.pack()
    label_pres = Label(text="Druck: {}hPa".format( entries["pres (hPa)"]), fg="#0A116B")
    label_pres.pack()
    label_aalt = Label(text="Höhe über NN: {}m".format( entries["aalt (m)"]), fg="#0A116B")
    label_aalt.pack()
    label_humi = Label(text="Luftfeuchte: {}%".format( entries["hum (%)"]), fg="#0A116B")
    label_humi.pack()


    button_out1_hi = Button(text='1', command=one, background="#DC0F16", fg="#000000")
    button_out1_hi.pack()
    button_out2_hi = Button(text='2', command=two, background="#DC0F16", fg="#000000")
    button_out2_hi.pack()
    button_out3_hi = Button(text='3', command=three, background="#DC0F16", fg="#000000")
    button_out3_hi.pack()
    root.mainloop()


        
    