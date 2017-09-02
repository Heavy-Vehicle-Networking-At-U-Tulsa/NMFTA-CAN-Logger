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
        
        
        self.main_widget = QWidget()
        
        self.setGeometry(200,200,1600,900)

        self.load_button = QPushButton('Load Data',self)
        self.load_button.clicked.connect(self.load_message_table)
     
        self.mean_label = QLabel("Mean: Not Computed Yet",self)
        self.std_label = QLabel("Std Dev: Not Computed Yet",self)
        
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
        table_box_layout.addWidget(self.load_button)
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
        
        #repeat the box layout for Statistics
        stats_box = QGroupBox("Summary Statistics")
        stats_box_layout = QVBoxLayout()
        stats_box_layout.addWidget(self.mean_label)
        stats_box_layout.addWidget(self.std_label)
        stats_box.setLayout(stats_box_layout)

        #Ignore the box creation for now, since the graph box would just have 1 widget
        #graph_box = QGroupBox("Data and Probability Graph")
        
        #Create some distribution options
        #Start with creating a check button.
        self.normal_checkbox = QCheckBox('Normal Distribution',self)
        # We want to run the plotting routine for the distribution, but 
        # we need to know the statistical values, so we'll calculate statistics
        # first.
        self.normal_checkbox.stateChanged.connect(self.compute_stats)
        
        #Repeat for additional distributions.
        self.log_normal_checkbox = QCheckBox('Log-Normal Distribution',self)
        
        distribution_box = QGroupBox("Distribution Functions")
        distribution_box_layout= QVBoxLayout()
        distribution_box_layout.addWidget(self.normal_checkbox)
        distribution_box_layout.addWidget(self.log_normal_checkbox)
        distribution_box.setLayout(distribution_box_layout)

        #Now we can set all the previously defined boxes into the main window
        self.grid_layout = QGridLayout(self.main_widget)
        self.grid_layout.addWidget(can_id_box,0,0) 
        self.grid_layout.addWidget(stats_box,1,1)
        self.grid_layout.addWidget(self.graph_canvas,1,0) 
        self.grid_layout.addWidget(table_box,0,1)
        
        self.main_widget.setLayout(self.grid_layout)
        self.setCentralWidget(self.main_widget)
        
        self.setWindowTitle(program_title)
        #self.activateWindow()
        #self.raise_()
        self.show()
        
        
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
                        self.ID_dict[message["ID"]]["Data"].append((message["Rel Time"],message["Payload"]))
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
                                                       "Data":[ (message["Rel Time"],message["Payload"]) ]}
        self.load_can_id_table()
        
    def load_can_id_table(self):
        #Set the headers
        self.can_id_table_columns = ["Hex CAN ID", "PGN","DA", "SA","Count","Period (ms)", "Freq. (Hz)",]
        self.can_id_table.setColumnCount(len(self.can_id_table_columns))
        self.can_id_table.setHorizontalHeaderLabels(self.can_id_table_columns)
        self.can_id_table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        self.can_id_table.clearContents()
        
        self.can_id_table.setRowCount(len(self.ID_dict))
        row = 0
        for key,item in sorted(self.ID_dict.items()):
            #print(key)
            if item["DA"] == 255:
                DA_entry = ""
            else:
                DA_entry = "{}".format(item["DA"])

            period = 1000*(item["EndTime"] - item["StartTime"])/item["Count"]
            if period > 0:
                frequency = 1000/period
            else:
                frequency = 0
            self.ID_dict[key]["Frequency"] = frequency
            self.ID_dict[key]["Period"] = period #in Milliseconds
            
            row_values = ["{:08X}".format(key),
                          "{:8d}".format(item["PGN"]), 
                          DA_entry,
                          "{:3d}".format(item["SA"]),
                          "{:12d}".format(item["Count"]),
                          "{:5d}".format(int(period)),
                          "{:9.3f}".format(frequency)]
            for col in range(self.can_id_table.columnCount()):
                entry = QTableWidgetItem(row_values[col])
                self.can_id_table.setItem(row,col,entry)
            row += 1
            self.statusBar().showMessage("Found {} unique IDs.".format(row))            
            self.data_table.resizeColumnsToContents()

        self.can_id_table.setSortingEnabled(True)
        self.can_id_table.setSelectionBehavior(QAbstractItemView.SelectRows)
        #self.can_id_table.doubleClicked.connect(self.load_message_table)
        self.can_id_table.resizeColumnsToContents()
        
    def load_message_table(self):

        #print("getting selection")
        item_list=[]
        row_indicies = self.can_id_table.selectionModel().selectedRows()
        print(row_indicies)
        id_selection_list=[]
        for index in row_indicies:
            print(index.row(),end=' ')
            id_item = self.can_id_table.item(index.row(), 0)
            print(id_item.text())
            id_selection_list.append(id_item.text())
        #print(id_selection_list)
        
        self.data_table.clear()
        self.data_table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        
        self.data_table.setRowCount(len(self.message_list))            

        self.loading_table_progress = QProgressDialog(self)
        self.loading_table_progress.setMinimumWidth(300)
        self.loading_table_progress.setWindowTitle("Filling Table with Data")
        self.loading_table_progress.setMinimumDuration(4)
        self.loading_table_progress.setMaximum(len(self.message_list)-1)
        self.loading_table_progress.setWindowModality(Qt.ApplicationModal)

        self.data_table.setSortingEnabled(False)
        #Set the headers
        data_table_columns = ["Real Time", "Rel. Time[s]","Micros", "CAN ID", "PGN","DA","SA","DLC"]
        for i in range(8):
           data_table_columns.append("B{}".format(i))
        self.data_table.setColumnCount(len(data_table_columns))
        self.data_table.setHorizontalHeaderLabels(data_table_columns)
        #Load the data
        filled_rows = 0
        for row in range(len(self.message_list)):
            self.loading_table_progress.setValue(row)
            if self.loading_table_progress.wasCanceled():
                break
            #self.data_table.insertRow(row) #this is slow
            id_text = "{:08X}".format(self.message_list[row]["ID"])
            if id_text in id_selection_list:
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
