#!/bin/bash
SERIAL=0 ## to control serial or parallel execution
NUM_JOBS=8 ## to control nº of threads in parallel

# Define the function to execute on Ctrl+C
handle_interrupt() {
    echo "Simulation interrupted."
    exit 1;
}

# Set up the trap for SIGINT (Ctrl+C)
trap handle_interrupt SIGINT
NUMBER_OF_JOBS=8


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

# rm out_log.ans
# ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance| tee -a out_log.ans # FOR LOGGING

### Run the SimMM1K executable (loop) ###
rm out_log.ans

temp_file=$(mktemp)

for bandwidth_STA in $(seq $start_bandwidth $step_bandwidth $end_bandwidth); do
    echo -e "\n\n********************************** COST results for bandwidth_STA = $bandwidth_STA **********************************\n"
    
    if [ "$SERIAL" ]; then
        ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance
    fi
    echo ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance >> "$temp_file"    

    
done


parallel -j "$NUMBER_OF_JOBS" < "$temp_file"
rm "$temp_file"

echo "ALL SIMS COMPLETED!!!"
