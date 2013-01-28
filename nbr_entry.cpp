#include "nbr_entry.h"

nbr_entry::nbr_entry(){
    arrival = true;
    last_recorded = -1;
}

nbr_entry::nbr_entry(char* addr){
    arrival = true;
    last_recorded = -1;
    address = addr;
}

void nbr_entry::print(){
    printf("%s      ",
           address);
    if (arrival){
        printf("True        ");
    }
    else{
        printf("False       ");
    }
    printf("%i \n",last_recorded);
}
