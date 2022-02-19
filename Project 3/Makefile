# file: Makefile
# used to compile the programs and produce the executables
# compiler
CC = gcc

# modules path
MDLS = ./src

# header files path
HDRS = ./include

# bin folder
BIN = ./bin

# objs
# travel monitor objs
TRV_M_C_OBJS = $(MDLS)/trv_mon_client/travelMonitorClient.o

# monitor objs
MON_S_OBJS = $(MDLS)/mon_server/monitorServer.o

# common objs
C_OBJS =

# compile flags
CFLAGS = -g3 -Wall -O3 -I$(HDRS)

# needed libs
LIBS = -lpthread

# executables name
TRV_M_C = travelMonitorClient
MON_S = monitorServer

# valgrind memcheck utility program flags
VALFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all --trace-children=yes

# program arguments
PRGRMFLAGS = -m 4 -b 512 -c 10 -s 102400 -i ./sample/input_dir -t 4

include $(MDLS)/source.inc

# main target, compiling both programs
all : $(TRV_M_C) $(MON_S)
	@echo "Created executables, moving binaries to bin/ directory."
	@mv $(TRV_M_C_OBJS) $(BIN)
	@mv $(MON_S_OBJS) $(BIN)
	@mv $(C_OBJS) $(BIN)

# compile travel monitor
$(TRV_M_C) : $(TRV_M_C_OBJS) $(C_OBJS)
	@echo "Compiled Travel Monitor Client Program."
	@$(CC) $(CFLAGS) $(TRV_M_C_OBJS) $(C_OBJS) -o $(TRV_M_C) $(LIBS)

# compile monitor
$(MON_S) : $(MON_S_OBJS) $(C_OBJS)
	@echo "Compiled Monitor Server Program."
	@$(CC) $(CFLAGS) $(MON_S_OBJS) $(C_OBJS) -o $(MON_S) $(LIBS)

.PHONY : run
run :
	@make -s all
	@echo "Creating input directory and running program."
	@./$(TRV_M_C) $(PRGRMFLAGS)

# memcheck tool, VALGRIND MUST BE INSTALL ON THE SYSTEM
.PHONY : valgrind
valgrind :
	@echo "Running program with memcheck."
	@valgrind $(VALFLAGS) ./$(TRV_M_C) $(PRGRMFLAGS)

# delete .o files from bin
.PHONY : clean
clean :
	@echo "Deleting binaries from bin/ directory."
	@rm $(BIN)/*

# delete the above and the executables
.PHONY : fullclean
fullclean :
	@echo "Deleting binaries from bin/ directory and executables from main directory."
	@rm $(BIN)/* $(TRV_M_C) $(MON_S)

.PHONY : errorclean
errorclean :
	@echo "Deleting binaries from source files directories."
	@rm $(TRV_M_C_OBJS) $(MON_S_OBJS) $(C_OBJS)