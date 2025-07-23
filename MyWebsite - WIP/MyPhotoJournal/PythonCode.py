#https://www.digitalocean.com/community/tutorials/how-to-make-a-web-application-using-flask-in-python-3#step-2-creating-a-base-application-->
#https://realpython.com/python-http-server/#how-to-start-pythons-httpserver-in-the-command-line
#https://www.geeksforgeeks.org/todo-list-app-using-flask-python/
#service workers to fix the background JS issue that makes the latency spike:
##https://stackoverflow.com/questions/18894830/how-to-run-javascript-function-in-background-without-freezing-ui
#bandwidth notes:
##change ping packet size Ping -l (L) command
#DOS Attacks
##https://thepythoncode.com/article/syn-flooding-attack-using-scapy-in-python
##https://thepythoncode.com/article/make-a-network-usage-monitor-in-python#network-usage-per-nic
##https://www.cisa.gov/news-events/news/understanding-denial-service-attacks
##dedicated ports: https://www.geeksforgeeks.org/what-is-ports-in-networking/#


from flask import Flask, request, jsonify, render_template
from prettytable import PrettyTable
import subprocess, re

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('MyPhotoJournalWebsite.LifePlot.html')

CSV_FILE_PATH = 'Data.csv'

###############write data##################
@app.route('/WriteData', methods=['GET'])
def WriteData():

    return 0

################read data#################
@app.route('/ReadData', methods=['GET'])
def ReadData():
    # open csv file
    a = open("read_file.csv", 'r')
    
    # read the csv file
    a = a.readlines()
    
    # Separating the Headers
    l1 = a[0]
    l1 = l1.split(',')
    
    # headers for table
    t = PrettyTable([l1[0], l1[1]])
    
    # Adding the data
    for i in range(1, len(a)) :
        t.add_row(a[i].split(','))
    
    code = t.get_html_string()
    html_file = open('CSVIntoTable.html', 'w')
    html_file = html_file.write(code)
    return 1
    




if __name__ == '__main__':
    app.run(debug=True, port=8000) #development
    #app.run(host='0.0.0.0', port=8000) #utilization by other computers
    ##to make accessible to other computers add $ flask run --host=0.0.0.0 to the command line