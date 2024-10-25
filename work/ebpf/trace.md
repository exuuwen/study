trace_event:

1. tracepoint
a. enable
b. set_event

//id is read number of events/xxx/id
attr.config = id;
c. sys_perf_event_open(attr,xxx, bpf_fd): this mode can attach ebpf


2. k/retprobe
/sys/kernel/debug/tracing/kprobe_events
1). echo xxx > kprobe_events: the trace file come out, register the kprobe in kernel 
Then like in the tracepoint


3.sys_enter/exit
This is like in the tracepoint

4. uporbe
This is like in the kprobe

5. perf_event
Only work in the sys_perf_event_open





