/*
 * infocast - Make robots visible
 * Copyright 2012-2013 Tobias Kalbitz.
 * Subject to the AGPL, version 3.
 */

#include "infocast.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <sockethelper.h>

#include "infocast_sensors.h"
#include "infocast_network.h"

static uint16_t    multicast_port = 5880;
static uint16_t    say_port       = 5882;
static std::string multicast_ip   = "230.0.0.1";

void parse_options(int argc, char** argv)
{
    boost::program_options::options_description arguments_description("Usage");
    arguments_description.add_options()
        ("help,h","Print this help")
        ("multicastIp,m",   boost::program_options::value<std::string>()->default_value("230.0.0.1"), "Set the Multicast IP")
        ("multicastPort,p", boost::program_options::value<uint16_t>()->default_value(5880), "Set port to send to")
        ("pongPort,p",      boost::program_options::value<uint16_t>()->default_value(5881), "Set port to send pong to")
        ("sayPort,s",       boost::program_options::value<uint16_t>()->default_value(5882), "Set port to send say text to")
    ;

    boost::program_options::variables_map varmap; 
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, arguments_description), varmap);

    if(access("/etc/default/infocast", R_OK) == 0) {
	boost::program_options::store(boost::program_options::parse_config_file<char>("/etc/default/infocast", arguments_description), varmap);
    }

    boost::program_options::notify(varmap); 

    multicast_ip   = varmap["multicastIp"].as<std::string>();
    multicast_port = varmap["multicastPort"].as<uint16_t>();

    printf("Infocast:MulticastIp   = %s\n", multicast_ip.c_str());
    printf("Infocast:sayPort       = %d\n", say_port);
    printf("Infocast:MulticastPort = %d\n", multicast_port);
}

#define WRITE(ptr, len) {memcpy(start, (ptr), (len));  start += (len);}
int main(int argc, char** argv)
{
    const uint8_t MAGIC_BYTE = 0x11;
    const uint8_t VERSION    = 0x02;

    struct sockaddr_in saddr;
    int status;
    char hostname[255];

    parse_options(argc, argv);

    if(gethostname(hostname, sizeof(hostname))) {
        perror("Infocast: gethostname");
        exit(1);
    }  
 
    int hlen = strlen(hostname) + 1;
    int len = 512;
    char* message = (char*)malloc(len);

    while(1) {
        unsigned char lanMacAddress[6];
        unsigned char wifiMacAddress[6];
        uint32_t ipLan, ipWLan;

        int8_t  bat          = (int8_t)(get_battery_info() * 100);
        uint8_t wifiStrength = get_wifi_quality();
        uint8_t cpuTemp      = get_cpu_temperature();

        get_ip_address("eth0",  ipLan,  lanMacAddress);
        get_ip_address("wlan0", ipWLan, wifiMacAddress);

        char* start = message;
        *start = MAGIC_BYTE; start++;
        *start = VERSION;    start++;
        WRITE(&ipLan,   sizeof(ipLan));
        WRITE(&ipWLan,  sizeof(ipWLan));
        WRITE(&bat,     sizeof(bat));
        WRITE(&cpuTemp, sizeof(cpuTemp));
        WRITE(&wifiStrength,  sizeof(wifiStrength));
        WRITE(lanMacAddress,  sizeof(lanMacAddress));
        WRITE(wifiMacAddress, sizeof(wifiMacAddress));
        memcpy(start, hostname, hlen); start += hlen;

        /*
         * We have to do this every iteration because we want see the Nao also
         * when a new interface becomes ready. For example eth0 gets an IP address
         * This should be no problem. Creating a socket is cheap.
         */
        std::list<int> sockList = open_send_multicast_socket(saddr, multicast_ip.c_str(), multicast_port);
        for(std::list<int>::iterator it = sockList.begin(); it != sockList.end(); ++it) {
            int sock = *it;
            status = sendto(sock, message, start - message, 0,
                            (struct sockaddr*)&saddr,sizeof(saddr));
            close(sock);
            if(status < 0)
                perror("Infocast: sendto");
        }
        sleep(5);
    }
}
