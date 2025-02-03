# UKHC Metadata Generator Entrance Script
# Author(s):
#   Abdallah F. Sher (abdallah.f.sher@gmail.com)
# Description: Entry point for application
# Last Modified: Jan. 27, 2025

from PyQt6 import QtWidgets
import json
import sys

class Window(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        self.button = QtWidgets.QPushButton('Select a JSON file...')
        self.button.clicked.connect(self.LoadJsonButtonClicked)
        
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self.button)

        self.jsonText = QtWidgets.QLabel()
        self.jsonText.setWordWrap(True)
        self.jsonText.setMinimumSize(400, 200)

        self.jsonScrollArea = QtWidgets.QScrollArea()
        self.jsonScrollArea.setWidgetResizable(True)
        self.jsonScrollArea.setWidget(self.jsonText)
        self.jsonScrollArea.setLayout(QtWidgets.QVBoxLayout())

        layout.addWidget(self.jsonScrollArea)

    def LoadJsonButtonClicked(self):
        print('Button Clicked')
        # options = QtWidgets.QFileDialog.options()
        # options |= QtWidgets.QFileDialog.DontUseNativeDialog
        file_name, _ = QtWidgets.QFileDialog.getOpenFileName(self, 'Open JSON File', '', 'JSON Files (*.json)')
        if file_name:
            print(file_name)
            with open(file_name, 'r') as file:
                data = json.load(file)
                print(data)
            self.jsonText.setText(json.dumps(data, indent=4))
            return data
        return None
        
if __name__ == '__main__':
    # sys.argv contains the list of command-line arguments passed to a Python script
    # could be used to create command-line interface for the application, enabling scripting
    app = QtWidgets.QApplication(sys.argv)

    window = Window()
    window.resize(800, 600)
    window.show()
    sys.exit(app.exec())