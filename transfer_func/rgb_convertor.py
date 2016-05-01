import matplotlib.image as image
import struct
import sys

filepath = sys.argv[1]
im = image.imread(filepath)
data = im[0]

for pixel in data:
    binStr = struct.pack('BBBB', int(pixel[0]*255), 
                                 int(pixel[1]*255), 
                                 int(pixel[2]*255), 1)
    sys.stdout.buffer.write(binStr)
