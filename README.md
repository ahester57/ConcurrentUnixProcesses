# Concurrent Unix Processes

Austin Hester

run make to build  
run ./master to run

Uses message queues to talk to children.

Semaphores to control:
	- Locking file I/O  
	- Limiting max # of processes   
	- Parent knowing when to close msg queues and semaphores  

Reads strings from 'strings.in'  
Outputs palindromes to 'palin.out'  
Outputs not palindromes to 'nopalin.out'  

