import socket
import sys
import struct

typedict = {0x0000: "misdirected",
                0x0001: "unimplemented protocol",
                0x0002: "uimplemented function",
                0x0003: "ACK",
                0x0004: "inverter wakeup",
                0x1000: "set phase",
                0x1001: "set amplitude",
                0x1002: "set real power",
                0x1003: "set reactive power",
                0x1004: "set frequency",
                0x2000: "set grid following mode",
                0x2001: "set standalone mode",
                0x2002: "disconnect",
                0x2003: "connect",
                0x2004: "lock screen",
                0x2005: "unlock screen",
                0x2006: "select screen 1",
                0x2007: "select screen 2",
                0x2008: "select screen 3",
                0x2009: "select screen 4",
                0x3000: "request phase",
                0x3001: "request amplitude",
                0x3002: "request mode",
                0x3003: "request PWM period",
                0x3004: "request nsamples",
                0x3005: "request output voltage",
                0x3006: "request output current",
                0x3007: "request real power",
                0x3008: "request reactive power",
                0x3009: "request frequency",
                0x4000: "response phase",
                0x4001: "response amplitude",
                0x4002: "response mode",
                0x4003: "response PWM period",
                0x4004: "response nsamples",
                0x4005: "response output voltage",
                0x4006: "response output current",
                0x4007: "response real power",
                0x4008: "response reactive power",
                0x4009: "response frequency"
                }

replydict = {0x1000: 0x0003,
                0x1001: 0x0003,
                0x1002: 0x0003,
                0x1003: 0x0003,
                0x1004: 0x0003,
                0x2000: 0x0003,
                0x2001: 0x0003,
                0x2002: 0x0003,
                0x2003: 0x0003,
                0x2004: 0x0003,
                0x2005: 0x0003,
                0x2006: 0x0003,
                0x2007: 0x0003,
                0x2008: 0x0003,
                0x2009: 0x0003,
                0x3000: 0x4000,
                0x3001: 0x4001,
                0x3002: 0x4002,
                0x3003: 0x4003,
                0x3004: 0x4004,
                0x3005: 0x4005,
                0x3006: 0x4006,
                0x3007: 0x4007,
                0x3008: 0x4008,
                0x3009: 0x4009
                }

valdict = {0x3000 : .1123,
            0x3001 : .8,
            0x3002 : 1,
            0x3003 : .016,
            0x3004 : 266,
            0x3005 : 17.89,
            0x3006 : 2.10,
            0x3007 : 31.75,
            0x3008 : 20.08,
            0x3009 : 59.98
            }

def hexprint(buffer):
    disp = ""
    for c in buffer:
        disp = disp + ("{:02x}".format(ord(c)))

    print(disp)


 

HOST = ''
PORT = 33000

try :
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print("Socket created")
except socket.error, msg :
    print("Failed to create socket. \nCode: {code} \nMessage: {message}".format(code = msg[0], message = msg[1]))  
    sys.exit()

try:
    sock.bind((HOST,PORT))
except socket.error, msg :
    print("Bind failed. \nCode: {code} \nMessage: {message}".format(code = msg[0], message = msg[1]))
    sys.exit()

print("Socket bound")

while True :
    #receive data from client
    d = sock.recvfrom(1024)
    data = d[0]
    addr = d[1]
    print("rec from address {addy}".format(addy = addr))
    if not data:
        break
    
    hexprint(data)

    if ord(data[0]) == int("a3",16) and ord(data[1])==int("cf",16):
        print("is a GP packet")
    
    if ord(data[2]) == int("01",16):
        print("inverter context")

    print("{n} message components according to header".format(n = ord(data[3])))
    seqnum = struct.unpack('>H',data[4:6])[0]
    print("seqnum is {sqn}".format(sqn = seqnum))
       
    count = 0
    index = 6
    messagecomps = []
    replycomps = []
    while index < len(data):
        code = struct.unpack('>H',data[(6+count*10):(8+count*10)])[0]
        value = struct.unpack('>d',data[(8+count*10):(16+count*10)])[0]
        messagecomps.append([code,value]) 
        count+=1
        index = 6 + 10*count
    
    for mes in messagecomps:
        print("name: {na} value; {va}".format(na = typedict[mes[0]], va = mes[1]))

    magic = 0xac3f
    context = 0x01
    count = 0
    for comp in messagecomps:
        print("processing code: {co}".format(co =comp[0]))
        reply = replydict.get(comp[0],None)
        if reply != None:
            value = valdict.get(comp[0],0.0)
            replycomps.append([reply,value])
            count += 1
    
    send = []
    send = struct.pack('>H',magic);
    hexprint(send)
    send = send + struct.pack('B',context)
    hexprint(send)
    send = send + struct.pack('B',count)
    hexprint(send)
    send = send + struct.pack('>H',seqnum)
    hexprint(send)
    if reply != None:
        for rep in replycomps:
            send = send + struct.pack('>H',rep[0])
            send = send + struct.pack('>d',rep[1])
            hexprint(send)
    
    hexprint(send)

    sock.sendto(send, addr)
    

    
sock.close()


