TARGET = reconneutronMain 
CC = g++
ROOTCINT = $(ROOTSYS)/bin/rootcint
DICTNAME = reconneutronMain_dict
SRCS = $(addsuffix .C, $(TARGET))
DIR = .
SOURCES = $(DIR)/reconneutronClass.C 
HEADERS = $(DIR)/reconneutronClass.h
#DEPS = $(DIR)/G4DSdata.hh
OBJS = $(addsuffix .o, $(notdir $(basename $(SRCS))))
SOBJS = $(addsuffix .o, $(notdir $(basename $(SOURCES))))
ROOTCFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags) #-Wall -fPIC -g 
ROOTLIBS   = $(shell $(ROOTSYS)/bin/root-config --libs) -lProof -lProofPlayer
ROOTLDFLAGS = $(shell $(ROOTSYS)/bin/root-config --ldflags)

all: $(DICTNAME).C
	$(CC) $(ROOTCFLAGS) $(ROOTLIBS) $(SRCS) $(SOURCES) $^ -o $(TARGET) 

$(DICTNAME).C: $(HEADERS) $(DEPS)
	$(ROOTCINT) -f $@ -c $^

.PHONY: clean

clean:
	rm -rf $(DIR)/*_dict.h $(DIR)/*_dict.C