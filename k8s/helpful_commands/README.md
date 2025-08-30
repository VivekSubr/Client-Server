* Set of useful commands for debugging in kubernetes clusters *


** Execute commands from worker node on pods
1. exec onto worker 
2. ps -ef 
3. sudo nsenter -t ${PID} -n iptables-save

Or any other command, to execute stuff from worker node on pod namespace