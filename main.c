#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "input_handler/input_handler.h"
#include "command/command.h"
#include "error/error.h"
#include "bittorrent/bendecoder/funzix-code/bencode/bencode.h" 
#include "bittorrent/torrent.h"
#include <time.h>
#include <curl/curl.h>

#define BUF_SIZE 1024
#define S_SIZE 256
#define MAX_IN 32
#define MAX_ARGZ 10

T_LIST* torrent_list;

static fd_set read_fds;
static int max_fd;
char peer_id[20];

void
generate_peer_id(char* buf){
    const int size = 20;
    memset(buf, 0, size);
    sprintf(buf,"%lu%d",time(NULL), getpid());
    CURL* curl = curl_easy_init();
    if(curl){
        char* encoded = curl_easy_escape(curl, buf, 0);
        strncpy(buf, encoded, size);
        curl_free(encoded);
    }
    
    fflush(stdout);
}

void
add_fd(int fd){
    FD_SET(fd, &read_fds);
    if(fd > max_fd){
        max_fd = fd;
    }
}

static void
help(){
    printf("pTorrent - uTorrent clone \n\n\
Downloads torrents from the command line.\n\
Options:\n\
    -s -- start the program with all torrents stopped\n\
    -p -- start the program with all torrents paused\n\
    -h -- print this help message\n\
\n\
Author: Cosma Adrian aka NoNy\n\n");
}

static void 
usage(char* pname){
    printf("Usage: %s[-h] [-l <torrent-name>]\n\n", pname);
}


int 
main(int argc, char** argv){

    int c, lflag = 0;
    char* torrent_name = (char*)calloc(1, S_SIZE*sizeof(char));

    while((c = getopt(argc, argv, "hl:")) != -1){
        switch(c){
            case 'l':
                lflag = 1;
                strncpy(torrent_name, optarg, strlen(optarg));
                break;
            case 'h':
                help();
                return 0;
            default:
                usage(argv[0]);
                return -1;
        }
    }

    torrent_list = t_list_init();
    generate_peer_id(peer_id);

    if (lflag){
        c_load(1, &torrent_name);
    }
    free(torrent_name);

    char buffer[BUF_SIZE];
    int r;
    fd_set tmp_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    FD_SET(STDIN_FILENO, &read_fds);
    max_fd = STDIN_FILENO;
    int i = 0;
    for(;;) {
        tmp_fds = read_fds;
        memset(buffer, 0, MAX_IN);
        if(select(max_fd + 1, &tmp_fds, NULL, NULL, NULL) >= 0){
            
            if(FD_ISSET(STDIN_FILENO, &tmp_fds)) {
                if(fgets(buffer, MAX_IN, stdin) == NULL){
                    printf("Error fetching input.\n");
                    continue;
                } else {
                    buffer[strlen(buffer)-1] = '\0';
                }
                char* argz[MAX_ARGZ];
                memset(argz, 0 , MAX_ARGZ);
                
                int arg_count = 0;
                if((r = process(buffer, argz, &arg_count)) < 0){
                    print_error(r);
                    continue;
                }

                void (*cmd)(int, char**) = NULL;
                if((r = get_command_info(argz[0], arg_count - 1, &cmd)) >= 0){
                    (*cmd)(arg_count, argz);
                } else {
                    print_error(r);
                    continue;
                }
            } 
        }
    }

    return 0;
}
