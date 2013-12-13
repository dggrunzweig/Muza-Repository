
CXX=g++
INCLUDES=

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
FLAGS=-D__LINUX_ALSASEQ__ -D__UNIX_JACK__ -c
#FLAGS=-D__LINUX_JACK__ -D__UNIX_JACK__ -c
LIBS=-lasound -lpthread -ljack -lstdc++ -lm
endif
ifeq ($(UNAME), Darwin)
FLAGS=-D__MACOSX_CORE__ -c
LIBS=-framework CoreAudio -framework CoreMIDI -framework CoreFoundation \
	-framework IOKit -framework Carbon  -framework OpenGL \
	-framework GLUT -framework Foundation -framework AppKit \
	-lstdc++ -lfluidsynth -lm 
endif


OBJS=  main.o RtAudio.o chuck_fft.o Thread.o Stk.o muza-globals.o mfluidsynth.o x-audio.o raka-audio.o muza-personality.o

main: $(OBJS)
	$(CXX) -o main $(OBJS) $(LIBS)

main.o: main.cpp RtAudio.h chuck_fft.h Stk.h Thread.h muza-globals.h mfluidsynth.h x-audio.h raka-audio.h muza-personality.h
	$(CXX) $(FLAGS) main.cpp

Thread.o: Thread.cpp Thread.h 
	$(CXX) $(FLAGS) Thread.cpp

Stk.o: Stk.h Stk.cpp
	$(CXX) $(FLAGS) Stk.cpp

RtAudio.o: RtAudio.h RtAudio.cpp RtError.h
		$(CXX) $(FLAGS) RtAudio.cpp

chuck_fft.o: chuck_fft.h chuck_fft.c
		$(CXX) $(FLAGS) chuck_fft.c

mfluidsynth.o: mfluidsynth.h mfluidsynth.cpp
		$(CXX) $(FLAGS) mfluidsynth.cpp

muza-globals.o: muza-globals.h muza-globals.cpp
		$(CXX) $(FLAGS) muza-globals.cpp

x-audio.o: x-audio.h x-audio.cpp
		$(CXX) $(FLAGS) x-audio.cpp

raka-audio.o: raka-audio.h raka-audio.cpp
		$(CXX) $(FLAGS) raka-audio.cpp

muza-personality.o: muza-personality.h muza-personality.cpp
		$(CXX) $(FLAGS) muza-personality.cpp

clean:
	rm -f *~ *# *.o main
