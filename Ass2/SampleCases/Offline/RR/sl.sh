# run inner loop 10 times
for i in $(seq 1 $1);
do
	for j in $(seq 1 100);
	do
		sleep 0.01
	done
done