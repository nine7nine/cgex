# CGex (CGroups Exhibitor)

CGex is a set of tools for managing and exploring CGroups. This repo is also an experiment in using ChatGPT to develop these tools.

Currently, CGex consists of 3 parts:

### LibCGex C Library
- libcgex.h
- libcgex.c

### cgex CLI interface
- cgex.c

NOTE: A cgexd daemon and Gtk3 app are planned, but for now there is other work to do first.

CGex CLI Compilation:

```
gcc -o cgex cgex.c libcgex.c
```

Usage:

```
./cgex -g <cgroup_path> [-r <all|setting> | -s <setting> <value> | -t <type>]
```

Some of the basics are in-place. Such as being able to query, filter and modify cgroups data in /sys/fs/cgroup, which is usfeul, as is.
For example, if I wanted to know not only the cgroup.threads (PIDs) for a specific cgroup, but also what actual process name is associated
with each PID (something not provided in the cgroup.threads data!), I can easily get that information:

```
$ sudo ./cgex -g /sys/fs/cgroup/audio_rt -r cgroup.threads
cgroup.threads:
1857 - jackdbus
2448 - jackdbus
2455 - jackdbus
2456 - jackdbus
1825 - pipewire
1855 - pw-data-loop
2467 - pipewire
2468 - pipewire
2474 - pipewire
1827 - pipewire-pulse
1854 - pw-data-loop
570 - irq/166-snd_hda_intel:card0
200 - irq/126-xhci_hcd
```

or in another scenario, if I wanted to look at just the cpu related settings in a cgroup:

```
$ sudo ./cgex -g /sys/fs/cgroup/audio_rt -t cpu
cpuset.cpus.exclusive.effective: 
cpuset.cpus.exclusive: 
cpu.weight: 8668
cpu.stat: usage_usec 1185763772
cpu.weight.nice: -20
cpu.pressure: some avg10=0.00 avg60=0.04 avg300=0.02 total=196289789
cpu.idle: 0
cpu.stat.local: throttled_usec 0
cpuset.cpus.partition: member
cpuset.cpus.effective: 0-7
cpu.max.burst: 0
cpu.max: max 100000
cpu.uclamp.min: 0.00
cpuset.mems: 
cpu.latency.nice: -20
cpuset.mems.effective: 0
cpuset.cpus: 0-7
cpu.uclamp.max: max
```

If i want to modify a cgroup setting's value:

```
sudo ./cgex -g /sys/fs/cgroup/audio_rt -s cpu.latency.nice -- -20
Updated cpu.latency.nice: -20
```

More to come.
