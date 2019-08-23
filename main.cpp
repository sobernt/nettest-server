#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <nettest.h>

using namespace std;
int sock;
#define SOCKET_PORT 5001
#define SOCKET_READ_TIMEOUT 5
void close_socket(){
    shutdown(sock, SHUT_RDWR);
    close(sock);
}
string read_string(){
    int bytesReaded = 0;
    string result = "";
    nettest_body body;
    bzero(&body,sizeof body);
    uint16_t seq_count= 0;
    while(true){
        bytesReaded = recv(sock, (struct nettest_body *)&body, sizeof body,0);
        if(bytesReaded<=0){
            break;
        }
        result.append(body.data);
        if(body.seq==body.seq_count-1){
            return result;
        }
        usleep(SOCKET_READ_TIMEOUT);
    };
    return result;
}
int main(int argc, char *argv[])
{    
    struct sockaddr_in sockAddr;
    int sock_connect;

    if(argc < 3){
        printf("usage: nettest-server <host> <command>\n");
        /*
         * nettest-server 127.0.0.1 ping 88.88.88.88 tcp 120 4 10
         * ip адрес команда адрес теста ,тип пакета (TCP/UDP), длина пакета,  количество пакетов, время теста,
        */
        return EXIT_FAILURE;
    }
    if(argc >3 && argc<7){
        printf("usage: nettest-server <host> <command> <testing_ip> <type> <packetLength> <packetCount> <testTime> \n");
        return EXIT_FAILURE;
    }
        sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (sock == -1) {
            perror("socket create error");
            return EXIT_FAILURE;
        }

        memset(& sockAddr, 0, sizeof ( sockAddr));

        const char* addr = argv[1];

         sockAddr.sin_family = PF_INET;
         sockAddr.sin_port = htons(SOCKET_PORT);

         printf("connect to adress %s \n",addr);

        sock_connect = inet_pton(PF_INET, addr, &(sockAddr.sin_addr));//argv[0]

        if (sock_connect < 0) {
            perror("error: undefined host");
            close_socket();
            return EXIT_FAILURE;
        } else if (!sock_connect) {
            perror("socket host error");
            close_socket();
            return EXIT_FAILURE;
        }

        if (connect(sock, (struct sockaddr*) & sockAddr, sizeof ( sockAddr)) == -1) {
            perror("connect error");
            close_socket();
            return EXIT_FAILURE;
        }

        nettest_header header;
        bzero(&header,sizeof header);

        size_t out_length = 0;
        int bytes_writed = 0;
        if(strstr( argv[2], (char*)"ping" ) != NULL){
            query_ping query;
            out_length = sizeof header + sizeof query;
            header.command = nettest_command_ping;
            header.size = sizeof query;
            inet_aton(argv[3], &query.ip);
            query.is_tcp = ((strstr( argv[2], (char*)"tcp" ) != NULL || strstr( argv[2], (char*)"TCP" ) != NULL));
            query.packet_length = stoi(argv[5]);
            query.packet_count = stoi(argv[6]);
            query.test_time = stoi(argv[7]);

            bytes_writed = send(sock ,&header , sizeof header,0);
            if(bytes_writed == -1){
                cout << "can't send data into socket.\n";
            }
            bytes_writed = send(sock ,&query , sizeof query,0);
            if(bytes_writed == -1){
                cout << "can't send data into socket.\n";
            }
        }
        else{
            header.command = nettest_command_exit;
            header.size = 0;
            bytes_writed = send(sock ,&header , sizeof header,0);
            if(bytes_writed == -1){
                cout << "can't send data into socket.\n";
            }
        }


        nettest_header n_header;
        bzero(&n_header,sizeof n_header);
        int bytesReaded = 0;
        while(true){
                bytesReaded =  recv(sock,(nettest_header *)&n_header,(sizeof n_header),MSG_WAITALL);
                if (bytesReaded < 0) {
                    break;
                }else if (bytesReaded < (int)sizeof n_header) {
                    usleep(SOCKET_READ_TIMEOUT);
                    continue;
                }
                break;
        }
                if(n_header.command == nettest_command_exit_req){
                    cout << "app killed.\n";
                } else
                if(n_header.command == nettest_command_ping_reply){
                    string pingout=read_string();
                        cout << "we has reply for ping:\n";
                        cout << endl << endl << "------------------------" << endl
                             << pingout
                             << endl << endl << "------------------------" << endl;
                        char user_answer = 'a';
                        do{
                            cout << "save log to file? y/n" << endl;
                            cin >> user_answer;
                        } while (!std::cin.fail() && user_answer!='y' && user_answer!='n');
                        if(user_answer=='y'){
                            //TODO::save log to file
                        }

                } else
                if(n_header.command == nettest_command_optimal_frame_req){
                    cout << "sorry, this test development inporgress.\n";
                    //TODO: get req grom optimal rate test
                } else {
                    cout << "data packet have undefined reply code.go out."<< endl;
                }
        cout << "bye.\n";
        close_socket();
        return 0;
}
