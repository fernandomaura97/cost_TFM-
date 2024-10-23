#!/bin/bash

# Simulation parameters
seed=1
simTime=20
bandwidth_STA=12E6
distance=1
clear

# Function to show ellipsis while compiling
show_dots() {
    while true
    do
        printf "."
        sleep 0.5
    done
}

echo -e "\nCompiling\n\n"

# Start displaying dots in the background
show_dots &
dots_pid=$!

# Compile the project
./build

# Stop displaying dots once compilation is done
kill $dots_pid
wait $dots_pid 2>/dev/null
echo "OK!"
echo -e "\n\n********************************** COST results **********************************\n"

# Run the SimMM1K executable


# ./MM1K $seed $simTime $bandwidth_STA $bandwidth_departures $K_system $L_packets $PRINT_PROBABILITIES_SS $N_STAs $distance_STAs $SIMPLE_TX_MODEL $DBG_DETERMINISTIC_LOC | more

rm out_log.ans
./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance| tee -a out_log.ans # FOR LOGGING
# ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance