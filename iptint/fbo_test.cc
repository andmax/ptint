#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>

bool openGLStatus() {
  printf("        GL Status: ");

  GLenum status = (GLenum)glGetError();
  switch(status) {
    case GL_NO_ERROR:
      printf("OK\n");
      return true;
    case GL_INVALID_ENUM:
      printf("GL_INVALID_ENUM\n");
      break;
    case GL_INVALID_VALUE:
      printf("GL_INVALID_VALUE\n");
      break;
    case GL_INVALID_OPERATION:
      printf("GL_INVALID_OPERATION\n");
      break;
    case GL_STACK_OVERFLOW:
      printf("GL_STACK_OVERFLOW\n");
      break;
    case GL_STACK_UNDERFLOW:
      printf("GL_STACK_UNDERFLOW\n");
      break;
    case GL_OUT_OF_MEMORY:
      printf("GL_OUT_OF_MEMORY\n");
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
      printf("GL_INVALID_FRAMEBUFFER_OPERATION_EXT\n");
      break;
    default:
      printf("UNKNWON\n");
  }

  return false;
}

void framebufferStatus() {
  printf("        FB Status: ");

  GLenum status = (GLenum)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      printf("<<<< OK >>>>\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n");
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      printf("GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n");
      break;
    default:
      printf("UNKNWON\n");
  }
}

void checkComponents(GLenum target, GLint wrap, GLenum format, GLint components) {
  // Allocate and bind an OpenGL texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(target, texture);

  // Set interpolation function to nearest-neighbor
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Set desired wrapping type
  glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);

  // Set texture format and component count
  glTexImage2D(target, 0, components, 16, 16, 0, format, GL_FLOAT, 0);

  // Check and show the OpenGL status; if no errors occured so far...
  if (openGLStatus()) {
    // Allocate and bind a framebuffer object
    GLuint framebuffer;
    glGenFramebuffersEXT(1, &framebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

    // Attempt to attach the texture to the framebuffer object
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, texture, 0);

    // Check and show the framebuffer status
    framebufferStatus();

    // Detach the texture from the framebuffer object
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 0, 0, 0);

    // Free the framebuffer object
    glDeleteFramebuffersEXT(1, &framebuffer);
  }

  // Free the OpenGL texture
  glDeleteTextures(1, &texture);
}

void checkFormat(GLenum target, GLint wrap, GLenum format) {
  printf("      Components: GL_FLOAT_R32_NV\n");
  checkComponents(target, wrap, format, GL_FLOAT_R32_NV);
  printf("      Components: GL_LUMINANCE_FLOAT32_ATI\n");
  checkComponents(target, wrap, format, GL_LUMINANCE_FLOAT32_ATI);
  printf("      Components: GL_LUMINANCE32F_ARB\n");
  checkComponents(target, wrap, format, GL_LUMINANCE32F_ARB);
}

void checkFormat(GLenum target, GLint wrap) {
  printf("      Components: GL_FLOAT_RGBA32_NV\n");
  checkComponents(target, wrap, GL_RGBA, GL_FLOAT_RGBA32_NV);
  printf("      Components: GL_RGBA_FLOAT32_ATI\n");
  checkComponents(target, wrap, GL_RGBA, GL_RGBA_FLOAT32_ATI);
  printf("      Components: GL_RGBA32F_ARB\n");
  checkComponents(target, wrap, GL_RGBA, GL_RGBA32F_ARB);
  printf("      Components: GL_RGBA\n");
  checkComponents(target, wrap, GL_RGBA, GL_RGBA);
}

void checkWrap(GLenum target, GLint wrap) {
  printf("    Format: GL_LUMINANCE\n");
  checkFormat(target, wrap, GL_LUMINANCE);
  printf("    Format: GL_RED\n");
  checkFormat(target, wrap, GL_RED);
  printf("    Format: GL_RGBA\n");
  checkFormat(target, wrap);
}

void checkTarget(GLenum target) {
  printf("  Wrap: GL_CLAMP\n");
  checkWrap(target, GL_CLAMP);
  printf("  Wrap: GL_CLAMP_TO_EDGE\n");
  checkWrap(target, GL_CLAMP_TO_EDGE);
}

int main(int argumentCount, char **arguments) {
  glutInit (&argumentCount, arguments);
  glutCreateWindow("");
  glewInit();

  // Clear OpenGL errors
  glGetError();

  printf("Target: GL_TEXTURE_RECTANGLE_NV\n");
  checkTarget(GL_TEXTURE_RECTANGLE_NV);
  printf("Target: GL_TEXTURE_RECTANGLE_ARB\n");
  checkTarget(GL_TEXTURE_RECTANGLE_ARB);
  printf("Target: GL_TEXTURE_2D\n");
  checkTarget(GL_TEXTURE_2D);

  return 0;
}
