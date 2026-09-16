from tkinter import Tk, Label, Button, ttk
import serial
import serial.tools.list_ports
import time

root = Tk()
root.title("Corner Balancing App")

FRONT = 1
REAR = 3
LEFT = 0
RIGHT = 1
POSITION_MAP = {
"Front Left": (FRONT, LEFT),
"Front Right": (FRONT, RIGHT),
"Rear Left": (REAR, LEFT),
"Rear Right": (REAR, RIGHT)
}

#heading labels
current_label = Label(root, text="Current Data", font=("Arial", 12, "bold"))
saved_label = Label(root, text="Saved Data", font=("Arial", 12, "bold"))

#scale labels
scale_labels = [Label(root, text=f"Scale {i+1}", font=("Arial", 12, "bold")) for i in range(4)]

#scale data
scale_datas = [Label(root, text="Disconnected", font=("Arial", 10)) for i in range(4)]

#CALCULATIONS!
data_label = Label(root, text="Calculations", font=("Arial", 12, "bold"))

#calculation labels
total_weight_label = Label(root, text="Total Weight:", font=("Arial", 10, "bold"))
total_leftP_label = Label(root, text="Left:", font=("Arial", 10, "bold"))
total_rightP_label = Label(root, text="Right:", font=("Arial", 10, "bold"))
total_frontP_label = Label(root, text="Front:", font=("Arial", 10, "bold"))
total_rearP_label = Label(root, text="Rear:", font=("Arial", 10, "bold"))
total_frrlP_label = Label(root, text="FRRL Diagonal:", font=("Arial", 10, "bold"))
total_flrrP_label = Label(root, text="FLRR Diagonal:", font=("Arial", 10, "bold"))

#calculation data
total_weight_data = Label(root, text="0 lb", font=("Arial", 10))
total_leftP_data = Label(root, text="0.0", font=("Arial", 10))
total_rightP_data = Label(root, text="0.0", font=("Arial", 10))
total_frontP_data = Label(root, text="0.0", font=("Arial", 10))
total_rearP_data = Label(root, text="0.0", font=("Arial", 10))
total_frrlP_data = Label(root, text="0.0", font=("Arial", 10))
total_flrrP_data = Label(root, text="0.0", font=("Arial", 10))

index_map = {
    0:0,
    1:1,
    2:2,
    3:3
}
#fl, fr, rl, rr
#dropdown_state
dropdown_state = ["Front Left", "Front Right", "Rear Left", "Rear Right"]
def on_select_scale(event, idx):
    selected = pickers[idx].get()
    #if nothing else is the target position
    if selected == "Unassigned" or selected not in dropdown_state:
        dropdown_state[idx] = selected
        if selected != "Unassigned":
            index_map[idx] = PLUG_IN_ORDER.index(selected)
            #.grid(row=FRONT, column=LEFT, padx=10, pady=10)
            scale_labels[idx].grid(row=POSITION_MAP[selected][0], column=POSITION_MAP[selected][1], padx=10, pady=10)
            scale_datas[idx].grid(row=POSITION_MAP[selected][0]+1, column=POSITION_MAP[selected][1], padx=10, pady=10)
            scale_labels_saved[idx].grid(row=POSITION_MAP[selected][0], column=POSITION_MAP[selected][1]+3, padx=10, pady=10)
            scale_datas_saved[idx].grid(row=POSITION_MAP[selected][0]+1, column=POSITION_MAP[selected][1]+3, padx=10, pady=10)
    else:
        pickers[idx].current(options.index(dropdown_state[idx]))
#dropdown
options = ["Unassigned","Front Left", "Front Right", "Rear Left", "Rear Right"]

#scale dropdown label
scale1_dropdown_label = Label(root, text="Scale 1:", font=("Arial", 10, "bold"))
scale2_dropdown_label = Label(root, text="Scale 2:", font=("Arial", 10, "bold"))
scale3_dropdown_label = Label(root, text="Scale 3:", font=("Arial", 10, "bold"))
scale4_dropdown_label = Label(root, text="Scale 4:", font=("Arial", 10, "bold"))

# Create Combobox
pickers = [ttk.Combobox(root, values=options, state="readonly") for i in range(4)]
for i in range(4):
    pickers[i].current(i+1)  # Set default selection to first item

#bind each dropdown to a function
pickers[0].bind("<<ComboboxSelected>>", lambda event: on_select_scale(event, 0))
pickers[1].bind("<<ComboboxSelected>>", lambda event: on_select_scale(event, 1))
pickers[2].bind("<<ComboboxSelected>>", lambda event: on_select_scale(event, 2))
pickers[3].bind("<<ComboboxSelected>>", lambda event: on_select_scale(event, 3))

#saved scale labels
scale_labels_saved = [Label(root, text=f"Scale {i+1}:", font=("Arial", 12, "bold")) for i in range(4)]

#saved scale data
scale_datas_saved = [Label(root, text="Disconnected", font=("Arial", 10)) for i in range(4)]

#saved calculation labels
total_weight_label_saved = Label(root, text="Total Weight:", font=("Arial", 10, "bold"))
total_leftP_label_saved = Label(root, text="Left:", font=("Arial", 10, "bold"))
total_rightP_label_saved = Label(root, text="Right:", font=("Arial", 10, "bold"))
total_frontP_label_saved = Label(root, text="Front:", font=("Arial", 10, "bold"))
total_rearP_label_saved = Label(root, text="Rear:", font=("Arial", 10, "bold"))
total_frrlP_label_saved = Label(root, text="FRRL Diagonal:", font=("Arial", 10, "bold"))
total_flrrP_label_saved = Label(root, text="FLRR Diagonal:", font=("Arial", 10, "bold"))

#saved calculation data
total_weight_data_saved = Label(root, text="0 lb", font=("Arial", 10))
total_leftP_data_saved = Label(root, text="0.0", font=("Arial", 10))
total_rightP_data_saved = Label(root, text="0.0", font=("Arial", 10))
total_frontP_data_saved = Label(root, text="0.0", font=("Arial", 10))
total_rearP_data_saved = Label(root, text="0.0", font=("Arial", 10))
total_frrlP_data_saved = Label(root, text="0.0", font=("Arial", 10))
total_flrrP_data_saved = Label(root, text="0.0", font=("Arial", 10))

#save data function for button
def save_data():
    #saved scale data
    for i in range(4):
        scale_datas_saved[i].config(text=scale_datas[i].cget("text"))
    #saved calculation data
    total_weight_data_saved.config(text=total_weight_data.cget("text"))
    total_leftP_data_saved.config(text=total_leftP_data.cget("text"))
    total_rightP_data_saved.config(text=total_rightP_data.cget("text"))
    total_frontP_data_saved.config(text=total_frontP_data.cget("text"))
    total_rearP_data_saved.config(text=total_rearP_data.cget("text"))
    total_frrlP_data_saved.config(text=total_frrlP_data.cget("text"))
    total_flrrP_data_saved.config(text=total_flrrP_data.cget("text"))

#save button
save_button = Button(root, text="Save Data", command=save_data)

#heading grid
current_label.grid(row=0, column=0, padx=10, pady=10)
saved_label.grid(row=0, column=3, padx=10, pady=10)

# FRONT = 1
# REAR = 3
# LEFT = 0
# RIGHT = 1
#scale labels grid

scale_labels[0].grid(row=FRONT, column=LEFT, padx=10, pady=10)
scale_labels[1].grid(row=FRONT, column=RIGHT, padx=10, pady=10)
scale_labels[2].grid(row=REAR, column=LEFT, padx=10, pady=10)
scale_labels[3].grid(row=REAR, column=RIGHT, padx=10, pady=10)

#CALCULATIONS! grid
data_label.grid(row=5, column=2, padx=10, pady=10)

#scale data grid
scale_datas[0].grid(row=2, column=0, padx=10, pady=10)
scale_datas[1].grid(row=2, column=1, padx=10, pady=10)
scale_datas[2].grid(row=4, column=0, padx=10, pady=10)
scale_datas[3].grid(row=4, column=1, padx=10, pady=10)

#calculation label grid
total_weight_label.grid(row=6, column=0, padx=10, pady=10)
total_leftP_label.grid(row=7, column=0, padx=10, pady=10)
total_rightP_label.grid(row=8, column=0, padx=10, pady=10)
total_frontP_label.grid(row=9, column=0, padx=10, pady=10)
total_rearP_label.grid(row=10, column=0, padx=10, pady=10)
total_frrlP_label.grid(row=11, column=0, padx=10, pady=10)
total_flrrP_label.grid(row=12, column=0, padx=10, pady=10)

#calculation data grid
total_weight_data.grid(row=6, column=1, padx=10, pady=10)
total_leftP_data.grid(row=7, column=1, padx=10, pady=10)
total_rightP_data.grid(row=8, column=1, padx=10, pady=10)
total_frontP_data.grid(row=9, column=1, padx=10, pady=10)
total_rearP_data.grid(row=10, column=1, padx=10, pady=10)
total_frrlP_data.grid(row=11, column=1, padx=10, pady=10)
total_flrrP_data.grid(row=12, column=1, padx=10, pady=10)

#save button grid
save_button.grid(row=1, column=2, padx=10, pady=10)

#saved scale labels grid
scale_labels_saved[0].grid(row=1, column=3, padx=10, pady=10)
scale_labels_saved[1].grid(row=1, column=4, padx=10, pady=10)
scale_labels_saved[2].grid(row=3, column=3, padx=10, pady=10)
scale_labels_saved[3].grid(row=3, column=4, padx=10, pady=10)

#saved scale data grid
scale_datas_saved[0].grid(row=2, column=3, padx=10, pady=10)
scale_datas_saved[1].grid(row=2, column=4, padx=10, pady=10)
scale_datas_saved[2].grid(row=4, column=3, padx=10, pady=10)
scale_datas_saved[3].grid(row=4, column=4, padx=10, pady=10)

#saved calculation labels grid
total_weight_label_saved.grid(row=6, column=3, padx=10, pady=10)
total_leftP_label_saved.grid(row=7, column=3, padx=10, pady=10)
total_rightP_label_saved.grid(row=8, column=3, padx=10, pady=10)
total_frontP_label_saved.grid(row=9, column=3, padx=10, pady=10)
total_rearP_label_saved.grid(row=10, column=3, padx=10, pady=10)
total_frrlP_label_saved.grid(row=11, column=3, padx=10, pady=10)
total_flrrP_label_saved.grid(row=12, column=3, padx=10, pady=10)

total_weight_data_saved.grid(row=6, column=4, padx=10, pady=10)
total_leftP_data_saved.grid(row=7, column=4, padx=10, pady=10)
total_rightP_data_saved.grid(row=8, column=4, padx=10, pady=10)
total_frontP_data_saved.grid(row=9, column=4, padx=10, pady=10)
total_rearP_data_saved.grid(row=10, column=4, padx=10, pady=10)
total_frrlP_data_saved.grid(row=11, column=4, padx=10, pady=10)
total_flrrP_data_saved.grid(row=12, column=4, padx=10, pady=10)

#dropdown labels grid
scale1_dropdown_label.grid(row=13, column=0, padx=10, pady=10)
scale2_dropdown_label.grid(row=14, column=0, padx=10, pady=10)
scale3_dropdown_label.grid(row=15, column=0, padx=10, pady=10)
scale4_dropdown_label.grid(row=16, column=0, padx=10, pady=10)

#dropdowns grid
for i in range(4):
    pickers[i].grid(row=13 + i, column=1, padx=10, pady=10)

#scan all COM ports
ports = serial.tools.list_ports.comports()

#how many scales should be connected (should be 4 for corner balancing)
NUM_OF_SCALES = 4
PLUG_IN_ORDER = ["Front Left", "Front Right", "Rear Left", "Rear Right"]
serial_objects = []
PREFERRED_UNIT = 'k' #kgs, automatically switches to lbs if majority is selected as lbs

scale_dict = {} #port: scale index

updating = False

#wait until all scales are connected
scale_counter = 0
def wait_for_scales_to_connect():
    global scale_counter, ports
    print(f"Connect {PLUG_IN_ORDER[scale_counter]} Scale.")
    #rescan ports
    ports = serial.tools.list_ports.comports()
    #if the new scale is connected
    if len(ports) > scale_counter:
        print(f"{PLUG_IN_ORDER[scale_counter]} Scale is connected.")
        setup_serial_object(scale_counter)
        scale_counter += 1
    if (scale_counter < NUM_OF_SCALES):
        root.after(100, wait_for_scales_to_connect)
    else:
        print("All scales connected")

def get_disconnnected_scales():
    disconnected_indices = [0, 1, 2, 3]
    port_list = serial.tools.list_ports.comports()
    port_names = [port_list[i].device for i in range(len(port_list))]
    for recorded_port_name in scale_dict.keys():
        if recorded_port_name in port_names:
            if (not serial_objects[scale_dict[recorded_port_name]].is_open):
                serial_objects[scale_dict[recorded_port_name]].open()
            disconnected_indices.remove(scale_dict[recorded_port_name])
        else:
            serial_objects[scale_dict[recorded_port_name]].close()
    return disconnected_indices



def setup_serial_object(i):
    global serial_objects, ports, updating
    #create serial object and add them to a list

    #open serial port
    try:
        ser = serial.Serial(
            port=ports[i].device,
            baudrate=9600,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0
        )
    except serial.SerialException as e:
        print(e)
        root.after(1000, lambda: setup_serial_object(i))
        return

    scale_dict[ports[i].device] = i
    #add objects to list
    serial_objects.append(ser)
    if not updating:
        updating = True
        root.after(100, update_scales)

#function to return weight read from a serial port
def read_weight(ser_object):
    #write data to recieve data rear
    ser_object.write(b"\r")
    line = ser_object.readline()
    #if data received --> filter and output number
    if line:
        decoded_line = line.split()
        if_negative = 0
        if chr(decoded_line[0][0] & 0x7F) == '-':
            if_negative = 1
        output_str = ''
        for b in decoded_line[if_negative]:
            #bitmask removes most significant bit to filter data
            output_str += chr(b & 0x7F)
        unit = chr(decoded_line[if_negative + 1][0] & 0x7F) 
        if if_negative == 1:
            return (-float(output_str), unit)
        else:
            return (float(output_str), unit)
    #error code (failed data read)
    return (-1, '')
    

#units to kg map
unit_conversion_map = {
    'k': 1.0,
    's': 6.35029,
    'l': 0.453592
}

#class that contains all necessary calculations
class CornerBalance:
    def __init__(self):
        #basic weights
        self.total_weight = 0
        self.fl = (None, None)
        self.fr = (None, None)
        self.rl = (None, None)
        self.rr = (None, None)
        #side percentages
        self.front = 0
        self.rear = 0
        self.left = 0
        self.right = 0
        #diagonal percentages
        self.flrr = 0
        self.frrl = 0
    def update(self, ordered_weights, ordered_units):
        global PREFERRED_UNIT
        #main weigth values
        self.fl = (ordered_weights[0], ordered_units[0])
        self.fr = (ordered_weights[1], ordered_units[1])
        self.rl = (ordered_weights[2], ordered_units[2])
        self.rr = (ordered_weights[3], ordered_units[3])

        if None in ordered_weights and None in ordered_units:
            return 
        #converted weights all to kg
        self.converted_weights = [ordered_weights[i] * unit_conversion_map[ordered_units[i]] for i in range(NUM_OF_SCALES)]
        self.cfl, self.cfr, self.crl, self.crr = self.converted_weights
        self.ctotal_weight = sum(self.converted_weights)
        self.total_weight = self.ctotal_weight
        #switch total to lbs if preferred
        if ordered_units.count('l') > (NUM_OF_SCALES // 2):
             self.total_weight *= 2.20462
             PREFERRED_UNIT = 'l'
        else:
            PREFERRED_UNIT = 'k'
        
        #percentage calcs if weight is there
        if (self.total_weight != 0):
            #side percentages
            self.front = (self.cfl + self.cfr) / self.ctotal_weight * 100.0
            self.rear = (self.crl + self.crr) / self.ctotal_weight * 100.0
            self.left = (self.cfl + self.crl) / self.ctotal_weight * 100.0
            self.right = (self.cfr + self.crr) / self.ctotal_weight * 100.0
            #diagonal percentages
            self.flrr = (self.cfl + self.crr) / self.ctotal_weight * 100.0
            self.frrl = (self.cfr + self.crl) / self.ctotal_weight * 100.0

#create balance object
balance = CornerBalance()

#reading data loop and store weights into weights list
weights = [None] * NUM_OF_SCALES #[fl, fr, rl, rr]
units = [None] * NUM_OF_SCALES
unit_display_map = {
    'l' : 'lb',
    's' : 'st',
    'k' : 'kg'
}
def update_scales():
    global scale_datas, serial_objects

    disconnected_scales = get_disconnnected_scales()

    #for each scale
    for i in range(len(serial_objects)):
        #read current scale
        if i not in disconnected_scales:
            weight, unit = read_weight(serial_objects[i])
            #update weights list if data read is successful
            if weight != -1:
                weights[i] = weight
                units[i] = unit
        else:
            weights[i] = None
            units[i] = None
    #update all calculations
    if dropdown_state.count("Unassigned") == 0:
        transformedWeights = [weights[index_map[i]] for i in range(NUM_OF_SCALES)]
        balance.update(transformedWeights, units)
    
    if (weights[0] is not None):
        scale_datas[0].config(text=str(weights[0]) + " " + unit_display_map[balance.fl[1]])
    else:
        scale_datas[0].config(text="Disconnected")
    if (weights[1] is not None):
        scale_datas[1].config(text=str(weights[1]) + " " + unit_display_map[balance.fr[1]])
    else:
        scale_datas[1].config(text="Disconnected")
    if (weights[2] is not None):
        scale_datas[2].config(text=str(weights[2]) + " " + unit_display_map[balance.rl[1]])
    else:
        scale_datas[2].config(text="Disconnected")
    if (weights[3] is not None):
        scale_datas[3].config(text=str(weights[3]) + " " + unit_display_map[balance.rr[1]])
    else:
        scale_datas[3].config(text="Disconnected")
    #total weight update
    total_weight_data.config(text=f"{balance.total_weight:.2f} {unit_display_map[PREFERRED_UNIT]}")
    #side percentage update
    total_frontP_data.config(text=f"{balance.front:.2f}%")
    total_rearP_data.config(text=f"{balance.rear:.2f}%")
    total_leftP_data.config(text=f"{balance.left:.2f}%")
    total_rightP_data.config(text=f"{balance.right:.2f}%")
    #diagonal percentage update
    total_flrrP_data.config(text=f"{balance.flrr:.2f}%")
    total_frrlP_data.config(text=f"{balance.frrl:.2f}%")
    root.after(100, update_scales)

root.after(100, wait_for_scales_to_connect)
try:
    root.mainloop()
except KeyboardInterrupt as e:
    print(e)
    for serial_object in serial_objects:
        if serial_object.is_open:
            serial_object.close()
    root.destroy()
except serial.SerialException as e:
    print(e)
    for serial_object in serial_objects:
        if serial_object.is_open:
            serial_object.close()
    root.destroy()