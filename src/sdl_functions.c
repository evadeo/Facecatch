#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <err.h>
#include "pixel_operations.h"
#include "sdl_functions.h"
#include "adab.h"
#include "s4.h"
#include "integral_image.h"

void wait_for_keypressed(void) {
  SDL_Event             event;
  // Infinite loop, waiting for event
  for (;;) {
    // Take an event
    SDL_PollEvent( &event );
    // Switch on event type
    switch (event.type) {
    // Someone pressed a key -> leave the function
    case SDL_KEYDOWN: return;
    default: break;
    }
  // Loop until we got the expected event
  }
}

void init_sdl(void) {
  // Init only the video part
  if( SDL_Init(SDL_INIT_VIDEO)==-1 ) {
    // If it fails, die with an error message
    errx(1,"Could not initialize SDL: %s.\n", SDL_GetError());
  }

  // We don't really need a function for that ...
}

SDL_Surface* load_image(char *path) {
  SDL_Surface          *img;
  // Load an image using SDL_image with format detection
  img = IMG_Load(path);
  if (!img)
    // If it fails, die with an error message
    errx(3, "can't load %s: %s", path, IMG_GetError());
  return img;
}

SDL_Surface* display_image(SDL_Surface *img) {
  SDL_Surface          *screen;
  // Set the window to the same size as the image
  screen = SDL_SetVideoMode(img->w, img->h, 0, SDL_SWSURFACE|SDL_ANYFORMAT);
  if ( screen == NULL ) {
    // error management
    errx(1, "Couldn't set %dx%d video mode: %s\n",
         img->w, img->h, SDL_GetError());
  }

   /* Blit onto the screen surface */
  if(SDL_BlitSurface(img, NULL, screen, NULL) < 0)
    warnx("BlitSurface error: %s\n", SDL_GetError());

   // Update the screen
  SDL_UpdateRect(screen, 0, 0, img->w, img->h);

   // wait for a key
  wait_for_keypressed();

   // return the screen for further uses
  return screen;
}

void grey(SDL_Surface *img,Uint32 *tab){
  Uint8 r,g,b;
  Uint32 pixel;
  int i,j;
  Uint32 sum;
  for (i = 0; i < img->w ; i++) {
    for (j=0; j < img->h; j++) {
      pixel = getpixel(img,i,j);
      SDL_GetRGB(pixel,img->format,&r,&g,&b);
      sum= 0.3*r+0.59*g+0.11*b;
      *(tab + i + img->w * j) = sum;
      // i = x and j = y
      // pixel= SDL_MapRGB(img->format,r,g,b);
      //putpixel(img,i,j,pixel);
    }
  }
}


SDL_Surface* contrast_level (SDL_Surface *img)
{
  Uint8 r,g,b;
  Uint32 pixel1;
  Uint32 pixel2;
  for(int i=0;i<img->w ;i++)
  {
     for(int j=0;j<img->h;j++)
     {
       pixel1 = getpixel(img,i,j);
       SDL_GetRGB(pixel1,img->format,&r,&g,&b);
       pixel2 = SDL_MapRGB(img->format,255-r,255-r,255-r);
       putpixel(img,i,j,pixel2);
     }
   }
  return img;
}



//Position (x,y) refers to the top left corner
void draw_square(SDL_Surface *img, int x, int y, int width)
{
    Uint32 green = SDL_MapRGB(img->format, 0, 255, 0);
    for(int i = 0; i < width; i++)
    {
        putpixel(img, x, y+i, green);
        putpixel(img, x + width, y+i, green);
        if(i == 0 || i == width-1)
        {
            for(int j = 1; j < width; j++)
                putpixel(img, x + j, y + i, green);
        }
    }
}

void draw_maybe(SDL_Surface *img, int x, int y, int width)
{
    Uint32 green = SDL_MapRGB(img->format, 255, 0, 255);
    for(int i = 0; i < width; i++)
    {
        putpixel(img, x, y+i, green);
        putpixel(img, x + width, y+i, green);
        if(i == 0 || i == width-1)
        {
            for(int j = 1; j < width; j++)
                putpixel(img, x + j, y + i, green);
        }
    }
}

void draw_red(SDL_Surface *img, int x, int y, int width)
{
    Uint32 green = SDL_MapRGB(img->format, 255, 0, 0);
    for(int i = 0; i < width; i++)
    {
        putpixel(img, x, y+i, green);
        putpixel(img, x + width, y+i, green);
        if(i == 0 || i == width-1)
        {
            for(int j = 1; j < width; j++)
                putpixel(img, x + j, y + i, green);
        }
    }
}


int min(int a, int b)
{
  if(a > b)
    return a;
  return b;
}

float my_abs(float x)
{
   if(x< 0)
     x *= -1;
   return x;
 }

void detect_faces(SDL_Surface *img, strong_classif* sc, int x, int y)
{
  Uint32* pixels = malloc(img->w*img->h*sizeof(Uint32));
  grey(img, pixels);

  float res = 0;

  //SDL_Surface* img_sel = SDL_CreateRGBSurface(SDL_SWSURFACE, 
    //                                          24,24,32,0,0,0,0);
  int e = 24;

  //Extracting Selection
  Uint32* selection = malloc(e*e*sizeof(Uint32));
  for(int i = 0; i < 24; i++)
  {
    for(int j = 0; j < 24; j++)
    {
      *(selection + i + 24*j) = *(pixels + i + x + 24*(j+y)); 
    }
  }

  feature* database; // = malloc(162336*sizeof(feature));
  database = haar_features(selection, 24, 0, 0);
  
  for(int f = 0; f < sc->length; f++)
  {
    if(database[sc->w[f].d->index].res > sc->w[f].d->threshold)
      res += sc->w[f].coef;
    else
      res += -sc->w[f].coef;
  }
  
  if(res >  0)
    draw_square(img, x, y, 24);
  else
    draw_red(img, x, y, 24);


    
  //SDL_FreeSurface(img_sel);
  free(database);
  free(selection);
  free(pixels);
}


void analyse_image(SDL_Surface* img)
{
  //Loading Strong Classifier
  strong_classif* sc = malloc(sizeof(strong_classif));
  sc->w = malloc(250*sizeof(weak_classif));
  for(int m = 0; m < 250; m++)
    sc->w[m].d = malloc(sizeof(decision));
  load_classif(sc);


  for(int y = 0; y < img->h - 24; y+=5)
  {
    for(int x = 0; x < img->w - 24; x+=5)
    {
      detect_faces(img, sc, x, y);
      //display_image(img);
      //printf("Processing(%i,%i)\n",x,y);
    }
  }
  display_image(img);
  free(sc);
}
