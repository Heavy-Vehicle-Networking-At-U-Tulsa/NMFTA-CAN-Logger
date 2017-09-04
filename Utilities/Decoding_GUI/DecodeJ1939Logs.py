#!/bin/env/python
# An introduction sample source code for some basic statistics


#Import 
import sys
from PyQt5.QtWidgets import (QMainWindow,
                             QWidget, 
                             QTreeView, 
                             QMessageBox, 
                             QHBoxLayout, 
                             QFileDialog, 
                             QLabel, 
                             QSlider, 
                             QCheckBox, 
                             QLineEdit, 
                             QVBoxLayout, 
                             QApplication, 
                             QPushButton,
                             QTableWidget,
                             QTableView,
                             QTableWidgetItem,
                             QScrollArea,
                             QAbstractScrollArea,
                             QAbstractItemView,
                             QSizePolicy,
                             QGridLayout,
                             QGroupBox,
                             QAction,
                             QProgressDialog)
from PyQt5.QtCore import (Qt, 
                          QTimer, 
                          QCoreApplication)
from PyQt5.QtGui import QIcon
import numpy as np
import os
import struct
import time
import json
from functools import partial

from graphing import * #this is a custom class file for graphics

rcParams.update({'figure.autolayout': True}) #Depends of matplotlib from graphing

program_title = "NMFTA CAN Data Analyzer"
program_version = "0.1beta"
program_author = "Jeremy Daily"
program_position = "Department of Mechanical Engineering"
program_affiliation = "The University of Tulsa"

      
class CANDecoderMainWindow(QMainWindow):

    def __init__(self):
        super().__init__()
        self.home_directory = os.getcwd()
        self.setGeometry(100,100,1615,900)
        # Upon startup, run a user interface routine
        self.init_ui()
        #self.showMaximized()
              
    def init_ui(self):
        #Builds Graphical User Interface (GUI)
        
        #Start with a status bar
        self.statusBar().showMessage('Ready')
        
        #Build common menu options
        menubar = self.menuBar()
        
        #File Menu Items
        file_menu = menubar.addMenu('&File')
        open_file = QAction(QIcon(r'icons8_Open_48px_1.png'), '&Open', self)
        open_file.setShortcut('Ctrl+O')
        open_file.setStatusTip('Open new File')
        open_file.triggered.connect(self.load_data)
        file_menu.addAction(open_file)

        exit_action = QAction(QIcon(r'icons8_Exit_Sign_48px.png'), '&Exit', self)        
        exit_action.setShortcut('Ctrl+Q')
        exit_action.setStatusTip('Exit application')
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)

        #Data Menu Items
        data_menu = menubar.addMenu('&Data')
        load_action = QAction(QIcon(r'icons8_Data_Sheet_48px.png'), '&Load Selected IDs', self)        
        load_action.setStatusTip('Filter selected message IDs into the Data Table')
        load_action.triggered.connect(self.load_message_table)
        data_menu.addAction(load_action)
       
        #Help Menu Items
        help_menu = menubar.addMenu('&Help')
        about_action = QAction(QIcon(r'icons8_Info_40px.png'), '&About', self)        
        about_action.setStatusTip('Show Program Information')
        about_action.triggered.connect(self.show_about_dialog)
        help_menu.addAction(about_action)
        
        
        #build the entries in the dockable tool bar
        self.main_toolbar = self.addToolBar("Main")
        self.main_toolbar.addAction(open_file)
        self.main_toolbar.addAction(exit_action)
        self.main_toolbar.addAction(about_action)

        self.data_toolbar = self.addToolBar("Data")
        self.data_toolbar.addAction(load_action)
        
        self.main_widget = QWidget()
        
        
        
        #Set up a Table to display CAN Messages data
        self.data_table = QTableWidget()
        #self.data_table.itemSelectionChanged.connect(self.compute_stats)
  
        #Set up a table to display CAN ID data
        self.can_id_table = QTableWidget()
           
        self.main_widget = QWidget(self)
        self.graph_canvas = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        
        #Define where the widgets go in the window
        #We start by defining some boxes that we can arrange
        
        #Create a GUI box to put all the table and data widgets in
        table_box = QGroupBox("Data Table")
        #Create a layout for that box using the vertical
        table_box_layout = QVBoxLayout()
        #Add the widgets into the layout
        table_box_layout.addWidget(self.data_table)
        #setup the layout to be displayed in the box
        table_box.setLayout(table_box_layout)


        can_id_box = QGroupBox("CAN ID Table")
        #Create a layout for that box using the vertical
        can_id_box_layout = QVBoxLayout()
        #Add the widgets into the layout
        can_id_box_layout.addWidget(self.can_id_table)
        #setup the layout to be displayed in the box
        can_id_box.setLayout(can_id_box_layout)

        #Setup the area for plotting SPNs
        self.control_scroll_area = QScrollArea()
        #self.control_scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        #self.control_scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.control_scroll_area.setWidgetResizable(True)
        #Create the container widget
        self.control_box = QWidget()
        #put the container widget into the scroll area
        self.control_scroll_area.setWidget(self.control_box)
        #create a layout strategy for the container 
        self.control_box_layout = QVBoxLayout()
        #assign the layout strategy to the container
        self.control_box.setLayout(self.control_box_layout)
        #set the layout so labels are at the top
        self.control_box_layout.setAlignment(Qt.AlignTop)
        
        label = QLabel("Select a CAN ID to see and plot the available Suspect Pameter Numbers.")
        self.control_box_layout.addWidget(label)
        
        #Setup the area for plotting SPNs
        self.info_scroll_area = QScrollArea()
        self.info_scroll_area.setWidgetResizable(True)
        #Create the container widget
        self.info_box = QGroupBox("Suspect Paramter Number (SPN) Information")
        #put the container widget into the scroll area
        self.info_scroll_area.setWidget(self.info_box)
        #create a layout strategy for the container 
        self.info_box_layout = QVBoxLayout()
        #assign the layout strategy to the container
        self.info_box.setLayout(self.info_box_layout)
        #set the layout so labels are at the top
        self.info_box_layout.setAlignment(Qt.AlignTop)
        
       
        
        #Ignore the box creation for now, since the graph box would just have 1 widget
        #graph_box = QGroupBox("Plots")

        #Now we can set all the previously defined boxes into the main window
        self.grid_layout = QGridLayout(self.main_widget)
        self.grid_layout.addWidget(can_id_box,0,0,1,2) 
        self.grid_layout.addWidget(self.control_scroll_area,1,0)
        self.grid_layout.addWidget(self.info_scroll_area,1,1)
        self.grid_layout.addWidget(self.graph_canvas,1,2) 
        self.grid_layout.addWidget(table_box,0,2)
        self.grid_layout.setRowStretch(0, 3)
        
        self.main_widget.setLayout(self.grid_layout)
        self.setCentralWidget(self.main_widget)
        
        self.setWindowTitle(program_title)
        self.show()
        self.load_table("example.bin")
        
    def show_about_dialog(self):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)
        msg.setText("{}\nVersion: {}\nWritten by:\n\t{}\n\t{}\n\t{}".format(
                program_title,
                program_version,
                program_author,
                program_position,
                program_affiliation))
        msg.setInformativeText("""Windows icons by Icons8\nhttps://icons8.com/download-huge-windows8-set""")
        msg.setWindowTitle("About")
        msg.setDetailedText("The full path of the file is \n{}".format(os.path.abspath(os.getcwd())))
        msg.setStandardButtons(QMessageBox.Ok)
        msg.setWindowModality(Qt.ApplicationModal)
        msg.exec_() 
        
        
    def load_data(self):
        print("Loading Data")
        options = QFileDialog.Options()
        options |= QFileDialog.Detail
        data_file_name,data_file_type = QFileDialog.getOpenFileName(self,
                                        "Open Binary Log File", 
                                        #"{}".format(self.home_directory),
                                        self.home_directory,
                                        "Binary Log Files (*.bin);;All Files (*)",
                                        options=options)
        if data_file_name:
            print(data_file_name)
            
            
            self.statusBar().showMessage(
                    "Successfully Opened {}.".format(data_file_name))
            self.load_table(data_file_name)
        else:
           self.statusBar().showMessage("Failed to open file.")
        
    def load_table(self,data_file_name):        
        #This may be a long process, so let's show a progress bar:
        
        
        fileLocation = 0
        startTime = None
        file_size = os.path.getsize(data_file_name)
        print("File size is {} bytes".format(file_size))

        self.loading_progress = QProgressDialog(self)
        self.loading_progress.setMinimumWidth(300)
        self.loading_progress.setWindowTitle("Loading and Converting Binary")
        self.loading_progress.setMinimumDuration(0)
        self.loading_progress.setMaximum(file_size)
        self.loading_progress.setWindowModality(Qt.ApplicationModal)

        self.message_list = []
        self.ID_dict={}
        self.PGN_dict={}

        with open(data_file_name,'rb') as binFile:         
            while (fileLocation < file_size):
                block = binFile.read(512)
                fileLocation+=512
                binFile.seek(fileLocation)
                #print(".",end='')
                self.loading_progress.setValue(fileLocation)
                if self.loading_progress.wasCanceled():
                    break
                for recordNum in range(21):
                    record = block[4+recordNum*24:4+(recordNum+1)*24]

                    
                    timeSeconds = struct.unpack("<L",record[0:4])[0]
                    timeMicrosecondsAndDLC = struct.unpack("<L",record[8:12])[0]
                    timeMicroseconds = timeMicrosecondsAndDLC & 0x00FFFFFF

                    message = {}
                    message["Real Time"] = timeSeconds + timeMicroseconds * 0.000001
                    message["DLC"] = (timeMicrosecondsAndDLC & 0xFF000000) >> 24
                    message["Logger Micros"] = struct.unpack("<L",record[4:8])[0]
                    message["ID"] = struct.unpack("<L",record[12:16])[0]
                    message["Bytes"] = record[16:24]
                    message["Payload"] = struct.unpack("BBBBBBBB",record[16:24])
                    message["SA"] = (0x000000FF & message["ID"])
                    message["PF"] = (0x00FF0000 & message["ID"]) >> 16
                    message["DA"] = (0x0000FF00 & message["ID"]) >> 8
                    
                    if message["PF"] >= 240: #PDU2 format
                      message["PGN"] = message["PF"]*256+message["DA"]
                      message["DA"] = 0xFF
                    else:
                      message["PGN"] = message["PF"]*256

                    if startTime == None:
                        startTime=timeSeconds + timeMicroseconds * 0.000001
                    
                    message["Rel Time"] = message["Real Time"]-startTime
                    
                    self.message_list.append(message)
                    
                    if message["ID"] in self.ID_dict.keys():
                        #Count and append to the existing field
                        self.ID_dict[message["ID"]]["Count"] += 1
                        self.ID_dict[message["ID"]]["Data"].append((message["Rel Time"],message["Bytes"]))
                        self.ID_dict[message["ID"]]["EndTime"]=message["Real Time"]
                    else:
                        #initialize the data field
                        self.ID_dict[message["ID"]] = {"Count":1,
                                                       "StartTime":message["Real Time"],
                                                       "EndTime":message["Real Time"],
                                                       "DLC":message["DLC"],
                                                       "PGN":message["PGN"],
                                                       "PGN Name":"",
                                                       "SA":message["SA"],
                                                       "DA":message["DA"],
                                                       "Data":[ (message["Rel Time"],message["Bytes"]) ]}
        self.load_can_id_table()
        
    def load_can_id_table(self):
        #load the J1939 Database
        with open("J1939db.json",'r') as j1939_file:
            self.j1939db = json.load(j1939_file)
        pgn_names = self.j1939db["J1939PGNdb"]
        source_address_names = self.j1939db["J1939SATabledb"]
        
        #Set the headers
        self.can_id_table_columns = ["Hex CAN ID", "PGN","Acronym","DA","SA","Source","Count","Period (ms)", "Freq. (Hz)"]
        self.can_id_table.setColumnCount(len(self.can_id_table_columns))
        self.can_id_table.setHorizontalHeaderLabels(self.can_id_table_columns)
        self.can_id_table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        self.can_id_table.clearContents()
        
        self.can_id_table.setRowCount(len(self.ID_dict))
        row = 0
        for key,item in sorted(self.ID_dict.items()):
            #print(item["PGN"])
            
            if item["DA"] == 255:
                DA_entry = "(255)"
                DA_name = "All"
            else:
                DA_entry = "{:3d}".format(item["DA"])
                try:
                    DA_name = source_address_names[DA_entry.strip()] #Clean whitespace off the string
                except KeyError:
                    DA_name = "Unknown"
                    
            SA_entry = "{:3d}".format(item["SA"])
            try:
                SA_name = source_address_names[SA_entry.strip()] #Clean whitespace off the string
            except KeyError:
                SA_name = "Unknown"
                
            period = 1000*(item["EndTime"] - item["StartTime"])/item["Count"]
            if period > 0:
                frequency = 1000/period
            else:
                frequency = 0
            self.ID_dict[key]["Frequency"] = frequency
            self.ID_dict[key]["Period"] = period #in Milliseconds

            try:
                acronym = pgn_names["{}".format(item["PGN"])]["Label"]
            except KeyError:
                acronym = "Unknown"
            SPNs = pgn_names["{}".format(item["PGN"])]["SPNs"]
            
            row_values = ["{:08X}".format(key),
                          "{:8d}".format(item["PGN"]),
                          acronym,
                          DA_entry,
                          #DA_name,
                          SA_entry,
                          SA_name,
                          "{:12d}".format(item["Count"]),
                          "{:5d}".format(int(period)),
                          "{:9.3f}".format(frequency),
                          ]
            for col in range(self.can_id_table.columnCount()):
                entry = QTableWidgetItem(row_values[col])
                entry.setFlags(entry.flags() & ~Qt.ItemIsEditable)
                self.can_id_table.setItem(row,col,entry)
            row += 1
            self.statusBar().showMessage("Found {} unique IDs.".format(row))            
            self.data_table.resizeColumnsToContents()

        self.id_selection_list=[] #create an empty list
        self.can_id_table.setSortingEnabled(True)
        self.can_id_table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.can_id_table.doubleClicked.connect(self.load_message_table)
        self.can_id_table.itemSelectionChanged.connect(self.create_spn_plot_buttons)
        self.can_id_table.resizeColumnsToContents()
        self.find_transport_pgns()
        
   
        
        
    def plot_SPN(self,spn,id_key):
        if self.spn_plot_checkbox[spn].isChecked():
            #look up items in the database
            name = self.j1939db["J1939SPNdb"]["{}".format(spn)]["Name"]
            units = self.j1939db["J1939SPNdb"]["{}".format(spn)]["Units"]
            spn_start = self.j1939db["J1939SPNdb"]["{}".format(spn)]["StartBit"]
            spn_end = self.j1939db["J1939SPNdb"]["{}".format(spn)]["EndBit"]                  
            spn_length = self.j1939db["J1939SPNdb"]["{}".format(spn)]["SPNLength"]
            scale = self.j1939db["J1939SPNdb"]["{}".format(spn)]["Resolution"]
            offset = self.j1939db["J1939SPNdb"]["{}".format(spn)]["Offset"]
            
            if spn_length <= 8:
                fmt = "B"
                rev_fmt = "B"
            elif spn_length <= 16:
                fmt = ">H"
                rev_fmt = "<H"
            elif spn_length <= 32:
                fmt = ">L"
                rev_fmt = "<L"
            elif spn_length <= 64:
                fmt = ">Q"
                rev_fmt = "<Q"
            else:
                self.statusBar().showMessage("Not a plottable SPN.")  
                return

            shift = 64 - spn_start - spn_length
            mask = 0
            for m in range(spn_length):
                mask += 1 << (63 - m - spn_start) 
            #print("Mask: 0x{:016X}".format(mask)) 
            if scale <= 0:
                scale = 1
                
            self.info_box_layout.addWidget(QLabel("SPN {}: {}".format(spn,name)))
            self.info_box_layout.addWidget(QLabel("  Resolution: {}, Units: {}".format(scale,units)))
            self.info_box_layout.addWidget(QLabel("  Offset: {} {}, Length: {} bits".format(offset,units,spn_length)))
            self.info_box_layout.addWidget(QLabel("  Start Bit: {}, End Bit: {}".format(spn_start,spn_end)))
            self.info_box_layout.addWidget(QLabel("  Mask: 0x{:016X}".format(mask)))                              
            
            times = []
            values = []
            for entry in self.ID_dict[id_key]["Data"]:
                #print(entry)
                times.append(entry[0])
                #print("Entry: " + "".join("{:02X} ".format(d) for d in entry[1]))
                decimal_value = struct.unpack(">Q",entry[1])[0] & mask
                #the < takes care of reverse byte orders
                #print("masked decimal_value: {:08X}".format(decimal_value ))
                shifted_decimal = decimal_value >> shift
                #reverse the byte order
                reversed_decimal = struct.unpack(fmt,struct.pack(rev_fmt,shifted_decimal))[0]
                #print("shifted_decimal: {:08X}".format(shifted_decimal))
                spn_value = reversed_decimal * scale + offset
                #print("SPN value: {}\n".format(spn_value))
                values.append(spn_value)
            #Plot the data
            self.graph_canvas.plot_data(times,values,"SPN {}".format(spn))
            self.graph_canvas.title("")
            self.graph_canvas.xlabel("Time (sec)")
            self.graph_canvas.ylabel("{} ({})".format(name,units))
            
            #Disable the button to show it's been plotted
            while self.spn_plot_checkbox[spn].isChecked() == False:
                self.spn_plot_checkbox[spn].setChecked(True)
            self.spn_plot_checkbox[spn].setEnabled(False)
            
            
    def clear_plots(self):
        self.graph_canvas.clear()
        #Clear the information box
        while self.info_box_layout.count():
            item = self.info_box_layout.takeAt(0)
            item.widget().deleteLater()

        for spn in sorted(self.spn_list):
            self.spn_plot_checkbox[spn].setEnabled(True)
            self.spn_plot_checkbox[spn].setChecked(False)
            
    def create_spn_plot_buttons(self):
        try:
            self.clear_plots()
        except:
            pass
        #print("getting selection")
        row_indicies = self.can_id_table.selectionModel().selectedRows()
        self.id_selection_list=[]
        for index in row_indicies:
            #print(index.row(),end=' ')
            id_item = self.can_id_table.item(index.row(), 0)
            #print(id_item.text())
            self.id_selection_list.append(id_item.text())

        # clear a layout and delete all widgets
        while self.control_box_layout.count():
            item = self.control_box_layout.takeAt(0)
            item.widget().deleteLater()
        #add a clear plot button
        clear_button = QPushButton("Clear and Reset Plot",self)
        clear_button.clicked.connect(self.clear_plots)
        self.control_box_layout.addWidget(clear_button)
    
        self.spn_list=[]
        
        self.spn_plot_checkbox={}
        for id_text in self.id_selection_list:
            #we need to look up the PGN that was put into the ID_dict. The key was the ID as an integer
            id_key=int(id_text,16)
            pgn = self.ID_dict[id_key]["PGN"]
            #print("PGN: {}".format(pgn))
            for spn in sorted(self.j1939db["J1939PGNdb"]["{}".format(pgn)]["SPNs"]):
                spn_name = self.j1939db["J1939SPNdb"]["{}".format(spn)]["Name"]
                self.spn_list.append(spn)
                self.spn_plot_checkbox[spn]= QCheckBox("Plot SPN {}: {}".format(spn,spn_name),self)
                self.spn_plot_checkbox[spn].stateChanged.connect(partial(self.plot_SPN,spn,id_key)) #We need to pass the SPN to the plotter
        for spn in sorted(self.spn_list):
            self.control_box_layout.addWidget(self.spn_plot_checkbox[spn])

        #set newly updated widget to display in the scroll box
        self.control_scroll_area.setWidget(self.control_box)
        
    def load_message_table(self):
        if len(self.id_selection_list) == 0:
            #don't do anything if there's nothing selected.
            return
        
        #Do some preliminary Table operations
        self.data_table.clear()
        self.data_table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        self.data_table.setSortingEnabled(False)
        self.data_table.setRowCount(len(self.message_list))            

        self.loading_table_progress = QProgressDialog(self)
        self.loading_table_progress.setMinimumWidth(300)
        self.loading_table_progress.setWindowTitle("Filling Table with Data")
        self.loading_table_progress.setMinimumDuration(3000)
        self.loading_table_progress.setMaximum(len(self.message_list)-1)
        self.loading_table_progress.setWindowModality(Qt.ApplicationModal)

        
        #Set the headers
        data_table_columns = ["Real Time", "Rel. Time[s]","Micros", "CAN ID", "PGN","DA","SA","DLC"]
        for i in range(8):
           data_table_columns.append("B{}".format(i))
        self.data_table.setColumnCount(len(data_table_columns))
        self.data_table.setHorizontalHeaderLabels(data_table_columns)
        self.data_table.resizeColumnsToContents()
        
        #Load the data
        filled_rows = 0
        for row in range(len(self.message_list)):
            self.loading_table_progress.setValue(row)
            if self.loading_table_progress.wasCanceled():
                break
            #self.data_table.insertRow(row) #this is slow
            id_text = "{:08X}".format(self.message_list[row]["ID"])
            if id_text in self.id_selection_list: #This list was created in a different function
                row_values = [time.strftime("%Y-%m-%d %H:%M:%S",
                                            time.localtime(self.message_list[row]["Real Time"]))+
                                              "{:.6f}".format(self.message_list[row]["Real Time"]%1)[1:],
                              "{:0.6f}".format(self.message_list[row]["Rel Time"]), 
                              self.message_list[row]["Logger Micros"],
                              id_text,
                              self.message_list[row]["PGN"],
                              self.message_list[row]["DA"],
                              self.message_list[row]["SA"],
                              self.message_list[row]["DLC"]]
                for i in range(len(self.message_list[row]["Payload"])):
                    row_values.append("{:02X}".format(self.message_list[row]["Payload"][i]))
                for col in range(len(data_table_columns)):
                    entry = QTableWidgetItem("{}".format(row_values[col]))
                    entry.setFlags(entry.flags() & ~Qt.ItemIsEditable)
                    self.data_table.setItem(filled_rows,col,entry)
                filled_rows+=1
        self.data_table.setRowCount(filled_rows)
        
        self.statusBar().showMessage("Filled {} rows.".format(filled_rows))            
        self.data_table.resizeColumnsToContents()
        #self.data_table.setSortingEnabled(True)
        
    def find_transport_pgns(self):

        loading_transport_progress = QProgressDialog(self)
        loading_transport_progress.setMinimumWidth(500)
        loading_transport_progress.setWindowTitle("Looking for J1939 Transport Layer Messages")
        loading_transport_progress.setMinimumDuration(0)
        loading_transport_progress.setMaximum(len(self.message_list)-1)
        loading_transport_progress.setWindowModality(Qt.ApplicationModal)

        self.id_selection_list = []
        for trial_id in self.ID_dict.keys():
            #print("{:08X}".format(trial_id & 0x00FF0000))
            if (trial_id & 0x00FF0000) == 0x00EC0000:
                self.id_selection_list.append("{:08X}".format(trial_id))
            if (trial_id & 0x00FF0000) == 0x00EB0000:
                self.id_selection_list.append("{:08X}".format(trial_id))
        #print(self.id_selection_list)
        self.load_message_table()
        #Load the data
        filled_rows = 0
        self.BAMs = {}
        new_pgn = {}
        new_data = {}
        new_packets = {}
        new_length = {}
        BAM_byte_counter = 0
        for row in range(len(self.message_list)):
            loading_transport_progress.setValue(row)
            if loading_transport_progress.wasCanceled():
                break
            #self.data_table.insertRow(row) #this is slow
            trial_id = self.message_list[row]["ID"]
            sa = self.message_list[row]["SA"]
            if (trial_id & 0x00FF0000) == 0x00EC0000: #connection management message
                message = self.message_list[row]["Bytes"]
    
                if message[0] == 32: #BAM,CTS
                    new_pgn[sa] = (message[7] << 16) + (message[6] << 8) + message[5]
                    new_length[sa] = (message[2] << 8) + message[1]
                    new_packets[sa] = message[3]
                    new_data[sa] = [0xFF for i in range(7*new_packets[sa])]
                    
            elif (trial_id & 0x00FF0000) == 0x00EB0000: # Data Transfer
                message = self.message_list[row]["Bytes"]
                #print("{:08X}".format(trial_id) + "".join(" {:02X}".format(d) for d in message))
                if sa in new_data.keys():
                    for b,i in zip(message[1:],range(7)):
                        new_data[sa][i+7*(message[0]-1)] = b
                    if message[0] == new_packets[sa]:
                        data_bytes = bytes(new_data[sa][0:new_length[sa]])
                        if sa in self.BAMs.keys():
                            self.BAMs[sa].append( (self.message_list[row]["Real Time"],data_bytes,new_pgn[sa]) )
                        else:
                            self.BAMs[sa]= [ (self.message_list[row]["Real Time"],data_bytes,new_pgn[sa]) ]
                
        
        print(self.BAMs)
        
    def compute_stats(self):
        
        #setup array
        item_list=[]
        items = self.data_table.selectedItems()
        for item in items:
            try:
                item_list.append(float(item.text()))
            except:
                pass
        
        if len(item_list) > 1: #Check to see if there are 2 or more samples
            data_array = np.asarray(item_list)
            mean_value = np.mean(data_array)
            stdev_value = np.std(data_array)
            
            print("Mean = {0:5f}".format(mean_value))
            self.mean_label.setText("Mean = {:0.3f}".format(mean_value))
            self.std_label.setText("Std Dev = {:0.4f}".format(stdev_value))
            
            self.graph_canvas.plot_histogram(data_array)
            if self.normal_checkbox.isChecked():
                self.graph_canvas.plot_normal(mean_value,stdev_value)

 
        

def excepthook(excType, excValue, tracebackobj):
    """
    Global function to catch unhandled exceptions.
    
    @param excType exception type
    @param excValue exception value
    @param tracebackobj traceback object
    """
    separator = '-' * 80
    logFile = "simple.log"
    notice = \
        """An unhandled exception occurred. Please report the problem\n"""\
        """using the error reporting dialog or via email to <%s>.\n"""\
        """A log has been written to "%s".\n\nError information:\n""" % \
        ("yourmail at server.com", "")
    versionInfo="0.0.1"
    timeString = time.strftime("%Y-%m-%d, %H:%M:%S")
    
    
    tbinfofile = cStringIO.StringIO()
    traceback.print_tb(tracebackobj, None, tbinfofile)
    tbinfofile.seek(0)
    tbinfo = tbinfofile.read()
    errmsg = '%s: \n%s' % (str(excType), str(excValue))
    sections = [separator, timeString, separator, errmsg, separator, tbinfo]
    msg = '\n'.join(sections)
    try:
        f = open(logFile, "w")
        f.write(msg)
        f.write(versionInfo)
        f.close()
    except IOError:
        pass
    errorbox = QtGui.QMessageBox()
    errorbox.setText(str(notice)+str(msg)+str(versionInfo))
    errorbox.exec_()
sys.excepthook = excepthook
    
if __name__ == '__main__':
    #Start the program this way according to https://stackoverflow.com/questions/40094086/python-kernel-dies-for-second-run-of-pyqt5-gui
    app = QCoreApplication.instance()
    if app is None:
        app = QApplication(sys.argv)
    execute = CANDecoderMainWindow()
    sys.exit(app.exec_())
