cd src
make
cd ..
./src/prog
python ./htmlOperation/htmlOp.py 1
python ./graphWalk/graphWalkmysql.py 2 

#mysqldump -uroot -h localhost -p robot FCSbookTripleW > ./models/FCSbookTripleW.sql

#python ./graphWalk/graphWalkmysql.py 1 ic:questionContainer c:inputOutput
