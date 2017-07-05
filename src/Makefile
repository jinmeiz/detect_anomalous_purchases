# tested compilers: clang++, g++-7 and g++-6 
CXX = g++

CXXFLAGS =	-std=c++11 -O2 -g -Wall -fmessage-length=0

OBJS = main.o user_info.o network.o

TARGET =	anomaly_detection

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

main.o: main.cpp network.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<  
	
network.o: network.cpp network.h user_info.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $< 
	
user_info.o: user_info.cpp user_info.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

clean:
	rm -f $(OBJS) $(TARGET)
