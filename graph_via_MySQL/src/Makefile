all:prog readtsv
prog:ontologyProg.c ebookProg.c
	g++ -o prog  -I/usr/include/mysql -I/home/noel/libraries/FaCT++-src-v1.6.3/src/FaCT++.C  -I/home/noel/libraries/FaCT++-src-v1.6.3/src/Kernel /home/noel/libraries/FaCT++-src-v1.6.3/src/FaCT++.C/fact.cpp  ontologyProg.c ebookProg.c -L/home/noel/libraries/FaCT++-src-v1.6.3/src/Kernel -lKernel -L/usr/lib/x86_64-linux-gnu -lmysqlclient   

readtsv:readtsv.c	
	g++ -o readtsv -I/usr/include/mysql readtsv.c  -L/usr/lib/x86_64-linux-gnu -lmysqlclient	
	
	
