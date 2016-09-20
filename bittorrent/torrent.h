#ifndef __TORRENT_H__
#define __TORRENT_H__

#include "bendecoder/funzix-code/bencode/bencode.h"

enum T_STATUS { DOWNLOADING, PAUSED, STOPPED };

typedef struct t_list{
    be_node *torrent;
    char label[32];
    int id;
    enum T_STATUS t_status;
} T_LIST;

T_LIST* t_list_init();
be_node* t_list_get_by_label(T_LIST**, char* );
int t_list_add(T_LIST**, be_node*, char*);
int t_list_remove(T_LIST**, be_node*);
int t_list_print_names(T_LIST**);

int inc_id();

#endif