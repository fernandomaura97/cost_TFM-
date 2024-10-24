#!/bin/bash

# Simulation parameters
seed=1
simTime=1E3
distance=30

# Define bandwidth parameters
start_bandwidth=2.5E6
end_bandwidth=40E6
step_bandwidth=2.5E6

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
sleep 2

echo -e "\n\n********************************** COST results **********************************\n"

### Run the SimMM1K executable (once) ###

bandwidth_STA=50E6

rm out_log.ans
./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance| tee -a out_log.ans # FOR LOGGING

### Run the SimMM1K executable (loop) ###

# for bandwidth_STA in $(seq $start_bandwidth $step_bandwidth $end_bandwidth); do
#     echo -e "\n\n********************************** COST results for bandwidth_STA = $bandwidth_STA **********************************\n"
#     ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance
#      # Create a directory named after the current bandwidth_STA value with reduced decimals
#     folder_name=$(echo "$bandwidth_STA" | awk '{printf "%.1fMbps\n", $1/1E6}')
#     mkdir -p "Results/$folder_name"
    
#     # Move CSV files into the corresponding folder
#     mv Results/*.csv "Results/$folder_name/"

#      folder_list+=("Results/$folder_name") # To compress folders
# done


# zip -r Results_TFM.zip "${folder_list[@]}"

