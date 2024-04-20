import socket
import sys
import os
from os import path

FX_OP_LOADPHIVECONFIG = 0
FX_OP_PHIVETOOBJ = 1
FX_OP_OBJTOPHIVE = 2
FX_RESP_ERROR = 0
FX_RESP_LOADEDPHIVECONFIG = 1
FX_RESP_OBJANDMATS = 2
FX_RESP_PHIVE = 3

fxsock = None

def btoi(b):
	return int.from_bytes(b, byteorder='little')
	
def itob(i):
	return i.to_bytes(length=4, byteorder='little')

def sendFx(opType, msg_content):
	if(fxsock == None):
		return False
	
	msg = itob(opType) + itob(len(msg_content)) + msg_content
	
	sent_size = 0
	
	while(sent_size < len(msg)):

		try:

			size = fxsock.send(msg[sent_size:])

		except TimeoutError:

			return False

		if(size == 0):
			return False
		
		sent_size+=size

	return True

def recvFxRaw(data_size):
	if(fxsock == None):
		return None

	msg_data = bytes()

	while(len(msg_data) < data_size):

		try:

			data = fxsock.recv(data_size - len(msg_data))

		except TimeoutError:

			return None

		if(len(data) == 0):
			return None
		
		msg_data+=data

	return msg_data

def recvFx():
	if(fxsock == None):
		return [None, None]
	
	base_data = recvFxRaw(8)

	if(base_data == None):
		return [None, None]

	return [btoi(base_data[0:4]), recvFxRaw(btoi(base_data[4:8]))]

def loadPhiveConfig():
	
	try:
		with open('fxconfig.json', 'rb') as f:
			data = f.read()
	except:
		print("Failed to read phive config file!")
		return False

	sendFx(FX_OP_LOADPHIVECONFIG, data)

	resp = recvFx()
	if(resp[0] == FX_RESP_ERROR):
		print(resp[1].decode('utf-8'))
		return False

	if(resp[0] == FX_RESP_LOADEDPHIVECONFIG):
		return True

	return False

def convertPhiveToObj(phivePath, outPath):
	
	with open(phivePath, 'rb') as f:
		data = f.read()
  
	outName = path.basename(phivePath).replace(".Nin_NX_NVN.bphsh", "").replace(".bphsh", "")
		
	sendFx(FX_OP_PHIVETOOBJ, data)

	resp = recvFx()

	if(resp[0] == FX_RESP_ERROR):
		print(resp[1].decode('utf-8'))
		return False
		
	if(resp[0] == FX_RESP_OBJANDMATS):
	 
		objSize = btoi(resp[1][0:4])
		matsSize = btoi(resp[1][4:8])	
  
		with open(path.join(outPath, outName + ".obj"), 'wb') as f:
			f.write(resp[1][8:8+objSize])
   
		with open(path.join(outPath, outName + ".json"), 'wb') as f:
			f.write(resp[1][8+objSize:8+objSize+matsSize])
   
		print("Saved obj and mats to", path.abspath(outPath))
   
		return True

	return False

def convertObjToPhive(objPath, matInfoPath, outPath):
	
	with open(objPath, 'rb') as f:
		co = f.read()
	cj = bytes()
	if(matInfoPath != None):
		with open(matInfoPath, 'rb') as f:
			cj = f.read()
	data = itob(len(co)) + itob(len(cj)) + co + cj
  
	sendFx(FX_OP_OBJTOPHIVE, data)

	resp = recvFx()

	if(resp[0] == FX_RESP_ERROR):
		print(resp[1].decode('utf-8'))
		exit(0)
		
	if(resp[0] == FX_RESP_PHIVE):
	 
		with open(outPath, 'wb') as f:
			f.write(resp[1])
   
		print("Saved bphsh to", path.abspath(outPath))
   
		return True

	return False

def connectToClient():
	
	global fxsock
	
	fxsock = socket.socket(socket.AF_INET,
						socket.SOCK_STREAM)
	fxsock.settimeout(8)
	fxsock.connect(("127.0.0.1", 8778))

if(__name__ == "__main__"):
	
	if(len(sys.argv) < 2 or sys.argv[1] == "-h" or sys.argv[1] == "help"):
		print("Usage(out path optional):")
		print("\nPhive to Obj + Mat Info json:")
		print("py fivex.py -p PHIVEFILEPATH -o OUTDIR")
		print("\nObj + Mat Info to Phive (mat info is optional):")
		print("py fivex.py -obj OBJPATH -mat MATJSONPATH -o OUTFILE")
		print("\nAlternatively, you can drag and drop the file(s) onto the python script.")
		print("For example, drag and dropping a .bphsh file will generate the .obj and .json in its' dir.")
		print("You can also select a .obj and .json mat info file at the same time and drop them and itll make a bphsh.")
		exit(0)
	
	isPhiveToObj = (sys.argv[1] == "-p")
	phivePath = sys.argv[2] if isPhiveToObj and len(sys.argv) >= 2 else None
 
	isObjToPhive = (sys.argv[1] == "-obj")
	objPath = sys.argv[2] if isObjToPhive and len(sys.argv) >= 2 else None
 
	outPath = ""
	matPath = None
 
	if(path.isfile(sys.argv[1])):
		if(sys.argv[1].endswith(".bphsh")):
			isPhiveToObj = True
			phivePath = sys.argv[1]
		if(sys.argv[1].endswith(".obj") or (len(sys.argv) >= 3 and sys.argv[2].endswith(".obj"))):
			isObjToPhive = True
			if(sys.argv[1].endswith(".obj")):
				objPath = sys.argv[1]
				if(len(sys.argv) >= 3):
					matPath = sys.argv[2]
			else:
				matPath = sys.argv[1]
				objPath = sys.argv[2]
	else:
		for i in range(len(sys.argv)):
			if(sys.argv[i] == "-o" and i < len(sys.argv) - 1):
				outPath = sys.argv[i + 1]
			if(sys.argv[i] == "-m" and i < len(sys.argv) - 1):
				matPath = sys.argv[i + 1]
	
	if not isPhiveToObj and not isObjToPhive:
		print("Unknown options, please check -h or help for usage")
		exit(0)
	
	try:
		connectToClient()
	except:
		print("Failed to connect to client! Ensure fivex is running on your emulator.")
		exit(0)
	
	if not loadPhiveConfig():
		print("Failed to load phive config!")
		exit(0)
  
	os.makedirs(path.dirname(path.abspath(outPath)), exist_ok=True)
	
	if(isPhiveToObj):
	 
		if(phivePath == None):
			print("Please give the input file path")
			exit(0)
   
		if(outPath == ""):
			outPath = path.dirname(phivePath)
  
		convertPhiveToObj(phivePath, outPath)
	
	if(isObjToPhive):
	 
		if(objPath == None):
			print("Please give the input file path")
			exit(0)
   
		if(outPath == ""):
			outPath = objPath.replace(".obj", "") + ".bphsh"
		if not outPath.endswith(".bphsh"):
			outPath+=".bphsh"
			
		convertObjToPhive(objPath, matPath, outPath)