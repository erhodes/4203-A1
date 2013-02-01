#include "nbr_entry.h"

nbr_entry::nbr_entry(){
    arrival = true;
    last_recorded = -1;
}

nbr_entry::nbr_entry(string addr){
    arrival = true;
    last_recorded = -1;
    address = addr;
}

void nbr_entry::newBeacon(){
    arrival = true;
    last_recorded = -1;
}

void nbr_entry::print(){
    cout << address << "     ";
    if (arrival){
        cout << "True       ";
    }
    else{
        cout << "False       ";
    }
    cout << last_recorded << endl;
}

void nbr_entry::update(){
    last_recorded++;
    arrival = false;
}
