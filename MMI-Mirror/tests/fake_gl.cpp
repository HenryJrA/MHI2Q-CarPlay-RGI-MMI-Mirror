#include "fake_gl.h"

#include <string.h>

FakeGlState fake_gl;
static GLuint next_id = 1;
static const GLfloat *positions = 0;
static const GLfloat *texcoords = 0;

void fake_gl_reset() {
    memset(&fake_gl, 0, sizeof(fake_gl));
    positions = texcoords = 0;
    next_id = 1;
}

void glShaderSource(GLuint, GLsizei, const GLchar **, const GLint *) {}
void glCompileShader(GLuint) {}
void glGetShaderiv(GLuint, GLenum, GLint *value) { *value = GL_TRUE; }
void glGetShaderInfoLog(GLuint, GLsizei, GLsizei *length, GLchar *log) {
    if (length) *length = 0;
    if (log) *log = '\0';
}
GLuint glCreateShader(GLenum) { return next_id++; }
GLuint glCreateProgram() { ++fake_gl.programs_created; return next_id++; }
void glAttachShader(GLuint, GLuint) {}
void glLinkProgram(GLuint) {}
void glGetProgramiv(GLuint, GLenum, GLint *value) { *value = GL_TRUE; }
void glGetProgramInfoLog(GLuint, GLsizei, GLsizei *length, GLchar *log) {
    if (length) *length = 0;
    if (log) *log = '\0';
}
GLint glGetAttribLocation(GLuint, const GLchar *name) {
    return strcmp(name, "aPosition") == 0 ? 0 : 1;
}
GLint glGetUniformLocation(GLuint, const GLchar *name) {
    return strcmp(name, "uTexture") == 0 ? 2 : 3;
}
void glGenTextures(GLsizei count, GLuint *textures) {
    for (int i = 0; i < count; ++i) textures[i] = next_id++;
    fake_gl.textures_created += count;
}
void glBindTexture(GLenum, GLuint) {}
void glTexParameteri(GLenum, GLenum, GLint) {}
void glDisable(GLenum) {}
void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    fake_gl.clear_color[0] = r; fake_gl.clear_color[1] = g;
    fake_gl.clear_color[2] = b; fake_gl.clear_color[3] = a;
}
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    fake_gl.viewport[0] = x; fake_gl.viewport[1] = y;
    fake_gl.viewport[2] = width; fake_gl.viewport[3] = height;
}
void glPixelStorei(GLenum, GLint) {}
void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) {
    ++fake_gl.image_uploads;
}
void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) {
    ++fake_gl.subimage_uploads;
}
GLenum glGetError() { return GL_NO_ERROR; }
void glClear(GLbitfield mask) { ++fake_gl.clears; fake_gl.clear_mask = mask; }
void glUseProgram(GLuint) {}
void glActiveTexture(GLenum) {}
void glUniform1i(GLint, GLint) {}
void glUniform1f(GLint location, GLfloat value) {
    if (location == 3) fake_gl.swap_rb = value;
}
void glEnableVertexAttribArray(GLuint) {}
void glVertexAttribPointer(GLuint index, GLint, GLenum, GLboolean, GLsizei, const GLvoid *data) {
    if (index == 0) positions = static_cast<const GLfloat *>(data);
    if (index == 1) texcoords = static_cast<const GLfloat *>(data);
}
void glDrawArrays(GLenum primitive, GLint first, GLsizei count) {
    if (positions) memcpy(fake_gl.vertices, positions, sizeof(fake_gl.vertices));
    if (texcoords) memcpy(fake_gl.texcoords, texcoords, sizeof(fake_gl.texcoords));
    fake_gl.primitive = primitive; fake_gl.first_vertex = first; fake_gl.vertex_count = count;
    ++fake_gl.draws;
}
void glDisableVertexAttribArray(GLuint) {}
void glDeleteTextures(GLsizei, const GLuint *) {}
void glDeleteProgram(GLuint) {}
void glDeleteShader(GLuint) {}
