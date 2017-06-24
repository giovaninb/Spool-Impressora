# comp310ast2

Giovani Nícolas Bettoni

giovanib@ufcspa.edu.br

260688650

Everything is working fine and tested, a picture is joined to show an example on my computer.
The two terminals on the left side are running `client` and are sending jobs to the queue. On the top one we can see a job waiting on the semaphore when the queue is full. The two terminals on the right side are running `print_server` and are taking jobs from the queue.

`print_server` will prompt for the queue size then initialize the shared space and the semaphores and start taking print requests. For every jobs, it will print the information and then wait the corresponding time.
Calling any subsequent `print_server` will skip the initialization and start accepting print request immediately.

`client` will asks for a job name, a job duration (in seconds) and will log its PID with the job struct.
As described in the header file common.h, the queue holds job structs.
