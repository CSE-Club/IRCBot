/*
 * Bot.cpp
 *
 *  Created on: 15 Jul 2011
 *      Author: tyler
 */

#include "bot.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

using namespace std;
#define MAXDATASIZE 100

Bot::Bot(string _nick, string _usr, string _server, string _port)
{
    nick = _nick;
    usr = _usr;
    server = _server;
    port = _port;
}

Bot::~Bot()
{
    close(s);
}

void Bot::start()
{
    struct addrinfo hints, *servinfo;

    //Setup run with no errors
    setup = true;

    //Ensure that servinfo is clear
    memset(&hints, 0, sizeof hints); // make sure the struct is empty

    //setup hints
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    //Setup the structs if error print why
    int res;
    if ((res = getaddrinfo(server.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        setup = false;
        fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(res));
    }

    //setup the socket
    if ((s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("client: socket");
    }

    //Connect
    if (connect(s,servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close (s);
        perror("Client Connect");
    }

    //We dont need this anymore
    freeaddrinfo(servinfo);

    //Recv some data
    int numbytes;
    char buf[MAXDATASIZE];

    int count = 0;
    while (1) {
        //declars
        count++;

        switch (count) {
            case 3:
                    //after 3 recives send data to server (as per IRC protacol)
                    sendData(nick);
                    sendData(usr);
                break;
            case 4:
                    //Join a channel after we connect, this time we choose beaker
                sendData("JOIN ##cse-club\r\n");
            default:
                break;
        }

        //Recv & print Data
        numbytes = recv(s, buf, MAXDATASIZE-1, 0);
        buf[numbytes]='\0';
        cout << buf;
        //buf is the data that is recived

        //Pass buf to the message handeler
        msgHandle(buf);


        //If Ping Recived
        if (charSearch(buf,"PING")) {
            sendPong(buf);
        }

        //break if connection closed
        if (numbytes==0)
        {
            cout << "----------------------CONNECTION CLOSED---------------------------"<< endl;
            cout << timeNow() << endl;

            break;
        }
    }
}

bool Bot::charSearch(string toSearch, string searchFor)
{
    int len = toSearch.length();
    int forLen = searchFor.length(); // The length of the searchfor field

    //Search through each char in toSearch
    for (int i = 0; i < len;i++)
    {
        //If the active char is equil to the first search item then search toSearch
        if (searchFor[0] == toSearch[i])
        {
            bool found = true;
            //search the char array for search field
            for (int x = 1; x < forLen; x++)
            {
                if (toSearch[i+x]!=searchFor[x])
                {
                    found = false;
                }
            }

            //if found return true;
            if (found == true)
                return true;
        }
    }

    return 0;
}

bool Bot::isConnected(string buf)
{//returns true if "/MOTD" is found in the input strin
    //If we find /MOTD then its ok join a channel
    if (charSearch(buf,"/MOTD") == true)
        return true;
    else
        return false;
}

string Bot::timeNow()
{//returns the current date and time
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    return asctime (timeinfo);
}

bool Bot::sendData(string msg)
{
    int len = msg.length();
    int bytes_sent = send(s, msg.c_str(), len, 0);

    if (bytes_sent == 0)
        return false;
    else
        return true;
}

void Bot::sendPong(string buf)
{
    string toSearch = "PING ";

    for (size_t i = 0; i < buf.length(); ++i)
        {
            //If the active char is equil to the first search item then search toSearch
            if (buf[i] == toSearch[0])
            {
                bool found = true;
                //search the char array for search field
                for (int x = 1; x < 4; x++)
                {
                    if (buf[i+x]!=toSearch[x])
                    {
                        found = false;
                    }
                }

                //if found return true;
                if (found == true) {
                    int count = 0;
                    //Count the chars
                    for (size_t x = i + toSearch.length(); x < buf.length(); ++x) {
                        count++;
                    }

                    //Create the new char array
                    string returnHost = "PONG ";

                    count = 0;
                    //set the hostname data
                    for (size_t x = i + toSearch.length(); x < buf.length();x++) {
                        returnHost += buf[x];
                        count++;
                    }

                    //send the pong
                    if (sendData(returnHost))
                    {
                        cout << timeNow() <<"  Ping Pong" << endl;
                    }


                    return;
                }
            }
        }

}

void Bot::msgHandle(string buf)
{
    if (charSearch(buf,"hi scooby")) {
        sendData("PRIVMSG ##cse-club :I am alive!\r\n");
    }

}

