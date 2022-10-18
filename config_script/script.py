import os.path
from os import path

OVS_PATH="/home/ubuntu/oRSS-NIC/ovs"

fh = open("files.txt", "r")
lines = fh.readlines()
fh.close()

invalid_files = 0

for line in lines:
	files = line[:-1].split(" ")
	for file in files:
		absolute_path = file.replace("${OVS_PATH}", OVS_PATH)
		if (path.exists(absolute_path)):
			print(absolute_path)
		else:
			invalid_files += 1
