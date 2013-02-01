all:	wlan.hpp wlanrecv.cpp nbr_entry.cpp
	g++ -pthread -o wlanrecv wlanrecv.cpp nbr_table.cpp nbr_entry.cpp
	g++ -o wlansend wlansend.cpp
