# UKHC Metadata Generator Entrance Script
# Author(s):
#   Abdallah F. Sher (abdallah.f.sher@gmail.com)
# Description: Entry point for application
# Last Modified: Jan. 27, 2025

from PyQt6.QtWidgets import QApplication, QWidget
import sys

# sys.argv contains the list of command-line arguments passed to a Python script
# could be used to create command-line interface for the application, enabling scripting
app = QApplication(sys.argv)

window = QWidget()
window.show()

app.exec()