#**********************************************************
#
# by wolfplus
# 2006/04/04
#
#**********************************************************

#**********************************************************
#
#	user design
#**********************************************************


PROTOCOL_DIR := ../../../LobbyServer/branches/LobbyServer_qmmj/Protocol
LIBS_DIR := ../../../../libs

# DEBUG can be set to YES to include debug info, or NO otherwise
DEBUG := YES
OUTPUT	:= ../bin/RPDDZV2Server
DEFS	:= 
INCS	:= -I./ \
	-I../ \
	-I$(PROTOCOL_DIR) \
	-I$(PROTOCOL_DIR)/gen_src \
	-I$(LIBS_DIR)/hglibs \
	-I$(LIBS_DIR)/hglibs/BD_Common/libprotobuf-2.5 -I$(LIBS_DIR)/boost_1_48_0 \
	-I$(LIBS_DIR)/hglibs/BD_BaseHelper/sqlite \
	-I/usr/include/mysql

LIBS	:= -L$(LIBS_DIR)/libraries -lboost_system-s \
	-L/usr/lib -lmysqlclient -lbd_basehelper -ldl -lpthread \
	$(LIBS_DIR)/libraries/libprotobuf.a

#**********************************************************

CXX		:= g++
LD		:= g++
AR		:= ar rc
RANLIB	:= ranlib

DEBUG_CXXFLAGS		:= -Wall -g -DDEBUG
RELEASE_CXXFLAGS	:= -Wall -O3 -DNDEBUG

DEBUG_LDFLAGS		:= -g
RELEASE_LDFLAGS		:=

ifeq (YES, $(DEBUG))
	CXXFLAGS		:= $(DEBUG_CXXFLAGS) $(DEFS)
	LDFLLAGS		:= $(DEBUG_LDFLAGS) $(DEFS)
else
	CXXFLAGS		:= $(RELEASE_CXXFLAGS) $(DEFS)
	LDFLLAGS		:= $(RELEASE_CSSFLAGS) $(DEFS)
endif

all:$(OUTPUT)

SRCS	:= $(wildcard *.cpp) $(wildcard ../Common/*.cpp) $(wildcard Protocol/BaseGameServer/*.cpp) $(wildcard ../gen_src/*.cpp) \
   	$(wildcard $(PROTOCOL_DIR)/gen_src/world*.cpp) \
   	$(wildcard $(PROTOCOL_DIR)/gen_src/base*.cpp)

OBJS	:= $(addsuffix .o, $(basename $(SRCS)))

DEPS	:= $(addsuffix .d, $(basename $(SRCS)))
#**********************************************************
#	OUTPUT
#**********************************************************

$(OUTPUT):$(OBJS)
	$(LD) -o $@ $(CXXFLAGS) $(OBJS) $(LIBS)
	
#**********************************************************
#	common rule
#**********************************************************
include $(subst .cpp,.d,$(SRCS))

%.d : %.cpp
	$(CXX) -M $(CPPFLAGS) $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$;

%.o : %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCS) $<

clean:
	rm -f $(OBJS) $(OUTPUT) $(DEPS)
