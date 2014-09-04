#!/usr/bin/octave -qf

clear;
arg_list = argv();

# get output filename from the command line arguments
output_filename = arg_list{1};

# load errors
trainerror = load("trainerror");
testerror = load("testerror");

# plot errors
plot(trainerror, 'b', 'LineWidth', 2, testerror, ' r', 'LineWidth', 2);
xlabel('epoch');
ylabel('error');
print(output_filename, '-color');
