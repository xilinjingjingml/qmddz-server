#**********************************************************
#
# by wolfplus
# creator:	2006/04/04
# update:	2006/08/10
#**********************************************************

#**********************************************************
#
#	user design
#**********************************************************

# DEBUG can be set to YES to include debug info, or NO otherwise
DEBUG := YES
OUTPUT	:= ../bin/RPDDZRobot

PROTOCOL_DIR := ../../../LobbyServer/branches/LobbyServer_qmmj/Protocol
LIBS_DIR := ../../../../libs



DEFS	:= 
INCS	:= -I./ \
	-I$(PROTOCOL_DIR) \
	-I$(LIBS_DIR)/hglibs \
	-I$(LIBS_DIR)/boost_1_48_0 \
	-I/usr/include/mysql \
	-I$(LIBS_DIR)/hglibs/BD_BaseHelper/sqlite

LIBS	:= -L$(LIBS_DIR)/libraries -lbd_basehelper -ldl -lpthread \
	-L/usr/include/mysql -lmysqlclient -lboost_system-s

#**********************************************************

CXX		:= g++
LD		:= g++
AR		:= ar rc
RANLIB	:= ranlib

DEBUG_CXXFLAGS		:=  -Wall -g -DDEBUG
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

SRCS	:= $(wildcard *.cpp)

OBJS	:= $(addsuffix .o, $(basename $(SRCS)))

DEPS	:= $(addsuffix .d, $(basename $(SRCS)))
#**********************************************************
#	OUTPUT
#**********************************************************

$(OUTPUT):$(OBJS)
	$(LD) -o $@ $(CXXFLAGS) $(OBJS) $(LIBS);
	
#**********************************************************
#	common rule
#**********************************************************

%.o : %.cpp
	$(CXX) -c  $(CXXFLAGS) $(INCS) $<
include $(subst .cpp,.d,$(SRCS))

%.d : %.cpp
	$(CXX) -M $(CPPFLAGS) $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$;

clean:
	rm -f $(OBJS) $(OUTPUT) $(DEPS)
