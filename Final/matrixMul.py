#!/usr/bin/env python
import threading
import numpy as np
import time
def mult(X, Y, size):
    result = [[0]*size]
    for z in range(len(Y[0])):
        for k in range(len(Y)):
            result[0][z] += (X[0] * Y[k][z])       
def setUp(size):
    matA = np.random.randint(10, size=(size, size))
    matB = np.random.randint(10, size=(size, size))
    threads =list();
    start = time.perf_counter()
    for i in range(len(matA[0])):
        x = threading.Thread(target = mult, args= (matA[i], matB, size))
        threads.append(x)
        x.start()
    for index, thread in enumerate(threads):
        thread.join()
    end = time.perf_counter()
    print(end-start)
print("size 10")
for i in range(299):
    size = 10
    setUp(size)

