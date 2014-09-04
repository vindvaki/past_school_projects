schmorpho : schmorpho-asm
schmorpho-asm : thattari.o lesgrein.o CodeGen.cpp CodeGen.h
	g++ -o schmorpho-asm thattari.o lesgrein.o CodeGen.cpp -lfl
thattari.cpp thattari.hpp : thattari.y
	bison -d thattari.y -o thattari.cpp
thattari.o : thattari.cpp
	g++ -c thattari.cpp 
lesgrein.cpp : lesgrein.l 
	flex -o lesgrein.cpp lesgrein.l 
lesgrein.o : lesgrein.cpp thattari.hpp
	g++ -c lesgrein.cpp

clean :
	rm -f *.bak thattari thattari.cpp lesgrein.cpp \
		thattari.o lesgrein.o thattari.hpp
