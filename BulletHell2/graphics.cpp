#include <stdio.h>
#include <iostream>
#include <fstream>

#include "BulletHell2.hpp"
#include "direct.h"
#include "string.h"

#include "Functions.h"

#include "lpng/png.h"

#include "window.hpp"

#define POSITION_ATTRIB 0
#define COLOR_ATTRIB 1
#define TEX_ATTRIB 2

int vert_count = 4;

GLfloat vert_data[] = {
   -0.5, 0.5, 0.0,
   -0.5, -0.5, 0.0,
   0.5, 0.5, 0.0,
   0.5, -0.5, 0.0
};

GLfloat col_data[] = {
   1.0, 0.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 0.0, 1.0,
   1.0, 1.0, 1.0
};

GLfloat tex_data[] = {
   0.0, 1.0,
   0.0, 0.0,
   1.0, 1.0,
   1.0, 0.0
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
}



void init()
{
   char* buffer;

   // Get the current working directory: 
   if ((buffer = _getcwd(NULL, 0)) == NULL)
      perror("_getcwd error");
   else
   {
      printf("%s \nLength: %d\n", buffer, strnlen(buffer, 99999));
      free(buffer);
   }

   GLuint vao;
   //GLuint bufs[2];
   GLuint bufs[3];
   glGenVertexArrays(1, &vao);
   //glGenBuffers(2, bufs);
   glGenBuffers(3, bufs);

   glBindVertexArray(vao);
   glEnableVertexAttribArray(POSITION_ATTRIB);
   glEnableVertexAttribArray(COLOR_ATTRIB);
   glEnableVertexAttribArray(TEX_ATTRIB);

   glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
   glBufferData(GL_ARRAY_BUFFER, 3 * vert_count * sizeof(GLfloat), vert_data, GL_STATIC_DRAW);
   glVertexAttribPointer(POSITION_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
   glBufferData(GL_ARRAY_BUFFER, 3 * vert_count * sizeof(GLfloat), col_data, GL_STATIC_DRAW);
   glVertexAttribPointer(COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ARRAY_BUFFER, bufs[2]);
   glBufferData(GL_ARRAY_BUFFER, 2 * vert_count * sizeof(GLfloat), tex_data, GL_STATIC_DRAW);
   glVertexAttribPointer(TEX_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);

   int size = 64;
   png_texture_load("png\\test1.png", &size, &size);

   GLuint prog;
   GLuint vert;
   GLuint frag;

   prog = glCreateProgram();
   vert = glCreateShader(GL_VERTEX_SHADER);
   frag = glCreateShader(GL_FRAGMENT_SHADER);

   std::string vert_shader;
   std::string frag_shader;

   read_shaders(vert_shader, frag_shader);

   const char *vert_char = vert_shader.c_str();
   const char *frag_char = frag_shader.c_str();

   glShaderSource(vert, 1, &vert_char, 0);
   glShaderSource(frag, 1, &frag_char, 0);
   glCompileShader(vert);
   glCompileShader(frag);

   //int status_vert;
   //glGetShaderiv(vert, GL_COMPILE_STATUS, &status_vert);
   //if (status_vert == GL_FALSE)
   //{
   //	int breakfail = 0;
   //}
   //int status_frag;
   //glGetShaderiv(frag, GL_COMPILE_STATUS, &status_frag);
   //if (status_frag == GL_FALSE)
   //{
   //	int breakfail = 0;
   //}

   glAttachShader(prog, vert);
   glAttachShader(prog, frag);

   glBindAttribLocation(prog, POSITION_ATTRIB, "position");
   glBindAttribLocation(prog, COLOR_ATTRIB, "color");
   glBindAttribLocation(prog, TEX_ATTRIB, "tex_in");

   glUniform1i(glGetUniformLocation(prog, "tex"), 0);

   glLinkProgram(prog);
   glUseProgram(prog);

   glValidateProgram(prog);
}

void render(GLFWwindow* window)
{
   float ratio;
   int width, height;
   glfwGetFramebufferSize(window, &width, &height);
   ratio = width / (float)height;
   glViewport(0, 0, width, height);
   glClear(GL_COLOR_BUFFER_BIT);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glfwSwapBuffers(window);



   //////
   //glClear(GL_COLOR_BUFFER_BIT);

   //glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
   //glBegin(GL_TRIANGLES);
   //glColor3f(1.f, 0.f, 0.f);
   //glVertex3f(-0.6f, -0.4f, 0.f);
   //glColor3f(0.f, 1.f, 0.f);
   //glVertex3f(0.6f, -0.4f, 0.f);
   //glColor3f(0.f, 0.f, 1.f);
   //glVertex3f(0.f, 0.6f, 0.f);
   //glEnd();
   //glfwSwapBuffers(window);
   glfwPollEvents();
}

void getOpenGLVersion(int& major, int& minor)
{
   major = 1;
   minor = 1; //some error happened:  assume this is a software implementation.
   std::string versionString = (char*)glGetString(GL_VERSION);
   if (versionString.length() < 3) return;

   std::string majorStr = "";
   std::string minorStr = "";
   const char* cursor = versionString.c_str();
   while (*cursor != '.')
   {
      majorStr.push_back(*cursor);
      ++cursor;
   }
   ++cursor;
   while (*cursor != '.' && *cursor != ',' && *cursor != '\0')
   {
      minorStr.push_back(*cursor);
      ++cursor;
   }

   major = atoi(majorStr.c_str());
   minor = atoi(minorStr.c_str());
}

void read_shaders(std::string& vert_shader, std::string& frag_shader)
{
   //std::string vert_shader;
   //std::string frag_shader;
   std::string line;

   std::ifstream vertfile("shaders/vertex_shader_1.txt");
   if (vertfile.is_open())
   {
      while (getline(vertfile, line))
      {
         vert_shader += line;
         vert_shader += "\n";
      }
      vertfile.close();
   }

   std::ifstream fragfile("shaders/fragment_shader_1.txt");
   if (fragfile.is_open())
   {
      while (getline(fragfile, line))
      {
         frag_shader += line;
         frag_shader += "\n";
      }
      fragfile.close();
   }
}

//https://github.com/DavidEGrayson/ahrs-visualizer/blob/master/png_texture.cpp
GLuint png_texture_load(const char * file_name, int * width, int * height)
{
   // This function was originally written by David Grayson for
   // https://github.com/DavidEGrayson/ahrs-visualizer

   png_byte header[8];

   FILE *fp = fopen(file_name, "rb");
   if (fp == 0)
   {
      perror(file_name);
      return 0;
   }

   // read the header
   fread(header, 1, 8, fp);

   if (png_sig_cmp(header, 0, 8))
   {
      fprintf(stderr, "error: %s is not a PNG.\n", file_name);
      fclose(fp);
      return 0;
   }

   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
   {
      fprintf(stderr, "error: png_create_read_struct returned 0.\n");
      fclose(fp);
      return 0;
   }

   // create png info struct
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      fprintf(stderr, "error: png_create_info_struct returned 0.\n");
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      fclose(fp);
      return 0;
   }

   // create png info struct
   png_infop end_info = png_create_info_struct(png_ptr);
   if (!end_info)
   {
      fprintf(stderr, "error: png_create_info_struct returned 0.\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      return 0;
   }

   // the code in this if statement gets called if libpng encounters an error
   if (setjmp(png_jmpbuf(png_ptr))) {
      fprintf(stderr, "error from libpng\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      fclose(fp);
      return 0;
   }

   // init png reading
   png_init_io(png_ptr, fp);

   // let libpng know you already read the first 8 bytes
   png_set_sig_bytes(png_ptr, 8);

   // read all the info up to the image data
   png_read_info(png_ptr, info_ptr);

   // variables to pass to get info
   int bit_depth, color_type;
   png_uint_32 temp_width, temp_height;

   // get info about png
   png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
      NULL, NULL, NULL);

   if (width) { *width = temp_width; }
   if (height) { *height = temp_height; }

   //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

   if (bit_depth != 8)
   {
      fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
      return 0;
   }

   GLint format;
   switch (color_type)
   {
   case PNG_COLOR_TYPE_RGB:
      format = GL_RGB;
      break;
   case PNG_COLOR_TYPE_RGB_ALPHA:
      format = GL_RGBA;
      break;
   default:
      fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
      return 0;
   }

   // Update the png info struct.
   png_read_update_info(png_ptr, info_ptr);

   // Row size in bytes.
   int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

   // glTexImage2d requires rows to be 4-byte aligned
   rowbytes += 3 - ((rowbytes - 1) % 4);

   // Allocate the image_data as a big block, to be given to opengl
   png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte) + 15);
   if (image_data == NULL)
   {
      fprintf(stderr, "error: could not allocate memory for PNG image data\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      fclose(fp);
      return 0;
   }

   // row_pointers is for pointing to image_data for reading the png with libpng
   png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
   if (row_pointers == NULL)
   {
      fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      free(image_data);
      fclose(fp);
      return 0;
   }

   // set the individual row_pointers to point at the correct offsets of image_data
   for (unsigned int i = 0; i < temp_height; i++)
   {
      row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
   }

   // read the png into image_data through row_pointers
   png_read_image(png_ptr, row_pointers);

   // Generate the OpenGL texture object
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   // clean up
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   free(image_data);
   free(row_pointers);
   fclose(fp);
   return texture;
}


class Window::Impl
{
public:
   Impl(WindowInit const& initData)
   {
      glfwInit(); //yeah this should probably be elsewhere


      window = glfwCreateWindow(initData.width, initData.height, initData.title.c_str(), NULL, NULL);
      if (!window)
      {
         glfwTerminate();
         std::cout << "GLFW window failed :<" << std::endl;


         std::cin.get();
         return;
      }

      glfwMakeContextCurrent(window);

      GLenum err = glewInit();
      //if(GLEW_OK != err)
      //{
      //   std::cout << "It didnt work :<" << std::endl;
      //   std::cout << glewGetErrorString(err) << std::endl;
      //   std::cin.get();
      //   return 0;
      //}

      int major, minor;
      getOpenGLVersion(major, minor);
      //std::cout << "Your openGL version is: " << major << "." << minor << std::endl;
      //std::cout << "boop!" << std::endl;
      //std::cin.get();

      glfwSwapInterval(1);
      glfwSetKeyCallback(window, key_callback);

      init();
   }
   GLFWwindow* window;
};


Window::Window(WindowInit const& initData)
   : pImpl(new Impl(initData))
{
}
Window::~Window() 
{
   if (!pImpl->window) return;

   glfwDestroyWindow(pImpl->window);
   glfwTerminate();
}
void Window::update()
{
   if (isOpen())
   {
      render(pImpl->window);
   }
}
bool Window::isOpen()
{
   return pImpl->window && !glfwWindowShouldClose(pImpl->window);
}