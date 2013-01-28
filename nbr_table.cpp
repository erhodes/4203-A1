#include "nbr_table.h"

nbr_table::nbr_table()
{
}

void nbr_table::beaconRecieved(char *senderAddress){
    entries = new nbr_entry(senderAddress);
}

void nbr_table::printTable(){
    printf("Neighbour Discovery Table\n");
    printf("Address            Arrival           Last Recorded\n");
    entries->print();
    printf("\n");
}
