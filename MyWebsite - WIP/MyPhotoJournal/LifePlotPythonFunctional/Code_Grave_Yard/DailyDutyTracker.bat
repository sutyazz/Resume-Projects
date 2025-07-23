@echo off
start "Opening CSV file with Visual Studio" "C:\Users\stone\AppData\Local\Programs\Microsoft VS Code\Code.exe" "C:\Users\stone\Desktop\MyPhotoJournal\LifePlotPythonFunctional\RunningData.csv"

pause

taskkill /FI "WINDOWTITLE eq RunningData*" /F

python LifePlotPythonMult.py

start "Opening Plot with Opera" "C:\Users\stone\AppData\Local\Programs\Opera\opera.exe" "C:\Users\stone\Desktop\MyPhotoJournal\LifePlotPythonFunctional\my_plotly_figure.html"

& exit