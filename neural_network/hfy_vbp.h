/*
 *
 *
 * TYPEDEFS
 *
 *
 */

typedef struct _vbp_neural_layer {
    double *a;        // node activations
    double *w;        // (nodes x (nodes_in+1)) weight matrix
    double *dw;       // (nodes x (nodes_in+1)) delta weight matrix
    double *dwlast;   // (nodes x (nodes_in+1)) last delta weight matrix

    int nodes;        // number of non-bias nodes in this layer
    int nodes_in;     // number of non-bias nodes in the previous layer

    // NOTE: every layer has a bias-node, namely node number 0; we always have
    // a[0]=1.0 and w[0] is always the bias.

    double *delta;    // error deltas
    double *netsum;   // network activation

} vbp_neural_layer;

typedef struct _vbp_neural_network {
    int number_of_layers;
    vbp_neural_layer *layers;
} vbp_neural_network;

/*
 *
 *
 * MEMORY MANAGEMENT
 *
 *
 */

/*
 * Allocates memory to the contents of a single, given, layer. Useful when
 * allocating an array of neural layer structs.
 */
void vbp_allocate_layer_contents( int nodes, int nodes_in, 
        vbp_neural_layer *layer);

/*
 * Allocates memory for the contents of the neural network `net` with
 * `number_of_layers` layers, such that layer `i` has `nodes[i]` nodes.  
 * Useful when allocating an array of neural network structs.
 */
void vbp_allocate_net_contents(int number_of_layers, 
        int *nodes, vbp_neural_network *net);

/* 
 * Allocates memory for a new network and returns a pointer to the memory 
 */
vbp_neural_network *vbp_allocate_net(int number_of_layers, int *nodes);

/* 
 * Free the memory held by the layer `layer` and its contents, assuming `layer`
 * is not the first layer in every network containing it.
 */
void vbp_free_layer_contents(vbp_neural_layer *layer);

/*
 * Free the memory held by the network `net` and its contents, assuming the
 * network to be well formed.
 */
void vbp_free_net(vbp_neural_network *net);

/*
 *
 *
 * NEURAL NETWORK FUNCTIONS
 *
 *
 */

/*
 * Perform a single forward sweep on the network `net` with input `x`, and, if
 * y!=NULL, copy the output to y.
 */
void vbp_forward_sweep( double *x, double *y, vbp_neural_network *net); 

/*
 * Back-propagate the weights for the network `net`, which _must_ by fully
 * allocated on entry.
 *
 * If use_old_weights != 0 then we continue using the weights already set in
 * the network (this, of course, requires them to be well defined). Otherwise,
 * train using new weights.
 *
 */
void vbp(double **input,
        double **target,
        int number_of_patterns,
        int number_of_layers,
        int number_of_epochs,
        double eta,
        double mu,
        vbp_neural_network *net,
        double *net_errors,
        int use_old_weights); 

double logistic_sigmoid(double x);
double dlogistic_sigmoid(double x);
double cross_entropy(int n, double *target, double *output);
double norm(int n, double *target, double *output);


/*
 *
 *
 * UTILITY FUNCTIONS
 *
 *
 */

/* 
 * Generate a random permutation using Fisher-Yates shuffle
 */
void vbp_perm(int n, int* perm);

