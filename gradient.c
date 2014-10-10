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

#define TEX_SIZE 512
#define LAYER_1_SIZE 128
#define C_RED 0
#define C_GREEN 1
#define C_BLUE 2

extern int errno;

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
} rgba_pix;

pixel image[2*TEX_SIZE][TEX_SIZE];

rgba_pix alpha[LAYER_1_SIZE][LAYER_1_SIZE];

GLuint texture;

void load_image() {
    int x,y;
	FILE *f = fopen("image.rgb", "rb");
	if(f == NULL) {
		printf("Error opening image: %s \n", strerror(errno));
		exit(1);
	}
	fread(image, TEX_SIZE*TEX_SIZE*sizeof(pixel), 1, f);
	fclose(f);

	FILE *f2 = fopen("alpha.rgba", "rb");
	if(f2 == NULL) {
	    printf("Error opening alpha: %s \n", strerror(errno));
	    exit(1);
	}
	fread(alpha, sizeof(alpha), 1, f2);

	fclose(f2);
	printf("Alpha is %d\n", alpha[112][109].a);
}

void set_color(int x, int y, int red, int green, int blue) {
	if(red > 255) {
		image[x][y].r = 255;
	} else if(red < 0) {
		image[x][y].r = 0;
	} else {
		image[x][y].r = red;
	}

	if(green > 255) {
		image[x][y].g = 255;
	} else if(green < 0) {
		image[x][y].g = 0;
	} else {
		image[x][y].g = green;
	}

	if(blue > 255) {
		image[x][y].b = 255;
	} else if(blue < 0) {
		image[x][y].b = 0;
	} else {
		image[x][y].b = blue;
	}
}

void convolution_transform_3x(int x, int y, float kernel[3][3], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;
	int efx = TEX_SIZE+x;

	for(kx = x - 1; kx <= x+1; kx++) {
		if(kx >= 0 && kx < TEX_SIZE) {
			for(ky = y - 1; ky <= y+1; ky++) {
				if(ky >= 0 && ky < TEX_SIZE) {
					int kerneln = kernel[(kx-x)+1][(ky-y)+1];
					red += round(image[kx][ky].r*kerneln);
					green += round(image[kx][ky].g*kerneln);
					blue += round(image[kx][ky].b*kerneln);
				}
			}
		}
	}

	set_color(efx, y, red*dividor, green*dividor, blue*dividor);
}

void convolution_transform_5x(int x, int y, float kernel[5][5], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;
	int efx = TEX_SIZE+x;

	for(kx = x - 2; kx <= x+2; kx++) {
		if(kx >= 0 && kx < TEX_SIZE) {
			for(ky = y - 2; ky <= y+2; ky++) {
				if(ky >= 0 && ky < TEX_SIZE) {
					int kerneln = kernel[(kx-x)+2][(ky-y)+2];
					red += round(image[kx][ky].r*kerneln);
					green += round(image[kx][ky].g*kerneln);
					blue += round(image[kx][ky].b*kerneln);
				}
			}
		}
	}

	set_color(efx, y, red*dividor, green*dividor, blue*dividor);
}

void convolution3x(float kernel[3][3], float dividor) {
	int x, y;

	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			convolution_transform_3x(x,y,kernel, dividor);
		}
	}
}

void convolution5x(float kernel[5][5], float dividor) {
	int x,y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			convolution_transform_5x(x,y,kernel,dividor);
		}
	}
}

void blur() {
	float kernel[3][3] = {
		{1,1,1},
		{1,1,1},
		{1,1,1}
	};

	convolution3x(kernel, 1.00/9.00);
}

void blur5x() {
	float kernel[5][5] = {
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1}
	};

	convolution5x(kernel, 1.00/25.0);
}

void edge_detection1() {
	float kernel[3][3] = {
		{1,0,-1},
		{0,0,0},
		{-1,0,1}
	};

	convolution3x(kernel, 1.00);
}

void edge_detection3() {
	float kernel[3][3] = {
		{-1, -1, -1},
		{-1,  8, -1},
		{-1, -1, -1}
	};

	convolution3x(kernel, 1.0);
}

void sharpen() {
    float kernel[3][3] = {
        {0,-1,0},
        {-1,5,-1},
        {0,-1,0}
    };

    convolution3x(kernel, 1.0);
}

void (*effect)() = blur;

void handle_keyboard(unsigned char ch, int x, int y) {
    switch(ch) {
        case 'b':
            effect = blur;
            break;
        case 's':
            effect = sharpen;
            break;
        case 'e':
            effect = edge_detection1;
            break;
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
    load_image();
}

void blend() {
    int x,y, x2, y2;
    for(x=TEX_SIZE; x < TEX_SIZE+LAYER_1_SIZE; x++) {
        for(y=0; y< LAYER_1_SIZE; y++) {
            set_color(x, y,
                ((alpha[x-TEX_SIZE][y].a/255.0)*alpha[x-TEX_SIZE][y].r) + round(image[x][y].r*(1-(alpha[x-TEX_SIZE][y].a/255.0))),
                ((alpha[x-TEX_SIZE][y].a/255.0)*alpha[x-TEX_SIZE][y].g) + round(image[x][y].g*(1-(alpha[x-TEX_SIZE][y].a/255.0))),
                ((alpha[x-TEX_SIZE][y].a/255.0)*alpha[x-TEX_SIZE][y].b) + round(image[x][y].b*(1-(alpha[x-TEX_SIZE][y].a/255.0)))
//                alpha[x-TEX_SIZE][y].r,
//                alpha[x-TEX_SIZE][y].g,
//                alpha[x-TEX_SIZE][y].b
            );
        }
    }
}

// Generate and display the image.
void display() {
    // Call user image generation

    effect();
    blend();
	//blur();
	//blur5x();
	//sharpen();
    // Copy image to texture memory
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, 2*TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
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
    glutInitWindowSize(TEX_SIZE, 2*TEX_SIZE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutCreateWindow("OpenGL Window");
    // Set up OpenGL state
    init();
    // Run the control loop
    glutDisplayFunc(display);
    glutKeyboardFunc(handle_keyboard);
    glutMainLoop();
    return EXIT_SUCCESS;
}
