import openpilot
import sys

n = 0
timenow = openpilot.time()

while n < 120:
	n = n+1 
	openpilot.debug(n, timenow)
	timenow = openpilot.delayUntil(timenow, 1000)
	if openpilot.hasStopRequest():
		sys.exit()



