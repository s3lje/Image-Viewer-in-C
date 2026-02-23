/*
 * Image Viewer written in C
 *
 * The program allows the user to specify a .ppm image file
 * and displays it in a window.
 *
 * build options:
 *      gcc main.c -o test \
 *      `pkg-config --cflags glfw3 glew` \
 *      `pkg-config --libs glfw3 glew` \
 *      -framework OpenGL \
 *      -std=c99 -Wall
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600

typedef struct {
    int width, height;
    unsigned char* data;
} PPMImage;

PPMImage* loadPPM(const char*);
void freePPM(PPMImage*);

int main(int argc, char* argv[]){
    PPMImage* image;

    if (argc < 2){
        printf("usage: %s [image-file] ...\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        image = loadPPM(argv[1]);
        if (!image){
            printf("Could not load file: %s...\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    
    // Initialize glfw
    if (!glfwInit()){
        printf("Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
            "Image Viewer", NULL, NULL);
    if (!window){
        printf("Failed to create window...\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK){
        printf("Failed to initialize GLEW...\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
 
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
            0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Main Program Loop
    while (!glfwWindowShouldClose(window)){

        glfwPollEvents();
    }

    freePPM(image);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

PPMImage* loadPPM(const char* filename){
    char format[3];
    int width, height, maxval;
    unsigned char* data; 
    size_t read_size; 
    PPMImage* image; 

    FILE* f = fopen(filename, "rb");
    if (!f){
        fprintf(stderr, "Error: could not open file %s\n", filename);
        return NULL;
    }
    
    // Check valid format
    fscanf(f, "%2s", format);
    if (strcmp(format, "P6")){
        fprintf(stderr, "Error: not a P6 PPM file\n");
        fclose(f);
        return NULL;
    }
    
    // Skip comments
    while (1){
        int c = fgetc(f);
        if (c == '#'){
            while (fgetc(f) != '\n');
        } else {
            ungetc(c, f);
            break;
        }
    }
    
    fscanf(f, "%d %d", &width, &height);
    fscanf(f, "%d", &maxval);

    if (maxval != 255){
        fprintf(stderr, "Error: only maxval 255 supported...\n");
        fclose(f);
        return NULL;
    }
    fgetc(f);

    data = (unsigned char*)malloc(width * height * 3);
    if (!data){
        fprintf(stderr, "Error: memory allocation failed...\n");
        free(data);
        fclose(f);
        return NULL;
    }
    
    read_size = fread(data, 1, width * height * 3, f);
    if (read_size != width * height * 3){
        fprintf(stderr, "Error: failed to read image data...\n");
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);

    image = (PPMImage*)malloc(sizeof(PPMImage));
    image->width = width;
    image->height = height;
    image->data = data;

    return image; 
}

void freePPM(PPMImage* image){
    if (image){
        free(image->data);
        free(image);
    }
}
