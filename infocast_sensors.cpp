/*
 * infocast - Make robots visible
 * Copyright 2012-2013 Tobias Kalbitz.
 * Subject to the AGPL, version 3.
 */

#include "infocast_sensors.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <limits>

#include <alcore/alerror.h>
#include <alproxies/almemoryproxy.h>

#include "infocast.h"

uint8_t get_cpu_temperature()
{
    static char filename[] = "/proc/acpi/thermal_zone/THRM/temperature";
    int temp = 0;

    FILE* fp = fopen(filename, "rb");

    if(fp == NULL) {
        perror("Error opening file for temperature");
        fprintf(stderr, "File was %s\n", filename);
        fprintf(stderr, "");
        return 0;
    }

    const int rc = fscanf(fp, "temperature:%*[ ]%d", &temp);
    fclose(fp);

    if(rc == EOF || rc == 0) {
        fprintf(stderr, "Error parsing temperature. Please review the format.\n");
        return 0;
    }

    return (uint8_t)temp;
}

uint8_t get_wifi_quality()
{
    int rc, sock;
    struct iwreq iwr;
    struct iw_statistics stats;

    char ifname[] = "wlan0";

    memset(&stats, 0, sizeof(stats));
    memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, ifname, sizeof(ifname));

    iwr.u.data.pointer = &stats;
    iwr.u.data.length = sizeof(stats);
    iwr.u.data.flags  = 1;

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Couldn't create wireless stat socket");
        exit(EXIT_FAILURE);
    }

    rc = ioctl(sock, SIOCGIWSTATS, &iwr);
    close(sock);

    if(rc != 0) {
        return 0;
    }

    return stats.qual.qual;
}

void get_ip_address(const char* device, uint32_t& ip, unsigned char* mac)
{
    int sock;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("Error creating socket");
        fprintf(stderr, "Error: creating socket in file %s line %d. Exiting.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, device, IFNAMSIZ-1);

    if(ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        perror("get_ip_address: Error calling ioctl-1");
        ip = std::numeric_limits<uint32_t>::max();
        mac[0] = mac[1] = mac[2] = mac[3] = mac[4] = mac[5] = 0xff;
        close(sock);
        return;
    }

    ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

    if(ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        perror("get_ip_address: Error calling ioctl-2");
        mac[0] = mac[1] = mac[2] = mac[3] = mac[4] = mac[5] = 0xff;
        close(sock);
        return;
    }

    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    close(sock);
}


template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

float get_battery_info()
{
    static const char* const batteryCurrent = "Device/SubDeviceList/Battery/Current/Sensor/Value";
    static const char* const batteryCharge  = "Device/SubDeviceList/Battery/Charge/Sensor/Value";

    float bat = 0.f;

    try {
        AL::ALMemoryProxy memory("localhost", NAOQI_PORT);
        bat = (float)memory.getData(batteryCharge) * sgn((float)memory.getData(batteryCurrent));
    } catch(const AL::ALError& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    return bat;
}

