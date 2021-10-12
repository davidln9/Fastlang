

num_failed=0
total=0
for i in t*.lyt
do
	echo "testing $i"
	./$1 $i > $i.output 2>&1
	diff $i.output $i.expected > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo $i failed!
		echo "$i.output:" 
		cat $i.output
		echo ""
		num_failed=$((num_failed + 1))
	fi
	total=$((total+1))
done

echo "******************************************"
echo "*****results for tests without input******"
echo "tests: $total"
echo "failed: $num_failed"
echo "******************************************"

echo ""

if [ $num_failed -gt 0 ]; then
	exit 1
fi

