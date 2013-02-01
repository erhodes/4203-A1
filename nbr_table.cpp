#include "nbr_table.h"

nbr_table::nbr_table()
{
}

void nbr_table::beaconRecieved(string senderAddress){
    /*
    if (!contains(senderAddress)){
        nbr_entry temp(senderAddress);
        entries.push_back(temp);
    }
    else{
        //entries.newBeacon();
    }
    */
    for (int i = 0; i < entries.size(); i++){
        if (entries.at(i).getAddress() == senderAddress){
            entries.at(i).newBeacon();
            return;
        }
    }
    //if the program makes it here, we need to add a new entry
    nbr_entry temp(senderAddress);
    entries.push_back(temp);
}

void nbr_table::update(){
    for (int i = 0; i < entries.size(); i++){
        entries.at(i).update();
        if (entries.at(i).getLastRecorded() > MAX_ELAPSE_TIME){
	    //let the user know that something was deleted
	    cout <<"IARP: neighbour lost protocol: " << entries.at(i).getAddress() << endl;
            //delete the current element
            entries.erase(entries.begin()+i);
            //modify i to acommodate the change
            i--;
        }
    }
}

void nbr_table::printTable(){
    printf("Neighbour Discovery Table\n");
    printf("Address            Arrival           Last Recorded\n");
    for (int i = 0; i < entries.size(); i++){
        entries.at(i).print();
    }
    printf("\n");
}
