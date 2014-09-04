#include <pam.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <getopt.h>

#include "hfy_vbp.h"

/*
 * NOTE: There are a couple of types, such as 'sample' and 'tuple' which are
 * imported from 'pam.h'  
 */

/*
 * Classify the first n paths into classes, such that path p is in class c if c
 * is a substring of the filename in p 
 */
void classify_paths(const int N, int number_of_classes, 
        char **classes, char **paths, double **target) 
{
    int i, j;
    char *start;
    for(i = 0; i < N; ++i) {
        // init path
        start = strrchr(paths[i] , '/');
        if(start == NULL) 
            start = paths[i];

        for(j = 0; j < number_of_classes; ++j) {
            if(strstr(start, classes[j]) != NULL)
                target[i][j] = 1.0; 
            else
                target[i][j] = 0.0;
        }
    }
}

/*
 * Reads n image files with paths in 'paths'; the images should only be
 * pgm-files and they should all have the same dimensions. The arrays maxvals,
 * imdata and imnormal should already point to storage reserved for the output
 * data.
 */
void loadfaces(char **paths, int n, int *maxvals, 
        unsigned long **imdata, double **imnormal) 
{
    // Local variables
    FILE *buf;

    int i, row, column, pos;
    unsigned long sample_val;
    
    struct pam imstruct; 
    tuple **imtuple;

    // read the files
    for(i = 0; i < n; ++i) {
        // open image at path[i]
        buf = fopen(paths[i], "r");
        // allocate space for the next image and retrieve it from buf
        imtuple = pnm_readpam(buf, &imstruct, PAM_STRUCT_SIZE(tuple_type));
        // close the image
        fclose(buf);

        // from the image tuple to the output
        maxvals[i] = imstruct.maxval; // value of white
        pos = 0; // position within imdata[i] and imnormal[i]
        for(row = 0; row < imstruct.height; ++row) {
            for(column = 0; column < imstruct.width; ++column) {
                sample_val = imtuple[row][column][0];
                imdata[i][pos] = sample_val;
                imnormal[i][pos] = ((double)sample_val)/maxvals[i];
                ++pos;
            }
        }

        // free the allocated space
        pnm_freepamarray(imtuple, &imstruct);
    }
}

/*
 * -n, --nodes n1,...,nN
 *  Comma separated list of nodes per hidden layer, NO SPACES ALLOWED.
 *  The number of layers is deduced from the number of arguments given to this
 *  option.
 *
 * -E, --epochs E
 *  Number of epochs.
 *
 * -e, --eta eta
 *  Learning rate, a floating point value between 0.0 and 2.0
 *
 * -m, --mu mu
 *  Learning momentum, a floating point value between 0.0 and 1.0
 *
 * -f, --files f1,...,fN 
 *  Comma separated list of files, NO SPACES ALLOWED. Each filename in path f
 *  must contain the classes c that the it is part of (and no other classes).
 *
 * -c, --classes c1,...,cN 
 *  Comma separated list of classes, NO SPACES ALLOWED.
 *
 * -x, --training-error f
 * -y, --testing-error g
 *  Write training/testing errors to files f/g
 *
 * -w, --weight-img f
 *  Write transfer weights from input to the next layer as an image file f.
 *  This works because each transfer can be visualized using images by
 *  reshaping the column-vectors of the first weight-matrix and scaling the
 *  values of each vector between 0 and 255.
 *
 */
int main(int argc, char *argv[]) {
    // insignificant variables 
    int i, j, k, n;
    int *perm; // permutation
    void *tmp;

    // gnu getline stuff
    char short_options [] = "n::E:e:m:f:c:x:y:w:";
    struct option long_options [] = {
        { "nodes" ,         optional_argument, 0, 'n' },
        { "epochs",         required_argument, 0, 'E' },
        { "eta" ,           required_argument, 0, 'e' },
        { "mu",             required_argument, 0, 'm' },
        { "files",          required_argument, 0, 'f' },
        { "classes",        required_argument, 0, 'c' },
        { "training-error", required_argument, 0, 'x' },
        { "testing-error",  required_argument, 0, 'y' },
        { "weight-img",     required_argument, 0, 'w' },
        {0, 0, 0, 0}
    };
    int c, option_index;  

    FILE *buf;
    struct pam imstruct; 
    tuple **imtuple;


    // user configurable through command line arguments (argv)
    int *nodes;
    int number_of_layers;
    int number_of_epochs;
    double eta; // learning rate
    double mu; // momentum
    char **paths; // input files
    char **classes; // actual class strings
    char *trainerror_path;
    char *testerror_path;
    char *weight_img_path;

    // other variables
    int imcount; // total number of images
    int imsize; // total pixel count of each image (constant)
    int number_of_classes; // total number of classes

    int *maxvals; // values of white for the images
    unsigned long **imdata;
    double **imnormal;
    double **target; // target classifications

    vbp_neural_network *net;

    // training and testing splits
    int training_size;
    double **training_input;
    double **training_target;
    double *training_error;

    int testing_size;
    double **testing_input;
    double **testing_target;
    double *testing_output;
    double *testing_error;

    char tmpstr[1000];

    // init netpbm
    pm_init(argv[0], 0);

    // read command line arguments (black magic)
    while(1) {
        c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if(c == -1)
            break;

        switch(c) {

            case 0:
                break;

            case 'f':
                // count the paths
                i = 0; // position within optarg
                imcount = 1; // number of files is 1 + number of commas
                while(optarg[i] != ' ' && optarg[i] != '\0') {
                    if(optarg[i] == ',')
                        imcount++;
                    i++;
                }
                // imcount = number of files
                paths = (char**)malloc(imcount*sizeof(char*));

                // read the paths
                i = 0; // position within optarg
                j = 0; // length of path[k-1]
                k = 0; // number of paths read so far
                while(k != imcount) {
                    j = strcspn(optarg+i, ", ");
                    paths[k] = (char*)malloc((j+1)*sizeof(char));
                    strncpy(paths[k], optarg+i, j);
                    paths[k][j] = '\0';
                    i += j+1;
                    k++;
                }
                break;

            case 'c':
                // count the classes
                i = 0; // position within optarg
                number_of_classes = 1; // number of classes is 1 + number of commas
                while(optarg[i] != ' ' && optarg[i] != '\0') {
                    if(optarg[i] == ',')
                        number_of_classes++;
                    i++;
                }
                classes = (char**)malloc(number_of_classes*sizeof(char*));

                // read the paths
                i = 0; // position within optarg
                j = 0; // length of class[k-1]
                k = 0; // number of classes read so far
                while(k != number_of_classes) {
                    j = strcspn(optarg+i, ", ");
                    classes[k] = (char*)malloc((j+1)*sizeof(char));
                    strncpy(classes[k], optarg+i, j);
                    classes[k][j] = '\0';
                    i += j+1;
                    k++;
                }
                break;

            case 'n':
                number_of_layers = 2;

                if(optarg != NULL) {
                    // count the hidden layers
                    i = 0; // position within optarg
                    number_of_layers = 3; // number of layers is 3 + number of commas
                    // since we always have input and output
                    while(optarg[i] != ' ' && optarg[i] != '\0') {
                        if(optarg[i] == ',')
                            number_of_layers++;
                        i++;
                    }
                }

                nodes = (int*)calloc(number_of_layers,sizeof(int));

                if(optarg != NULL) {
                    // read the number nodes per hidden layer
                    i = 0; // position within optarg
                    j = 0; // number of digits in the number
                    k = 1; // number of layers so far
                    while(k != number_of_layers-1) {
                        j = strcspn(optarg+i, ", ");
                        strncpy(tmpstr, optarg+i, j);
                        tmpstr[j] = '\0';
                        nodes[k] = atoi(tmpstr);
                        i += j+1;
                        k++;
                    }
                }
                break;

            case 'E':
                // read the number of epochs
                j = strcspn(optarg, " "); // length of the representation
                strncpy(tmpstr, optarg, j);
                tmpstr[j] = '\0';
                number_of_epochs = atoi(tmpstr);
                break;

            case 'e':
                // read eta
                j = strcspn(optarg, " "); // length of the representation
                strncpy(tmpstr, optarg, j);
                tmpstr[j] = '\0';
                eta = atof(tmpstr);
                break;

            case 'm':
                // read mu
                j = strcspn(optarg, " "); // length of the representation
                strncpy(tmpstr, optarg, j);
                tmpstr[j] = '\0';
                mu = atof(tmpstr);
                break;

            case 'x':
                // training error output path
                j = strcspn(optarg, " "); // length of the representation
                trainerror_path = (char*)malloc((j+1)*sizeof(char));
                strncpy(trainerror_path, optarg, j);
                trainerror_path[j] = '\0';
                break;

            case 'y':
                // testing error output path
                j = strcspn(optarg, " "); // length of the representation
                testerror_path = (char*)malloc((j+1)*sizeof(char));
                strncpy(testerror_path, optarg, j);
                testerror_path[j] = '\0';
                break;

            case 'w':
                // weight images output path
                j = strcspn(optarg, " "); // length of the representation
                weight_img_path = (char*)malloc((j+1)*sizeof(char));
                strncpy(weight_img_path, optarg, j);
                weight_img_path[j] = '\0';
                break;

            default:
                break;
        }
    }
    // generate a permutation
    perm = (int*)calloc(imcount,sizeof(int));
    vbp_perm(imcount, perm);
    // shuffle the paths; later on, this will allow us to split the data into
    // 70% training values and 30% testing values by simply using the first
    // 70% of each array for training and the rest as testing.
    for(i = 0; i < imcount; ++i) {
        tmp = (void*)paths[i];
        paths[i] = paths[perm[i]];
        paths[perm[i]] = (char*)tmp;
    }

    // extract the image size from a sample image
    buf = fopen(paths[0], "r");
    imtuple = pnm_readpam(buf, &imstruct, PAM_STRUCT_SIZE(tuple_type));
    fclose(buf);
    imsize = imstruct.width * imstruct.height;
    pnm_freepamarray(imtuple, &imstruct);

    // input and output nodes
    nodes[0] = imsize;
    nodes[number_of_layers-1] = number_of_classes;

    // allocate memory
    maxvals = (int*)calloc(imcount, sizeof(int));
    imdata = (unsigned long**)calloc(imcount, sizeof(unsigned long*));
    imnormal = (double**)calloc(imcount, sizeof(double*));

    target = (double**)calloc(imcount, sizeof(double*));

    // training split
    training_size = (int) round(0.7*imcount);
    testing_size = imcount - training_size;

    training_input = (double**)calloc(training_size, sizeof(double*));
    training_target = (double**)calloc(training_size, sizeof(double*));
    training_error = (double*)calloc(number_of_epochs, sizeof(double));

    testing_input = (double**)calloc(testing_size, sizeof(double*));
    testing_target = (double**)calloc(testing_size, sizeof(double*));
    testing_output = (double*)calloc(number_of_classes, sizeof(double));
    testing_error = (double*)calloc(number_of_epochs, sizeof(double));

    net = vbp_allocate_net(number_of_layers, nodes);

    for(i = 0; i < imcount; ++i) {
        imdata[i] = (unsigned long*)calloc(imsize, sizeof(unsigned long));
        imnormal[i] = (double*)calloc(imsize, sizeof(double));
        target[i] = (double*)calloc(number_of_classes, sizeof(double));

        // training and testing arrays share memory with the original data
        if(i < training_size) {
            training_input[i] = imnormal[i];
            training_target[i] = target[i];
        }
        else {
            testing_input[i-training_size] = imnormal[i];
            testing_target[i-training_size] = target[i];
        }
    }

    // load and classify faces
    loadfaces(paths, imcount, maxvals, imdata, imnormal);
    classify_paths(imcount, number_of_classes, classes, paths, target);

    // train neural network
    printf("Input data size is %d\n", imsize);
    printf("Training with %d samples over %d epochs\n", training_size, number_of_epochs);
    printf("Testing against %d samples\n", testing_size);
    printf("Output classes are %d\n", number_of_classes);

    // initial run to set the weights
    vbp(training_input, training_target, 
                training_size, number_of_layers, 0,
                eta, mu, net, training_error, 0);
    // continue training
    for(i = 0; i < number_of_epochs; ++i) {
        // train network for epoch number i
        // the training error is written in training_error[i]
        vbp(training_input, training_target, 
                training_size, number_of_layers, 1,
                eta, mu, net, training_error+i, 1);

        // test the network against all testing data
        testing_error[i] = 0.0;

        for(j = 0; j < testing_size; ++j) {
            vbp_forward_sweep(testing_input[j], testing_output, net);
            testing_error[i] += 
                cross_entropy(number_of_classes, testing_target[j], testing_output);

            // check activations in the first non-input layer
            if(i == number_of_epochs-1) {
                // print node activations
                printf("Node activations:   ");
                for(k = 0; k < nodes[1]; ++k) {
                    printf("%f ", (net->layers[1]).a[k+1]);
                }
                printf("\n");
                printf("output activations: ");
                for(k = 0; k < number_of_classes; ++k) {
                    printf("%f ", testing_output[k]);
                }
                printf("\n");
                // print target vector for this input
                printf("Target activations: ");
                for(k = 0; k < number_of_classes; ++k) {
                    printf("%f ", testing_target[j][k]);
                }
                printf("\n\n");
            }
        }
        testing_error[i] /= testing_size;
    }

    // output errors
    buf = fopen(trainerror_path, "w"); 
    for(i = 0;i < number_of_epochs; ++i) {
        fprintf(buf, "%f ", training_error[i]);
    }
    fprintf(buf, "\n");
    fclose(buf);
    buf = fopen(testerror_path, "w"); 
    for(i = 0;i < number_of_epochs; ++i) {
        fprintf(buf, "%f ", testing_error[i]);
    }
    fprintf(buf, "\n");
    fclose(buf);

    // draw images of weights into the second layer
    //
    // we rely on 'imstruct' previously initialized when gathering information
    // from  the sample image
    //
    // all images are written into the same file on the filesystem; since they
    // can easily be split up later (using pamsplit from netpbm)
    buf = fopen(weight_img_path, "w");
    imstruct.plainformat = 1;
    imstruct.maxval = 255;
    imtuple = pnm_allocpamarray(&imstruct);
    imstruct.file=buf;
    for(k = 0; k < nodes[1]; ++k) {
        double *w = net->layers[1].w;
        double minw, maxw;
        // find the minimum and maximums of w for scaling
        minw = maxw = w[k + nodes[1]];
        for(i = 1; i <= nodes[0]; ++i) {
            if(minw > w[k + i*nodes[1]]) minw = w[k + i*nodes[1]];
            if(maxw < w[k + i*nodes[1]]) maxw = w[k + i*nodes[1]];
        }
        printf("minw=%f, maxw=%f\n", minw,maxw);
        // scale weights to take values in 0..255 and write the output to an
        // image 
        n = 1;
        for(i = 0; i < imstruct.height; ++i) {
            for(j = 0; j < imstruct.width; ++j) {
                imtuple[i][j][0] = (sample) 
                    round( 255.0 * ( (w[k + n*nodes[1]]-minw)/(maxw-minw) ) );
                ++n;
            }
        }
        // write the image
        pnm_writepam(&imstruct, imtuple);
    }
    // clean up
    pnm_freepamarray(imtuple, &imstruct);
    fclose(buf);

    // free memory
    for(i = 0; i < imcount; ++i) {
        free(paths[i]);
        free(imdata[i]);   
        free(imnormal[i]);
        free(target[i]);
    }
    free(paths);
    free(perm);

    free(testerror_path);
    free(trainerror_path);
    free(weight_img_path);

    for(i = 0; i < number_of_classes; ++i) {
        free(classes[i]);
    }
    free(classes);

    free(imdata);
    free(imnormal);
    free(target);
    free(maxvals);
    free(nodes);

    free(training_input);
    free(training_target);
    free(training_error);

    free(testing_input);
    free(testing_target);
    free(testing_output);
    free(testing_error);

    vbp_free_net(net);

    return 0;
}
