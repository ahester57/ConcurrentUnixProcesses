## Concurrent Unix Processes

### Austin Hester  
##### University of Missouri - St. Louis  
##### CS4760 - Operating Systems 
##### Sanjiv Bhatia  

To build and run:  
```  
make  
./master  
```

Uses message queues to talk to children.  
Messages contain:

	* mType := the index for child to select its assigned message  
	* mText := the message string itself

Protects each child's critical section (write to file) with semaphores.  
Semaphores to control:  

	* Locking file I/O  
	* Limiting max # of processes   
	* Parent knowing when to close msg queues and semaphores  

Utilizes signal handlers to properly clean up when receiving `SIGINT` from `Ctrl^C`.  
Child blocks signals during critical section, exits after with a trap.  

	* Child processes exit if not in critical section  
	* Removes messages in queue  
	* Removes semaphore sets  
	* Child in critical section exits when in remainder section  

Reads strings from 'strings.in'  
Outputs palindromes to 'palin.out'  
Outputs not palindromes to 'nopalin.out'  

