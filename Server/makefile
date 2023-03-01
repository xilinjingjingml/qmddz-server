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

# DEBUG can be set to YES to include debug info, or NO otherwise
DEBUG := YES
OUTPUT	:= ../bin/RPDDZV2Server
DEFS	:= 
INCS	:= -I./ \
	-I../ \
	-I../../../../../LobbyServer/tags/Protocol/20200907 \
	-I../../../../../LobbyServer/tags/Protocol/20200907/gen_src \
	-I../../../../../../libs/hglibs \
	-I../../../../../../libs/hglibs/BD_Common/libprotobuf-2.5 -I../../../../../../libs/boost_1_48_0 \
	-I../../../../../../libs/hglibs/BD_BaseHelper/sqlite \
	-I/usr/include/mysql

LIBS	:= -L../../../../../../libs/libraries -lboost_system-s \
	-L/usr/lib -lmysqlclient -lbd_basehelper -ldl -lpthread \
	../../../../../../libs/libraries/libprotobuf.a

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
   	$(wildcard ../../../../../LobbyServer/tags/Protocol/20200907/gen_src/world*.cpp) \
   	$(wildcard ../../../../../LobbyServer/tags/Protocol/20200907/gen_src/base*.cpp)

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
