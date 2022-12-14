#!/usr/bin/env python
import numpy as np
import time
import os
import random
from multiprocessing import Process, Queue, Lock
size = 250
A = [[0]*size]*size
B = [[0]*size]*size
Res = [[0]*size]*size 
def setup():
    global A, B 
    A = np.random.randint(10, size=(size, size))
    B = np.random.randint(10, size=(size, size))

def mult(i):
    global Res
    for j in range(size):
        for k in range(size):
            Res[i][j] += A[i][k] * B[k][j]
    #print(Res[i])
for i in range(100):
    setup()
    processes = []
    start = time.perf_counter()
    for q in range(size):
        p = Process(target = mult, args=(q,))
        processes.append(p)

    for p in processes:
        p.start()

    for p in processes:
        p.join()
    end = time.perf_counter()
    print(end - start)
#for r in A:
 #   print(r)
#for r in B:
 #   print(r)
#for r in Res:
 #   print(r)

