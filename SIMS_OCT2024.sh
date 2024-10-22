#!/bin/bash


seed=1
simtime=1E3

L_bg=12E3
d_STA=1
d_BG=30

n_BG=1

n_XR=0  ## unused
fps=90  ## unused
d_XR=6 ## unused
R_xr=1E3

# Define bandwidth parameters
start_bandwidth=2.5E6
end_bandwidth=40E6
step_bandwidth=2.5E6

BG_mode=0 ## 0 = DL only
          ## 1 = UL , 2 = UL+DL

./build

# "./XRWiFi_P1 [NUM] 1E3       1      90   10E6    1     1   20E6  10000    0       1                      0.3   0.9     1"
#                    T_end    n_xr  fps   R_xr   d_XR  Nbg BG_Rate #L_bg   BG_mode   RCA       (unused MABs: alpha, gamma, T_update)
                                                                        #  ^
                                                                        #  |
                                                                        #   -- (2 = UL + DL)
                                                                        #           (1 = UL) 
                                                                        #           (0 = DL)


rm out_log.ans
for bandwidth_STA in $(seq $start_bandwidth $step_bandwidth $end_bandwidth); do
    # echo -e "Running with $bandwidth_STA!!\n"
    ./XRWiFi_P1 $seed $simtime $n_XR $fps $R_xr $d_XR $n_BG $bandwidth_STA $L_bg $BG_mode 1 0.3 0.9 1 | tee -a out_log.ans
done
