/*
 OpenGL visualization skeleton for displaying bitmap images. Just provide a GenerateImage function.
 Good starting point for all image processing exercises for parallel programming.

 Example of generating bitmaps using GenerateImage and the prepared GLUT OpenGL visualization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/freeglut.h>
  #include <GL/freeglut_ext.h>
#endif

#define MAX(a,b) (a>b) ? a : b
#define MIN(a,b) (a<b) ? a : b
#define ABS(a) (a>=0) ? a : (-1*a)
#define IN_RANGE(row,col) ( (row) >=0 && (row) < TEX_SIZE && (col) >=0 && (col) <TEX_SIZE )


#define TEX_SIZE 512
GLuint texture;

int t = 0;
int dt = 1;

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;
} pixelAlpha;

pixel image[TEX_SIZE][TEX_SIZE];            //cielovy, tento vykreslujeme
pixel source[TEX_SIZE][TEX_SIZE];           //nacitavame

pixelAlpha a_layer[TEX_SIZE][TEX_SIZE];
pixelAlpha b_layer[TEX_SIZE][TEX_SIZE];

char animate=0;
int zoom = 1;

//OREZANIE
float clamp(float number){
    if(number>1.0){
        return 1.0;
    }
    if(number<0.0){
        return 0.0;
    }
    return number;
}

//Normalizuj z <0-255> do intervalu <0-1>
float colorToFloat(GLubyte color){
    return (float) color/255;
}

GLubyte floatToColor(float f){
    return f * 255;
}

//Load a,b layers from files
void floadLayers(){
    FILE *fimageA,*fimageB;
    fimageA = fopen("C:\\Users\\Jakub\\Dropbox\\5. sem\\PPGSO\\net.rgba","rb");
    fimageB = fopen("C:\\Users\\Jakub\\Dropbox\\5. sem\\PPGSO\\chl_small.rgba","rb");
    if(fimageA && fimageB){
      fread(a_layer,sizeof(a_layer),1,fimageA);
      fread(b_layer,sizeof(b_layer),1,fimageB);
      fclose(fimageA);
      fclose(fimageB);
    }else{
      exit(1);
    }

}

//Load image from file to source
void floadImageToSource(){
    int x,y;
    FILE *fimage;
    fimage = fopen("C:\\Users\\Jakub\\Dropbox\\5. sem\\PPGSO\\Lenna.rgb","rb");            // b - cita ako binarny subor, defaultne ako textovy

    if(fimage){     //Load from file
        fread(source,sizeof(image),1,fimage);
        //ALEBO DETAILNEJSI SPOSOB
//      int x,y;
//      for (x=0;x<TEX_SIZE;x++) {
//        for(y=0;y<TEX_SIZE;y++){
//          image[x][y].r = getc(fimage);
//          image[x][y].g = getc(fimage);
//          image[x][y].b = getc(fimage);
//        }
//      }
      fclose(fimage);
    }
}

//All loading from files
void loadFromFiles(){
    floadImageToSource();
    memcpy(image,source,sizeof(image));
    floadLayers();
}


void juliaSet(){
  int x,y;
  double cReal = -0.4, cImag = 0.651;
  double zReal, zRealOld, zImag, zImagOld;
  double moveX = 0, moveY = 0;
  int maxIterations = 300;

  for(x=0;x<TEX_SIZE;x++){
    for(y=0;y<TEX_SIZE;y++){
      //initial point in graph
      zReal = 2 * (x - TEX_SIZE / 2.0) / (zoom * TEX_SIZE / 2.0) + moveX;
      zImag = 2 * (y - TEX_SIZE / 2.0) / (zoom * TEX_SIZE / 2.0) + moveY;

        int n;
        for (n = 0; n < maxIterations; n++)
        {
            zRealOld = zReal;
            zImagOld = zImag;

            zReal = zRealOld * zRealOld - zImagOld * zImagOld + cReal;
            zImag = 2 * zRealOld * zImagOld + cImag;

            //if the point is outside the circle with radius 2, stop
            if ((zReal * zReal + zImag * zImag) > 4)
            {
                break;
            }
        }

        if (n == maxIterations)
        {
            image[x][y].r = 0;
            image[x][y].g = 0;
            image[x][y].b = 0;
        }else{  //set colors
            image[x][y].r = n % 255;
            image[x][y].g = n % 255;
            image[x][y].b = 0;
        }

    }
  }


}


//Put composite of source(RGB) + b layer(RGBA) + a layer(RGBA) to image
void compositeImageWithAlphaChannel(){
    int x,y;
    float a;
    pixelAlpha temp_layer[TEX_SIZE][TEX_SIZE];

    //C+B layer
    for(x=0;x<TEX_SIZE;x++){
      for(y=0;y<TEX_SIZE;y++){
          a = colorToFloat(b_layer[x][((y-t) + TEX_SIZE) % TEX_SIZE].a);
          temp_layer[x][y].r = b_layer[x][((y-t) + TEX_SIZE) % TEX_SIZE].r * a + (1-a)*source[x][y].r * 1;
          temp_layer[x][y].g = b_layer[x][((y-t) + TEX_SIZE) % TEX_SIZE].g * a + (1-a)*source[x][y].g * 1;
          temp_layer[x][y].b = b_layer[x][((y-t) + TEX_SIZE) % TEX_SIZE].b * a + (1-a)*source[x][y].b * 1;
          temp_layer[x][y].a = a + (1-a)*1;
      }
    }

    //+A layer
    for(x=0;x<TEX_SIZE;x++){
      for(y=0;y<TEX_SIZE;y++){
        a = colorToFloat(a_layer[x][y].a);
        image[x][y].r = a_layer[x][y].r *a + (1-a)*temp_layer[x][y].r * temp_layer[x][y].a;
        image[x][y].g = a_layer[x][y].g *a + (1-a)*temp_layer[x][y].g * temp_layer[x][y].a;
        image[x][y].b = a_layer[x][y].b *a + (1-a)*temp_layer[x][y].b * temp_layer[x][y].a;
      }
    }
}

//Apply a convolution filter to image
void convoltionFilter(int *kernel,int m,int n,int div,float bias){
    int x,y,i,j;
    int imageX,imageY;
    float r,g,b,sum_r,sum_g,sum_b;

    for(x=0;x<TEX_SIZE;x++){
        for(y=0;y<TEX_SIZE;y++){
            sum_r=0;sum_g=0;sum_b=0;

            for(i=0;i<m;i++){
                for(j=0;j<n;j++){
                    imageX = x - (m/2) + i;
                    imageY = y - (n/2) + j;
                    if( IN_RANGE(imageX,imageY) ){
                        r = colorToFloat(source[imageX][imageY].r);
                        g = colorToFloat(source[imageX][imageY].g);
                        b = colorToFloat(source[imageX][imageY].b);
                    }else{  //set as black color
                        r=0;
                        g=0;
                        b=0;
                    }

                    sum_r += r * (*((kernel+i*n)+j));
                    sum_g += g * (*((kernel+i*n)+j));
                    sum_b += b * (*((kernel+i*n)+j));
                }
            }

               image[x][y].r = floatToColor( clamp((sum_r/div)+bias) );
               image[x][y].g = floatToColor( clamp((sum_g/div)+bias) );
               image[x][y].b = floatToColor( clamp((sum_b/div)+bias) );
        }
    }
}

//First example of generating image
void GenerateImage() {
    int x,y;
    if(!image){  // Generate image with numbers from 0-255 => 1B
      for (x=0;x<TEX_SIZE;x++) {
        for (y=0;y<TEX_SIZE;y++) {
            image[x][y].r = x/2;
            image[x][y].g = y/2;
            image[x][y].b = 0;
        }
      }
    }
}


void distributeErrors(int x,int y){
  float error_r, error_g, error_b;

  error_r = colorToFloat(source[x][y].r) - colorToFloat(image[x][y].r);
  if(IN_RANGE(x+1,y)) { source[x+1][y].r = clamp(colorToFloat(source[x+1][y].r) + (7.0/16)*error_r)*255; }
  if(IN_RANGE(x-1,y+1)) {source[x-1][y+1].r = clamp(colorToFloat(source[x-1][y+1].r) + (3.0/16)*error_r)*255;}
  if(IN_RANGE(x,y+1)) {source[x][y+1].r = clamp(colorToFloat(source[x][y+1].r) + (5.0/16)*error_r)*255;}
  if(IN_RANGE(x+1,y+1)) {source[x+1][y+1].r = clamp(colorToFloat(source[x+1][y+1].r) + (1.0/16)*error_r)*255;}

  error_g = colorToFloat(source[x][y].g) - colorToFloat(image[x][y].g);
  if(IN_RANGE(x+1,y)) {source[x+1][y].g = clamp(colorToFloat(source[x+1][y].g) + (7.0/16)*error_g)*255;}
  if(IN_RANGE(x-1,y+1)) {source[x-1][y+1].g = clamp(colorToFloat(source[x-1][y+1].g) + (3.0/16)*error_g)*255;}
  if(IN_RANGE(x,y+1)) {source[x][y+1].g = clamp(colorToFloat(source[x][y+1].g) + (5.0/16)*error_g)*255;}
  if(IN_RANGE(x+1,y+1)) {source[x+1][y+1].g = clamp(colorToFloat(source[x+1][y+1].g) + (1.0/16)*error_g)*255;}

  error_b = colorToFloat(source[x][y].b) - colorToFloat(image[x][y].b);
  if(IN_RANGE(x+1,y)) {source[x+1][y].b = clamp(colorToFloat(source[x+1][y].b) + (7.0/16)*error_b)*255;}
  if(IN_RANGE(x-1,y+1)) {source[x-1][y+1].b = clamp(colorToFloat(source[x-1][y+1].b) + (3.0/16)*error_b)*255;}
  if(IN_RANGE(x,y+1)) {source[x][y+1].b = clamp(colorToFloat(source[x][y+1].b) + (5.0/16)*error_b)*255;}
  if(IN_RANGE(x+1,y+1)) {source[x+1][y+1].b = clamp(colorToFloat(source[x+1][y+1].b) + (1.0/16)*error_b)*255;}

}

//Dithering - Error diffusion dither - Floyd & Steinberg into 1bit
void errorDiffusionDither1(){
  int x,y;
  int truncation;

  for(x=0;x<TEX_SIZE;x++){
    for(y=0;y<TEX_SIZE;y++){
      truncation = ( (source[x][y].r + source[x][y].g + source[x][y].b) / 3 ) > 128 ? 1 : 0;
      image[x][y].r = floatToColor(truncation);
      image[x][y].g = floatToColor(truncation);
      image[x][y].b = floatToColor(truncation);
      distributeErrors(x,y);
    }
  }

floadImageToSource();        //broken source image, loading again
}

//Dithering - Error diffusion dither - Floyd & Steinberg into 8bit(R-3b,G-3b,B-2b)
void errorDiffusionDither8(){
  int x,y;
  float r,g,b;
  float error_r,error_g,error_b;

  for(x=0;x<TEX_SIZE;x++){
    for(y=0;y<TEX_SIZE;y++){
      r = floor(colorToFloat(source[x][y].r)*8) / 8.0f;
      g = floor(colorToFloat(source[x][y].g)*8) / 8.0f;
      b = floor(colorToFloat(source[x][y].b)*4) / 4.0f;

      image[x][y].r = floatToColor(r);
      image[x][y].g = floatToColor(g);
      image[x][y].b = floatToColor(b);

      distributeErrors(x,y);
    }
  }

floadImageToSource();        //broken source image, loading again
}


// Initialize OpenGL state
void init() {
	// Texture setup
    glEnable(GL_TEXTURE_2D);
    glGenTextures( 1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Other
    glClearColor(0,0,0,0);
    glMatrixMode(GL_PROJECTION);      // Jakub>Select the Projection matrix for operation
    glLoadIdentity();                 // Jakub>Reset Projection matrix
    gluOrtho2D(-1,1,-1,1);
    glLoadIdentity();
    glColor3f(1,1,1);

    //File readers
    loadFromFiles();

    if (!image) {
        GenerateImage();
    }
}

// Generate and display the image.
void display() {
    // Call user image generation
    //GenerateImage();
    if (animate != 0) {
        compositeImageWithAlphaChannel();             //animacia
    }
    // Copy image to texture memory
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // Clear screen buffer
    glClear(GL_COLOR_BUFFER_BIT);
    // Render a quad
    glBegin(GL_QUADS);
        glTexCoord2f(1,0); glVertex2f(1,1);
        glTexCoord2f(1,1); glVertex2f(1,-1);
        glTexCoord2f(0,1); glVertex2f(-1,-1);
        glTexCoord2f(0,0); glVertex2f(-1,1);
    glEnd();
    // Display result
    glFlush();
    glutPostRedisplay();
    glutSwapBuffers();
}

void keyboardEvents(unsigned char key, int x, int y){
    switch(key){
      case 'e' : case 'E' :
        {int emboss[][3] = {    {-2,-1,0},
                                {-1,0,1},
                                {0,1,2}     };
        convoltionFilter((int *) emboss,3,3,1,0.5f);
        }break;

      case 'n' : case 'N' :
        {
          animate = 0;
          memcpy(image,source,sizeof(image));
        }break;

      case 's' : case 'S' :
        {
          int sharpen[][3] = {  {-1,-1,-1},
                                {-1,9,-1},
                                {-1,-1,-1}  };
          convoltionFilter((int *) sharpen,3,3,1,0.0f);
        }
        break;

      case 'b' : case 'B' :
        {
          int blur[][3] = { {1,2,1},
                            {2,4,2},
                            {1,2,1} };
          convoltionFilter((int *) blur,3,3,16,0.0f);
        }
        break;

      case 54 :         //right
        {dt +=1;}
        break;

      case 52 :         //left
        {dt -=1;}
        break;

      case 'd':
        {errorDiffusionDither1();}
        break;

      case 'D':
        {errorDiffusionDither8();}
        break;

      case 'a': case 'A':
        {animate=1;}
        break;

      case 'j': case 'J':
        { zoom +=1;
          juliaSet();}
        break;

      case 'q' : case 27 :
        exit(EXIT_SUCCESS);
        break;
    }

  glutPostRedisplay();
}

void update() {
  t += dt;
  t = t % TEX_SIZE;
  glutPostRedisplay();
}


// Main entry function
int main(int argc, char ** argv) {
    // Init GLUT
    glutInit(&argc, argv);
    glutInitWindowSize(TEX_SIZE, TEX_SIZE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutCreateWindow("OpenGL Window");
    // Set up OpenGL state
    init();
    // Run the control loop
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardEvents);
    glutIdleFunc(update);
    glutMainLoop();
    return EXIT_SUCCESS;
}
