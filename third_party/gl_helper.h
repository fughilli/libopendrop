#ifndef THIRD_PARTY_GL_HELPER_H_
#define THIRD_PARTY_GL_HELPER_H_

#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__

#define GL_SILENCE_DEPRECATION
#include "gl.h"
#include "glext.h"

#else  // All other platforms.

#include <GL/gl.h>
#include <GL/glext.h>

#endif

#endif  // THIRD_PARTY_GL_HELPER_H_
