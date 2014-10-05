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

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

#define TEX_SIZE 512
pixel image[TEX_SIZE][TEX_SIZE];
pixel effect[TEX_SIZE][TEX_SIZE];
pixel output[2*TEX_SIZE][TEX_SIZE];

int kernel[3][3] = {
	{1,1,1},
	{1,1,1},
	{1,1,1}
};

float dividor = (float)1/(float)9;

GLuint texture;

void load_image() {
	FILE *f = fopen("image.rgb", "rb");
	if(f == NULL) {
		printf("Error opening image\n");
		exit(1);
	}
	fread(image, sizeof(image), 1, f);
	fclose(f);
}

void convolution_transform(int x, int y) {
	int kx, ky;
	int red = 0;
	int green = 0;
	int blue = 0;
	int division = 0;

	for(kx = x - 1; kx <= x+1; kx++) {
		if(kx >= 0) {
			for(ky = y - 1; ky <= y+1; ky++) {
				if(ky >= 0) {
					red += image[kx][ky].r;
					green += image[kx][ky].g;
					blue += image[kx][ky].b;
					division++;
				}
			}
		}
	}

	effect[x][y].r = red/division;
	effect[x][y].g = green/division; 
	effect[x][y].b = blue/division;
}

void convolution() {
	int unsigned x, y;
	for(x=0; x<TEX_SIZE; x++) {
		for(y=0; y<TEX_SIZE; y++) {
			convolution_transform(x, y);
		}
	}
}

void copy_to_output() {
	int x,y;
	for(x=0; x<TEX_SIZE; x++) {
		for(y=0; y<TEX_SIZE;y++) {
			int x2 = x + TEX_SIZE;
			output[x][y].r = image[x][y].r;
			output[x][y].g = image[x][y].g;
			output[x][y].b = image[x][y].b;
			output[x2][y].r = effect[x][y].r;
			output[x2][y].g = effect[x][y].g;
			output[x2][y].b = effect[x][y].b;
		}
	}
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
    gluOrtho2D(-1,1,-1,1);
    glLoadIdentity();
    glColor3f(1,1,1);
}

// Generate and display the image.
void display() {
    // Call user image generation
    load_image();
	convolution();
	copy_to_output();
    // Copy image to texture memory
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2*TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, output);
    // Clear screen buffer
    glClear(GL_COLOR_BUFFER_BIT);
    // Render a quad
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(1,1);
        glTexCoord2f(0,1); glVertex2f(1,-1);
        glTexCoord2f(1,1); glVertex2f(-1,-1);
        glTexCoord2f(1,0); glVertex2f(-1,1);
    glEnd();
    // Display result
    glFlush();
    glutPostRedisplay();
    glutSwapBuffers();
}

// Main entry function
int main(int argc, char ** argv) {
    // Init GLUT
    glutInit(&argc, argv);
    glutInitWindowSize(2*TEX_SIZE, TEX_SIZE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutCreateWindow("OpenGL Window");
    // Set up OpenGL state
    init();
    // Run the control loop
    glutDisplayFunc(display);
    glutMainLoop();
    return EXIT_SUCCESS;
}
