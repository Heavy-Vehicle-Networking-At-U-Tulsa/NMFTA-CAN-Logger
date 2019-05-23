# -*- coding: utf-8 -*-
"""
Created on Sat Sep  2 07:52:53 2017

@author: dailyadmin
"""
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QSizePolicy,
                             QMessageBox,
                             QFileDialog)
from matplotlib.backends import qt_compat
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from matplotlib import rcParams
import matplotlib.mlab as mlab
import os

class MyMplCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""
    def __init__(self, parent=None, width=5, height=4, dpi=100):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = self.fig.add_subplot(111)
        
        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        FigureCanvas.mpl_connect(self,'button_press_event', self.export)
        
    def export(self,event):
        """This is a utility that will prodxuce a pdf of the graph"""
        filename = "ExportedGraph.pdf"
        options = QFileDialog.Options()
        options |= QFileDialog.Detail
        filename,_ = QFileDialog.getSaveFileName(self,
                                        "Save Plot Graphics File", 
                                        #"{}".format(self.home_directory),
                                        os.getcwd(),
                                        "PDF Files (*.pdf);;PNG Graphics (*.png);;All Files (*)",
                                        options=options)
        print("Saving {}".format(filename))
        if filename:
            self.fig.savefig(filename)
##            msg = QMessageBox()
##            msg.setIcon(QMessageBox.Information)
##            msg.setText("Saved a copy of the graphics window to {}".format(filename))
##            msg.setWindowTitle("Saved PDF File")
##            msg.setDetailedText("The full path of the file is \n{}".format(os.path.abspath(os.getcwd())))
##            msg.setStandardButtons(QMessageBox.Ok)
##            msg.setWindowModality(Qt.ApplicationModal)
##            msg.exec_()
##            print("Exported PDF file")
        
class MyDynamicMplCanvas(MyMplCanvas):
    """A canvas that updates itself frequently with a new plot."""
    def __init__(self, *args, **kwargs):
        MyMplCanvas.__init__(self, *args, **kwargs)
        self.axes.set_xlabel("Time(s)")
        self.axes.set_ylabel("Y Label")
        self.axes.set_title("Title")
             
    def plot_histogram(self,data_array,data_label="Temperature",
                       title="Probability Density Function Plots",bins=50):
        self.axes.cla() #Clear axes
        self.axes.hist(data_array,bins=bins,
                       normed=True,label="Emperical",
                       edgecolor='b',color='y')
        self.axes.set_xlabel(data_label)
        self.axes.set_ylabel("Estimated Prob. Density Funct.")
        self.axes.set_title(title)
        self.axes.legend(shadow=True)
        self.draw()
        print("Finished Drawing Normalized Histogram.")
          
    def plot_normal(self,mu,sigma):
        xmin,xmax = self.axes.get_xlim()
        x = np.linspace(mu-3*sigma,mu+3*sigma, 100)
        y = mlab.normpdf(x, mu, sigma)
        self.axes.plot(x,y,label="Normal")
        self.axes.legend(shadow=True)
        self.draw()
        print("Finished Drawing Normal Distribution.")
    def plot_data(self,x,y,legend_label):
        self.axes.plot(x,y,label=legend_label)
        self.axes.legend(shadow=True)
        self.draw()
    def clear(self):
        self.axes.cla()
        self.draw()
    def title(self,title_string="Title"):
        self.axes.set_title(title_string)
        self.draw()
    def xlabel(self,xlabel_string="Time (s)"):
        self.axes.set_xlabel(xlabel_string)
        self.draw()
    def ylabel(self,ylabel_string="Units"):
        self.axes.set_ylabel(ylabel_string)
        self.draw()
    
