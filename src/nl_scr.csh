#!/bin/csh -f
foreach i (*.c)
echo $i
cp $i tmpfile
sed 's/\(.*\"\)\\n\(.*\)\(\".*\)/\1\2\\n\3/' tmpfile > $i
end
