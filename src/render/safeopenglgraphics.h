/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RENDER_SAFEOPENGLGRAPHICS_H
#define RENDER_SAFEOPENGLGRAPHICS_H

#include "main.h"
#if defined USE_OPENGL && !defined ANDROID

#include "render/graphics.h"

#include "resources/fboinfo.h"

#ifdef ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#ifndef USE_SDL2
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL_opengl.h>
#include <GL/glext.h>
#endif

class SafeOpenGLGraphics final : public Graphics
{
    public:
        SafeOpenGLGraphics();

        A_DELETE_COPY(SafeOpenGLGraphics)

        ~SafeOpenGLGraphics();

        void testDraw();

        #include "render/graphicsdef.hpp"

        #include "render/openglgraphicsdef.hpp"

        #include "render/openglgraphicsdef1.hpp"

    private:
        bool mTexture;
        bool mIsByteColor;
        Color mByteColor;
        float mFloatColor;
        bool mColorAlpha;
        FBOInfo mFbo;
};
#endif

#endif  // RENDER_SAFEOPENGLGRAPHICS_H
