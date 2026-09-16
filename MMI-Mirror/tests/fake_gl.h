#ifndef MMI_TEST_FAKE_GL_H
#define MMI_TEST_FAKE_GL_H

#include <GLES2/gl2.h>

struct FakeGlState {
    GLfloat vertices[8];
    GLfloat texcoords[8];
    GLint viewport[4];
    GLfloat clear_color[4];
    GLfloat swap_rb;
    int draws;
    int programs_created;
    int textures_created;
    int image_uploads;
    int subimage_uploads;
    int clears;
    GLbitfield clear_mask;
    GLenum primitive;
    GLint first_vertex;
    GLsizei vertex_count;
};

extern FakeGlState fake_gl;
void fake_gl_reset();

#endif
