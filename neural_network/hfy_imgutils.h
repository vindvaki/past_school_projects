/*
 * A simple wrapper for reading pgm files (probably using libnetpbm)
 *
 */

typedef struct _hfy_img {
    double *data; // (height x width) column major array with normalized pgm data
    int height;
    int width;
} hfy_img;

// returns normalized image data from the file at path 
hfy_img *hfy_pgmread(char *path);

// frees memory allocated to img
void hfy_pgmfree(hfy_img *img);

// write the image data in img into the file at path
void hfy_pgmwrite(hfy_img *img, char *path);
