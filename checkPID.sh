#!/usr/bin/env bash
pid=$(pgrep GoogleDrummingArm)
if [ -n "$pid" ]
then
#    echo "killing $pid (GoogleDrummingArm)"
    kill "$pid"
fi

pid=$(pgrep EMGDataServer)
if [ -n "$pid" ]
then
#    echo "killing $pid (EMGDataServer)"
    kill "$pid"
fi
