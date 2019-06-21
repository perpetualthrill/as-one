#!/usr/bin/env python3

import sys
import csv
import json

with open(sys.argv[1], newline='') as f:
    dictionary = {}
    array = [None] * 135
    reader = csv.DictReader(f)
    line = 0
    for row in reader:
        for j in range(31):
            value = row["{}".format(j)]
            if (value):
                array[int(value)] = { 'x': j, 'y': line }
                ##dictionary[int(value)] = { 'x': j, 'y': line }
        line = line + 1
    #print(json.dumps(dictionary))
    print(json.dumps(array))


