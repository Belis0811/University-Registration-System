a) name: Anzhe Cheng

b) USCID: 8732892247

c) What I have done: 
	Implemented web registration system in USC. Coded five parts for the system, including serverM(main srever for handling username and password encryption, server choose and feedback transmit), server C(compare received username and password with exsisted ones, and return comparison result), serverCS(store existed CS courses information and prepare for client inquiry), serverEE(store existed EE courses information and prepare for client inquiry) and client(input username and password to enter system, inquiry course information). 

d) code files: 
		serverM.cpp : 
			— create TCP connection with client
			— create UDP connection with serverC, serverCS and serverEE
			— encrypt username and password and send to serverC
			— send course inquiry to serverEE or serverCS(handling by serverM)

		serverC.cpp :  
			— read "cred.txt" to get registered username and password(encrypted)
			— judge if the received username and the password matches registered ones

		serverCS.cpp :  
			— read "cs.txt" to get CS course information
			— judge if the course client inquired does exist
			— return required information to client(through serverM)

		serverEE.cpp :
			— read "ee.txt" to get EE course information
			— judge if the course client inquired does exist
			— return required information to client(through serverM)
	
		client.cpp :
			— send user input username and password to serverM to test validation 
			— inquire courses in CS and EE departments

e) format: 
		serverM:
			 input_buf: data from client
			 process_buf: process data from client(e.g. username, password, course)
			 C_write_result: result get from serverC(only one char)
			 CS_return_buf: info get from serverCS
			 EE_return_buf: info get from serverEE

		serverC:
			 recv_buf: data from serverM
 		           write_result: result send to serverC(only one char)
			 new_name: store registerd name
			 new_pass: store registered password 

		serverCS:
			 recv_buf: data from serverM
			 write_result: result send to serverC
			 write_buf: read registered data
			 new_course: store registerd course
			 new_credit: store registered credit 
			 new_professor: store registerd professor
			 new_date: store registerd date
			 new_name: store registerd name

		serverEE:
			 recv_buf: data from serverM
			 write_result: result send to serverC
			 write_buf: read registered data
			 new_course: store registerd course
			 new_credit: store registered credit 
			 new_professor: store registerd professor
			 new_date: store registerd date
			 new_name: store registerd name

		client:
			 write_buf: process received data from serverM
			 write_result: result get from serverM
			 name: input username
			 pass: input password
			 detail: input category
  			 course: input course code

g) idiosyncrasy: project runs on given environment and no errors occurred.

h) Reused Code: 
		1. Beej's Code: http://www.beej.us/guide/bgnet/
				— Create and bind sockets
				— Send and receive data
		2. Tutorials on Socket Programming with C++: http://c.biancheng.net
				— file transfer between server and client
				—  read files from txt