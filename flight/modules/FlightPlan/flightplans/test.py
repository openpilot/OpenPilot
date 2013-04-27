import sys
#mb = sys.heap()
#mb = mb[0]
import openpilot
#ma = sys.heap()
#ma = ma[0]
#print('import openpilot')
#print(mb-ma)
#mb = sys.heap()
#mb = mb[0]
import flightplanstatus
#ma = sys.heap()
#ma = ma[0]
#print('import flightplanstatus')
#print(mb-ma)
#mb = sys.heap()
#mb = mb[0]
import mixersettings
#ma = sys.heap()
#ma = ma[0]
#print('import mixersettings')
#print(mb-ma)

n = 0
timenow = sys.time()
fpStatus = flightplanstatus.FlightPlanStatus()

while n < 120:
	n = n+1 
	#openpilot.debug(n, timenow)
	fpStatus.read()
	fpStatus.Debug.value[0] = n
	fpStatus.Debug.value[1] = timenow
	fpStatus.write()
	timenow = openpilot.delayUntil(timenow, 1000)
	if openpilot.hasStopRequest():
		sys.exit()



