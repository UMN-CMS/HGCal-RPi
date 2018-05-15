#!/usr/bin/env python

import socket
import os, fnmatch, sys, io, argparse, subprocess
from thread import *
import threading
import time
import numpy as np

parser =  argparse.ArgumentParser(description='RPI DAQ: a TCP server')
parser.add_argument('-f', '--inputFile', dest="fname", type=str, default=None,
                    help="Raw data file to open")
parser.add_argument('--ev', dest="nEvents", type=int,  default=20, help="Number of events to take")
parser.add_argument('--ped', dest="ped", action="store_true", default=False, help="This is a pedestal Run")
parser.add_argument('--bit', dest="bit", type=int,  choices=[8,32], default=32,
                    help="Format of the input data file (32 or 8 bit)")

opt = parser.parse_args()

nEvents = opt.nEvents

MYHOSTIP = '192.168.222.113'
#MYHOSTIP = '127.0.0.1'
PORTTCP = 55511
HOSTUDP = '' # To be determined from connection
PORTUDP = 55512

workingPath = os.getcwd()
print 'My working directory is: ', workingPath

#PathToData = workingPath+'/'
PathToData = workingPath+'/data/'

executable = workingPath+'/bin/new_rdout.exe'

# Use this for testing (will not run any data-taking, but simply sleep):
if opt.fname!=None:
  executable = workingPath+'/bin/take_a_nap.exe'

t1_stop = threading.Event()

RAW_EV_SIZE = 123152
if opt.bit==8:
  RAW_EV_SIZE = 30787


#EventRate = 20 # Events per spill
#SpillTime = 5  # Duration of the spill in seconds
#InterSpill = 3 # Time between spills in seconds

def moveDataToNFS(PathToNFS='/sv_mount/disk2_2TB/July2017_TB_data/'):

  for fi in os.listdir(PathToData):
    print 'file in data path:', fi						
    if fnmatch.fnmatch(fi, '*RUN_*.*'):
      print 'Moving file..', PathToData+fi, ' to', PathToNFS+fi

      command = ' '.join(['cp', PathToData+fi, PathToNFS+fi, '&&','sudo','rm','-f', PathToData+fi]) 
      pro = os.system(command)
      if pro!=0:
        print "Could not move the file for some reason... fname = ", fi
        
def sendData(sudp, stop_event, runNumber = 3):
  rawData = None

  prefix= 'RUN_'
  if opt.ped:
    prefix='PED_RUN_'
    
  fname = opt.fname
  if fname==None:
    for fi in os.listdir(PathToData):
      print 'Looking for file..', fi
      if fnmatch.fnmatch(fi, prefix+ '%04d'% (runNumber)+'_*.raw'):
        fname = PathToData+fi
        print fi, fname
        break

  if fname==None:
    print 'Cant find a file name to send for run:', runNumber, '\n \t\t is it pedestal:', opt.ped
          
  try:
    f = io.open(fname, "rb")
    print f
  except EnvironmentError:
    print "Can't open file?", opt.fname
    raise
  
  ev=0
  
  while (not stop_event.is_set()):

    rawData = f.read(RAW_EV_SIZE)
        
    #if ev%EventRate==0:
    #  stop_event.wait(InterSpill)

    if len(rawData) < RAW_EV_SIZE:
      print '\n \t We have reached the end of file, or did not read the full event.'
      print '\t We must return to the previous position and wait for more data to come (if any)'
      print 'Or, maybe, that is all data we can get and in that case you must \n \t ** STOP the RUN from EUDAQ **'
      f.seek(-len(rawData), 1)
      # print 'tell after seek:', f.tell()
      time.sleep(3)
      continue
                              
    if rawData=='':
      print 'We reached EOF'
      print 'Will wait for a bit, maybe more data will appear'
      print 'Actualy, THIS SHOULD NEVER HAPPEN. If you see this message -- let me know.'
      time.sleep(3)
      continue
    
    if rawData!=None:
      d = rawData
      print rawData[0].encode('hex') , len(d)
      # d = rawData[evmod*RAW_EV_SIZE:(evmod+1)*RAW_EV_SIZE]
      if opt.bit==8:
        npd = np.fromstring(d, dtype=np.uint8)
        #print len(npd), npd.nbytes, npd
        npd32 = npd.astype(np.uint32)
        #print len(npd32), npd32.nbytes, npd32.nbytes/4, npd32
        half1 = npd32[0:15500]
        half2 = np.append(npd32[15500:], [0x0a0b0c0d]).astype(np.uint32) # this is to check endienness
        # print len(half1), half1.nbytes, len(half2), half2.nbytes, half2.dtype
        # print half2[-3], half2[-2], half2[-1]
      else:
        npd32 = np.fromstring(d, np.uint32)

        half1 = npd32[0:15500]
        half2 = npd32[15500:]

    else:
      d = 'Hello World'

    print 'Submitting data, ev: ', ev,  len(d)

    try:
      # print 'UDP host:', HOSTUDP
      # The data is too large now to send in one go. So, plit it in two packets:
      sudp.sendto(half1, (HOSTUDP,PORTUDP))
      time.sleep(0.01)
      sudp.sendto(half2, (HOSTUDP,PORTUDP))
      #conn.sendall('-->>>  Here is your data <<< --')
    except socket.error:
      print 'The client socket is probably closed. We stop sending data.'
      break
    
    time.sleep(0.1)
    #time.sleep(SpillTime/EventRate)
    # stop_event.wait(SpillTime/EventRate)
    ev+=1
    

def clientthread(conn, sudp):
    #Sending message to connected client
    #conn.send('Welcome to the server. Receving Data...\n')

    #infinite loop so that function do not terminate and thread do not end.

    pro = None # To store the Popen instance of the local daq executable
    
    while True:
        #Receiving from client
        try:
            data = conn.recv(1024)
        except socket.error:
            print 'The client socket is probably closed. break this thread'
            break

        if not data:
            break
        
        print 'Command recieved:', str(data), type(data)

        if str(data)[0:9]=='START_RUN':
            t1_stop.clear()

            try:
                runNumber = (str(data).split())[1]
            except:
                print 'Run number is not provided!'
                conn.sendall('NO_START_FOR_YOU\n')
                continue
            
            # Now let's start the local daq code
            with  open('logs/log_run_'+str(runNumber)+'.log', 'w') as logFile:
              print 'The executable to run is:', executable
              print 'The log file of the executable will be:', logFile

              pro = subprocess.Popen(['sudo', executable, str(runNumber), str(nEvents), str(int(opt.ped))], stdin=None, stdout=logFile, stderr=logFile, shell=False)

            # Let's take a quick sleep, to let the DAQ create the needed datafile.
            time.sleep(3)

            # And start a thread to trasmit raw data to DAQ server via UDP:
            start_new_thread(sendData, (sudp,t1_stop,int(runNumber)))
            
            # Now, let's reply to the DAQ that we received her command and started the run:
            conn.sendall('GOOD_START\n')
            
            print 'Sent GOOD_START confirmation. Runnumber = ', runNumber

        elif str(data)=='STOP_RUN':
            t1_stop.set()
            
            try:
              ret = pro.poll()
            except:
              ret = -1
            while (ret==None):
              try:
                ret = pro.poll()
              except:
                ret = -1
                
              print 'We must wait until the daq finishes, because it is too fragile to kill'
              time.sleep(1)
              
            print 'Return code of the daq:', ret
            
            time.sleep(2)
            conn.sendall('STOPPED_OK')
            #sudp.sendto('STOPPED_OK', (HOST,PORTUDP))
            print 'Sent STOPPED_OK confirmation'

            print 'Now moving the data files to NFS directory'
            moveDataToNFS()
            print 'Done moving'
            
        else:
            conn.sendall('Message Received at the server: %s\n' % str(data))
            #sudp.sendto('Message Received at the server: %s\n' % str(data), (HOST,PORTUDP))


    conn.close()
            

if __name__ == "__main__":
  
  # This socket is to be used for communication:
  sTcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  
  print 'Socket created:', sTcp

  try:
    sTcp.bind((MYHOSTIP, PORTTCP))
  except socket.error , msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()

  print 'Socket bind complete'

  sTcp.listen(3)
  print 'Socket now listening'

  # This one is to be used for sending the data
  sUdp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  
  print 'Socket created:', sUdp

  '''
  try:
    sUdp.bind((HOSTUDP, PORTUDP))
  except socket.error , msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
  '''
  
  try:
    while True:
      # wait to accept a connection
      conn, addr = sTcp.accept()
      print 'Connected with ' + addr[0] + ':' + str(addr[1])

      HOSTUDP=addr[0]

      #start new thread
      start_new_thread(clientthread ,(conn,sUdp,))

  except KeyboardInterrupt:
    print 'Interrupted by keyboard. Closing down...'
    sTcp.close()
    sUdp.close()
    
  finally:
    sTcp.close()
    sUdp.close()
