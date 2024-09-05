# run inner loop 10 times
for i in $(seq 1 $1);
do
	for j in $(seq 1 10);
	do
		sleep 0.1
	done
done