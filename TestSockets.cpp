// TestSockets.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <list>
#include <string>
#include <vector>

using namespace std;

//define constants
#define BUFFERSIZE 1024 
#define MAXPENDINGCONN 5 


typedef struct playerInfo {
    int id;
    SOCKET client;
}PlayerInfo;

typedef struct sessionInfo {
    int id;
    string name;
    string serverip;
    int port;
    int numplayers;
}SessionInfo;

list<PlayerInfo> players;
list<SessionInfo> sessions;

int playerIDCount = 0;
int sessionIDCount = 0;

//messages it can receive:
//get -> g|#
//host -> h|name(session)|player ip|player port|#
//if full -> j|id|#
void InterpretMessage(char* message, PlayerInfo player)
{
    string temp = "";
    char cmd = -1;
    vector<string> parameters;

    for (int i = 0; message[i] != '#'; i++)
    {
        if ((message[i] == '|') && (cmd == -1))
        {
            cmd = temp.at(0);
            temp = "";
        }
        else if ((message[i] == '|') && (cmd != -1))
        {
            parameters.push_back(temp);
            temp = "";
        }
        else
        {
            temp = temp + message[i];
        }
    }

    if (cmd == 'g')
    {
        //session list -> s|id|name of the session| server ip| port| 
        string message = "s|";

        if (sessions.size() > 0) {

            for (list<SessionInfo>::iterator it = sessions.begin(); it != sessions.end(); it++)

            {
                cout << it->numplayers << endl;
                if (it->numplayers < 2) {
                    message = message + to_string(it->id) + "|" + it->name + "|" + it->serverip + "|" + to_string(it->port) + "|";
                }

            }

        }

        else
        {
            message = message + "null|";
        }

        if (send(player.client, message.c_str(), strlen(message.c_str()) + 1, 0) == SOCKET_ERROR)
        {
            cout << "send() FAILED!";
        }
    }

    else if (cmd == 'h')
    {
        SessionInfo session;
        session.id = sessionIDCount;
        session.name = parameters.at(0);
        session.serverip = parameters.at(1);
        session.port = stoi(parameters.at(2));
        session.numplayers = 1;

        sessions.push_back(session);

        sessionIDCount++;

        //the response to the client
        //c|ip|port 
        string message = "c|" + session.serverip + "|" + to_string(session.port) + "|";

        if (send(player.client, message.c_str(), strlen(message.c_str()) + 1, 0) == SOCKET_ERROR)
        {
            cout << "send() FAILED!";
        }
    }

    else if (cmd == 'j') {
        //parameters.at(0) session ID
       
        for (list<SessionInfo>::iterator it = sessions.begin(); it != sessions.end(); it++) {

            if (it->id == stoi(parameters.at(0))) {

                if (it->numplayers < 2) {

                    it->numplayers = it->numplayers + 1;
                    //string hostip = GetIPFromHost();

                   //string message = "a|" + it->serverip + "|" + to_string(it->port) + "|";
                   string message = "a|" + it->serverip + "|" + "7777" + "|";
                   //string message = "a|94.63.246.240|" + to_string(it->port) + "|";
                   

                    if (send(player.client, message.c_str(), strlen(message.c_str()) + 1, 0) == SOCKET_ERROR)
                    {
                        cout << "send() FAILED!";
                    }

                }

                else {

                    string message = "f|#";
                    if (send(player.client, message.c_str(), strlen(message.c_str()) + 1, 0) == SOCKET_ERROR)
                    {
                        cout << "send() FAILED!";
                    }

                }

            }

        }

    }

    else {
        cout << "Unknown message: " << message << endl;
    }
}

/*string initMessage = "";
   if (send(player.client, initMessage.c_str(), strlen(initMessage.c_str()) + 1, 0) == SOCKET_ERROR)
   {
       cout << "send() FAILED!";
   }*/



void HandleClientConnectionThread(PlayerInfo player)
{
    char message[BUFFERSIZE];

    while (recv(player.client, message, sizeof(message), 0) > 0)
    {
        cout << message << endl;

        InterpretMessage(message, player);

        memset(message, 0, sizeof(message));
    }


    if (closesocket(player.client) == SOCKET_ERROR)
    {
        cout << "closesocket() FAILED!";
    }
    cout << "Player " << player.id << " disconnected!" << endl;
}

int main()
{
    //Declare a socket object (variable)
    SOCKET server; //A socket is a connection between a port and an ip address
    SOCKADDR_IN serveraddr, clientaddr; //Variables
    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 0), &wsadata) != NO_ERROR)
    {
        cout << "WSAStartup FAILED!";
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        cout << "socket() FAILED!";
        return 1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(7776);

    if (bind(server, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        cout << "bind";
        return 1;
    }

    if (listen(server, MAXPENDINGCONN) == SOCKET_ERROR)
    {
        cout << "listen() FAILED!";

        return 1;
    }

    cout << "Server started!" << endl;

    while (true)
    {
        SOCKET client;
        int clientLength = sizeof(clientaddr);
        client = accept(server, (struct sockaddr*)&clientaddr, &clientLength);
        if (client == INVALID_SOCKET)
        {
            cout << "listen() FAILED!";
        }
        char ipaddclient[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipaddclient, INET_ADDRSTRLEN);
        cout << "Connection form " << ipaddclient << endl;

        PlayerInfo player;
        player.client = client;
        player.id = playerIDCount;
        players.push_back(player);

        playerIDCount++;

        /*//testing purposes
        SessionInfo session;
        session.id = sessionIDCount;
        session.name = "My Session";
        session.serverip = "127.0.0.1";
        session.port = 3333;

        sessions.push_back(session);

        sessionIDCount++;

        SessionInfo session2;
        session2.id = sessionIDCount;
        session2.name = "My Session2";
        session2.serverip = "127.0.0.1";
        session2.port = 3233;

        sessions.push_back(session2);

        sessionIDCount++;
        ///////////////////////////////////////////////////////////*/


        thread* clientThread = new thread(HandleClientConnectionThread, player);
        
    }

    WSACleanup();

    return 0;
}

