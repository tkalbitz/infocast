/*
 * infocast - Make robots visible
 * Copyright 2012-2013 Tobias Kalbitz.
 * Subject to the AGPL, version 3.
 */

#include "say_thread.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <alcore/alerror.h>
#include <alproxies/altexttospeechproxy.h>

#include "infocast.h"

#define SAY_VERSION    1
#define SAY_MAGIC_BYTE 0x548ab5eb

void* say_thread(void* say_port);

void start_say_thread(uint16_t& say_port) {
    uint16_t* arg_say_port = (uint16_t*)malloc(sizeof(*arg_say_port));
    if(arg_say_port == NULL) {
        fprintf(stderr, "Infocast: End of the world. Memory allocation failed for say_port.\n");
        exit(EXIT_FAILURE);
    }

    *arg_say_port = say_port;

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, say_thread, arg_say_port);
    if(rc) {
        perror("Infocast: pthread_create");
        fprintf(stderr, "Error: thread detaching failed in file %s line %d: %d\nExiting.\n", __FILE__, __LINE__, rc);
        exit(EXIT_FAILURE);
    }

    rc = pthread_setname_np(thread, "say_thread");
    if(rc) {
        perror("Infocast: say thread couldn't be named.");
        fprintf(stderr, "Error: thread naming failed in file %s line %d: %d\nExiting.\n", __FILE__, __LINE__, rc);
        exit(EXIT_FAILURE);
    }

    rc = pthread_detach(thread);
    if(rc) {
        perror("Infocast: say thread couldn't be detached.");
        fprintf(stderr, "Error: thread detaching failed in file %s line %d: %d\nExiting.\n", __FILE__, __LINE__, rc);
        exit(EXIT_FAILURE);
    }
}

void* say_thread(void* say_port) {

    struct sockaddr_in saddr;
    int sock, status;
    int on = 1;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = htons(*(uint16_t*)say_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        perror("Infocast sayThread: Error creating socket");
        exit(1);
    }

    status = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                         (char*)&on, sizeof(on));
    if(status) {
        perror("Infocast sayThread: setsockopt reuseaddr:");
        exit(1);
    }

    if(bind(sock, (struct sockaddr*) &saddr, sizeof(saddr)) == -1) {
        perror("Infocast sayThread: bind");
        exit(1);
    }

    char message[1024];

    while(1) {
        if(recv(sock, message, sizeof(message), 0) < 0) {
            perror("Say Thread Recv");
            continue;
        }

        char* ptr = message;
        int32_t magicByte;

        memcpy(&magicByte, ptr, sizeof(magicByte)); ptr += sizeof(magicByte);
        if(magicByte != (int32_t)SAY_MAGIC_BYTE) {
            printf("Magic byte miss match. Is %x should be %x\n", magicByte, SAY_MAGIC_BYTE);
            continue;
        }

        char version = *ptr; ptr++;
        if(version != SAY_VERSION) {
            printf("Version miss match. Is %d should be %d\n", version, SAY_VERSION);
            continue;
        }

        uint8_t len;
        memcpy(&len, ptr, sizeof(len)); ptr += sizeof(len);

        char* say = (char*)malloc(len+1);
        memcpy(say, ptr, len); ptr += len;
        say[len] = '\0';

        try {
            AL::ALTextToSpeechProxy tts("localhost", NAOQI_PORT);
            tts.say(say);
            free(say);
        } catch(const AL::ALError& e) {
            free(say);
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }

    return NULL;
}
