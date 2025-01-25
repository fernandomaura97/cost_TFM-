#!/bin/bash
SERIAL=0 ## to control serial or parallel execution
NUMBER_OF_JOBS=8 ## to control nº of threads in parallel

# Define the function to execute on Ctrl+C
handle_interrupt() {
    echo "Simulation interrupted."
    exit 1;
}

# Set up the trap for SIGINT (Ctrl+C)
trap handle_interrupt SIGINT


# Simulation parameters
seed=1
simTime=1E2
distance=10
# N_BG=(1 2 3 4 5 6 7 8 9 10) 
N_BG=(2 3 4)
# alt_bandwidths=(10E6 50E6 100E6 200E6)
alt_bandwidths=(10E6 20E6 30E6 40E6 50E6 60E6 70E6 80E6 90E6 100E6)

# Define bandwidth parameters
# start_bandwidth=10E6
# end_bandwidth=80E6
# step_bandwidth=10E6
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
# rm out_log.ans

temp_file=$(mktemp)
for num_stas in "${N_BG[@]}"; do 

    # for bandwidth_STA in $(seq $start_bandwidth $step_bandwidth $end_bandwidth); do
    for bandwidth_STA in "${alt_bandwidths[@]}"; do
        echo -e "\n\n********************************** COST results for bandwidth_STA = $bandwidth_STA **********************************\n"
        
        if [ "$SERIAL" == 1 ]; then
            echo "SERIAL with out_log!!!!\n"
            sleep 1
            ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance $num_stas | tee out_log.ans
        fi
        echo ./SimpleSim $seed $simTime $bandwidth_STA 12000 $distance $num_stas >> "$temp_file"    

    done
done


parallel -j "$NUMBER_OF_JOBS" < "$temp_file"
rm "$temp_file"

echo "ALL SIMS COMPLETED!!!"
