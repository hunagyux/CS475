import sys
import csv
import matplotlib.pyplot as plt

with open(sys.argv[1], 'r') as fd:
    ecoReader = csv.reader(fd, delimiter=',')
    headers = []
    plotDict = dict()
    i = 0
    for row in ecoReader:
        if i == 0:
            for h in row:
                headers.append(h)
                plotDict[h] = []
        else:
            for j in range(0,len(row)):
                plotDict[headers[j]].append(float(row[j]))
        i+=1
    #for key, value in plotDict.items():
    #    print("{}".format(key))

fd.close()

x = [(int(plotDict['Month'][i]) + 1) + ( (int(plotDict['Year'][i]) - 2020) * 12) for i in range(0,len(plotDict['Month']))]

