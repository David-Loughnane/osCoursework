./start 5 &
sleep 1
./producer 1 8 & 
./consumer 1 &
