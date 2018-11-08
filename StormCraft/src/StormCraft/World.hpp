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

#include <StormGraph/IO/BspGenerator.hpp>
#include <StormGraph/IO/Ctree2Generator.hpp>
#include <StormGraph/Engine.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/HeightMap.hpp>
#include <StormGraph/Image.hpp>
#include <StormGraph/Scene.hpp>

#include <littl/cfx2.hpp>

#include <wx/treectrl.h>

#define DRAG_THRESHOLD 0.25f

class LightMapping;
class PolygonWn;
class Resources;
class StormCraftFrame;
class World;
class WorldNode;

using namespace StormGraph;

struct Property
{
    enum Type { boolean, colour, enumeration, floating, lightMapArea, text, vector, vector2f, vector2i } type;
    String name;

    bool boolValue;
    String textValue;
    Colour colourValue;
    double floatingValue;
    Vector<> vectorValue;
    Vector2<> vector2fValue;
    Vector2<int> vector2iValue;
    List<String> enumValues;
    int enumValue;

    Property() : enumValue( 0 )
    {
    }

    Property( const char* name, Type type )
            : type( type ), name( name ), enumValue( 0 )
    {
    }

    Property( const char* name, bool boolValue )
            : type( boolean ), name( name ), boolValue( boolValue )
    {
    }

    Property( const char* name, double floatingValue )
            : type( floating ), name( name ), floatingValue( floatingValue )
    {
    }

    Property( const char* name, const String& textValue, Type type = text )
            : type( type ), name( name ), textValue( textValue )
    {
    }

    Property( const char* name, const Colour& colourValue )
            : type( colour ), name( name ), colourValue( colourValue )
    {
    }

    Property( const char* name, const Vector<>& vectorValue )
            : type( vector ), name( name ), vectorValue( vectorValue )
    {
    }

    Property( const char* name, const Vector2<>& vector2fValue )
            : type( vector2f ), name( name ), vector2fValue( vector2fValue )
    {
    }

    Property( const char* name, const Vector2<int>& vector2iValue )
            : type( vector2i ), name( name ), vector2iValue( vector2iValue )
    {
    }
};

class WorldNode
{
    public:
        wxTreeItemId itemId;

    public:
        World* world;
        Resources* res;

        String className, name;

        bool selected;
        unsigned pickingId;

        bool getMaterial( const String& materialName, MaterialStaticProperties* properties );
        IMaterial* getRenderMaterial( const String& materialName );
        void renderBoundingBox( const Vector<>& pos, const Vector<>& size );

    public:
        WorldNode( World* world, Resources* res, const char* className, const char* name = nullptr );
        virtual ~WorldNode();

        const String& getClassName() const { return className; }
        const String& getName() const { return name; }

        virtual void breakIntoFaces( List<PolygonWn*>& faces ) {}
        virtual void buildCtree2( List<Ct2Line>& lines ) {}
        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) {}
        virtual bool canBreakIntoFaces( size_t& numFaces );
        virtual WorldNode* clone();
        virtual void drag( Vector<float>& vec ) {}
        virtual bool getCenter( Vector<>& center ) { return false; }
        virtual void getProperties( List<Property>& properties ) = 0;
        void load( cfx2::Node& node );
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) {}
        virtual void onSelect( StormCraftFrame* frame ) {}
        virtual void rotate( const Vector<>& vector, float angle ) {}
        virtual cfx2::Node save( cfx2::Node& parent );
        virtual void setProperties( List<Property>& properties );
        virtual bool setProperty( Property& property );
        virtual void startup() {}
        virtual void render( StormCraftFrame* frame, bool isPicking ) = 0;
};

class GroupWorldNode : public WorldNode
{
    public:
        List<WorldNode*> children;

    public:
        GroupWorldNode( World* world, Resources* res, const char* className = "Group", const char* name = nullptr );
        GroupWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~GroupWorldNode();

        void add( WorldNode* child );
        void remove( WorldNode* child );

        virtual void buildCtree2( List<Ct2Line>& lines ) override;
        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) override;
        virtual WorldNode* clone() override;
        virtual void drag( Vector<float>& vec ) override;
        virtual bool getCenter( Vector<>& center ) override;
        virtual void getProperties( List<Property>& properties );
        void load( cfx2::Node& node );
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void onSelect( StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual cfx2::Node save( cfx2::Node& parent );
        virtual void startup() override;
        virtual bool setProperty( Property& property ) override;
};

class LitWorldNode
{
    public:
        int lightingMode;
        String lightMapArea;

    public:
        LitWorldNode();
        LitWorldNode( cfx2::Node node );
        virtual ~LitWorldNode();

        virtual void getProperties( List<Property>& properties );
        virtual bool setProperty( Property& property );
        void setUpLighting( World* world, MaterialStaticProperties* materialProperties, LightMapping* lightMapping );
};

class RootWorldNode : public GroupWorldNode
{
    public:
        struct Viewport
        {
            bool perspective;
            float fov;
            bool culling, wireframe, shaded;
        };

    public:
        GroupWorldNode* editorView;

        Colour sceneAmbient;

        bool displayGroundPlane;
        Vector<> placeObjectsAt;

        Camera camera[4];
        Viewport viewports[4];

        Reference<IModel> grid;

        static Camera frontCamera() { return Camera( Vector<>( 0.0f, 10.0f, 0.0f ), Vector<>(), Vector<>( 0.0f, 0.0f, 1.0f ) ); }
        static Camera leftCamera() { return Camera( Vector<>( -10.0f, 0.0f, 0.0f ), Vector<>(), Vector<>( 0.0f, 0.0f, 1.0f ) ); }
        static Camera topCamera() { return Camera( Vector<>( 0.0f, 0.0f, 10.0f ), Vector<>(), Vector<>( 0.0f, -1.0f, 0.0f ) ); }

    public:
        RootWorldNode( World* world, Resources* res, const char* name );
        RootWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~RootWorldNode();

        Ct2Node* buildCtree2();
        BspTree* buildGeometry();
        void getCameraProperties( List<Property>& properties );
        virtual void getProperties( List<Property>& properties );
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual void startup() override;
        virtual bool setProperty( Property& property ) override;
};

class CuboidWorldNode : public WorldNode, public LitWorldNode
{
    protected:
        Vector<> anchor, size;
        bool wireframe;
        String material;
        int visibility;
        bool front, back, left, right, top, bottom;
        bool ctree2Solid;

        bool needRebuild;
        Reference<IModel> model;

        void rebuild();

    public:
        CuboidWorldNode( World* world, Resources* res, const char* name = nullptr );
        CuboidWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~CuboidWorldNode();

        virtual void breakIntoFaces( List<PolygonWn*>& faces ) override;
        virtual void buildCtree2( List<Ct2Line>& lines ) override;
        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) override;
        virtual bool canBreakIntoFaces( size_t& numFaces ) override;
        virtual void drag( Vector<float>& vec ) override;
        //void generateFaces( BspPolygon polygons[6], unsigned materialIndex );
        virtual void getProperties( List<Property>& properties );
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual bool setProperty( Property& property ) override;
        virtual void startup();
};

class FaceWorldNode : public WorldNode, public LitWorldNode
{
    public:
        String material;

        Vector<> a, b, c, d;
        Vector<> na, nb, nc, nd;
        Vector2<> uva, uvb, uvc, uvd;

        bool needRebuild;
        Reference<IModel> model;

        void rebuild();

    public:
        FaceWorldNode( World* world, Resources* res, const char* name = nullptr );
        FaceWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~FaceWorldNode();

        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) override;
        virtual void drag( Vector<float>& vec ) override;
        virtual void getProperties( List<Property>& properties ) override;
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual bool setProperty( Property& property ) override;
};

class LightWorldNode : public WorldNode
{
    public:
        bool enabled;
        Vector<> position, direction;
        int type;
        Colour ambient, diffuse;
        float range;

        float fov;
        bool cubeShadowMapping;
        unsigned shadowMapDetail;

    public:
        LightWorldNode( World* world, Resources* res, const char* name = nullptr );
        LightWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~LightWorldNode();

        virtual void drag( Vector<float>& vec ) override;
        virtual void getProperties( List<Property>& properties );
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual bool setProperty( Property& property ) override;
};

class PolygonWn : public WorldNode, public LitWorldNode
{
    public:
        String materialName;
        bool generateNormals;
        StormGraph::Polygon polygon;
        bool ctree2Solid;

        bool needRebuild;
        Reference<IModel> model;
        Vector<> displacement, minimum, maximum;
        unsigned pickingIds[Polygon::MAX_VERTICES];
        int dragVertex;

        void rebuild();

    public:
        PolygonWn( World* world, Resources* res, const char* name = nullptr );
        PolygonWn( World* world, Resources* res, cfx2::Node node );
        virtual ~PolygonWn();

        virtual void buildCtree2( List<Ct2Line>& lines ) override;
        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) override;
        virtual void drag( Vector<float>& vec ) override;
        virtual bool getCenter( Vector<>& center ) override;
        virtual void getProperties( List<Property>& properties ) override;
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void onSelect( StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual void rotate( const Vector<>& vector, float angle ) override;
        virtual bool setProperty( Property& property ) override;
};

class StaticMeshWn : public WorldNode, public LitWorldNode
{
    public:
        Vector<> position;
        String modelName;

        Reference<IModel> model;

    public:
        StaticMeshWn( World* world, Resources* res, const char* name = nullptr );
        StaticMeshWn( World* world, Resources* res, cfx2::Node node );
        virtual ~StaticMeshWn();

        virtual void getProperties( List<Property>& properties ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual bool setProperty( Property& property ) override;
};

class TerrainWorldNode : public WorldNode, public LitWorldNode
{
    protected:
        Vector<> anchor, size;
        Vector2<int> heightMapResolution;
        bool wireframe;
        String material;

        bool needRebuild;
        Object<IHeightMap> heightMap;
        Reference<IModel> terrain;

        List<unsigned> pickingIds;
        Vector<unsigned> selectedPoint;

        void rebuild();

    public:
        TerrainWorldNode( World* world, Resources* res, const char* name = nullptr );
        TerrainWorldNode( World* world, Resources* res, cfx2::Node node );
        virtual ~TerrainWorldNode();

        virtual void buildGeometry( Bsp* bsp, List<BspPolygon>& polygons ) override;
        virtual void drag( Vector<float>& vec );
        virtual void getProperties( List<Property>& properties );
        virtual void onPickingFinished( unsigned id, StormCraftFrame* frame ) override;
        virtual void render( StormCraftFrame* frame, bool isPicking ) override;
        virtual bool setProperty( Property& property ) override;
        virtual void startup();
};

class AabbCollisionWn : public WorldNode
{
};

struct Resources
{
    struct
    {
        Reference<ITexture> texture;
        Reference<IMaterial> material;
    }
    welcome;

    Reference<IResourceManager> resMgr;

    Reference<IMaterial> diamondMaterial;
    Reference<IModel> boundingBox, diamond, lightbulb, gizmo[6], pointBox;
    Reference<ILight> worldLight;

    Reference<IFont> font;
    Object<IGui> gui;

    Reference<IPopupMenu> viewMenu;
    unsigned topCmd, frontCmd, leftCmd, perspectiveTgl, cullingTgl, wireframeTgl, shadedTgl;
};

class World
{
    public:
        struct Fs
        {
            String name;
            bool cloneInPackage;
        };

        struct LightMap
        {
            String name;
            Image* image;
        };

        struct LightMapDesc
        {
            String name;
            Vector2<unsigned> resolution;
            Image::StorageFormat format;
        };

    public:
        // World File Name
        String fileName;

        // Export Settings
        struct
        {
            String fileName;
            bool exportGeometry, cloneFileSystems;
            int bspPolygonLimit;
            Vector<float> bspVolumeLimit;
            bool generateCtree2;
            int ctree2SegLimit;
        }
        exportSettings;

        // File Systems
        List<Fs> fileSystems;

        // Light Maps
        List<LightMapDesc> lightMapDescs;
        List<LightMap> lightMaps;       // used during export

        // Dynamic Resources - Base
        Reference<IUnionFileSystem> vfs;
        Reference<IResourceManager> resMgr;

        // Dynamic Resources - cfx2
        cfx2::Document mapInfoDoc;

        // Dynamic Resources - World Tree
        Object<RootWorldNode> rootNode;
        WorldNode* gizmoObject;

        // Scratch variables
        String tmpDir;

        const Vector<> getObjectPlacePosition() const { return rootNode->placeObjectsAt; }
};

struct LightMapping
{
    World::LightMap* lightMap;
    Vector2<float> uv[2];
    Vector2<unsigned> coords[2];
};
