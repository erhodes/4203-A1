all:	wlan.hpp wlanrecv.cpp
	g++ -o wlanrecv wlanrecv.cpp
	g++ -o wlansend wlansend.cpp
