#ifndef MMI_TEST_GL2_H
#define MMI_TEST_GL2_H

/* Minimal GLES2 API used by the real renderer, implemented by fake_gl.cpp. */
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef unsigned char GLboolean;
typedef char GLchar;
typedef void GLvoid;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NO_ERROR 0
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_LINEAR 0x2601
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_DEPTH_TEST 0x0B71
#define GL_BLEND 0x0BE2
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_TEXTURE0 0x84C0
#define GL_FLOAT 0x1406
#define GL_TRIANGLE_STRIP 0x0005

void glShaderSource(GLuint, GLsizei, const GLchar **, const GLint *);
void glCompileShader(GLuint);
void glGetShaderiv(GLuint, GLenum, GLint *);
void glGetShaderInfoLog(GLuint, GLsizei, GLsizei *, GLchar *);
GLuint glCreateShader(GLenum);
GLuint glCreateProgram();
void glAttachShader(GLuint, GLuint);
void glLinkProgram(GLuint);
void glGetProgramiv(GLuint, GLenum, GLint *);
void glGetProgramInfoLog(GLuint, GLsizei, GLsizei *, GLchar *);
GLint glGetAttribLocation(GLuint, const GLchar *);
GLint glGetUniformLocation(GLuint, const GLchar *);
void glGenTextures(GLsizei, GLuint *);
void glBindTexture(GLenum, GLuint);
void glTexParameteri(GLenum, GLenum, GLint);
void glDisable(GLenum);
void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat);
void glViewport(GLint, GLint, GLsizei, GLsizei);
void glPixelStorei(GLenum, GLint);
void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLenum glGetError();
void glClear(GLbitfield);
void glUseProgram(GLuint);
void glActiveTexture(GLenum);
void glUniform1i(GLint, GLint);
void glUniform1f(GLint, GLfloat);
void glEnableVertexAttribArray(GLuint);
void glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
void glDrawArrays(GLenum, GLint, GLsizei);
void glDisableVertexAttribArray(GLuint);
void glDeleteTextures(GLsizei, const GLuint *);
void glDeleteProgram(GLuint);
void glDeleteShader(GLuint);

#endif
