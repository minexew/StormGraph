#pragma once

//#define StormGraph_Render_Mode StormGraph_Render_OpenGL_2_Fixed

#define StormGraph_Render_OpenGL_2_Fixed 0
#define StormGraph_Render_OpenGL_Multi 1
#define StormGraph_Render_OpenGL_3 2

#ifndef StormGraph_Render_Mode
#define StormGraph_Render_Mode StormGraph_Render_OpenGL_Multi
#endif

#define _USE_MATH_DEFINES

#include <confix2.h>
#include <littl.hpp>

#ifndef StormGraph_No_Helium
#include <Helium.hpp>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#undef main

#ifndef __StormGraph_Internal__
struct TTF_Font;
#endif

#include <StormGraph/Base.hpp>
#include <StormGraph/BasicStructs.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/IO.hpp>
#include <StormGraph/SceneGraph.hpp>
#include <StormGraph/Resource.hpp>
#include <StormGraph/Timing.hpp>

namespace StormGraph
{
    class ViewFrustum
    {
    	enum { top = 0, bottom, left, right, /*nearClip, farClip, */numPlanes };

        public:
            enum { outside, intersect, inside };

        	Plane planes[numPlanes];

        	Vector<float> ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
        	float nearD, farD, ratio, angle,tang;
        	float nw, nh, fw, fh;

        	ViewFrustum();
        	~ViewFrustum();

        	void setProjection( float angle, float ratio, float nearD, float farD );
        	void setView( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up );
        	int pointInFrustum( const Vector<float>& point );
        	int sphereInFrustum( const Vector<float>& center, float radius );
        	//int boxInFrustum( AABox &b );
    };

    class Camera
    {
        public:
            Vector<float> eye;

            static Vector<float> currentCamera, currentUp;

        public:
            static void look( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up );
            static void look( const Vector<float>& center, float dist, float angle, float angle2 );

            void moveEye( const Vector<float>& vector, bool relative = true );
            virtual void select() = 0;
    };

    class Light : public Object
    {
        protected:
            LightType type;

            Colour ambient, diffuse, specular;
            Vector<float> direction;
            float cutoffAngle, range;

        public:
            Light( LightType type, const Vector<float>& loc, const Vector<float> direction,
                    const Colour& ambient, const Colour& diffuse, float range );
            virtual ~Light();

#ifndef StormGraph_No_Helium
            static Light* create( bool directional, const Helium::VectorFixedObject* loc, const Helium::VectorFixedObject* direction,
                    const Helium::ColourFixedObject* ambient, const Helium::ColourFixedObject* diffuse, float range )
            {
                return new Light( directional ? Light_directional : Light_positional, loc,
                        direction ? direction : Vector<double>(), ambient, diffuse, range );
            }
#endif

            virtual void pick( Picking* picking );
            virtual void render();
            Light* setCutoff( float angle );
    };

    class Scene : public ReferencedClass
    {
        public:
            virtual ~Scene();

            virtual bool closeButtonAction();
            virtual void keyStateChange( unsigned short key, bool pressed, Utf8Char character );
            virtual void mouseButton( int x, int y, bool right, bool down );
            virtual void mouseMove( int x, int y );
            virtual void mouseWheel( bool down );

            virtual void render();
    };

#ifndef StormGraph_No_Helium
    class HeliumScene : public Scene
    {
        Helium::HeVM* vm;
        Helium::AutoVariable closeButtonEvent, keyStateChangeEvent, mouseButtonEvent, mouseMoveEvent, renderEvent, updateEvent;

        List<Helium::Variable> arguments;

        public:
            HeliumScene( Helium::HeVM* vm, Helium::Variable object );
            virtual ~HeliumScene();

            virtual bool closeButtonAction();
            virtual void keyStateChange( unsigned short key, bool pressed, Utf8Char character );
            virtual void mouseButton( int x, int y, bool right, bool down );
            virtual void mouseMove( int x, int y );

            virtual void render();
    };
#endif

    class FpsCamera : public Camera
    {
        Vector<float> center;

        public:
            void moveCenter( const Vector<float>& vector, bool relative = true );
            virtual void select();
    };

    class TopDownCamera : public Camera
    {
        public:
            virtual void select();
    };

    class Math
    {
        public:
            template <class Result, class Unit> static Result angleToY( const Vector<Unit>& origin, const Vector<Unit>& target )
            {
                return ( Result )( -atan2( ( double )( origin.y - target.y ), ( double )( origin.x - target.x ) ) + M_PI );
            }

#ifndef StormGraph_No_Helium
            double angleToY( const Helium::VectorFixedObject* origin, const Helium::VectorFixedObject* target ) const
            {
                return angleToY<double, double>( origin, target );
            }
#endif

            bool boxesCollide( const Vector<float>& min1, const Vector<float>& max1, const Vector<float>& min2, const Vector<float>& max2 );
            bool liesInBox( const Vector<float>& point, const Vector<float>& min, const Vector<float>& max );
    };

    class Engine
    {
        static Engine* instance;
        static GraphicsDriver* driver;





        cfx2_Node* configDoc;
        SDL_Surface* display;

        String appName;
        Scene* scene;

        FpsMeter fps;

        bool isRunning;

        Stack<BlendMode> blendModeStack;
        Program* currentShader;
        Vector<unsigned short> window;

        void* cursor;

        ViewFrustum frustum;
        unsigned nextLightId;

        List<FileSystem*> fileSystems;

        friend class Camera;
        friend class Light;
        friend class Material;
        friend class Program;

        public:
            Engine( const String& appName );
            ~Engine();

            void addFileSystem( const String& fs );
            static void assertionFail( const String& sourceUnit, int line, const String& className, const String& methodName, const String& assertion, const String& desc );

#ifndef StormGraph_No_Helium
            Helium::Variable initMembers( Helium::Variable obj );
#endif

            void changeScene( Scene* newScene );
#ifndef StormGraph_No_Helium
            void changeScene( Helium::HeVM* vm, Helium::Variable newScene );
#endif
            //void detachShader();
            void disableDepthTesting();
            void drawLine( const Vector<float>& begin, const Vector<float>& end, const Colour& colour );
            void drawRect( const Vector<float>& begin, const Vector<float>& end, const Colour& colour );

            void enableDepthTesting();

#ifndef StormGraph_No_Helium
            void exception( const char* className, const char* methodName, const char* title, const char* description );
#endif

            const char* getConfig( const char* path, bool essential = true );
            int getConfigInt( const char* path );
            Vector<unsigned short> getDisplayMode() const { return Vector<unsigned short>( window.x, window.y, 32 ); }
            static Engine* getInstance() { return instance; }
            static unsigned short getKey( const char* name );
            static String getKeyName( unsigned short key );
            unsigned getLightId() { return nextLightId++; }
            double getTimeDelta() const { return fps.getLastMicros() / 1000000.f; }

            int isPointVisible( const Vector<float>& point );
        	int isSphereVisible( const Vector<float>& center, float radius );

#ifndef StormGraph_No_Helium
            Helium::Variable loadCfx2( const char* fileName, bool essential = true );
#endif
            cfx2_Node* loadCfx2Asset( const char* fileName, bool essential = true );
            GraphicsDriverProvider loadGraphicsDriver( const char* name );
            void loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned count );
#ifndef StormGraph_No_Helium
            Helium::Variable loadKeyBindings( const char* fileName );
#endif
            String loadTextAsset( const char* fileName, bool essential = true );

            static void logEvent( const char* className, const char* event );
            SeekableInputStream* open( const char* fileName );
            static void printEventLog( OutputStream* output );
            Engine* popBlendMode();
            Engine* pushBlendMode( BlendMode bm );

            void run( Scene* scene );

#ifndef StormGraph_No_Helium
            void run( Helium::HeVM* vm, Helium::Variable scene );
#endif

            void setColour( const Colour& colour );
            void setCursor( bool enable, const char* fileName );
            Engine* setMode( const char* windowTitle, unsigned width = 0, unsigned height = 0, int fullscreen = -1 );
            Engine* setOrthoProjection( double near = -1.0, double far = 1.0 );
            Engine* setPerspectiveProjection( double near = 1.0, double far = 1000.0 );
            void setPickingId( unsigned id );
            void setSceneAmbient( const Colour& colour );
    };

    class Picking
    {
        Program* pickingProgram;

        unsigned nextId, idAt;

        public:
            Picking();
            virtual ~Picking();

            void begin();
            void end( unsigned x, unsigned y );

            unsigned generateId();
            unsigned getId() const;
    };
}
