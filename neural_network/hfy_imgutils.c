#include <stdio.h>
#include <stdlib.h>

#include <pam.h>
#include "hfy_imgutils.h"

hfy_img *hfy_pgmread(char *path) {
    // input image file
    struct pam imgstruct;
    FILE *buf = fopen(path, "r");
    tuple **imgtuple = pnm_readpam(buf, &imgstruct, PAM_STRUCT_SIZE(tuple_type));
    fclose(buf);

    // output image data
    hfy_img *img = (hfy_img *)malloc(sizeof(hfy_img));
    img->height = imgstruct.height;
    img->width = imgstruct.width;
    img->data = (double *)calloc(img->height*img->width, sizeof(double));

    // export normalized image data
    int pos = 0;
    int i, j;
    for(i = 0; i < img->height; ++i) {
        for(j = 0; j < img->width; ++j) {
            img->data[pos++] = ((double)imgtuple[i][j][0]) / imgstruct.maxval;
        }
    }
    // clean and return
    pnm_freepamarray(imgtuple, &imgstruct);
    return img;
}

void hfy_pgmfree(hfy_img *img) {
    free(img->data);
    free(img);
}

void hfy_pgmwrite(hfy_img *img, char *path){
    // loop varibales
    int i, j;
    
    // pam info containers
    FILE *buf = fopen(path, "w");
    struct pam imgstruct; 
    imgstruct.size = PAM_STRUCT_SIZE(tuple_type);
    imgstruct.len = imstruct.size;
    imgstruct.file = buf;
    imgstruct.format = PGM_FORMAT;
    imgstruct.plainformat = 1;
    imgstruct.height = img->height;
    imgstruct.width = img->width;
    imgstruct.depth = 1;
    imgstruct.maxval = 255;
    imgstruct.tuple_type = PAM_PGM_TUPLETYPE;
    imgstruct.comment_p = NULL;

    // image data containers
    tuple **imgtuple = pnm_allocpamarray(&imgstruct);
    double *data = img->data;

    // export scaled data into imgtuple
    int pos = 0;
    for(i = 0; i < img->height; ++i) {
        for(j = 0; j < img->width; ++j) {
            imgtuple[i][j][0] = (sample)
                round( 255.0 * data[pos] );
            ++pos;
        }
    }
    // write the image file
    pnm_writepam(&imgstruct, imgtuple);

    // clean up
    pnm_freepamarray(imgtuple, &imgstruct);
    fclose(path);
}
