#include "hfy_vbp.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    int i, j;

    int number_of_patterns = 4;
    int number_of_layers = 3;
    int number_of_epochs = 2000;

    // xor test
    double x [4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double y [4][1] = { {0}, {1}, {1}, {0} };
    int nodes [] = {2, 2, 1};

    double **input  = (double**)malloc(number_of_patterns*sizeof(double*));
    double **target = (double**)malloc(number_of_patterns*sizeof(double*));
    double *output = (double*)calloc(nodes[number_of_layers-1], sizeof(double));

    double eta = 1.3;
    double mu = 0;
    double net_errors[number_of_epochs];

    vbp_neural_network *net = vbp_allocate_net(number_of_layers, nodes);

    for(i = 0; i < number_of_patterns; ++i) {
        input[i]  = (double*)calloc(nodes[0],sizeof(double));
        target[i] = (double*)calloc(nodes[0],sizeof(double));
        for(j = 0; j < nodes[0]; ++j) {
            input[i][j]  = x[i][j];
            target[i][j] = y[i][j];
        }
    }

    vbp(input, target, nodes, 
            number_of_patterns, 
            number_of_layers, 
            number_of_epochs, 
            eta, mu, net, 
            net_errors); 

    printf("Final network error:\n  %f\n\n", net_errors[number_of_epochs-1]);
    printf("Example run output:\n  ");

    for(i = 0; i < number_of_patterns; ++i) {
        vbp_forward_sweep(input[i], output, net);
        for(j = 0; j < nodes[number_of_layers-1]; ++j) {
            printf("%f ", output[j]);
        }
    }
    printf("\n\nTarget values:\n  ");
    for(i = 0; i < number_of_patterns; ++i) {
        for(j = 0; j < nodes[number_of_layers-1]; ++j) {
            printf("%f ", target[i][j]);
        }
    }
    printf("\n");

    for(i = 0; i < number_of_patterns; ++i) {
        free(input[i]);
        free(target[i]);
    }

    vbp_free_net(net);
    free(input);
    free(target);
    free(output);

    return 0;
}
