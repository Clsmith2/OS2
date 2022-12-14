#!/usr/bin/env python
import time
import os
import random
from multiprocessing import Process, Queue, Lock
import dns.resolver
# Producer function that places data on the Queue
def producer(queue, lock):
    # Synchronize access to the console
    with lock:
        print('Starting producer => {}'.format(os.getpid()))
    f = open("names100.txt", "r")     
    # Place our names on the Queue
    name = f.readline()
    while name:
        queue.put(name)
        name = f.readline()
 
    # Synchronize access to the console
    with lock:
        print('Producer {} exiting...'.format(os.getpid()))
 
 
# The consumer function takes data off of the Queue
def consumer(queue, lock):
    # Synchronize access to the console
    with lock:
        print('Starting consumer => {}'.format(os.getpid()))
    time.sleep(0.1) 
    # Run indefinitely
    while True:
         
        # If the queue is empty, queue.get() will block until the queue has data
        name = queue.get()
        name = str(name).strip()
        try:
            dns.resolver.query(name, 'A')
        except dns.resolver.NXDOMAIN:
            print("no domain")
        except dns.resolver.NoAnswer:
            print("no answer")
        except dns.resolver.NoNameservers:
            print("servfail")
        # Synchronize access to the console
        with lock:
            #print('{} got {}'.format(os.getpid(), name))
            if(queue.empty()):
                exit()
 
 
if __name__ == '__main__':
     
 
    # Create the Queue object
    queue = Queue()
     
    # Create a lock object to synchronize resource access
    lock = Lock()
 
    producers = []
    consumers = []
    start = time.perf_counter() 
    # Create our producer processes by passing the producer function and it's arguments
    producers.append(Process(target=producer, args=(queue, lock)))
 
    # Create consumer processes
    for i in range(6):
        p = Process(target=consumer, args=(queue, lock))
         
        # This is critical! The consumer function has an infinite loop
        # Which means it will never exit unless we set daemon to true
        p.daemon = True
        consumers.append(p)
 
    # Start the producers and consumer
    # The Python VM will launch new independent processes for each Process object
    for p in producers:
        p.start()
 
    for c in consumers:
        c.start()
 
    # Like threading, we have a join() method that synchronizes our program
    for p in consumers:
        p.join()
    end = time.perf_counter()
    print(end-start)
    print('Parent process exiting...')
