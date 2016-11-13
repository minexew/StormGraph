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

#include <StormGraph/Core.hpp>
#include <StormGraph/IO/FileSystem.hpp>

#include <littl/cfx2.hpp>
#include <littl/HashMap.hpp>
#include <littl/Library.hpp>

namespace StormGraph
{
    class ICommandLine;
    class IFont;
    class IGraphicsDriver;
    class IGui;
    class IGuiDriver;
    class IHeightMap;
    class IKeyScanner;
    class IImageLoader;
    class ILineOutput;
    class IOnScreenLog;
    class IProfiler;
    class IResourceManager;
    class IScene;
    class ISceneGraph;
    class ISoundDriver;

    struct DisplayMode;
    struct LevelOfDetail;
    struct ScreenRect;

    /**
     *
     */
    class IVariable : public ReferencedClass
    {
        protected:
            ~IVariable() {}

        public:
            li_ReferencedClass_override( IVariable )

            virtual String getValue() = 0;
            virtual bool setValue( const String& value ) = 0;
    };

    /**
     *  @brief The heart of StormGraph engine.
     *
     *  This class manages the modules of StormGraph Engine and contains various common methods,
     *  including those to create other classes from the Core library.
     */
    class IEngine : public IEventListener
    {
        public:
            /**
             *  Structure representing an installed file system driver.
             *  Such driver can create virtual file systems through the protocol assigned to it.
             */
            struct RegisteredFsDriver
            {
                String protocol;
                IFileSystemDriver* driver;
            };

        public:
            virtual ~IEngine() {}

            /**
             *  @brief Create a virtual file system and add it to the default file system union.
             *
             *  @param fs file system locator
             *  @param required throw an exception if file system couldn't be opened?
             *
             *  @see createFileSystem
             *  @return true if successful
             */
            virtual bool addFileSystem( const String& fs, bool required = true ) = 0;

            /**
             *  @brief Install a virtual file system driver.
             *
             *  @param protocol the protocol this driver serves
             *  @param driver the file system driver to install. Ownership of the reference is transferred.
             */
            virtual void addFileSystemDriver( const char* protocol, IFileSystemDriver* driver ) = 0;

            /**
             *  @brief Load a string mapping table.
             *
             *  The specified file must be a valid confix2 document.
             *
             *  @param fileName path to the string table document within the default file system union
             */
            virtual void addStringTable( const char* fileName ) = 0;

            /**
             *  @brief Enqueue a scene change.
             *
             *  The actual change will happen at the begin of the following frame.
             *
             *  @param newScene the new scene; ownership of the reference is transferred.
             */
            virtual void changeScene( IScene* newScene ) = 0;

            /**
             *  @brief Execute a string command.
             *
             *  The engine will first parse the space-delimited tokens and then iterate over registered command listeners
             *  until one of them recognizes the command.
             *
             *  @param command the command string
             */
            virtual void command( const String& command ) = 0;

            virtual IVariable* createBoolVariable( bool value ) = 0;
            virtual IVariable* createBoolRefVariable( bool& value ) = 0;
            
            /**
             *  TEMPORARY (FIXME: Reimplement without depending on a GUI)\n\n
             *  Create an in-game Quake-style command line.
             *
             *  @param the GUI instance to create the command line in.
             */
            virtual ICommandLine* createCommandLine( IGui* gui ) = 0;

            /**
             *  @brief Create a virtual file system.
             *
             *  The file system locator must comprise of the protocol, followed by a colon and the actual path. (if applicable)\n
             *  The engine will iterate over registered file system drivers until one of them recognizes the protocol.
             *
             *  @param fs file system locator
             *
             *  @return pointer to the newly created file system or nullptr if no suitable driver was found
             *  @see addFileSystem
             */
            virtual IFileSystem* createFileSystem( const String& fs ) = 0;

            virtual IVariable* createIntVariable( int value ) = 0;
            virtual IKeyScanner* createKeyScanner() = 0;
            virtual IHeightMap* createHeightMap( const Vector2<unsigned>& resolution ) = 0;
            virtual IOnScreenLog* createOnScreenLog( const ScreenRect& area, IFont* font = nullptr ) = 0;
            virtual IProfiler* createProfiler() = 0;

            /**
             *  @brief Create a resource manager.
             *
             *  @param name name of the newly created resource manager
             *  @param addDefaultPath add the default empty search path?
             *  @param fileSystem the file system to be used by the resource manager. If null, the default file system union will be used.
             *      Otherwise, ownership of the reference is transferred.
             */
            virtual IResourceManager* createResourceManager( const char* name, bool addDefaultPath, IFileSystem* fileSystem = nullptr ) = 0;

            virtual ISceneGraph* createSceneGraph( const char* name ) = 0;
            virtual IVariable* createStringVariable( const char* value ) = 0;
            virtual IUnionFileSystem* createUnionFileSystem() = 0;
            virtual void executeFile( const char* fileName, IFileSystem* fileSystem = nullptr ) = 0;
            virtual void exit() = 0;

            /**
             *  @brief Retrieve a string value from application configuration.
             *
             *  The value can be either a confix2 attribute value or confix2 node text.
             *
             *  @param path cfx2 path to the value
             *  @param required throw an exception if not found?
             *
             *  @return confix2 value of the specified object or nullptr if not found
             */
            virtual const char* getConfig( const char* path, bool required = true ) = 0;

            /**
             *  @brief Retrieve a integer value from application configuration.
             *
             *  The value can be either a confix2 attribute value or confix2 node text.
             *
             *  @param path cfx2 path to the value
             *  @param required throw an exception if not found?
             *
             *  @return confix2 value of the specified object converted to integer or 0 if not found
             */
            virtual int getConfigInt( const char* path, bool required = true ) = 0;

            /**
             *  @brief Retrieve the default display mode from application configuration.
             *
             *  An exception will be thrown if one or more of the parameters cannot be found.
             *
             *  @param displayMode (output) structure receiving the default display mode
             */
            virtual void getDefaultDisplayMode( DisplayMode* displayMode ) = 0;

            /**
             *  @brief Retrieve the default level-of-detail settings from application configuration.
             *
             *  @param lod (output) structure receiving the default level-of-detail settings
             */
            virtual void getDefaultLodSettings( LevelOfDetail* lod ) = 0;

            /**
             *  @brief Get the engine build version and target.
             *
             *  Example: "TRUNK gcc4-win32"
             *
             *  @return engine build version and target
             */
            virtual String getEngineBuild() = 0;

            /**
             *  @brief Get the engine release name
             *
             *  Example: "StormGraph TRUNK gcc4-win32"
             *
             *  @return engine release name
             */
            virtual String getEngineRelease() = 0;

            /**
             *  @brief Get the default file system union.
             *
             *  @return the default file system union; reference count is NOT implicitly increased
             */
            virtual IUnionFileSystem* getFileSystem() = 0;

            virtual IGuiDriver* getGuiDriver() = 0;
            virtual IGraphicsDriver* getGraphicsDriver() = 0;
            virtual IImageLoader* getImageLoader() = 0;

            /**
             *  @brief Get a pointer to the shared resource manager.
             *
             *  Whenever possible, the client application should create its own resource manager instead of using the shared one.
             *
             *  @return the shared resource manager; reference count is NOT implicitly increased
             */
            virtual IResourceManager* getSharedResourceManager() = 0;

            virtual ISoundDriver* getSoundDriver() = 0;

            virtual String getString( const char* key ) = 0;
            virtual String getVariableValue( const char* name, bool required ) = 0;

            /**
             *  @brief List the installed file system drivers.
             *
             *  @param drivers (output) the list to append driver information to
             */
            virtual void listFileSystemDrivers( List<RegisteredFsDriver>& drivers ) = 0;

            virtual cfx2_Node* loadCfx2Asset( const char* fileName, bool required = true, IFileSystem* fileSystem = nullptr ) = 0;
            virtual String loadTextAsset( const char* fileName, bool required = true, IFileSystem* fileSystem = nullptr ) = 0;

            virtual void registerCommandListener( ICommandListener* listener ) = 0;
            virtual void registerEventListener( IEventListener* eventListener ) = 0;

            virtual void removeAllFileSystems() = 0;
            virtual void run( IScene* scene ) = 0;

            /**
             *  @brief Set the line output stream.
             *
             *  @param lineOutput Line output stream. Ownership of the reference is transferred.
             */
            virtual void setLineOutput( ILineOutput* lineOutput ) = 0;

            virtual void setProfiler( IProfiler* profiler ) = 0;

            virtual void setVariable( const char* name, IVariable* variable, bool allowOverride ) = 0;
            virtual bool setVariableValue( const char* name, const char* value ) = 0;

            virtual void startup() = 0;
            virtual void startupGraphics() = 0;

            virtual void unregisterCommandListener( ICommandListener* listener ) = 0;
            virtual void unregisterEventListener( IEventListener* eventListener ) = 0;
            virtual bool unsetVariable( const char* name ) = 0;
    };
}
