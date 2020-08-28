#!/bin/bash

sed -i 's/[[:blank:]]*$//' $1
sed -i 's/while(/while (/' $1
sed -i 's/for(/for (/' $1
sed -i 's/switch(/switch (/' $1
sed -i 's/if(/if (/' $1
sed -i 's/){/) {/' $1

