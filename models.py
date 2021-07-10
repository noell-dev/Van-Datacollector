import logging
class Sensor:
    def __init__(self, name, description, value, unit):
        self.description = description
        self.name = name
        self.value = value
        self.unit = unit
        # self.label = Label(name=self.name, text="{}: {}{}".format(self.description, self.value, self.unit), fg="#000000")

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
        logging.debug("updateState: {} of type {}".format(value, type(value)))
        if (int(value) == 0):
            self.is_on = False
            self.color = self.off_color    
        elif (int(value) == 1):
            self.is_on = True
            self.color = self.on_color
        else:
            self.color =  self.error_color
        return self.color