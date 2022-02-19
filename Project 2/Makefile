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
TRV_M_OBJS = $(MDLS)/trvl_mon/travelMonitor.o

# monitor objs
MON_OBJS = $(MDLS)/mon/monitor.o

# common objs
C_OBJS =

# compile flags
CFLAGS = -g3 -Wall -O3 -I$(HDRS)

# executables name
TRV_M = travelMonitor
MON = Monitor

# valgrind memcheck utility program flags
VALFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all --trace-children=yes

# program arguments
PRGRMFLAGS = -m 4 -b 512 -s 102400 -i ./input_dir

include $(MDLS)/source.inc

# main target, compiling both programs
all : $(TRV_M) $(MON)
	@echo "Created executables, moving binaries to bin/ directory."
	@mv $(TRV_M_OBJS) $(BIN)
	@mv $(MON_OBJS) $(BIN)
	@mv $(C_OBJS) $(BIN)

# compile travel monitor
$(TRV_M) : $(TRV_M_OBJS) $(C_OBJS)
	@echo "Compiled Travel Monitor Program."
	@$(CC) $(CFLAGS) $(TRV_M_OBJS) $(C_OBJS) -o $(TRV_M)

# compile monitor
$(MON) : $(MON_OBJS) $(C_OBJS)
	@echo "Compiled Monitor Program."
	@$(CC) $(CFLAGS) $(MON_OBJS) $(C_OBJS) -o $(MON)

.PHONY : run
run :
	@make -s all
	@echo "Creating input directory and running program."
	@./create_infiles.sh ./sample/citizenRecordsFile.txt input_dir 10
	@./$(TRV_M) $(PRGRMFLAGS)

# memcheck tool, VALGRIND MUST BE INSTALL ON THE SYSTEM
.PHONY : valgrind
valgrind :
	@echo "Running program with memcheck."
	@valgrind $(VALFLAGS) ./$(TRV_M) $(PRGRMFLAGS)

# delete .o files from bin
.PHONY : clean
clean :
	@echo "Deleting binaries."
	@rm $(BIN)/*

# delete the above and the executables
.PHONY : fullclean
fullclean :
	@echo "Deleting binaries and executables."
	@rm $(BIN)/* $(TRV_M) $(MON)
