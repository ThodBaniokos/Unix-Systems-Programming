# Unix Systems Programming
These are my solution for the course K24 - Unix Systems Programming 2021

## Projects in Repository
This repository contains all three projects finished for the course, each one getting a score of 100/100, all implemented in C.

### Project 1
  The first project was oriented around the implementation and use of data structures, specificaly a <a href="https://en.wikipedia.org/wiki/Bloom_filter">Bloom Filter</a>, a <a href="https://en.wikipedia.org/wiki/Skip_list">Skip List</a>. A mix of these two data structures and others implemented for auxiliary purposes were used toghether with a generic way, in order to represent the desired output.
  
### Project 2
  The second project was oriented around process creation, IPC, low-level I/O, signal handling and bash scripting. The data structures used remained the same. All the processes are communicating with named pipes and in order to synchronize this system a poll struct was used.

### Project 3
  The third project was oriented around threads and thread safe programming, sockets and IPC through sockets. To achieve the communication a circular buffer was implemented and we also had to avoid busy-waiting as it is a bad practice to waste CPU cycles.
