#!/bin/bash
for i in {1..10}
do
	time ./multi-lookup names100.txt ref.out
	time ./multi-lookup names100B.txt ref.out
	sudo resolvectl flush-caches
	#resolvectl statistics
done	
