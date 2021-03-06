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

#ifndef RESOURCES_IMAGEHELPER_H
#define RESOURCES_IMAGEHELPER_H

#include "localconsts.h"

#include "render/rendertype.h"

#include "resources/resource.h"

#include <SDL_video.h>

class Dye;
class Image;

/**
 * Defines a class for loading and storing images.
 */
class ImageHelper notfinal
{
    friend class CompoundSprite;
    friend class Image;

    public:
        A_DELETE_COPY(ImageHelper)

        virtual ~ImageHelper()
        { }

        /**
         * Loads an image from an SDL_RWops structure.
         *
         * @param rw         The SDL_RWops to load the image from.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        Image *load(SDL_RWops *const rw) A_WARN_UNUSED;

        virtual Image *load(SDL_RWops *const rw, Dye const &dye) A_WARN_UNUSED;

#ifdef __GNUC__
        virtual Image *load(SDL_Surface *const) A_WARN_UNUSED = 0;

        virtual Image *createTextSurface(SDL_Surface *const tmpImage,
                                         const int width, const int height,
                                         float alpha) A_WARN_UNUSED = 0;
#else
        virtual Image *load(SDL_Surface *) A_WARN_UNUSED
        { return nullptr; }

        virtual Image *createTextSurface(SDL_Surface *const tmpImage,
                                         const float alpha) const A_WARN_UNUSED
        { return nullptr; }
#endif

        virtual SDL_Surface *create32BitSurface(int width,
                                                int height)
                                                const A_WARN_UNUSED;

        virtual void copySurfaceToImage(Image *const image A_UNUSED,
                                        const int x A_UNUSED,
                                        const int y A_UNUSED,
                                        SDL_Surface *const surface A_UNUSED)
                                        const
        { }

        static SDL_Surface *convertTo32Bit(SDL_Surface *const tmpImage)
                                           A_WARN_UNUSED;

        static void dumpSurfaceFormat(const SDL_Surface *const image);

        static void setEnableAlpha(const bool n)
        { mEnableAlpha = n; }

        static SDL_Surface *loadPng(SDL_RWops *const rw);

        static void setOpenGlMode(const RenderType useOpenGL)
        { mUseOpenGL = useOpenGL; }

        virtual RenderType useOpenGL() const A_WARN_UNUSED
        { return mUseOpenGL; }

        virtual void postInit()
        { }

    protected:
        ImageHelper()
        { }

        static bool mEnableAlpha;
        static RenderType mUseOpenGL;
};

extern ImageHelper *imageHelper;
extern ImageHelper *surfaceImageHelper;
#endif  // RESOURCES_IMAGEHELPER_H
