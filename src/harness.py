# Module Imports

import os
import sys
import datetime
import json
from flask import Flask
from flask import request
from flask import render_template
from flask import send_from_directory
from flask import redirect, url_for, flash

app = Flask(__name__)

# end points

@app.route('/savescan', methods=['GET', 'POST'])
def poster_action():

    #jsonData = request.get_json()
    #print( jsonData['serialnumber'])
    #print( jsonData['technician'])

    datetime=request.form.get("datetime")
    serialnumber=request.form.get("serialnumber")
    technician=request.form.get("technician")
    passfail=request.form.get("passfail")
    results=request.form.get("results")

    with open("harness_serials.csv", "a") as myfile:
        myfile.write(str(serialnumber)+","+str(technician)+","+str(passfail)+","+str(datetime)+"\r\n")
    
    print(str(serialnumber),end="")
    print(","+str(technician),end="")
    print(","+str(passfail),end="")
    print(","+str(datetime))
    #print("Results       "+str(results))


    response = app.response_class(
        response='{"OK":"1"}' , 
        status=200,
        mimetype='application/json'
    ) 

#    response = "OK"
#    response.headers.add('Access-Control-Allow-Origin', '*')
    return response

#
# main()
#

if __name__ == "__main__":
    app.run(host='0.0.0.0', port='8086')

print("The app has stopped running")
