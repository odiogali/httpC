gcc -c -o server.out server.c -g
gcc -c -o httpRequest.out httpRequest.c -g
gcc -o serv httpRequest.out server.out 
./serv
