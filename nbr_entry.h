#ifndef NBR_ENTRY_H
#define NBR_ENTRY_H

#include <stdio.h>
#include <stdlib.h>

class nbr_entry
{
public:
    nbr_entry();
    nbr_entry(char*);
    void print();

protected:
    char* address;
    int last_recorded;
    bool arrival;
};

#endif // NBR_ENTRY_H
