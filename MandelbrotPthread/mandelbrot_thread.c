//
// Created by HADDAD on 02/02/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#define OUTFILE "mandelbrot_thread.out"
#define NB_THREAD 8


double XMIN=-2;
double YMIN=-2;
double XMAX=2;
double YMAX=2;
double RESOLUTION=0.003;
int NITERMAX=10000;

struct Input{
    int startY;
    int maxX;
    int maxPixelY;
    int * tab;
    int nbPixelY
};


void * CalculPoint(void * _arg){

    struct Input * in = (struct Input *) _arg;

    for(int i = 0;i<(*in).maxX;i++)
        for (int ypixel = (*in).startY; ypixel < (*in).maxPixelY; ypixel++) {
            double xinit = XMIN + i * RESOLUTION;
            double yinit = YMIN + ypixel * RESOLUTION;
            double x = xinit;
            double y = yinit;
            int iter = 0;
            for (iter = 0; iter < NITERMAX; iter++) {
                double prevy = y, prevx = x;
                if ((x * x + y * y) > 4)
                    break;
                x = prevx * prevx - prevy * prevy + xinit;
                y = 2 * prevx * prevy + yinit;
            }
            (*in).tab[i * (*in).nbPixelY + ypixel] = iter;
        }

        pthread_exit(NULL);
}

int main(int argc,char **argv)
{
    clock_t start, stop;
    start = clock();
    int * itertab;
    int nbpixelx;
    int nbpixely;
    FILE * file;

/*calcul du nombre de pixel*/
    nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
    nbpixely = ceil((YMAX - YMIN) / RESOLUTION);
    pthread_t pthread[NB_THREAD];
    struct Input in[NB_THREAD];

/*allocation du tableau de pixel*/
    if( (itertab=malloc(sizeof(int)*nbpixelx*nbpixely)) == NULL)
    {
        printf("ERREUR d'allocation de itertab, errno : %d (%s) .\n",errno,strerror(errno));
        return EXIT_FAILURE;
    }

/*calcul des points*/
   int j = 0;
    for(int i = 0; i<NB_THREAD; i++){
        in[i].startY = j;
        in[i].maxX = nbpixelx;
        in[i].maxPixelY = j+(nbpixely/NB_THREAD);
        in[i].tab = itertab;
        in[i].nbPixelY = nbpixely;
        pthread_create(&pthread[i],NULL,CalculPoint,&in[i]);
        j = j+(nbpixely/NB_THREAD);
    }

    for(int i = 0; i<NB_THREAD; i++){
        pthread_join(&pthread[i],NULL);
    }

    stop = clock();
    printf("Temps d'execution : %ld ms",(stop-start)*100/CLOCKS_PER_SEC);

/*output des resultats compatible gnuplot*/
    if( (file=fopen(OUTFILE,"w")) == NULL ) {
        printf("Erreur à l'ouverture du fichier de sortie : errno %d (%s) .\n",errno,strerror(errno));
        return EXIT_FAILURE;
    }
    for(int xpixel=0;xpixel<nbpixelx;xpixel++) {
        for(int ypixel=0;ypixel<nbpixely;ypixel++) {
            double x = XMIN + xpixel * RESOLUTION;
            double y = YMIN + ypixel * RESOLUTION;
            fprintf(file,"%f %f %d\n", x, y,itertab[xpixel*nbpixely+ypixel]);
        }
        fprintf(file,"\n");
    }
    fclose(file);

/*clean*/
    free(itertab);

/*sortie du programme*/
    return EXIT_SUCCESS;
}