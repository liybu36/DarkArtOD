TARGET = SLADOD
#DEP=$(TARGET:.out=.d)
SRCS = $(addsuffix .C, $(TARGET))
DICTNAME = SLADOD_Dict
#DARKARTDIR = ../darkart/Products
#DARKARTHEADERS = $(DARKARTDIR)/EventInfo.hh $(DARKARTDIR)/Channel.hh $(DARKARTDIR)/Baseline.hh $(DARKARTDIR)/Pmt.hh $(DARKARTDIR)/WaveformInfo.hh $(DARKARTDIR)/Pulse.hh $(DARKARTDIR)/PulseParam.hh $(DARKARTDIR)/Roi.hh $ $(DARKARTDIR)/Spe.hh
OBJS = $(addsuffix .o, $(notdir $(basename $(SRCS))))
ROOTCFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS   = $(shell $(ROOTSYS)/bin/root-config --libs) -lProof -lHistPainter #-lMemStat
ROOTLDFLAGS = $(shell $(ROOTSYS)/bin/root-config --ldflags)
ROOTCINT = $(ROOTSYS)/bin/rootcint
CXXFLAGS   = $(ROOTCFLAGS) -Wall -g -fPIC  #-pthread 
CXXLIBS    = $(ROOTLIBS)
CC = g++ 


all: $(DICTNAME).so $(TARGET) 

%: %.o
	$(CC) $(CXXFLAGS) $(CXXLIBS) $(DICTNAME).so $< -o $@

$(OBJS): $(SRCS)
	$(CC) $(CXXFLAGS) -c $^


$(DICTNAME).cxx: 
	$(ROOTCINT) -f $@ -c -p $(SRCS)	 LinkDef.h 

$(DICTNAME).so: $(DICTNAME).cxx
	$(CC) -shared -o $@ $(ROOTLDFLAGS) $(CXXFLAGS) $(CXXLIBS) -I$(ROOTSYS)/include $^

clean:
	rm  -r $(OBJS)  $(DICTNAME).so $(DICTNAME).cxx $(DICTNAME).h

