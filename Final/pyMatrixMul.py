#!/usr/bin/env python
import time
import os
import random
from multiprocessing import Process, Queue, Lock
import numpy as np
counta = 0
countb = 0
size = 3
def mul():
    for r in matA:
        print(r)
    for r in matB:
        print(r)
   # for i in range(len(matA)):
    #    for j in range(len(matB[0])):
     #       for k in range(len(matB)):
      #          matRes[i][j] += ((matA[i][k] * matB[k][j]))
    for r in matRes:
        print(r)
    global counta, countb
    lock.acquire()
    while(counta < size):
        #print(counta, size)
        #print()
        if(countb == size):
            countbf = 0
            countb = 0
            counta += 1
        else:
            countbf = countb
            countb += 1
        countaf = counta
        lock.release()
        for i in range(size):
            for r in matRes:
                print(r)
            print()
            #print(matA[counta][i])
            #print(matB[i][countb])
            matRes[countaf][countbf] += (matA[countaf][i] * matB[i][countbf]);
        lock.acquire()
    lock.release()

matA = np.random.randint(10, size=(size, size))
matB = np.random.randint(10, size=(size, size))
matRes = [[0]*size]*size
#for i in range(size):
 #   for j in range(size):
  #      matA[i][j] = random.randint(1, 9)
   #     matB[i][j] = random.randint(1, 9)
    #    matRes[i][j] = 0

lock = Lock()
processes = []
start = time.perf_counter()
for i in range(1):
    p = Process(target = mul)
    processes.append(p)
for p in processes:
    p.start();
for p in processes:
    p.join()
end = time.perf_counter()
print(end - start);

