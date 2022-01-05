trace_event:


1. tracepoint
a. enable
b. set_event
c. sys_perf_event_open: this mode can attach ebpf


2. k/retprobe
1). echo xxx > kprobe_events: the trace file comeout, register the kprobe in kernel 
Then like in the tracepoint


3.sys_enter/exit
This is like in the tracepoint

4. uporbe
This is like in the kprobe

5. perf_event
Only work int the sys_perf_event_open





