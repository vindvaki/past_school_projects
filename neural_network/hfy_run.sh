#!/bin/bash

# example usage
#   ./hfy_run.sh prufa.pdf 

# recompile
#gcc -O3 hfy_loadfaces.c hfy_vbp.c -lblas -lnetpbm -o loadfaces
icc -fast hfy_loadfaces.c hfy_vbp.c -mkl -lnetpbm -o loadfaces

# set up the values to be passed to loadfaces
epochs=1000
eta=0.1
mu=0.3
files=`find faces -path "faces/*/*_4.pgm" -printf "%p,"`
files=${files%*,} # remove trailing comma

classes=sunglasses,open
nodes=2

#classes=right,left,straight,up,neutral,happy,sad,angry,sunglasses,open
#nodes=10

rm wimg*.pgm

./loadfaces -n$nodes -E$epochs -e$eta -m$mu -f$files -c$classes -x trainerror -y testerror -wwimg.pgm
pamsplit wimg.pgm wimg%d.pgm

# octave plot script
if [ $1 ]; then
    ./plot_data.m $1
else
    ./plot_data.m "run-nodes_$nodes-epochs_$epochs-eta_$eta-mu_$mu.eps"
fi
