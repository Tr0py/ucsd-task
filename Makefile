
#CXX=clang++
CXX=g++
CXXFLAGS=-std=gnu++17 -fopenmp -Wall -O4
TIME= 
#TIME=/usr/bin/time -v 

default: main

main: main.cpp svec.h config.h
	$(CXX) $< $(CXXFLAGS) -o $@ 

setup: data/804.mxs

data/804.mxs: data/804.mxs.gz
	gunzip -k $<

.PHONY: demo
demo: main
	DEMO=yes ./main data/804.mxs data/Query5.txt demo.json
	cat demo.json

.PHONY:test
test: test5

.PHONY:test1
test1: main  data/804.mxs
	$(TIME) ./main data/804.mxs data/Query1.txt 804-Query1.json
	#./check.py 804-Query1.json  < data/804-Query1-ref.json

.PHONY:test5
test5: main  data/804.mxs
	$(TIME) ./main data/804.mxs data/Query5.txt 804-Query5.json
	#./check.py 804-Query5.json  < data/804-Query5-ref.json

.PHONY:test20
test20: main  data/804.mxs
	$(TIME) ./main data/804.mxs data/Query20.txt 804-Query20.json
	#./check.py 804-Query1.json  < data/804-Query1-ref.json

.PHONY:test40
test40: main  data/804.mxs
	$(TIME) ./main data/804.mxs data/Query40.txt 804-Query40.json
	#./check.py 804-Query1.json  < data/804-Query1-ref.json

.PHONY: clean
clean:
	rm -rf main
	rm -rf task.tgz

.PHONY: tgz test-tgz
tgz:task.tgz

task.tgz:
	tar czf $@ $$(git ls-tree -r --name-only master)

test-tgz: task.tgz
	rm -rf task-test
	mkdir task-test
	cd task-test; tar xzf ../$<
	make test
