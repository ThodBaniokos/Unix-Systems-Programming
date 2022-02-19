# file: Makefile
# used to compile the program and produce the executable

# compiler
CC = gcc

# modules path
MDLS = ./src

# header files path
HDRS = ./inc

# compile flags
CFLAGS = -g3 -Wall -O3 -I$(HDRS)

# main .o file in objective files variable
OBJS = vaccineMonitor.o

# executable name
PRGRM = vaccineMonitor

# extra libraries
LIB = -lm

# valgrind memcheck utility program flags
VALFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all

# program arguments
PRGRMFLAGS = -c ./input/citizenRecordsFile.txt -b 102400

# include all needed modules
include $(MDLS)/modules.inc

# compile the program with make
$(PRGRM) : $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(PRGRM) $(LIB)
	@echo "Build success."
	
# command to run the program
run : $(PRGRM)
	@echo "Creating citizen record file."
	@./testFile.sh ./input/viruses.txt ./input/countries.txt 5000 0
	@echo "Created citizen record file."
	@echo "Starting program."
	@./$(PRGRM) $(PRGRMFLAGS)

# command to clean the root and sub folders of all the .o file
clean : $(OBJS)
	@echo "Cleaning all directories of binaries."
	@rm $(OBJS)

# command to clean the root and sub folders of all the .o file and the executable
fullclean : $(OBJS)
	@echo "Cleaning all directories of binaries and executable."
	@rm $(OBJS) $(PRGRM)

# command to run the program to test for memory leaks
valgrind : $(PRGRM)
	@echo "Running program with memcheck."
	@valgrind $(VALFLAGS) ./$(PRGRM) $(PRGRMFLAGS)
