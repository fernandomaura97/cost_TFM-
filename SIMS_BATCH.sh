#!/bin/bash



seed=1
simtime=1E3

L_bg=12E3
d_STA=1
d_BG=30

# Define bandwidth parameters
start_bandwidth=2.5E6
end_bandwidth=40E6
step_bandwidth=2.5E6

# Define an array of simulation commands with the [NUM] placeholder
simulations=(


"./XRWiFi_P1 [NUM] 1E3 1 90 10E6 1 1 20E6 10000 2 1 0.3 0.9 1"


)


# "./XRWiFi_P1 [NUM] 1E3       1      90   10E6    1     1   20E6  10000    0       1                      0.3   0.9     1"
#                    T_end    n_xr  fps   R_xr   d_XR  Nbg BG_Rate #L_bg   BG_mode   RCA       (unused MABs: alpha, gamma, T_update)
                                                                        #  ^
                                                                        #  |
                                                                        #   -- (2 = UL + DL)
                                                                        #           (1 = UL) 
                                                                        #           (0 = DL)

for bandwidth_STA in $(seq $start_bandwidth $step_bandwidth $end_bandwidth); do
  ./XRWiFi_P1 $seed $simtime 1 90 $bandwidth_STA 1 1 $bandwidth_STA 12000 0 0 0.3 0.9 1
done





# Check if input arguments are provided
if [ $# -gt 0 ]; then
  num_args=$#
  args=("$@")  # Store all input arguments in an array

  num_simulations=${#simulations[@]}
  total_commands=$((num_args * num_simulations)) # Total number of commands

  # Create an array to store the expanded simulations
  expanded_simulations=()

  # Loop through each input argument
  for ((j=0; j<num_args; j++)); do
    seed="${args[$j]}" # Get the value of the argument at position j

    # Loop through each simulation command
    for ((i=0; i<num_simulations; i++)); do
      sim="${simulations[$i]}"
      expanded_simulations+=("${sim/\[NUM\]/$seed}")
    done
  done

  # Assign the expanded simulations to the main simulations array
  simulations=("${expanded_simulations[@]}")
fi

# Loop through each simulation command and execute it
for sim in "${simulations[@]}"; do
  echo "Running simulation: $sim"
  $sim
done

