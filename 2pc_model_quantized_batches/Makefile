CXX = g++
TARGET = server client
CXXFLAGS = -O0 -g -Wall
LIBS = -l:libaby.a -l:libencrypto_utils.a -l:librelic_s.a -l:libotextension.a -lgmp -lgmpxx -lssl -lcrypto -pthread -lpthread -lboost_system -lboost_filesystem
OBJECTS = common/predict.o common/utils.o
INCLUDES = common/predict.h common/utils.h
LIB_PATH = -L/usr/local/lib
INC_PATH = -I/usr/local/include
OPT_FLAGS =

ifdef WITH_PCA
	OPT_FLAGS += -DPCA
endif

ifdef USE_V1
	OPT_FLAGS += -DV1
endif

ifdef SAVE
	OPT_FLAGS += -DSAVE
endif

all : $(TARGET)

server : server.o $(OBJECTS)
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) $(OPT_FLAGS) -o $@ $^ $(LIBS)

server.o : server.cpp $(INCLUDES)
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) $(OPT_FLAGS) -c $^ 

client : client.o $(OBJECTS) 
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) $(OPT_FLAGS) -o $@ $^ $(LIBS) 

client.o : client.cpp $(INCLUDES)
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) $(OPT_FLAGS) -c $^ 

common/predict.o : common/predict.cpp common/predict.h
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) $(OPT_FLAGS) -c -o $@ $< 

common/utils.o : common/utils.cpp common/utils.h
	$(CXX) $(LIB_PATH) $(INC_PATH) $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f $(TARGET) $(OBJECTS) *.o common/*.gch