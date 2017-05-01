CPP = g++
RM = rm -f

MYSQL_CONFIG = /usr/local/mysql/bin/mysql_config
MYSQL_INCS = `$(MYSQL_CONFIG) --include` \
			 `$(MYSQL_CONFIG) --cflags`
#MYSQL_LIBS = `$(MYSQL_CONFIG) --libs`
MYSQL_LIBS = -L/usr/local/lib -L/usr/local/mysql/lib \
			 -Wl,-Bstatic -lmysqlclient -Wl,-Bdynamic -lmygcc \
			 -lz -lcrypt -lnsl -lm -lc -lnss_files -lnss_dns \
			 -lresolv -lc -lnss_files -lnss_dns -lresolv

CPPFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX
INCS = -I. -I.. -I../../../ $(MYSQL_INCS) -I../../../../ext/c4b_4.1/include
LIBS = ../../../petlib.a \
	   ../../../../ext/ttc-2.1.9-sles10/lib/libttc.a \
	   ../../../../ext/c4b_4.1/lib/libc4b_api.a \
	   ../../../../ext/c4b_4.1/lib/libdnscli_V1.1R130.a

TARGETS = vip_api_tool vip_ttc_tool vip_state_tool vip_sync_tool

all:$(TARGETS)

vip_api_tool: vip_api_tool.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

vip_ttc_tool: vip_ttc_tool.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

vip_state_tool: vip_state_tool.o vip_tool.o
	$(CPP) $(INCS) $^ $(LIBS) $(MYSQL_LIBS) -o $@

vip_sync_tool: vip_sync_tool.o vip_tool.o
	$(CPP) $(INCS) $^ $(LIBS) $(MYSQL_LIBS) -o $@

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

install: all
	jmscp vip_ttc_tool 172.25.40.55 /usr/local/pet50/vip_tools
	jmscp vip_api_tool 172.25.40.55 /usr/local/pet50/vip_tools
	jmscp vip_state_tool 172.25.40.55 /usr/local/pet50/vip_tools 
	jmscp vip_sync_tool 172.25.40.55 /usr/local/pet50/vip_tools 
	#jmscp vip_tool.ini 172.25.40.55 /usr/local/pet50/vip_tools
	#jmscp ../vip_api.ini 172.25.40.55 /usr/local/pet50/conf

