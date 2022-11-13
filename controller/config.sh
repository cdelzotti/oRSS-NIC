#!/bin/bash
# Bridge name
BRIDGE=ovsbr1
# Link the bridge to the SDN controller
sudo ovs-vsctl set-controller ovsbr1 tcp:0.0.0.0:6633
# Enable OpenFlow
sudo ovs-vsctl set bridge ovsbr1 protocols=OpenFlow10,OpenFlow11,OpenFlow12,OpenFlow13,OpenFlow14,OpenFlow15
# Set controller as out-of-band
sudo ovs-vsctl set controller ovsbr1 connection-mode=out-of-band
