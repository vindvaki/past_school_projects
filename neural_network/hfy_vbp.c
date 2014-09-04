// #include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <cblas.h>

#include "hfy_vbp.h"

void vbp_allocate_layer_contents( int nodes, int nodes_in, 
        vbp_neural_layer *layer) 
{
    int asize = (1+nodes);
    int wsize = nodes*(nodes_in+1);
    int dsize = nodes;

    layer->nodes = nodes;
    layer->nodes_in = nodes_in;

    // use calloc so that all matrices and vectors start at 0.0
    layer->a = (double*)calloc(asize, sizeof(double));
    layer->w = (double*)calloc(wsize, sizeof(double));
    layer->dw = (double*)calloc(wsize, sizeof(double));
    layer->dwlast = (double*)calloc(wsize, sizeof(double));

    layer->delta = (double*)calloc(dsize, sizeof(double));
    layer->netsum = (double*)calloc(dsize, sizeof(double));

    (layer->a)[0] = 1.0;
}

void vbp_allocate_net_contents(int number_of_layers, int *nodes, 
        vbp_neural_network *net) 
{
    // local variables
    int i;

    net->number_of_layers = number_of_layers;
    // allocate layers
    vbp_neural_layer *layer = (vbp_neural_layer*)
        calloc(number_of_layers, sizeof(vbp_neural_layer));
    net->layers = layer;

    // allocate the contents of the first layer
    // (the first layer is always different)
    layer->a = (double*)calloc((1+nodes[0]), sizeof(double));
    (layer->a)[0] = 1.0;
    layer->nodes = nodes[0];
    layer->nodes_in = 0;
    layer->w = layer->dw = layer->dwlast = NULL;

    // allocate the contents of the rest of the layers
    for(i = 1; i < number_of_layers; ++i) { 
        vbp_allocate_layer_contents(nodes[i], nodes[i-1], layer+i);
    }
}

vbp_neural_network *vbp_allocate_net(int number_of_layers, int *nodes) {
    vbp_neural_network *net = 
        (vbp_neural_network*)calloc(1,sizeof(vbp_neural_network));
    vbp_allocate_net_contents(number_of_layers, nodes, net);
    return net;
}

void vbp_free_layer_contents(vbp_neural_layer *layer) {
    free(layer->a);
    free(layer->delta);
    free(layer->w);
    free(layer->dw);
    free(layer->dwlast);
    free(layer->netsum);
}

void vbp_free_net(vbp_neural_network *net) {
    vbp_neural_layer *layer = net->layers;
    const vbp_neural_layer *END = layer + (net->number_of_layers);
    // Free the first layer contents
    free(layer->a);
    // Free the contents of the other layers
    for(++layer; layer < END; ++layer) {
        vbp_free_layer_contents(layer);
    }
    // Memory allocated to the structs themselves
    free(net->layers); // all the layer structs were allocated together
    free(net);
}

double logistic_sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double dlogistic_sigmoid(double x) {
    double y = logistic_sigmoid(x);
    return y * (1.0 - y);
}

double cross_entropy(int n, double *target, double *output) {
    int i;
    double result;
    result = 0.0;
    for(i = 0; i < n; ++i) {
        result -= target[i]*log(output[i]) + (1-target[i])*log(1-output[i]);
    }
    return result;
}

double norm(int n, double *target, double *output) {
    int i;
    double sqsum, diff;
    sqsum = 0.0;
    for(i = 0; i < n; ++i) {
        diff = target[i]-output[i];
        sqsum += diff*diff;
    }
    return sqrt(sqsum);
}

void vbp_forward_sweep( double *x, double *y, vbp_neural_network *net) { 
    // local variables
    int i,j;
    vbp_neural_layer *layer = net->layers;
    vbp_neural_layer *END = (net->layers) + (net->number_of_layers);

    // (layer->a)[1..] = x
    cblas_dcopy(layer->nodes, x, 1, (layer->a)+1, 1);
    layer->a[0] = 1.0; // for the bias node

    // forward sweep
    for(++layer; layer < END; ++layer) {
        cblas_dgemv(CblasColMajor, CblasNoTrans,
                layer->nodes, (layer->nodes_in)+1,
                1.0, layer->w, layer->nodes, (layer-1)->a, 1,
                0.0, layer->netsum, 1);

        // evaluate layer activations
        for(i = 0; i < layer->nodes; ++i) {
            (layer->a+1)[i] = logistic_sigmoid((layer->netsum)[i]);
            // printf("%f ", (layer->a+1)[i]);
        }
        // printf("\n");
        layer->a[0] = 1.0; // for the bias node
    }
    --layer;
    if(y != NULL) {
        cblas_dcopy(layer->nodes, (layer->a)+1, 1, y, 1);
    }
}

void vbp(double **input, double **target,
        int number_of_patterns,
        int number_of_layers,
        int number_of_epochs,
        double eta, double mu,
        vbp_neural_network *net,
        double *net_errors,
        int use_old_weights) 
{
    // local variables
    int i,j, epoch, pattern, wsize;
    double *t;
    const double ZERO [] = { 0 };
    vbp_neural_layer *layer;
    vbp_neural_layer *END;

    // init the network
    END = (net->layers) + number_of_layers;
    if(use_old_weights == 0) {
        // initalize weights to random small numbers
        for(layer = net->layers+1; layer < END; ++layer) {
            wsize = (layer->nodes)*((layer->nodes_in)+1);
            for(i = 0; i < wsize; ++i) {
                (layer->w)[i] = 0.05*((2.0*rand())/RAND_MAX - 1.0);
                (layer->dwlast)[i] = 0.0;
            }
        }
    }

    // start training
    for(epoch = 0; epoch < number_of_epochs; ++epoch) {

        net_errors[epoch] = 0.0;

        // zero the change in weight
        for(layer = net->layers+1; layer < END; ++layer) {
            wsize = (layer->nodes)*((layer->nodes_in)+1);
            cblas_dcopy(wsize, ZERO, 0, layer->dw, 1);
        }

        // loop through each pattern
        for(pattern = 0; pattern < number_of_patterns; ++pattern) {
            t = target[pattern];

            // do a complete forward sweep
            vbp_forward_sweep(input[pattern], NULL, net);

            // top layer (output nodes)
            layer = END-1;

            // cross-entropy
            net_errors[epoch] += cross_entropy(layer->nodes, t, layer->a+1);

            // top layer deltas (cross-entropy)
            cblas_dcopy(layer->nodes, t, 1, layer->delta, 1);
            cblas_daxpy(layer->nodes, -1.0, (layer->a)+1, 1, layer->delta, 1);

            // back-propagate the lower layer deltas
            for(--layer; layer > net->layers; --layer) {
                for(i = 0; i < layer->nodes; ++i) {
                    (layer->delta)[i] = 
                        dlogistic_sigmoid((layer->netsum)[i])
                        * cblas_ddot((layer+1)->nodes,
                            (layer+1)->delta, 1,
                            (layer+1)->w + (i+1)*((layer+1)->nodes), 1);
                }
            }
            // cumulate the weight changes (batched learning)
            for(++layer; layer < END; ++layer) {
                cblas_dger(CblasColMajor,
                        layer->nodes, (layer->nodes_in)+1,
                        1.0, layer->delta, 1, (layer-1)->a, 1,
                        layer->dw, layer->nodes);
            }
        }
        // update the network weights
        for(layer = net->layers+1; layer < END; ++layer) {
            wsize = (layer->nodes)*((layer->nodes_in)+1);

            // update dw
            cblas_dscal(wsize, eta/number_of_patterns, layer->dw, 1);
            cblas_daxpy(wsize, mu, layer->dwlast, 1, layer->dw, 1);

            // update w
            cblas_daxpy(wsize, 1.0, layer->dw, 1, layer->w, 1);

            // update dwlast
            cblas_dcopy(wsize, layer->dw, 1, layer->dwlast, 1);
        }
        // mean error per epoch
        net_errors[epoch] /= number_of_patterns;
    }
}

void vbp_perm(int n, int *perm) {
    int i;
    srand(time(NULL));

    for(i = n-1; i > 0; --i) {
        perm[i] = rand()%(i+1);
    }
}

