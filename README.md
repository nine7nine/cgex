# CGex (CGroups Exhibitor)

CGex is a small C library, daemon and CLI tool for use with Linux Control Groups (CGroups). Yes, there are some other tools in Systemd, libcgroup, etc. that are fantastic (and I use them), but my motivation for this project is a bit different.

1. I wanted a set of cgroup management tools with less complexity than some of the alternatives.
2. I wanted to build a small library to assist in building a daemon, CLI and GTK app for cgroups management.
3. This is an experiment in using Generative AI to develop C code more complex than just 100s of LOC in a single .c file. 

Currently, CGex consists of a few parts:

### LibCGex C Library
- libcgex.h
- libcgex.c
- libcgex-utils.c
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

cgexd (daemon) must be running before using cgexd-cli tool. cgexd doesn't take any arguments, but must be run as root. Eventually, cgexd will be a systemd service, and cxegd-cli will not require root priviledges. But given it's very early in development, both programs require elevated priviledges. They communicate using unix sockets. Below are exmamples of usage:

```
sudo ./cgexd-cli -g <cg_group> [-r <all|cg_attr> | -s <cg_attr> <cg_value> | -t <cg_type>]
```

Some of the basics are in-place. Such as being able to query, filter and modify cgroups data in /sys/fs/cgroup, which is usfeul, as is.
For example, if I wanted to know not only the cgroup.threads (PIDs) for a specific cgroup, but also what actual process name is associated
with each PID (something not provided in the cgroup.threads data!), I can easily get that information:

```
$ sudo ./cgexd-cli -g audio_rt -r cgroup.threads
cgroup.threads: 
1733 - jackdbus
2326 - jackdbus
2328 - jackdbus
2329 - jackdbus
1698 - pipewire
1729 - pw-data-loop
2335 - pipewire
2336 - pipewire
2347 - pipewire
1700 - pipewire-pulse
1727 - pw-data-loop
550 - irq/166-snd_hda_intel:card0
203 - irq/126-xhci_hcd

```

or in another scenario, if I wanted to look at just the cpu related settings in a cgroup:

```
$ sudo ./cgexd-cli -g audio_rt -t cpu
cpu.weight: 8668
cpu.stat: usage_usec 1112107376
cpu.weight.nice: -20
cpu.pressure: some avg10=0.23 avg60=0.26 avg300=0.17 total=188280932
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
sudo ./cgexd-cli -g audio_rt -s cpu.latency.nice -- -20
Updated cpu.latency.nice: -20
```

More to come.
