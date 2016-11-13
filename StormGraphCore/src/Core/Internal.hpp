/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#pragma once

#include <StormGraph/Abstract.hpp>

namespace StormGraph
{
    class ICommandLine;
    class IEngine;
    class IFileSystem;
    class IFileSystemDriver;
    class IFont;
    class IGui;
    class IHeightMap;
    class IImageLoader;
    class IKeyScanner;
    class IOnScreenLog;
    class IProfiler;
    class IResourceManager;
    class ISceneGraph;
    class ISoundDriver;
    class IUnionFileSystem;

    struct ScreenRect;

    ICommandLine* createCommandLine( IEngine* engine, IGui* gui );
    IEngine* createEngine( const char* app, int argc, char** argv );
    IHeightMap* createHeightMap( IEngine* engine, const Vector2<unsigned>& resolution );
    IImageLoader* createImageLoader( IEngine* engine );
    IKeyScanner* createKeyScanner( IEngine* engine );
    IOnScreenLog* createOnScreenLog( IEngine* engine, const ScreenRect& area, IFont* font );
    IProfiler* createProfiler( IEngine* engine );
    IResourceManager* createResourceManager( IEngine* engine, const char* name, bool addDefaultPath, IFileSystem* fileSystem );
    ISceneGraph* createSceneGraph( IEngine* engine, const char* name );
    ISoundDriver* createSoundDriver( IEngine* engine );

    // File Systems
    IFileSystemDriver* createMoxFileSystemDriver();
    IFileSystemDriver* createNativeFileSystemDriver();
    IFileSystemDriver* createUnionFileSystemDriver();

    IUnionFileSystem* createUnionFileSystem();
}

namespace StormBase
{
    class ISys;

    ISys* createSys(const char* app, int argc, char** argv);
}
