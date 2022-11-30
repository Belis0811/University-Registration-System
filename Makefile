all: serverM.cpp serverC.cpp serverCS.cpp serverEE.cpp client.cpp
	g++ -o serverM serverM.cpp

	g++ -o serverC serverC.cpp

	g++ -o serverCS serverCS.cpp
	
	g++ -o serverEE serverEE.cpp	

	g++ -o client client.cpp

.PHONY: serverM
serverM: 
	./serverM

.PHONY: serverC
serverC:
	./serverC

.PHONY: serverCS
serverCS:
	./serverCS

.PHONY: serverEE
serverEE:
	./serverEE

.PHONY: client
client:
	./client