/*
 OpenGL visualization skeleton for displaying bitmap images. Just provide a GenerateImage function.
 Good starting point for all image processing exercises for parallel programming.

 Example of generating bitmaps using GenerateImage and the prepared GLUT OpenGL visualization.
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/freeglut.h>
  #include <GL/freeglut_ext.h>
#endif

#define MAX(a,b) (a>b) ? a : b
#define MIN(a,b) (a<b) ? a : b
#define ABS(a) (a>=0) ? a : (-1*a)


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


float clump(float number){    //Orezanie
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

void loadAlphaLayers(){
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

void generateAlphaImage(){
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

short loadImageFromFileToSource(){
    int x,y;
    FILE *fimage;
    fimage = fopen("C:\\Users\\Jakub\\Dropbox\\5. sem\\PPGSO\\party.rgb","rb");            // b - cita ako binarny subor, defaultne ako textovy

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
      return 1;
    }else{
      return 0;
    }
}

void loadFromFiles(){
    loadImageFromFileToSource();
    memcpy(image,source,sizeof(image));
    loadAlphaLayers();
}

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
                    if(checkRange(imageX,imageY)){
                        r = colorToFloat(source[imageX][imageY].r);
                        g = colorToFloat(source[imageX][imageY].g);
                        b = colorToFloat(source[imageX][imageY].b);
                    }else{
                        r=1;
                        g=1;
                        b=1;
                    }

                    sum_r += r * (*((kernel+i*n)+j));
                    sum_g += g * (*((kernel+i*n)+j));
                    sum_b += b * (*((kernel+i*n)+j));
                }
            }

               image[x][y].r = clump((sum_r/div)+bias) * 255;
               image[x][y].g = clump((sum_g/div)+bias) * 255;
               image[x][y].b = clump((sum_b/div)+bias) * 255;
        }
    }
}

//Povodne generovanie imagu
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




int checkRange(int row, int col){
   if ( row>=0 && row<TEX_SIZE && col>=0 && col<TEX_SIZE ) return 1;
   else return 0;
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

    //generateAlphaImage();
}

// Generate and display the image.
void display() {
    // Call user image generation
    //GenerateImage();                      !!
    //generateAlphaImage();
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
        memcpy(image,source,sizeof(image));
        break;

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
        dt +=1;
        break;

      case 52 :         //left
        dt -=1;
        break;


      case 'q' : case 27 :
        exit(EXIT_SUCCESS);
        break;

    }

}

void update() {
  t += dt;
  t = t % TEX_SIZE;
  printf("%d\n",t);
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
