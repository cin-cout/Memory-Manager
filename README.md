# Memory-Manager

Implement the memory management, including page table, TLB, Disk, frame, by 6 kinds of algorithms which you can set by yourself.
．
## Build the Project
* Install githook and compile output file ```MemManager```
```
make
```
* Delete output file
```
make clean
```
## Usage
* Execute
```
./MemManager
```
* Then, the process will read ```sys_config.txt``` to decide its policy. In this input type, we have below parameters
  * TLB Replacement Policy: ```LRU``` (least recently use) or ```RANDOM```
  * Page Replacement Policy: ```FIFO``` (first in first out) or ```CLOCK```
  * Frame Allocation Policy: ```GLOBAL``` or ```LOCAL```
  * Number of Processes: total process in this simulation
  * Number of Virtual Page: number of virtual page in each process
  * Number of Physical Frame: number of total frame
* After reading configuration file, the process will read ```trace.txt```, which is the reference input
  * Input format: ```Reference({process}, {page table number})```
* Finally, we will produce our output file in ```trace_output.txt```
  * Inside the file is like:
  ```
  Process A, TLB Hit, 17=>3
  Process A, TLB Miss, Page Hit, 32=>15
  Process A, TLB Hit, 32=>15
  Process A, TLB Miss, Page Hit, 34=>14
  Process A, TLB Hit, 34=>14
  Process A, TLB Miss, Page Fault, 12, Evict 68 of Process A to 28, 49<<-1
  Process A, TLB Hit, 49=>12
  ```
* The process also produce analysis.txt, inside the file we record each process's effective access time and Page fault
  * effective access time: α(m+t) + (1- α)(2m+t)
    * $\alpha$: hit ratio (TLB hit/ total TLB lookup)
    * m: memory cycle time, assume 100ns
    * t: TLB lookup time, assume 20ns
  * page fault rate: page fault rate/ total reference, (total reference didn't include TLB lookup which is after TLB miss)
