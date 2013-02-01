#ifndef NBR_TABLE_H
#define NBR_TABLE_H

#include "nbr_entry.h"
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>

using namespace std;

class nbr_table
{
public:
    nbr_table();
    //the random table update thing. Resets values to false, increments time?
    void update();
    /*called when a beacon is recieved. If the neighbor is already in the table, update as necessary.
      Otherwise add it into the table
      */
    void beaconRecieved(string senderAddress);

    /*
      prints out details on every element in entries
      */
    void printTable();
protected:
    vector<nbr_entry> entries;
    static const int MAX_ELAPSE_TIME = 30;

    bool contains(string address);
};

#endif // NBR_TABLE_H
