#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "BulletHell2.hpp"
#include "direct.h"
#include "string.h"

#include "Functions.h"

#include "lpng/png.h"

#include "window.hpp"

#include "constants.hpp"

#include "input.hpp"

#include "assert.h"

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

 //GLfloat vert_data[] = {
	//0.0, 1.0, 0.0,
	//0.0, 0.0, 0.0,
	//1.0, 1.0, 0.0,
	//1.0, 0.0, 0.0
 //};

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


GLuint prog2D;
GLuint prog3D;
GLuint progActive;
GLuint background_texture;

//Sprites :D
std::unordered_map<std::string, Sprite> sprite_map;
Sprite* bg1; //put this somewhere better eventually...

//Sprite properties
GLint uniform_xpos;
GLint uniform_ypos;
GLint uniform_xsize;
GLint uniform_ysize;     
GLint uniform_flip_horz; //flip horizontal
GLint uniform_flip_vert; //flip vertical
GLint uniform_rotation;  //first line of the rotation matrix as a 2D vector

//3D sprite properties
GLint uniform_projection;
GLint uniform_view;


static KeyPressType translateGLFWKeyEvent(int action)
{
   if (action == GLFW_PRESS) return KeyPressType::Down;
   else if (action == GLFW_RELEASE) return KeyPressType::Up;
   else if (action == GLFW_REPEAT) return KeyPressType::Repeated;

   //how'd I get here
   assert(0 && "bad enum state!");

   return KeyPressType::Down;
}
static int translateGLFWKey(int key)
{
   // r/shittyprogramming
   // this one is kinda annoying to do right, since you want to hide implementation details.  Probably should be a table something or somewhere when we have more cases?

   if (key >= 'a' && key <= 'z')
   {
      //I don't think GLFW ever does this, but eh, just fix it regardless
      return key - ('a' - 'A');
   }

   if (key == GLFW_KEY_LEFT) return KeyType::Left;
   else if (key == GLFW_KEY_RIGHT) return  KeyType::Right;
   else if (key == GLFW_KEY_UP) return  KeyType::Up;
   else if (key == GLFW_KEY_DOWN) return  KeyType::Down;
   else if (key == GLFW_KEY_LEFT_SHIFT) return  KeyType::LeftShift;
   else if (key == GLFW_KEY_RIGHT_SHIFT) return  KeyType::RightShift;

   return key;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);

   KeyEvent ev;
   ev.type = translateGLFWKeyEvent(action);
   ev.key = translateGLFWKey(key);

   inputPushEvent(ev);
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
   background_texture = png_texture_load("png\\bg1.png", &size, &size);

   prog2D = LoadShader("shaders/vertex_shader_1.txt", "shaders/fragment_shader_1.txt");

   glUniform2f(glGetUniformLocation(prog2D, "uCameraSize"), constants::cameraSize.x, constants::cameraSize.y);
   glUniform1i(glGetUniformLocation(prog2D, "tex"), 0);

   uniform_xpos  = glGetUniformLocation(prog2D, "x_position");
   uniform_ypos  = glGetUniformLocation(prog2D, "y_position");
   uniform_xsize = glGetUniformLocation(prog2D, "width");
   uniform_ysize = glGetUniformLocation(prog2D, "height");
   uniform_flip_horz = glGetUniformLocation(prog2D, "flip_horizontal");
   uniform_flip_vert = glGetUniformLocation(prog2D, "flip_vertical");
   uniform_rotation = glGetUniformLocation(prog2D, "rotation_vector");

   prog3D = LoadShader("shaders/vertex_shader_2.txt", "shaders/fragment_shader_2.txt");

   glUniform1i(glGetUniformLocation(prog3D, "tex"), 0);

   uniform_view = glGetUniformLocation(prog3D, "view_matrix");
   uniform_projection = glGetUniformLocation(prog3D, "projection_matrix");

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   bg1 = GetSprite("png/bg1.png");
}

void SetShader(GLuint shader)
{
   if (progActive == shader)
      return;

   progActive = shader;
   glUseProgram(progActive);
}

GLuint LoadShader(std::string vertex_path, std::string fragment_path)
{
   GLuint vert;
   GLuint frag;
   GLuint prog;

   prog = glCreateProgram();
   vert = glCreateShader(GL_VERTEX_SHADER);
   frag = glCreateShader(GL_FRAGMENT_SHADER);

   std::string vert_shader = read_shader(vertex_path);
   std::string frag_shader = read_shader(fragment_path);

   const char *vert_char = vert_shader.c_str();
   const char *frag_char = frag_shader.c_str();

   glShaderSource(vert, 1, &vert_char, 0);
   glShaderSource(frag, 1, &frag_char, 0);
   glCompileShader(vert);
   glCompileShader(frag);

   int status_vert;
   bool shader_failed = false;
   glGetShaderiv(vert, GL_COMPILE_STATUS, &status_vert);
   if (status_vert == GL_FALSE)
   {
      shader_failed = true;
   }
   int status_frag;
   glGetShaderiv(frag, GL_COMPILE_STATUS, &status_frag);
   if (status_frag == GL_FALSE)
   {
      shader_failed = true;
   }

   if (shader_failed)
   {
      std::cout << "Shader confirmed garbage. Learn how to program idiot" << std::endl;
      std::cin.get();
   }

   glAttachShader(prog, vert);
   glAttachShader(prog, frag);

   glBindAttribLocation(prog, POSITION_ATTRIB, "position");
   glBindAttribLocation(prog, COLOR_ATTRIB, "color");
   glBindAttribLocation(prog, TEX_ATTRIB, "tex_in");

   glLinkProgram(prog);
   glUseProgram(prog);

   glValidateProgram(prog);

   return prog;
}

//set viewport in virtual camera coordinates
void setVirtualViewport(GLFWwindow* window, Vec2 min, Vec2 max)
{
   int width, height;
   glfwGetFramebufferSize(window, &width, &height);

   float xRatio = width / constants::UICameraSize.x;
   float yRatio = height / constants::UICameraSize.y;

   glViewport(
      xRatio * min.x,
      yRatio * min.y,
      xRatio * max.x,
      yRatio * max.y);
}

void render(GLFWwindow* window)
{ 
   setVirtualViewport(window, Vec2(0.0f, 0.0f), constants::cameraSize);
   //glViewport(0, 0, width- UIPixels, height);
   glClear(GL_COLOR_BUFFER_BIT);

   //Draw the background
   Vec2 rotation_vector(cos(0), sin(0));
   glBindTexture(GL_TEXTURE_2D, background_texture);
   glUniform1f(uniform_xpos, constants::cameraSize.x/2);
   glUniform1f(uniform_ypos, constants::cameraSize.y/2);
   glUniform1f(uniform_xsize, constants::cameraSize.x);
   glUniform1f(uniform_ysize, constants::cameraSize.y);
   glUniform2fv(uniform_rotation, 1, (const GLfloat*)&rotation_vector);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

std::string read_shader(std::string file_path)
{
   std::string line;

   std::string shader;

   std::ifstream shaderfile(file_path);
   if (shaderfile.is_open())
   {
      while (getline(shaderfile, line))
      {
         shader += line;
         shader += "\n";
      }
      shaderfile.close();
   }

   return shader;
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
	test_sprite.SetFile("png\\fireball_1.png");
}
Window::~Window() 
{
   if (!pImpl->window) return;

   glfwDestroyWindow(pImpl->window);
   glfwTerminate();
} 
bool Window::isOpen()
{
   return pImpl->window && !glfwWindowShouldClose(pImpl->window);
}

void Window::updateInput()
{
   if (!isOpen()) return;

   //flush stale events
   inputFlushQueue();

   //get fresh events
   glfwPollEvents();
}

void Window::startDraw()
{
   if (!isOpen()) return;
   render(pImpl->window);
}
void Window::endDraw()
{
   if (!isOpen()) return;
   glfwSwapBuffers(pImpl->window);
}
void Window::targetViewport(Vec2 min, Vec2 max)
{
   setVirtualViewport(pImpl->window, min, max);
}
void Window::clear()
{
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Window::drawSpriteStretched(float x, float y, float width, float height, int flip_horizontal, int flip_vertical, float rotation, Sprite* sprite)
{
   SetShader(prog2D);

   if (!sprite) sprite = &test_sprite;
   //int flip_horizontal = 0;
   //int flip_vertical = 1;
   float radians = degToRad(rotation);
   Vec2 rotation_vector(cos(radians), sin(radians));
   glBindTexture(GL_TEXTURE_2D, sprite->texture_handle);
   glUniform1f(uniform_xpos, x);
   glUniform1f(uniform_ypos, y);
   glUniform1f(uniform_xsize, width);
   glUniform1f(uniform_ysize, height);
   glUniform1i(uniform_flip_horz, flip_horizontal);
   glUniform1i(uniform_flip_vert, flip_vertical);
   glUniform2fv(uniform_rotation, 1, (const GLfloat*)&rotation_vector);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Window::drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation)
{
	drawSprite(x, y, flip_horizontal, flip_vertical, rotation, nullptr);
}

void Window::drawSprite(float x, float y, int flip_horizontal, int flip_vertical, float rotation, Sprite* sprite)
{
   if (!sprite) sprite = &test_sprite;
   drawSpriteStretched(x, y, sprite->width, sprite->height, flip_horizontal, flip_vertical, rotation, sprite);
}

void Window::drawSprite3D(Sprite* sprite)
{
   SetShader(prog3D);

   if (!sprite) sprite = &test_sprite;

   Vec3 position = Vec3(0.0, -0.5, 0.5);
   Vec3 target = Vec3(0.0, 0.5, 0.0);
   Vec3 up = Vec3(0.0, 0.0, 1.0);

   Mat4 camera = Mat4::camera(position, target, up);

   Mat4 project = Mat4::ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

   glUniformMatrix4fv(uniform_view, 1, false, camera.data);
   glUniformMatrix4fv(uniform_projection, 1, false, project.data);

   glBindTexture(GL_TEXTURE_2D, sprite->texture_handle);
   
   
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   //float radians = degToRad(rotation);
   //Vec2 rotation_vector(cos(radians), sin(radians));
   //glBindTexture(GL_TEXTURE_2D, sprite->texture_handle);
   //glUniform1f(uniform_xpos, x);
   //glUniform1f(uniform_ypos, y);
   //glUniform1f(uniform_xsize, width);
   //glUniform1f(uniform_ysize, height);
   //glUniform1i(uniform_flip_horz, flip_horizontal);
   //glUniform1i(uniform_flip_vert, flip_vertical);
   //glUniform2fv(uniform_rotation, 1, (const GLfloat*)&rotation_vector);
   //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

Sprite* GetSprite(std::string file_path)
{
	std::unordered_map<std::string,Sprite>::iterator it = sprite_map.find(file_path);
	if (it != sprite_map.end())
		return &it->second;
	else
	{
		auto newit = sprite_map.insert(std::make_pair(file_path, Sprite()));
		newit.first->second.SetFile(file_path);
		return &newit.first->second;
	}
}