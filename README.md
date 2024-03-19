# CGex (CGroups Exhibitor)

CGex is a set of tools for managing and exploring CGroups. This repo is also an experiment in using ChatGPT to develop these tools.

Currently, CGex consists of 3 parts:

### LibCGex C Library
- libcgex.h
- libcgex.c
### cgex CLI interface
- cgex.c
### cgexd CLI and Daemon
- cgexd.c (daemon)
- cgexd-cli (cli for daemon)
### cgex-gtk (dummy app)
- cgex-gtk.c

NOTE: A Gtk3 app is planned, but for now there is much more work to do, first.

Compilation:

```
make
```
or with GCC's AddressSanitzer: ( it's C after all. haha! )

```
make SANITIZE=1
```

## Usage:

Both cgex and cgexd-cli have the same CLI interface. However, if using cgexd-cli; you must first start cgexd (as root). As well, with
cgexd-cli you will not see any output, as the cli will only execute, while cgexd hadles actually using libcgex. But othersie, they behave 
exactly the same in functionality.

```
sudo ./cgex -g <cg_group> [-r <all|cg_attr> | -s <cg_attr> <cg_value> | -t <cg_type>]
```

Some of the basics are in-place. Such as being able to query, filter and modify cgroups data in /sys/fs/cgroup, which is usfeul, as is.
For example, if I wanted to know not only the cgroup.threads (PIDs) for a specific cgroup, but also what actual process name is associated
with each PID (something not provided in the cgroup.threads data!), I can easily get that information:

```
$ sudo ./cgex -g audio_rt -r cgroup.threads
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
$ sudo ./cgex -g audio_rt -t cpu
cpu.weight: 8668
cpu.stat: usage_usec 263614372
cpu.weight.nice: -20
cpu.pressure: some avg10=0.00 avg60=0.00 avg300=0.00 total=57952388
cpu.idle: 0
cpu.stat.local: throttled_usec 0
cpu.max.burst: 0
cpu.max: max 100000
cpu.uclamp.min: 0.00
cpu.latency.nice: -20
cpu.uclamp.max: max
```

If i want to modify a cgroup setting's value:

```
sudo ./cgex -g audio_rt -s cpu.latency.nice -- -20
Updated cpu.latency.nice: -20
```

More to come.
