#import Python System Libraries
from time import sleep,strftime,time
from datetime import datetime,date
from datetime import timedelta 
# Import RFM9x
import requests
import adafruit_rfm9x

# Configure LoRa Radio
# Import Blinka Libraries
import busio
from digitalio import DigitalInOut, Direction, Pull
import board
import csv
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0)
prev_packet = None
dic=[["flujo","caudal"]]
ahora=datetime.now().replace(microsecond=0)
fecha=date.today()
fechastr=fecha.strftime("%Y-%m-%d")
#print(fechastr)
with open("/home/pi/WaterComsumptionPaipayales/data/tabla"+str(fechastr)+".csv","w",newline="") as file : #Paipayales por cualquier carpeta de archivos a guardarse
	writer = csv.writer(file,delimiter=";")
	writer.writerows(dic)
	while True:
		packet = None
		# check for packet rx
		packet = rfm9x.receive()
		if packet is not None:
			prev_packet = packet
			packet_text = str(prev_packet, "utf-8")
			print(packet_text)
			dato1,dato2= packet_text.split(",")
			sleep(5) #Tiempo de muestreo entre envíos
			enviar = requests.get("https://api.thingspeak.com/update?api_key=CPVEVA5ND36992WC&field1=0"+str(dato1))
			sleep(5) #Tiempo de muestreo entre envíos
			enviar2 = requests.get("https://api.thingspeak.com/update?api_key=CPVEVA5ND36992WC&field2=0"+str(dato2))
			if enviar.status_code==requests.codes.ok:
				if enviar.text !="0":
					print("\ndatos enviado correctamente")
				else:
					print("\ntiempo de espera insuficiente")
			else:
				print("error en el request:",enviar.estatus_code)
			
			#dic=["flujo","caudal"]
			#with open("/home/pi/tabla.csv","w",newline="") as file :
			#writer = csv.writer(file,delimiter=";")
			#writer.writerows(dic2)
			print(str(ahora))
			print()
			print(packet_text)
			file.write(packet_text+","+str(ahora)+"\n")
			print("se guardaron los registros")
			
