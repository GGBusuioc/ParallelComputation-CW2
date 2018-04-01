#
# Script to display the histogram (assumed to be 'hist.out' in the local directory).
#



import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy

grey, value = [], []
for line in open("hist.out","r").readlines():
	g, v = [ int(str) for str in line.split() ]
	grey.append( g )
	value.append( v )

# Display
plt.bar( grey, value )
plt.xlabel( "Greyscale value" )
plt.ylabel( "Count" )
plt.savefig('ss.png') # Any filename will do
