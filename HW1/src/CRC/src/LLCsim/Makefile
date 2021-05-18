all:
	g++ -DCRC_KIT -c main.cpp
	g++ -DCRC_KIT -c replacement_state.cpp
	g++ main.o replacement_state.o -o HW2
OJ:
	echo "#define CRC_KIT" > define.cpp
	cat define.cpp utils.h crc_cache_defs.h replacement_state.h replacement_state.cpp main.cpp > submit.cpp
	sed -i '/#include.*\"/d' submit.cpp
	g++ submit.cpp -o HW2 
