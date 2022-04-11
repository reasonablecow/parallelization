"""Prints given file where first column of dates is transformed into deltas.

Calculates difference between every two dates of first column, leaves the rest.
"""
import sys
from datetime import datetime

datetime_prev = None
graph_prev = None
with open(sys.argv[1]) as file:
    for line in file.readlines():
        parts = line.split()
        datetime_ = datetime.fromisoformat(parts[0])
        if datetime_prev is not None:
            print(graph_prev, datetime_ - datetime_prev)
        datetime_prev = datetime_
        graph_prev = parts[1:] if len(parts) >= 2 else None
