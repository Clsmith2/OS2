#!/usr/bin/env python
from multiprocessing import Process, Queue, Lock
import time
import dns.resolver
import threading
# Shared Memory variables
CAPACITY = 10
buffer = [-1 for i in range(CAPACITY)]
in_index = 0
out_index = 0
done = 0
counter = 0
# Declaring Semaphores
mutex = threading.Semaphore()
empty = threading.Semaphore(CAPACITY)
full = threading.Semaphore(0)
 
lock = Lock()
# Producer Thread Class
class Producer(Process):
  def run(self):
    print("here producer")
    global CAPACITY, buffer, in_index, out_index, counter, done
    global mutex, empty, full
    f = open("names.txt", "r")
    counter = 0
    name = f.readline()
    empty.acquire()
    mutex.acquire()
    while name:
        while counter > 9:
            mutex.release()
            time.sleep(0.01)
            mutex.acquire()
        buffer[in_index] = name
        counter += 1
        in_index = (in_index + 1)%CAPACITY
        print("Producer produced : ", name)
        name = f.readline()
        if not name:
            done = 1
        mutex.release()
        full.release()
# Consumer Thread Class
class Consumer(Process):
  def run(self):
    print("here")
    global CAPACITY, buffer, in_index, out_index, counter, done
    global mutex, empty, full
     
    items_consumed = 0
    while 1: 
        full.acquire()
        mutex.acquire()
        while counter == 0:
            while done == 1:
                mutex.release()
                empty.release()
                return
            mutex.release()
            time.sleep(0.01)
            mutex.acquire ()
        item = buffer[out_index]
        item = str(item).strip()
        counter -= 1
        print("counter & done", counter, done)
        out_index = (out_index + 1)%CAPACITY
        print("Consumer consumed item:", item)
        try:
            dns.resolver.query(item, 'A')
        except dns.resolver.NXDOMAIN:
            print("no domain")
        except dns.resolver.NoAnswer:
            print("no answer")
        except dns.resolver.NoNameservers:
            print("servfail")

        #for val in ip:
        #    print('A Record : ', val.to_text())
        mutex.release()
        full.release()
            
       
       
tasks = Queue()
# Creating Threads
start = time.perf_counter()
producer = Process(target = Producer)
consumer1 = Process(target = Consumer)
consumer2 = Process(target = Consumer)
# Starting Threads
consumer1.start()
producer.start()
consumer2.start()
# Waiting for threads to complete
consumer1.join()
consumer2.join()
end = time.perf_counter()
print(end - start)
