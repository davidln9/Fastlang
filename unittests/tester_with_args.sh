
num_tests=0
num_failed=0

for i in a*.lyt
do
	echo "testing $i"
	num_tests=$((num_tests+1))
	args=$(cat $i.args)
	./$1 $i $args > $i.output 2>&1
	diff $i.output $i.expected > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "test $i failed"
		num_failed=$((num_failed+1))
		echo "$i args:"
		echo $args
		echo "$i output:"
		cat $i.output
		echo ""
	fi
done



echo "******************************************"
echo "**** results for tests with arguments ****"
echo "tests: $num_tests"
echo "failed: $num_failed"
echo "******************************************"

echo ""
if [ "$num_failed" -ne "0" ]; then
	exit 1;
fi	
