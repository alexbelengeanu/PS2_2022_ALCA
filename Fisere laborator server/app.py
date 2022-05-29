from datetime import datetime
from flask import Flask
from flask import request

import smtplib, ssl
import serial
app = Flask(__name__)

ser = serial.Serial('COM3')
print(ser.name) 

def send_leak_mail():
	message = """S-a detectat o inundatie!"""
	# Important in Gmail, enable la POP3, IMAP si bifati https://myaccount.google.com/u/2/lesssecureapp 
	context = ssl.create_default_context()
	with smtplib.SMTP("smtp.gmail.com", 587) as server:
		server.starttls(context=context)
		server.login('ps.alca2022@gmail.com', '1234567890alca')
		server.sendmail('ps.alca2022@gmail.com', "anca.copos01@gmail.com", message)
    

@app.route('/')
def hello_world():
    text = 'Proiect Sincretic 2022'
    temp = '- Temperatura este '
    ser.flushInput()
    temp_serial = ser.readline()
    temp_serial = temp_serial.decode()
    print(temp_serial)
	
    ind_serial = ser.readline()
    ind_serial = ind_serial.decode()
    print(ind_serial)
    print(f'Este inundatie : {str(ind_serial).find("!INUNDATIE!") != -1}')
    if str(ind_serial).find("!INUNDATIE!") != -1:
        send_leak_mail()
        data = str(datetime.now()).split(".")[0]
        mesaj = "5 " + data
        print(mesaj)
        ser.write(mesaj.encode())
    
		
    string_butoane = '<p>LED1 State:<button onclick="document.location=\'led_off\'">LED OFF</button> <button onclick="document.location=\'led_on\'">LED ON</button></p>'
    color_picker = '<p>LED2 RGB Selector: <form method=\"get\" action=\"color\"><input name=\"colpicker\" type=\"color\"/> <input type=\"submit\" value=\"send\"></form></p>'
    text_form = '<p>Afiseaza text pe display: <form method=\"get\" action=\"mesaj\"><input name=\"msg\" type=\"text\"/> <input type=\"submit\" value=\"send\"></form></p>'

    return text + temp + temp_serial + string_butoane + color_picker + text_form
    
@app.route('/led_on')
def led_on():
    ser.write("1 A".encode())
    return "Am aprins ledul"

@app.route('/led_off')
def led_off():
    ser.write("1 S".encode())
    return "Am stins ledul"

@app.route('/color')
def color_picker():
    color=str(request.args['colpicker'])
    red = int("0x" + color[1:3], 16) * 99/255.0
    green = int("0x" + color[3:5], 16) * 99/255.0
    blue = int("0x" + color[5:7], 16) * 99/255.0
    
    color="2 " + str(int(red)).zfill(2) + str(int(green)).zfill(2) + str(int(blue)).zfill(2)
    print(color)
    ser.write(color.encode())
    return "Am modificat culoarea RGB"

@app.route('/mesaj')
def message_parser():
    mesaj = str(request.args['msg'])
    mesaj_serial = "6 " + mesaj
    ser.write(mesaj_serial.encode())
    return "Am transmis mesajul " + mesaj


