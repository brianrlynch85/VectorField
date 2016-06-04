/* 
 * ------------------------------------------------------------------------
 *
 *                                   vector_field.cpp
 *                                        V 0.01
 *
 *                              (c) Brian Lynch June, 2016
 *
 * ------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define UPI 3.14159265359
#define display_counter_MAX 1000

//Store the vector field information
typedef struct p_vector{
   double xcm;
   double ycm;
   double vx;
   double vy;
}p_vector;

//Dimensions of the image the data came from
static const unsigned int xsize = 300;
static const unsigned int ysize = 300;

//Scale factor used to scale the arrow size
const double scale = 0.05l;//2.75f;

//Array to store the vector field information structure
struct p_vector *par;

//Number of vectors to draw
unsigned int pnum = 0U;

//openGL stuff
static GLint attribute_posxy;
static GLuint program;
static GLfloat *arrowhead_vertices;
static GLfloat *arrowtail_vertices;

static unsigned int display_counter = 0U;

void print_usage();

/***************************************************************************************/
//read_file function to populate velocity vector data
void read_file(const char *filename){
   
   FILE *infile;
   infile = fopen(filename, "r");
   
   if(!infile){
      
      printf("Error reading input file %s\n",filename);
      exit(EXIT_FAILURE);
      
   }
   
   printf("Populating vector field from file\n");
   
   //Find the number of file lines. This likely isn't the best way to do this
   pnum = 0;
   double temp0, temp1, temp2, temp3, temp4, temp5;
   while(!feof(infile)){
      
      if(fscanf(infile,"%lf %lf %lf %lf %lf %lf",&temp0,&temp1,&temp2,&temp3,&temp4,&temp5) != 6){
         
         break;
         
      }
      
      ++pnum;
      
   }
   
   printf("Found %u total vectors\n",pnum);
   
   //Set the file pointer back to beginning
   rewind(infile);
   
   //Make room in the storage arrays for vector data
   par = (struct p_vector*)malloc(pnum * sizeof(struct p_vector));
   arrowhead_vertices = (GLfloat *)malloc(6 * pnum * sizeof(GLfloat));
   arrowtail_vertices = (GLfloat *)malloc(4 * pnum * sizeof(GLfloat));
   
   //Now store the vector data in arrays
   unsigned int q = 0;
   while(!feof(infile)){
      
      if(fscanf(infile,"%lf %lf %lf %lf %lf %lf",&temp0,&temp1,&par[q].xcm,&par[q].ycm,&par[q].vx,&par[q].vy) != 6){
         
         break;
         
      }
      
      //Convert to image coordinates
      par[q].xcm  = (2.0l * par[q].xcm / xsize) - 1.0l;
      par[q].ycm  = (2.0l * par[q].ycm / ysize) - 1.0l;
      par[q].ycm *= -1.0l;
      par[q].vy  *= -1.0l;
      
      ++q;
      
   }
   
   //Close the input file
   fclose(infile);

return;
}

/***************************************************************************************/
//GLSL shader programs and initializations
int init_shaders(void){
   
   /*
    * Vertex_shader is a GPU object that can be used to manipulate rendered geometries
    * We have to do this because shader object support is optional and GPU dependent
    */
   
   //Create an empty vertex shader
   GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
   
   //This is source code to be loaded onto the GPU shader object stored in a char*
   const GLchar *vertex_shader_source = 
                                        "#version 120                         \n"
                                        "attribute vec2 posxy;                  "
                                        "void main(void) {                      "
                                        "  gl_Position = vec4(posxy, 0.0, 1.0); "
                                        "}";
   
   //Replaces the original shader vs by vs_source
   glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
   
   //Compiles the source code stored in vertex shader
   glCompileShader(vertex_shader);
   
   //Returns compilation status of the shader object
   GLint compiled_vs = GL_FALSE;
   glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled_vs);
   
   //Exit Program if shader does not compile correctly
   if(GL_FALSE == compiled_vs){
      
      printf("Error in vertex shader initialization\n");
      exit(EXIT_FAILURE);
      
   }
   
   /*
    * Fragment shader is used to fill the points (given to the vertex shader)
    * in a pixel-wise fashion
    */
   
   //Create an empty fragment shader
   GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
   
   //Fragment shader source code.Since its RGB, note that (0,0,0) is white
   const GLchar *fragment_shader_source =
                                          "#version 120           \n"
                                          "void main(void) {        "
                                          "  gl_FragColor[0] = 0.0; "
                                          "  gl_FragColor[1] = 0.0; "
                                          "  gl_FragColor[2] = 0.0; "
                                          "}";
   
   //Repeat the same process as for the vertex shader
   glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
   
   //Compile the code stored in fragment shader
   glCompileShader(fragment_shader);
   
   //Returns compilation status of the shader object
   GLint compiled_fs = GL_FALSE;
   glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled_fs);
   
   //Exit program if shader does not compile correctly
   if(GL_FALSE == compiled_fs){
      
      printf("Error in fragment shader initialization\n");
      exit(EXIT_FAILURE);
      
   }
   
   //Creates a program object to link the vertex and fragment shaders
   program = glCreateProgram();
   
   //Attach the shaders
   glAttachShader(program, vertex_shader);
   glAttachShader(program, fragment_shader);
   
   //Create an executable to run on the GPU
   glLinkProgram(program);
   
   //Test whether or not the program linked correctly
   GLint linked = GL_FALSE;
   glGetProgramiv(program,GL_LINK_STATUS,&linked);
   
   //Exit program if unsuccessful compilation
   if(GL_FALSE == linked){
      
      printf("Failed to link shader programs");
      exit(EXIT_FAILURE);
      
   }
   
   const char* attribute_name = "posxy";
   //Attribute variables are used to store vertex shader data
   attribute_posxy = glGetAttribLocation(program, attribute_name);
   
   //Exit the program if the attributes do not bind
   if(-1 == attribute_posxy){
      
      printf("Could not bind attribute %s\n", attribute_name);
      exit(EXIT_FAILURE);
      
   }
   
   return true;
   
}

/***************************************************************************************/
//idle callback function
void idle(void){
   
   if(display_counter < display_counter_MAX){
      
      glutPostRedisplay();
      display_counter++;
      printf("display counter %d\r",display_counter);
      fflush(stdout);
      
   }else{
      
      printf("\nExit main\n");
      exit(0);
      
   }
}

/***************************************************************************************/
//initialization GL
void initGL(){
   
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   
}

/***************************************************************************************/
//reshape callback function
void reshape(int width, int height){
   
   //Set our viewport to the size of our window
   glViewport(0, 0, (GLsizei)width, (GLsizei)height);
   
   //Switch to the projection matrix so that we can manipulate how our scene is viewed
   glMatrixMode(GL_PROJECTION);
   
   //Reset the projection matrix to the identity matrix so that we don't get any artifacts
   //(cleaning up)
   glLoadIdentity();
   
   //Set the Field of view angle (in degrees), the aspect ratio of our window
   //and the new and far planes
   gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 100.0);
   
   //Switch back to the model view matrix, so that we can start drawing shapes correctly
   glMatrixMode(GL_MODELVIEW);
   
}

/***************************************************************************************/
//Calculate the geometry of vector arrows
inline void calculate_arrows(){
   
   //Scale parameters for the length of vector arrow and its tales
   double scale_frac = scale / xsize,
          tip_frac   = 0.75,
          side_frac  = 0.30,
          tail_frac  = 0.75;
   
   unsigned int i = 0;
   //Render the vector arrows
   for(unsigned int q = 0; q < 6 * pnum; q += 6){
      
      arrowhead_vertices[q]     = par[i].xcm + scale_frac * tip_frac * par[i].vx;
      arrowhead_vertices[q + 1] = par[i].ycm + scale_frac * tip_frac * par[i].vy;
      arrowhead_vertices[q + 2] = par[i].xcm - scale_frac * side_frac * par[i].vy;
      arrowhead_vertices[q + 3] = par[i].ycm + scale_frac * side_frac * par[i].vx;
      arrowhead_vertices[q + 4] = par[i].xcm + scale_frac * side_frac * par[i].vy;
      arrowhead_vertices[q + 5] = par[i].ycm - scale_frac * side_frac * par[i].vx;
      ++i;
   }
   
   //Tell shaders whats going on in with the vertices
   glVertexAttribPointer(
                            attribute_posxy,   //attribute_posxy
                            2,                 //each vertex is (x,y) point
                            GL_FLOAT,          //each element is a float
                            GL_FALSE,          //data taken as written 
                            0,                 //no bytes between data
                            arrowhead_vertices //pointer data
                        );
   
   //Push the triangle (velocity vector head) onto the buffer
   glDrawArrays(GL_TRIANGLES,0,3 * pnum);
   
   //Render the vector tails
   i = 0;
   for(unsigned int q = 0; q < 4 * pnum; q += 4){
      //Points given by middle of velocity vector base and scaled backwards to create tail
      arrowtail_vertices[q] = par[i].xcm;
      arrowtail_vertices[q + 1] = par[i].ycm;                                  
      arrowtail_vertices[q + 2] = par[i].xcm - scale_frac * tail_frac * par[i].vx;
      arrowtail_vertices[q + 3] = par[i].ycm - scale_frac * tail_frac * par[i].vy;
      ++i;
   }
   
   //Push the line (velocity vector tail) onto the buffer
   glLineWidth(2.25);
   
   //Tell vertex shader whats going on in OpenGL
   glVertexAttribPointer(
                            attribute_posxy,   
                            2,                 
                            GL_FLOAT,          
                            GL_FALSE,          
                            0,                 
                            arrowtail_vertices 
                        );
   
   //Push the line (velocity vector tail) onto the buffer
   glDrawArrays(GL_LINES,0,2 * pnum);
   
   return;
}

/***************************************************************************************/
//display callback function
void display(){
   
   //Set the background to white
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   
   //Clear the current buffer
   glClear(GL_COLOR_BUFFER_BIT);
   
   //Enable the program
   glUseProgram(program);
   
   //Enable the attribute array with index ID attribute_coord2d
   glEnableVertexAttribArray(attribute_posxy);
   
   calculate_arrows();
   
   //Disable the attribute array
   glDisableVertexAttribArray(attribute_posxy);
   
   //Display the result by swapping the buffer
   glutSwapBuffers();
   
}

/***************************************************************************************/
//Free the resources taken up by this OpenGL code
void free_resources(){
   
   //Free the memory and invalid the name associated with the program object
   glDeleteProgram(program);
   free(par);
   free(arrowhead_vertices);
   free(arrowtail_vertices);
   
}

/***************************************************************************************/
int main(int argc, char* argv[]){

   // Command line option parser variable
   int opt = 0;

   // Input filename
   char *filename;

   // Parse the command line
   if(1 == argc){
      
      print_usage(); 
      exit(EXIT_FAILURE);
       
   }
      
   while((opt = getopt(argc, argv,"-f:")) != -1) {
     
      switch (opt) {
         
         case 'f' : // Input filename option
            
            filename = optarg;
            printf("Input Filename: %s \n", filename);
            break;
            
         case '?': // Unrecognized command line option
            
            printf("Unrecognized command line option\n");
            print_usage();
            return (-1);
            
         case '\1': // The - in "-f:" finds non option command line params
            
            printf("Passed non-option to command line\n");
            print_usage();
            return (-1);
            
      }
        
   }
   //Read the data from file
   read_file(filename);
   
   //Initialize the GLUT library
   glutInit(&argc, argv);
   
   //Initialize colored window, multisampled, with double buffer
   glutInitDisplayMode(GLUT_RGB|GLUT_MULTISAMPLE|GLUT_DOUBLE);
   
   //Initialize window size and location
   glutInitWindowSize(xsize,ysize);
   glutInitWindowPosition(0,0);
   
   //Create the window and store the id number
   int windowID = glutCreateWindow("Velocity Field Rendering");
   
   //Initialize the openGL extension wrangler library (GLEW)
   GLenum glew_err = glewInit();
   if(GLEW_OK != glew_err){
      
      printf("Error: %s\n", glewGetErrorString(glew_err));
      exit(EXIT_FAILURE);
      
   }
   
   //Intialize the shaders
   if(init_shaders()){
      
      glutIdleFunc(idle);
      glutDisplayFunc(display);
      glutReshapeFunc(reshape);
      initGL();
      glutMainLoop();
      
   }
   
   //Free the resources
   free_resources();
   return 0;
   
}

/************************************************************************/
void print_usage(){
   
   printf("Usage:\n");
   printf("build/bin/vector_field -f <filename>\n");
   printf("build/bin/vector_field -f ExampleData/ExampleData.dat\n");
   
}
