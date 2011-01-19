import uavobjects
import sys

n=0
while n<200000:
	uavobjects.FlightPlanStatusUpdate(n)
	n=n+1 


