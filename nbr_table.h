#ifndef NBR_TABLE_H
#define NBR_TABLE_H

#include "nbr_entry.h"
#include <stdio.h>
#include <stdlib.h>

class nbr_table
{
public:
    nbr_table();
    //the random table update thing. Resets values to false, increments time?
    void update();
    /*called when a beacon is recieved. If the neighbor is already in the table, update as necessary.
      Otherwise add it into the table
      */
    void beaconRecieved(char* senderAddress);
    void printTable();
protected:
    nbr_entry* entries;
};

#endif // NBR_TABLE_H
