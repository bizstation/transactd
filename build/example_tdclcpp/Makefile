SRC_ROOT := ../../source
CINCLUDE := -I$(BOOST_ROOT) \
	-I$(SRC_ROOT) \
	-I$(SRC_ROOT)/linux
CFLAGS := -fPIC -g -O0 -fexceptions -frtti -fpermissive -fno-strict-aliasing -fno-omit-frame-pointer \
	-DPIC \
	-DLINUX \
	-DBOOST_ALL_NO_LIB \
	-DUSE_DBM_CONNECTION_POOL
LIBDIR := -L/usr/lib64
LIBS := -ltdclcpp_64m -lrt -lstdc++ -ldl

STATICLIBS := $(BOOST_ROOT)/stage/lib/libboost_timer.a \
	$(BOOST_ROOT)/stage/lib/libboost_system.a \
	$(BOOST_ROOT)/stage/lib/libboost_chrono.a \
	$(BOOST_ROOT)/stage/lib/libboost_thread.a 

ALL_PROJECTS := \
	changeSchema \
	createDatabase \
	deleteRecords \
	dropDatabase \
	insertRecords \
	readRecords \
	update_with_transaction \
	updateRecords \
	changeSchema_c \
	createDatabase_c \
	deleteRecords_c \
	dropDatabase_c \
	insertRecords_c \
	readRecords_c \
	updateRecords_c \
	update_with_transaction_c \
	connection_pool_c \
	ormap_c \
	query_c


# default rule (make all projects)
all: boost_check $(ALL_PROJECTS)
.PHONY: all

# clean all projects
clean:
	$(RM) $(ALL_PROJECTS) *.o
.PHONY: clean


query_c: queryData.o
connection_pool_c: connectionPool.o

%: %.o
	$(CC) $(LIBDIR) $(CFLAGS) -o $@ $^ $(STATICLIBS) $(LIBS)


connectionPool.o: $(SRC_ROOT)/bzs/db/protocol/tdap/client/connectionPool.cpp
	$(CC) -c $(CINCLUDE) $(CFLAGS) -o $@ $<

%.o: $(SRC_ROOT)/bzs/example/%.cpp
	$(CC) -c $(CINCLUDE) $(CFLAGS) -o $@ $<


boost_check:
ifndef BOOST_ROOT
	@echo "Please set boost library path to BOOST_ROOT."
	exit 1
endif
ifeq ($(BOOST_ROOT),)
	@echo "Please set boost library path to BOOST_ROOT."
	exit 1
endif
.PHONY: boost_check
