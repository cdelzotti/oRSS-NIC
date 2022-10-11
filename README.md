# oRSS-NIC

This is the offloaded part of oRSS that should run on the SmartNIC

## Overview

![](./img/Implementation.png)

The idea consists into offloading the balancing mechanism of RSS++ into a NVIDIA Bluefield-2 Smart-NIC. The main idea is to dispatch packets equally among cores while ensuring Flow-to-core affinity.

- **XDP programs** : Allows counting packets and bypassing the ARM Subsystem kernel on the data path.
- **RSS Offloading System** : Contains the offloading logic
  - **Connection Tracking** : Keeps track of connections and count packets
  - **Load balancing** : Based on connection tracking information, decide whether a flow must be moved from a core to another
  - **Packer reordering** : Reorders packets to improve cache efficiency
- **OVS** : Leverages OVS Hardware offloading support to offloads rules on the embedded Switch to avoid a huge performance penalty by going throught the ARM cores
- **Dispatching** : On the host, dispatch packets among cores accordingly to what has been decided on the NIC and ensure that each core starts processing new flows only when there isn't any waiting packets in other queues.

## OVS Customization

This project uses OVS, and requires a few changes once the project has been `./configure` :

- Checkout to v2.17.2 (Or any other version, depending on your installation)
- Change the run directory in `ovs/lib/dirs.c` to fit the one indicated in `systemctl status ovsdb-server`
- Change configuration in `ovs/config.h` to undefine any AVX512 variables