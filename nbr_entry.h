#ifndef NBR_ENTRY_H
#define NBR_ENTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

class nbr_entry
{
public:
    nbr_entry();
    nbr_entry(string);
    void print();
    void update();
    void newBeacon();

    string getAddress(){ return address;  }
    int getLastRecorded() { return last_recorded; }
    bool getArrival() { return arrival; }

protected:
    string address;
    int last_recorded;
    bool arrival;
};

#endif // NBR_ENTRY_H
