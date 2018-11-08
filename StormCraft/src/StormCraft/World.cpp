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

#include "StormCraftMain.hpp"

#include "World.hpp"

#include <StormGraph/IO/Bsp.hpp>
#include <StormGraph/IO/ImageWriter.hpp>
#include <StormGraph/GeometryFactory.hpp>

#define LIGHTMAP_BORDER 1

#define property_( name_ ) properties.add( Property( #name_, name_ ) );

#define boolProperty_( name_ ) else if ( property.name == #name_ && property.type == Property::boolean )\
    {\
        name_ = property.boolValue;\
    }

#define boolPropertyRebuild( name_ ) else if ( property.name == #name_ && property.type == Property::boolean )\
    {\
        name_ = property.boolValue;\
        needRebuild = true;\
    }

static const Colour selectionBlend { 1.0f, 0.75f, 0.75f, 1.0f };

class Lighting
{
    protected:
        static void calculateInNode( Colour& sum, const Vector<>& pos, const Vector<>& normal, GroupWorldNode* group )
        {
            iterate2 ( child, group->children )
            {
                LightWorldNode* light = dynamic_cast<LightWorldNode*>( *child );

                if ( light != nullptr && light->enabled )
                {
                    if ( light->type == 0 )
                    {
                        const Vector<> ray( light->position - pos );
                        float diffuseAmount = maximum( 0.0f, ray.normalize().dotProduct( normal ) );

                        float attenuation =  light->range / ( 2.0f * ray.getLength() + 0.001f );

                        sum += ( light->ambient + diffuseAmount * light->diffuse ) * attenuation;
                    }
                    else if ( light->type == 1 )
                    {
                        float diffuseAmount = maximum( 0.0f, ( -light->direction ).dotProduct( normal ) );

                        sum += light->ambient + diffuseAmount * light->diffuse;
                    }
                }

                GroupWorldNode* group = dynamic_cast<GroupWorldNode*>( *child );

                if ( group != nullptr )
                    calculateInNode( sum, pos, normal, group );
            }
        }

    public:
        static Colour calculateAt( const Vector<>& pos, const Vector<>& normal, World* world )
        {
            Colour sum = world->rootNode->sceneAmbient;

            calculateInNode( sum, pos, normal, world->rootNode );
            sum.a = 1.0f;

            return sum;
        }
};

/*static IMaterial* getMaterial( World* world, const String& name, const String& materialName )
{
    if ( materialName.isEmpty() )
    {
        Engine::logEvent( name, "<font color=red>Cannot build geometry for an object without any material</font>" );
        return nullptr;
    }

    try
    {
        return world->resMgr->getMaterial( materialName );
    }
    catch ( Exception& ex )
    {
        Engine::logEvent( name, "<font color=red>Failed to load material <b>" + materialName +  "</b> (<i>" + ex.getName() + "</i>)</font>" );
        return nullptr;
    }
}*/

static IModel* getRenderModel( World* world, const String& name, const String& modelName )
{
    Reference<IModel> model;

    if ( !modelName.isEmpty() )
    {
        try
        {
            model = world->resMgr->getModel( modelName );
        }
        catch ( Exception& ex )
        {
            Common::logEvent( name, "<font color=red>Failed to load model <b>" + modelName +  "</b> (<i>" + ex.getName() + "</i>)</font>" );
        }
    }

    return model.detach();
}

template <typename T>
static T sample( const T points[2][2], const Vector2<float>& uv )
{
    if ( !( uv < 0.0f ) && !( uv > 1.0f ) )
    {
        const Vector2<float> uv0 = uv.floor(), uv1 = uv.ceil();

        if ( uv0 == uv1 )
        {
            // Woohoo! Exact hit!

            return points[( unsigned ) uv0.x][( unsigned ) uv0.y];
        }
        else if ( uv0.x == uv1.x )
        {
            // X is integer, linear-interpolate Y

            T uv0sample = points[( unsigned ) uv0.x][0];
            T uv1sample = points[( unsigned ) uv1.x][1];

            return uv0sample + ( uv1sample - uv0sample ) * ( uv.y - uv0.y );
        }
        else if ( uv0.y == uv1.y )
        {
            // Y is integer, linear-interpolate X

            T uv0sample = points[0][( unsigned ) uv0.y];
            T uv1sample = points[1][( unsigned ) uv1.y];

            return uv0sample + ( uv1sample - uv0sample ) * ( uv.x - uv0.x );
        }
    }

    return points[0][0] * ( 1 - uv.x ) * ( 1 - uv.y ) + points[1][0] * uv.x * ( 1 - uv.y ) + points[0][1] * ( 1 - uv.x ) * uv.y + points[1][1] * uv.x * uv.y;
}

WorldNode::WorldNode( World* world, Resources* res, const char* className, const char* name )
        : world( world ), res( res ), className( className ), name( name ), selected( false )
{
    if ( name == nullptr )
        this->name = "Unnamed_" + this->className;
}

WorldNode::~WorldNode()
{
}

bool WorldNode::canBreakIntoFaces( size_t& numFaces )
{
    return false;
}

WorldNode* WorldNode::clone()
{
    WorldNode* duplicate = nullptr;

    if ( className == "Cuboid" )
        duplicate = new CuboidWorldNode( world, res, name );
    else if ( className == "Face" )
        duplicate = new FaceWorldNode( world, res, name );
    else if ( className == "Light" )
        duplicate = new LightWorldNode( world, res, name );
    else if ( className == "Polygon" )
        duplicate = new PolygonWn( world, res, name );
    else if ( className == "Terrain" )
        duplicate = new TerrainWorldNode( world, res, name );

    if ( duplicate != nullptr )
    {
        List<Property> properties;

        getProperties( properties );
        duplicate->setProperties( properties );
    }

    return duplicate;
}

bool WorldNode::getMaterial( const String& materialName, MaterialStaticProperties* properties )
{
    if ( materialName.isEmpty() )
    {
        Common::logEvent( name, "<font color=red>Cannot build geometry for an object without any material</font>" );
        return false;
    }

    try
    {
        world->resMgr->parseMaterial( materialName, properties );
        return true;
    }
    catch ( Exception& ex )
    {
        Common::logEvent( name, "<font color=red>Failed to load material <b>" + materialName +  "</b> (<i>" + ex.getName() + "</i>)</font>" );
        return false;
    }
}

IMaterial* WorldNode::getRenderMaterial( const String& materialName )
{
    Reference<IMaterial> material;

    if ( !materialName.isEmpty() )
    {
        try
        {
            material = world->resMgr->getMaterial( materialName, true );
        }
        catch ( Exception& ex )
        {
            Common::logEvent( name, "<font color=red>Failed to load material <b>" + materialName +  "</b> (<i>" + ex.getName() + "</i>)</font>" );
        }
    }

    if ( material == nullptr )
        material = graphicsDriver->getSolidMaterial();

    return material.detach();
}

void WorldNode::getProperties( List<Property>& properties )
{
    properties.add( Property( "name", name ) );
}

void WorldNode::load( cfx2::Node& node )
{
    List<Property> properties;

    getProperties( properties );

    iterate ( properties )
    {
        Property& property = properties.current();
        String attrib = node.getAttrib( property.name );

        switch ( property.type )
        {
            case Property::boolean:
                property.boolValue = attrib.toBool();
                break;

            case Property::colour:
                property.colourValue = Colour( attrib );
                break;

            case Property::enumeration:
                property.enumValue = attrib.toInt();
                break;

            case Property::floating:
                property.floatingValue = attrib.toFloat();
                break;

            case Property::lightMapArea:
            case Property::text:
                property.textValue = attrib;
                break;

            case Property::vector:
                property.vectorValue = Vector<>( attrib );
                break;

            case Property::vector2f:
                property.vector2fValue = Vector2<>( attrib );
                break;

            case Property::vector2i:
                property.vector2iValue = Vector2<int>( attrib );
                break;
        }

        setProperty( property );
    }
}

void WorldNode::renderBoundingBox( const Vector<>& pos, const Vector<>& size )
{
    Transform transforms[2];

    transforms[0] = Transform( Transform::scale, size );
    transforms[1] = Transform( Transform::translate, pos );

    graphicsDriver->setRenderFlag( RenderFlag::depthTest, false );
    res->boundingBox->render( transforms, 2 );
    graphicsDriver->setRenderFlag( RenderFlag::depthTest, true );
}

cfx2::Node WorldNode::save( cfx2::Node& parent )
{
    cfx2::Node node = parent.createChild( className );

    List<Property> properties;

    getProperties( properties );

    iterate ( properties )
    {
        const Property& property = properties.current();

        switch ( property.type )
        {
            case Property::boolean:
                node.setAttrib( property.name, String::formatBool( property.boolValue ) );
                break;

            case Property::colour:
                node.setAttrib( property.name, property.colourValue.toString() );
                break;

            case Property::enumeration:
                node.setAttrib( property.name, String::formatInt( property.enumValue ) );
                break;

            case Property::floating:
                node.setAttrib( property.name, String::formatFloat( property.floatingValue ) );
                break;

            case Property::lightMapArea:
            case Property::text:
                node.setAttrib( property.name, property.textValue );
                break;

            case Property::vector:
                node.setAttrib( property.name, property.vectorValue.toString() );
                break;

            case Property::vector2i:
                node.setAttrib( property.name, property.vector2iValue.toString() );
                break;

            case Property::vector2f:
                node.setAttrib( property.name, property.vector2fValue.toString() );
                break;
        }
    }

    return node;
}

void WorldNode::setProperties( List<Property>& properties )
{
    iterate2 ( i, properties )
        setProperty( *i );
}

bool WorldNode::setProperty( Property& property )
{
    if ( property.name == "name" && property.type == Property::text )
        name = property.textValue;
    else
        return false;

    return true;
}

GroupWorldNode::GroupWorldNode( World* world, Resources* res, const char* className, const char* name )
        : WorldNode( world, res, className, name )
{
}

GroupWorldNode::GroupWorldNode( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, node.getName() )
{
    load( node );
}

GroupWorldNode::~GroupWorldNode()
{
    iterate ( children )
        delete children.current();
}

void GroupWorldNode::add( WorldNode* child )
{
    children.add( child );
}

void GroupWorldNode::buildCtree2( List<Ct2Line>& lines )
{
    iterate2 ( child, children )
        child->buildCtree2( lines );
}

void GroupWorldNode::buildGeometry( Bsp* bsp, List<BspPolygon>& polygons )
{
    iterate2 ( child, children )
        child->buildGeometry( bsp, polygons );
}

WorldNode* GroupWorldNode::clone()
{
    GroupWorldNode* duplicate = new GroupWorldNode( world, res, name );

    iterate2 ( i, children )
        duplicate->children.add( i->clone() );

    return duplicate;
}

void GroupWorldNode::drag( Vector<float>& vec )
{
    iterate2 ( i, children )
        i->drag( vec );
}

bool GroupWorldNode::getCenter( Vector<>& center )
{
    size_t count = 0;

    iterate2 ( i, children )
    {
        Vector<> c;

        if ( i->getCenter( c ) )
        {
            center += c;
            count++;
        }
    }

    if ( count > 0 )
    {
        center /= count;
        return true;
    }
    else
        return false;
}

void GroupWorldNode::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );
}

void GroupWorldNode::load( cfx2::Node& node )
{
    WorldNode::load( node );

    iterate ( node )
    {
        const String& type = node.current().getName();

        WorldNode* child = nullptr;

        if ( type == "Group" )
            child = new GroupWorldNode( world, res, node.current() );
        else if ( type == "Cuboid" )
            child = new CuboidWorldNode( world, res, node.current() );
        else if ( type == "Face" )
            child = new FaceWorldNode( world, res, node.current() );
        else if ( type == "Light" )
            child = new LightWorldNode( world, res, node.current() );
        else if ( type == "Polygon" )
            child = new PolygonWn( world, res, node.current() );
        else if ( type == "Terrain" )
            child = new TerrainWorldNode( world, res, node.current() );

        if ( child != nullptr )
            add( child );
    }
}

void GroupWorldNode::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    iterate ( children )
        children.current()->onPickingFinished( id, frame );
}

void GroupWorldNode::onSelect( StormCraftFrame* frame )
{
    Vector<> center;

    if ( getCenter( center ) )
    {
        bool visible[3] = { true, true, true };

        frame->showGizmo( center, this, visible );
    }
}

void GroupWorldNode::remove( WorldNode* child )
{
    children.removeItem( child );
}

cfx2::Node GroupWorldNode::save( cfx2::Node& parent )
{
    cfx2::Node node = WorldNode::save( parent );

    iterate ( children )
        children.current()->save( node );

    return node;
}

void GroupWorldNode::startup()
{
    iterate ( children )
        children.current()->startup();
}

bool GroupWorldNode::setProperty( Property& property )
{
    return WorldNode::setProperty( property );
}

void GroupWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    iterate ( children )
        children.current()->render( frame, isPicking );
}

LitWorldNode::LitWorldNode() : lightingMode( 0 )
{
}

LitWorldNode::LitWorldNode( cfx2::Node node )
{
}

LitWorldNode::~LitWorldNode()
{
}

void LitWorldNode::getProperties( List<Property>& properties )
{
    Property lightingModeProperty( "lightingMode", Property::enumeration );
    lightingModeProperty.enumValues.add( "None" );
    lightingModeProperty.enumValues.add( "Lightmap" );
    lightingModeProperty.enumValues.add( "Dynamic" );
    lightingModeProperty.enumValue = lightingMode;

    properties.add( lightingModeProperty );
    properties.add( Property( "lightMap", lightMapArea, Property::lightMapArea ) );
}

bool LitWorldNode::setProperty( Property& property )
{
    if ( property.name == "lightingMode" && property.type == Property::enumeration )
        lightingMode = property.enumValue;
    else if ( property.name == "lightMap" && property.type == Property::lightMapArea )
        lightMapArea = property.textValue;
    else
        return false;

    return true;
}

void LitWorldNode::setUpLighting( World* world, MaterialStaticProperties* materialProperties, LightMapping* lightMapping )
{
    lightMapping->lightMap = nullptr;

    switch ( lightingMode )
    {
        case 0:
            materialProperties->dynamicLighting = false;
            break;

        case 1:
        {
            int lparen = lightMapArea.findChar( '(' );

            if ( lparen < 0 )
            {
                Common::logEvent( "LitWorldNode", "<font color=red>Malformed lightmap area '" + lightMapArea + "'</font>" );
                break;
            }

            int semicolon = lightMapArea.findChar( ';', lparen + 1 );

            if ( semicolon < 0 )
            {
                Common::logEvent( "LitWorldNode", "<font color=red>Malformed lightmap area '" + lightMapArea + "'</font>" );
                break;
            }

            String lightMapName = lightMapArea.left( lparen );

            iterate2 ( i, world->lightMaps )
            {
                World::LightMap& lightMap = i;

                if ( lightMap.name == lightMapName )
                {
                    lightMapping->lightMap = &lightMap;
                    break;
                }
            }

            if ( lightMapping->lightMap == nullptr )
            {
                Common::logEvent( "LitWorldNode", "<font color=red>Invalid lightmap '" + lightMapName + "'</font>" );
                break;
            }

            lightMapping->uv[0] = lightMapArea.left( semicolon ).dropLeft( lparen + 1 );
            lightMapping->uv[1] = lightMapArea.dropLeft( semicolon + 1 );

            lightMapping->coords[0] = ( lightMapping->uv[0] * lightMapping->lightMap->image->size.getXy() ).floor();
            lightMapping->coords[1] = ( lightMapping->uv[1] * lightMapping->lightMap->image->size.getXy() ).ceil();

            materialProperties->dynamicLighting = false;
            materialProperties->lightMapping = true;
            materialProperties->lightMapName = "_LIGHTMAPS/" + lightMapName;

            break;
        }

        default:
            break;
    }
}

RootWorldNode::RootWorldNode( World* world, Resources* res, const char* name )
        : GroupWorldNode( world, res, "Root", name ), sceneAmbient( Colour::white() ), displayGroundPlane( true )
{
    viewports[0].perspective = true;
    viewports[1].perspective = false;
    viewports[2].perspective = false;
    viewports[3].perspective = false;

    for ( size_t i = 0; i < lengthof( viewports ); i++ )
    {
        viewports[i].fov = 45.0f;
        viewports[i].culling = true;
        viewports[i].wireframe = false;
        viewports[i].shaded = true;
    }

    camera[0] = Camera( Vector<>(), 15.0f, M_PI * 1.5f, M_PI_4 );
    camera[1] = topCamera();
    camera[2] = frontCamera();
    camera[3] = leftCamera();
}

RootWorldNode::RootWorldNode( World* world, Resources* res, cfx2::Node node )
        : GroupWorldNode( world, res, "Root", nullptr )
{
    GroupWorldNode::load( node );
}

RootWorldNode::~RootWorldNode()
{
}

Ct2Node* RootWorldNode::buildCtree2()
{
    List<Ct2Line> lines;

    Object<Ctree2Generator> ct2Gen = Ctree2Generator::create( maximum( world->exportSettings.ctree2SegLimit, 1 ) );

    iterate2 ( child, children )
        child->buildCtree2( lines );

    if ( !lines.isEmpty() )
        return ct2Gen->generate( lines.getPtr(), lines.getLength() );
    else
    {
        Common::logEvent( "RootWorldNode.buildCtree2", "<font color=red>Warning: No collision data to export</font>" );
        return nullptr;
    }
}

BspTree* RootWorldNode::buildGeometry()
{
    List<BspPolygon> polygons;

    Object<BspGenerator> bsp = new BspGenerator( maximum( world->exportSettings.bspPolygonLimit, 1 ), world->exportSettings.bspVolumeLimit );

    iterate2 ( child, children )
        child->buildGeometry( bsp, polygons );

    return bsp->generate( polygons.getPtr(), polygons.getLength() );
}

void RootWorldNode::getCameraProperties( List<Property>& properties )
{
    for ( size_t i = 0; i < lengthof( viewports ); i++ )
    {
        String index = "[" + String::formatInt( i ) + "]";

        properties.add( Property( "viewportPerspective" + index, viewports[i].perspective ) );
        properties.add( Property( "viewportFov" + index, viewports[i].fov ) );
        properties.add( Property( "viewportCulling" + index, viewports[i].culling ) );
        properties.add( Property( "viewportWireframe" + index, viewports[i].wireframe ) );
        properties.add( Property( "viewportShaded" + index, viewports[i].shaded ) );
        properties.add( Property( "cameraEye" + index, camera[i].eye ) );
        properties.add( Property( "cameraCenter" + index, camera[i].center ) );
        properties.add( Property( "cameraUp" + index, camera[i].up ) );
    }
}

void RootWorldNode::getProperties( List<Property>& properties )
{
    GroupWorldNode::getProperties( properties );

    properties.add( Property( "sceneAmbient", sceneAmbient ) );
    properties.add( Property( "displayGroundPlane", displayGroundPlane ) );
    properties.add( Property( "placeObjectsAt", placeObjectsAt ) );
    getCameraProperties( properties );
}

void RootWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    graphicsDriver->setRenderFlag( RenderFlag::depthTest, true );

    if ( !isPicking )
    {
        if ( displayGroundPlane )
            grid->render();
    }

    GroupWorldNode::render( frame, isPicking );
}

bool RootWorldNode::setProperty( Property& property )
{
    do
    {
        if ( property.name == "sceneAmbient" && property.type == Property::colour )
            sceneAmbient = property.colourValue;
        else if ( property.name == "displayGroundPlane" && property.type == Property::boolean )
            displayGroundPlane = property.boolValue;
        else if ( property.name == "placeObjectsAt" && property.type == Property::vector )
            placeObjectsAt = property.vectorValue;
        else
            break;

        return true;
    }
    while ( false );

    for ( size_t i = 0; i < lengthof( viewports ); i++ )
    {
        String index = "[" + String::formatInt( i ) + "]";

        if ( property.name == "viewportPerspective" + index && property.type == Property::boolean )
            viewports[i].perspective = property.boolValue;
        else if ( property.name == "viewportFov" + index && property.type == Property::floating )
            viewports[i].fov = property.floatingValue;
        else if ( property.name == "viewportCulling" + index && property.type == Property::boolean )
            viewports[i].culling = property.boolValue;
        else if ( property.name == "viewportWireframe" + index && property.type == Property::boolean )
            viewports[i].wireframe = property.boolValue;
        else if ( property.name == "viewportShaded" + index && property.type == Property::boolean )
            viewports[i].shaded = property.boolValue;
        else if ( property.name == "cameraEye" + index && property.type == Property::vector )
            camera[i].eye = property.vectorValue;
        else if ( property.name == "cameraCenter" + index && property.type == Property::vector )
            camera[i].center = property.vectorValue;
        else if ( property.name == "cameraUp" + index && property.type == Property::vector )
            camera[i].up = property.vectorValue;
        else
            continue;

        return true;
    }

    return GroupWorldNode::setProperty( property );
}

void RootWorldNode::startup()
{
    MaterialProperties2 gridMaterial;
    memset( &gridMaterial, 0, sizeof( gridMaterial ) );

    gridMaterial.colour = Colour::grey( 0.5 );

    PlaneCreationInfo plane( Vector2<>( 10.0f, 10.0f ), Vector<>( 5.0f, 5.0f ), Vector2<>(), Vector2<>(), false, false,
            graphicsDriver->createMaterial( "gridMaterial", &gridMaterial, true ) );

    grid = graphicsDriver->createPlane( "grid", &plane );

    GroupWorldNode::startup();
}

CuboidWorldNode::CuboidWorldNode( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "Cuboid", name ), anchor( world->getObjectPlacePosition() ),
        size( 1.0f, 1.0f, 1.0f ), wireframe( false ), visibility( 0 ), front( true ), back( true ), left( true ), right( true ), top( true ), bottom( true ),
        ctree2Solid( false ), needRebuild( false )
{
}

CuboidWorldNode::CuboidWorldNode( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "Cuboid" ),
        size( 1.0f, 1.0f, 1.0f ), wireframe( false ), visibility( 0 ), front( true ), back( true ), left( true ), right( true ), top( true ), bottom( true ),
        ctree2Solid( false ), needRebuild( false )
{
    load( node );
}

CuboidWorldNode::~CuboidWorldNode()
{
}

void CuboidWorldNode::breakIntoFaces( List<PolygonWn*>& faces )
{
    CuboidCreationInfo2 creationInfo( anchor, size, Vector<>( size.x / 2, size.y / 2 ), true, 1, false, visibility == 0 ? ShapeVisibility::outside : ShapeVisibility::inside );
    CuboidBspProperties bspProperties( 0 );
    BspPolygon sides[6];

    creationInfo.front = front;
    creationInfo.back = back;
    creationInfo.left = left;
    creationInfo.right = right;
    creationInfo.top = top;
    creationInfo.bottom = bottom;

    unsigned numFaces = GeometryFactory::createCuboid( creationInfo, bspProperties, sides );

    for ( unsigned i = 0; i < numFaces; i++ )
    {
        auto polygon = new PolygonWn( world, res, name + "_" + String::formatInt( i ) );

        polygon->materialName = material;
        polygon->polygon.numVertices = 4;

        for ( int j = 0; j < 4; j++ )
            polygon->polygon.v[j] = sides[i].v[j];

        polygon->ctree2Solid = ctree2Solid;

        faces.add( polygon );
    }
}

void CuboidWorldNode::buildCtree2( List<Ct2Line>& lines )
{
    if ( !ctree2Solid )
        return;

    const Vector2<> a = anchor.getXy() + Vector2<>( -size.x / 2, -size.y / 2 );
    const Vector2<> b = anchor.getXy() + Vector2<>( -size.x / 2, size.y / 2 );
    const Vector2<> c = anchor.getXy() + Vector2<>( size.x / 2, size.y / 2 );
    const Vector2<> d = anchor.getXy() + Vector2<>( size.x / 2, -size.y / 2 );

    if ( front )
        lines.add( Ct2Line { b, c } );

    if ( back )
        lines.add( Ct2Line { a, d } );

    if ( left )
        lines.add( Ct2Line { a, b } );

    if ( right )
        lines.add( Ct2Line { c, d } );
}

void CuboidWorldNode::buildGeometry( Bsp* bsp, List<BspPolygon>& polygons )
{
    Object<MaterialStaticProperties> materialProperties = new MaterialStaticProperties;

    if ( !getMaterial( material, materialProperties ) )
        return;

    //String lightMapName = setUpLighting( world, materialProperties );

    unsigned materialIndex = bsp->registerMaterial( name + ".material", materialProperties.detach() );

    CuboidCreationInfo2 creationInfo( anchor, size, Vector<>( size.x / 2, size.y / 2 ), true, 1, false, visibility == 0 ? ShapeVisibility::outside : ShapeVisibility::inside );
    CuboidBspProperties bspProperties( materialIndex );
    BspPolygon sides[6];

    creationInfo.front = front;
    creationInfo.back = back;
    creationInfo.left = left;
    creationInfo.right = right;
    creationInfo.top = top;
    creationInfo.bottom = bottom;

    unsigned numFaces = GeometryFactory::createCuboid( creationInfo, bspProperties, sides );

    for ( unsigned i = 0; i < numFaces; i++ )
        polygons.add( sides[i] );
}

bool CuboidWorldNode::canBreakIntoFaces( size_t& numFaces )
{
    numFaces = front + back + left + right + top + bottom;

    return true;
}

void CuboidWorldNode::drag( Vector<float>& vec )
{
    anchor += vec;
}

void CuboidWorldNode::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    Property visibilityProperty( "visibility", Property::enumeration );
    visibilityProperty.enumValues.add( "From Outside" );
    visibilityProperty.enumValues.add( "From Inside" );
    visibilityProperty.enumValue = visibility;

    properties.add( Property( "anchor", anchor ) );
    properties.add( Property( "size", size ) );
    properties.add( Property( "wireframe", wireframe ) );
    properties.add( Property( "material", material ) );
    properties.add( visibilityProperty );
    properties.add( Property( "front", front ) );
    properties.add( Property( "back", back ) );
    properties.add( Property( "left", left ) );
    properties.add( Property( "right", right ) );
    properties.add( Property( "top", top ) );
    properties.add( Property( "bottom", bottom ) );
    property_( ctree2Solid )

    LitWorldNode::getProperties( properties );
}

void CuboidWorldNode::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    if ( id == pickingId )
    {
        bool visible[3] = { true, true, true };

        frame->showGizmo( anchor, this, visible );
    }
}

void CuboidWorldNode::rebuild()
{
    Reference<IMaterial> cuboidMaterial = getRenderMaterial( material );

    //CuboidCreationInfo cuboid( size, Vector<>( size.x / 2, size.y / 2 ), false, true, wireframe, cuboidMaterial->reference(), visibility == 1 );

    //model = graphicsDriver->createCuboid( name + ".model", &cuboid );

    CuboidCreationInfo2 creationInfo( Vector<>(), size, Vector<>( size.x / 2, size.y / 2 ), true, 1, wireframe, visibility == 0 ? ShapeVisibility::outside : ShapeVisibility::inside );
    //List<Vertex> vertices;
    //List<uint32_t> indices;

    creationInfo.front = front;
    creationInfo.back = back;
    creationInfo.left = left;
    creationInfo.right = right;
    creationInfo.top = top;
    creationInfo.bottom = bottom;

    //GeometryFactory::createCuboidTriangles( creationInfo, vertices, indices );

    //MeshCreationInfo2 meshCreationInfo { String(), MeshFormat::triangleList, MeshLayout::indexed, cuboidMaterial->reference(),
    //        vertices.getLength(), indices.getLength(), vertices.getPtr(), indices.getPtr() };

    //model = graphicsDriver->createModelFromMemory( name + ".model", &meshCreationInfo, 1, IModel::fullStatic );

    model = graphicsDriver->createCuboid( name + ".model", creationInfo, cuboidMaterial->reference() );

    needRebuild = false;
}

void CuboidWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    if ( needRebuild )
        rebuild();

    Transform transforms[1];

    transforms[0] = Transform( Transform::translate, anchor );

    if ( !isPicking )
    {
        if ( world->gizmoObject == this )
        {
            model->render( transforms, 1, selectionBlend );
            renderBoundingBox( anchor + Vector<>( 0.0f, 0.0f, size.z / 2 ), size );
        }
        else
            model->render( transforms, 1 );
    }
    else
        pickingId = model->pick( transforms, 1 );
}

bool CuboidWorldNode::setProperty( Property& property )
{
    if ( property.name == "anchor" && property.type == Property::vector )
        anchor = property.vectorValue;
    else if ( property.name == "size" && property.type == Property::vector )
    {
        size = property.vectorValue;
        needRebuild = true;
    }
    boolPropertyRebuild( wireframe )
    else if ( property.name == "material" && property.type == Property::text )
    {
        material = property.textValue;
        needRebuild = true;
    }
    else if ( property.name == "visibility" && property.type == Property::enumeration )
    {
        visibility = property.enumValue;
        needRebuild = true;
    }
    boolPropertyRebuild( front )
    boolPropertyRebuild( back )
    boolPropertyRebuild( left )
    boolPropertyRebuild( right )
    boolPropertyRebuild( top )
    boolPropertyRebuild( bottom )
    boolProperty_( ctree2Solid )
    else if ( !LitWorldNode::setProperty( property ) )
        return WorldNode::setProperty( property );

    return true;
}

void CuboidWorldNode::startup()
{
    rebuild();
}

FaceWorldNode::FaceWorldNode( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "Face", name ), needRebuild( true )
{
}

FaceWorldNode::FaceWorldNode( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "Face" ), needRebuild( true )
{
    load( node );
}

FaceWorldNode::~FaceWorldNode()
{
}

void FaceWorldNode::buildGeometry( Bsp* bsp, List<BspPolygon>& polygons )
{
    Object<MaterialStaticProperties> materialProperties = new MaterialStaticProperties;

    if ( !getMaterial( material, materialProperties ) )
        return;

    LightMapping lightMapping;

    setUpLighting( world, materialProperties, &lightMapping );

    unsigned materialIndex = bsp->registerMaterial( name + ".material", materialProperties.detach() );

    BspPolygon polygon;

    polygon.numVertices = 4;
    polygon.materialIndex = materialIndex;

    polygon.v[0].pos = a;
    polygon.v[0].normal = na;
    polygon.v[0].uv[0] = uva;

    polygon.v[1].pos = b;
    polygon.v[1].normal = nb;
    polygon.v[1].uv[0] = uvb;

    polygon.v[2].pos = c;
    polygon.v[2].normal = nc;
    polygon.v[2].uv[0] = uvc;

    polygon.v[3].pos = d;
    polygon.v[3].normal = nd;
    polygon.v[3].uv[0] = uvd;

    if ( lightMapping.lightMap != nullptr )
    {
        Image* lightMapImage = lightMapping.lightMap->image;

        SG_assert( lightMapImage->format == Image::Format::rgb )

        polygon.v[0].lightUv = lightMapping.uv[0];
        polygon.v[1].lightUv = Vector2<>( lightMapping.uv[0].x, lightMapping.uv[1].y );
        polygon.v[2].lightUv = lightMapping.uv[1];
        polygon.v[3].lightUv = Vector2<>( lightMapping.uv[1].x, lightMapping.uv[0].y );

        const Vector2<> range( lightMapping.coords[1].x - lightMapping.coords[0].x, lightMapping.coords[1].y - lightMapping.coords[0].y );

        Vector<> posSamples[2][2], normalSamples[2][2];

        posSamples[0][0] = a;
        posSamples[0][1] = b;
        posSamples[1][1] = c;
        posSamples[1][0] = d;

        normalSamples[0][0] = na;
        normalSamples[0][1] = nb;
        normalSamples[1][1] = nc;
        normalSamples[1][0] = nd;

        for ( unsigned y = maximum<int>( lightMapping.coords[0].y - LIGHTMAP_BORDER, 0 ); y < lightMapping.coords[1].y + LIGHTMAP_BORDER && y < lightMapImage->size.y; y++ )
        {
            unsigned x = maximum<int>( lightMapping.coords[0].x - LIGHTMAP_BORDER, 0 );
            size_t index = ( y * lightMapImage->size.x + x ) * 3;

            for ( ; x < lightMapping.coords[1].x + LIGHTMAP_BORDER && x < lightMapImage->size.x; x++ )
            {
                float u = ( int )( x - lightMapping.coords[0].x ) / range.x;
                float v = ( int )( y - lightMapping.coords[0].y ) / range.y;

                const Vector<> normal = sample( normalSamples, Vector2<>( u, v ) );
                const Vector<> pos = sample( posSamples, Vector2<>( u, v ) );

                Colour luxel = Lighting::calculateAt( pos, normal, world );

                lightMapImage->data[index++] = luxel.getR();
                lightMapImage->data[index++] = luxel.getG();
                lightMapImage->data[index++] = luxel.getB();
            }
        }
    }

    polygons.add( polygon );
}

void FaceWorldNode::drag( Vector<float>& vec )
{
    // TODO: add displacement property instead

    a += vec;
    b += vec;
    c += vec;
    d += vec;

    needRebuild = true;
}

void FaceWorldNode::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    properties.add( Property( "material", material ) );
    properties.add( Property( "a", a ) );
    properties.add( Property( "b", b ) );
    properties.add( Property( "c", c ) );
    properties.add( Property( "d", d ) );
    properties.add( Property( "na", na ) );
    properties.add( Property( "nb", nb ) );
    properties.add( Property( "nc", nc ) );
    properties.add( Property( "nd", nd ) );
    properties.add( Property( "uva", uva ) );
    properties.add( Property( "uvb", uvb ) );
    properties.add( Property( "uvc", uvc ) );
    properties.add( Property( "uvd", uvd ) );

    LitWorldNode::getProperties( properties );
}

void FaceWorldNode::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    if ( id == pickingId )
    {
        bool visible[3] = { true, true, true };

        frame->showGizmo( ( a + b + c + d ) / 4.0f, this, visible );
    }
}

void FaceWorldNode::rebuild()
{
    Reference<IMaterial> faceMaterial = getRenderMaterial( material );

    const float coords[] = { a.x, a.y, a.z, b.x, b.y, b.z, d.x, d.y, d.z, d.x, d.y, d.z, b.x, b.y, b.z, c.x, c.y, c.z };
    const float normals[] = { na.x, na.y, na.z, nb.x, nb.y, nb.z, nd.x, nd.y, nd.z, nd.x, nd.y, nd.z, nb.x, nb.y, nb.z, nc.x, nc.y, nc.z };
    const float uvs[] = { uva.x, uva.y, uvb.x, uvb.y, uvd.x, uvd.y, uvd.x, uvd.y, uvb.x, uvb.y, uvc.x, uvc.y };

    MeshCreationInfo3 mesh { MeshFormat::triangleList, MeshLayout::linear, faceMaterial->reference(), 6, 0, coords, normals, { uvs }, nullptr, nullptr };
    model = graphicsDriver->createModelFromMemory( name + ".model", &mesh, 1 );

    needRebuild = false;
}

void FaceWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    if ( needRebuild )
        rebuild();

    if ( !isPicking )
    {
        if ( world->gizmoObject == this )
        {
            Transform pointTransforms[2];
            pointTransforms[0] = Transform( Transform::scale, frame->getCameraScale() );
            pointTransforms[1].operation = Transform::translate;

            const Vector<> minimum = a.minimum( b.minimum( c.minimum( d ) ) );
            const Vector<> maximum = a.maximum( b.maximum( c.maximum( d ) ) );

            model->render( nullptr, 0, selectionBlend );
            renderBoundingBox( ( minimum + maximum ) / 2, maximum - minimum );

            pointTransforms[1].vector = a;
            frame->res->pointBox->render( pointTransforms, lengthof( pointTransforms ) );

            pointTransforms[1].vector = b;
            frame->res->pointBox->render( pointTransforms, lengthof( pointTransforms ) );

            pointTransforms[1].vector = c;
            frame->res->pointBox->render( pointTransforms, lengthof( pointTransforms ) );

            pointTransforms[1].vector = d;
            frame->res->pointBox->render( pointTransforms, lengthof( pointTransforms ) );
        }
        else
            model->render();
    }
    else
        pickingId = model->pick();
}

bool FaceWorldNode::setProperty( Property& property )
{
    if ( property.name == "material" && property.type == Property::text )
    {
        material = property.textValue;
        needRebuild = true;
    }
    else if ( property.name == "a" && property.type == Property::vector )
    {
        a = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "b" && property.type == Property::vector )
    {
        b = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "c" && property.type == Property::vector )
    {
        c = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "d" && property.type == Property::vector )
    {
        d = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "na" && property.type == Property::vector )
    {
        na = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "nb" && property.type == Property::vector )
    {
        nb = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "nc" && property.type == Property::vector )
    {
        nc = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "nd" && property.type == Property::vector )
    {
        nd = property.vectorValue;
        needRebuild = true;
    }
    else if ( property.name == "uva" && property.type == Property::vector2f )
    {
        uva = property.vector2fValue;
        needRebuild = true;
    }
    else if ( property.name == "uvb" && property.type == Property::vector2f )
    {
        uvb = property.vector2fValue;
        needRebuild = true;
    }
    else if ( property.name == "uvc" && property.type == Property::vector2f )
    {
        uvc = property.vector2fValue;
        needRebuild = true;
    }
    else if ( property.name == "uvd" && property.type == Property::vector2f )
    {
        uvd = property.vector2fValue;
        needRebuild = true;
    }
    else if ( !LitWorldNode::setProperty( property ) )
        return WorldNode::setProperty( property );

    return true;
}

LightWorldNode::LightWorldNode( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "Light", name ), enabled( true ), position( world->getObjectPlacePosition() ), type( 0 ), range( 5.0f ), fov( 90.0f ),
        cubeShadowMapping( false ), shadowMapDetail( 256 )
{
}

LightWorldNode::LightWorldNode( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "Light" ), enabled( true ), type( 0 ), range( 5.0f ), fov( 90.0f ), cubeShadowMapping( false ), shadowMapDetail( 256 )
{
    load( node );
}

LightWorldNode::~LightWorldNode()
{
}

void LightWorldNode::drag( Vector<float>& vec )
{
    position += vec;
}

void LightWorldNode::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    property_( enabled )

    Property typeProperty( "type", Property::enumeration );
    typeProperty.enumValues.add( "Positional" );
    typeProperty.enumValues.add( "Directional" );
    typeProperty.enumValue = type;

    properties.add( Property( "position", position ) );
    properties.add( Property( "direction", direction ) );
    properties.add( typeProperty );
    properties.add( Property( "ambient", ambient ) );
    properties.add( Property( "diffuse", diffuse ) );
    properties.add( Property( "range", range ) );
    properties.add( Property( "fov", fov ) );
    properties.add( Property( "shadowMapDetail", ( float ) shadowMapDetail ) );
}

void LightWorldNode::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    if ( id == pickingId )
    {
        bool visible[3] = { true, true, true };

        frame->showGizmo( position, this, visible );
    }
}

void LightWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    Transform transforms[1];

    transforms[0] = Transform( Transform::translate, position );

    if ( !isPicking )
    {
        if ( world->gizmoObject == this )
            frame->res->lightbulb->render( transforms, 1, selectionBlend );
        else
            frame->res->lightbulb->render( transforms, 1 );
    }
    else
        pickingId = frame->res->lightbulb->pick( transforms, 1 );
}

bool LightWorldNode::setProperty( Property& property )
{
    if (0);
    boolProperty_( enabled )
    else if ( property.name == "position" && property.type == Property::vector )
        position = property.vectorValue;
    else if ( property.name == "direction" && property.type == Property::vector )
        direction = property.vectorValue.normalize();
    else if ( property.name == "type" && property.type == Property::enumeration )
        type = property.enumValue;
    else if ( property.name == "ambient" && property.type == Property::colour )
        ambient = property.colourValue;
    else if ( property.name == "diffuse" && property.type == Property::colour )
        diffuse = property.colourValue;
    else if ( property.name == "range" && property.type == Property::floating )
        range = property.floatingValue;
    else if ( property.name == "fov" && property.type == Property::floating )
        fov = property.floatingValue;
    else if ( property.name == "shadowMapDetail" && property.type == Property::floating )
        shadowMapDetail = maximum<int>( 2, ( int ) property.floatingValue );
    else
        return WorldNode::setProperty( property );

    return true;
}

PolygonWn::PolygonWn( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "Polygon", name ), generateNormals( false ), ctree2Solid( false ), needRebuild( true )
{
    polygon.numVertices = 3;

    dragVertex = -1;
}

PolygonWn::PolygonWn( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "Polygon" ), generateNormals( false ), ctree2Solid( false ), needRebuild( true )
{
    polygon.numVertices = Polygon::MAX_VERTICES;

    load( node );

    dragVertex = -1;
}

PolygonWn::~PolygonWn()
{
}

void PolygonWn::buildCtree2( List<Ct2Line>& lines )
{
    if ( !ctree2Solid || polygon.numVertices < 2 )
        return;

    Vector2<> a = polygon.v[0].pos.getXy();

    for ( size_t i = 1; i < polygon.numVertices; i++ )
    {
        Vector2<> b = polygon.v[1].pos.getXy();

        if ( !a.equals( b, 0.01f ) )
        {
            lines.add( Ct2Line { a, b } );
            break;
        }
    }
}

void PolygonWn::buildGeometry( Bsp* bsp, List<BspPolygon>& polygons )
{
    Object<MaterialStaticProperties> materialProperties = new MaterialStaticProperties;

    if ( !getMaterial( materialName, materialProperties ) )
        return;

    LightMapping lightMapping;

    setUpLighting( world, materialProperties, &lightMapping );

    unsigned materialIndex = bsp->registerMaterial( name + ".material", materialProperties.detach() );

    BspPolygon bspPoly( polygon, materialIndex );

    /*if ( lightMapping.lightMap != nullptr )
    {
        Image* lightMapImage = lightMapping.lightMap->image;

        SG_assert( lightMapImage->format == Image::Format::rgb )

        polygon.v[0].lightUv = lightMapping.uv[0];
        polygon.v[1].lightUv = Vector2<>( lightMapping.uv[0].x, lightMapping.uv[1].y );
        polygon.v[2].lightUv = lightMapping.uv[1];
        polygon.v[3].lightUv = Vector2<>( lightMapping.uv[1].x, lightMapping.uv[0].y );

        const Vector2<> range( lightMapping.coords[1].x - lightMapping.coords[0].x, lightMapping.coords[1].y - lightMapping.coords[0].y );

        Vector<> posSamples[2][2], normalSamples[2][2];

        posSamples[0][0] = a;
        posSamples[0][1] = b;
        posSamples[1][1] = c;
        posSamples[1][0] = d;

        normalSamples[0][0] = na;
        normalSamples[0][1] = nb;
        normalSamples[1][1] = nc;
        normalSamples[1][0] = nd;

        for ( unsigned y = maximum<int>( lightMapping.coords[0].y - LIGHTMAP_BORDER, 0 ); y < lightMapping.coords[1].y + LIGHTMAP_BORDER && y < lightMapImage->size.y; y++ )
        {
            unsigned x = maximum<int>( lightMapping.coords[0].x - LIGHTMAP_BORDER, 0 );
            size_t index = ( y * lightMapImage->size.x + x ) * 3;

            for ( ; x < lightMapping.coords[1].x + LIGHTMAP_BORDER && x < lightMapImage->size.x; x++ )
            {
                float u = ( int )( x - lightMapping.coords[0].x ) / range.x;
                float v = ( int )( y - lightMapping.coords[0].y ) / range.y;

                const Vector<> normal = sample( normalSamples, Vector2<>( u, v ) );
                const Vector<> pos = sample( posSamples, Vector2<>( u, v ) );

                Colour luxel = Lighting::calculateAt( pos, normal, world );

                lightMapImage->data[index++] = luxel.getR();
                lightMapImage->data[index++] = luxel.getG();
                lightMapImage->data[index++] = luxel.getB();
            }
        }
    }*/

    polygons.add( bspPoly );
}

void PolygonWn::drag( Vector<float>& vec )
{
    if ( dragVertex < 0 )
    {
        for ( unsigned i = 0; i < polygon.numVertices; i++ )
            polygon.v[i].pos += vec;

        displacement += vec;
    }
    else
    {
        polygon.v[dragVertex].pos += vec;
        needRebuild = true;
    }
}

bool PolygonWn::getCenter( Vector<>& center )
{
    if ( polygon.numVertices > 0 )
    {
        for ( unsigned i = 0; i < polygon.numVertices; i++ )
            center += polygon.v[i].pos;

        center /= polygon.numVertices;
        return true;
    }
    else
        return false;
}

void PolygonWn::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    properties.add( Property( "material", materialName ) );
    property_( generateNormals )
    properties.add( Property( "numVertices", ( float ) polygon.numVertices ) );

    for ( unsigned i = 0; i < polygon.numVertices; i++ )
    {
        String index = "[" + String::formatInt( i ) + "]";

        properties.add( Property( "pos" + index, polygon.v[i].pos ) );
        properties.add( Property( "normal" + index, polygon.v[i].normal ) );
        properties.add( Property( "uv" + index + "[0]", polygon.v[i].uv[0] ) );
    }

    property_( ctree2Solid )

    LitWorldNode::getProperties( properties );
}

void PolygonWn::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    for ( unsigned i = 0; pickingIds[0] != 0 && i < polygon.numVertices; i++ )
        if ( pickingIds[i] == id )
        {
            bool visible[3] = { true, true, true };

            dragVertex = i;
            frame->showGizmo( polygon.v[i].pos, this, visible );

            return;
        }

    if ( id == pickingId )
        onSelect( frame );
}

void PolygonWn::onSelect( StormCraftFrame* frame )
{
    Vector<> center;

    if ( getCenter( center ) )
    {
        bool visible[3] = { true, true, true };

        dragVertex = -1;
        frame->showGizmo( center, this, visible );
    }
}

void PolygonWn::rebuild()
{
    polygon.numVertices = li::minimum( li::maximum<unsigned>( polygon.numVertices, 3 ), Polygon::MAX_VERTICES );

    Reference<IMaterial> material = getRenderMaterial( materialName );

    if ( generateNormals )
        for ( unsigned i = 0; i < polygon.numVertices; i++ )
        {
            unsigned index1 = ( i > 0 ) ? ( i - 1 ) : ( polygon.numVertices - 1 );
            unsigned index2 = ( i + 1 < polygon.numVertices ) ? ( i + 1 ) : 0;

            Vector<> pos1 = polygon.v[index1].pos - polygon.v[i].pos;
            Vector<> pos2 = polygon.v[index2].pos - polygon.v[i].pos;

            polygon.v[i].normal = pos1.crossProduct( pos2 ).normalize();
            //polygon.v[i].normal = pos1.normalize().crossProduct( pos2.normalize() );
        }

    List<uint32_t> indices;
    polygon.breakIntoTriangles( indices );
    MeshCreationInfo2 mesh = { String(), MeshFormat::triangleList, MeshLayout::indexed, material.detach(), polygon.numVertices, indices.getLength(), polygon.v, indices.getPtr() };
    model = graphicsDriver->createModelFromMemory( name + ".model", &mesh, 1 );

    displacement = Vector<>();
    minimum = maximum = polygon.v[0].pos;

    for ( unsigned i = 1; i < polygon.numVertices; i++ )
    {
        maximum = maximum.maximum( polygon.v[i].pos );
        minimum = minimum.minimum( polygon.v[i].pos );
    }

    needRebuild = false;
}

void PolygonWn::render( StormCraftFrame* frame, bool isPicking )
{
    if ( needRebuild )
        rebuild();

    Transform transform( Transform::translate, displacement );

    if ( !isPicking )
    {
        if ( world->gizmoObject == this )
        {
            model->render( &transform, 1, selectionBlend );
            renderBoundingBox( displacement + ( minimum + maximum ) / 2, maximum - minimum );
        }
        else
            model->render( &transform, 1 );
    }
    else
    {
        pickingId = model->pick( &transform, 1 );
        pickingIds[0] = 0;
    }

    if ( world->gizmoObject == this )
    {
        Transform pointTransforms[2];
        pointTransforms[0] = Transform( Transform::scale, frame->getCameraScale() );
        pointTransforms[1] = Transform( Transform::translate, Vector<>() );

        for ( unsigned i = 0; i < polygon.numVertices; i++ )
        {
            pointTransforms[1].vector = polygon.v[i].pos;

            if ( !isPicking )
                frame->res->pointBox->render( pointTransforms, lengthof( pointTransforms ) );
            else
                pickingIds[i] = frame->res->pointBox->pick( pointTransforms, lengthof( pointTransforms ) );
        }
    }
}

void PolygonWn::rotate( const Vector<>& vector, float angle )
{
    Vector<> center;

    if ( getCenter( center ) )
    {
        for ( unsigned i = 0; i < polygon.numVertices; i++ )
            polygon.v[i].pos = ( polygon.v[i].pos - center ).rotate( vector, angle ) + center;

        needRebuild = true;
    }
}

bool PolygonWn::setProperty( Property& property )
{
    do
    {
        if ( property.name == "material" && property.type == Property::text )
        {
            materialName = property.textValue;
            needRebuild = true;
        }
        boolPropertyRebuild( generateNormals )
        else if ( property.name == "numVertices" && property.type == Property::floating )
        {
            polygon.numVertices = li::minimum<unsigned>( li::maximum<int>( property.floatingValue, 3 ), Polygon::MAX_VERTICES );
            property.floatingValue = polygon.numVertices;
            needRebuild = true;
        }
        boolProperty_( ctree2Solid )
        else
            break;

        return true;
    }
    while ( false );

    for ( size_t i = 0; i < polygon.numVertices; i++ )
    {
        String index = "[" + String::formatInt( i ) + "]";

        if ( property.name == "pos" + index && property.type == Property::vector )
            polygon.v[i].pos = property.vectorValue;
        else if ( property.name == "normal" + index && property.type == Property::vector )
            polygon.v[i].normal = property.vectorValue;
        else if ( property.name == "uv" + index + "[0]" && property.type == Property::vector2f )
            polygon.v[i].uv[0] = property.vector2fValue;
        else
            continue;

        needRebuild = true;
        return true;
    }

    if ( !LitWorldNode::setProperty( property ) )
        return WorldNode::setProperty( property );

    return true;
}

StaticMeshWn::StaticMeshWn( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "StaticMesh", name ), position( world->getObjectPlacePosition() )
{
}

StaticMeshWn::StaticMeshWn( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "StaticMesh" )
{
    load( node );
}

StaticMeshWn::~StaticMeshWn()
{
}

void StaticMeshWn::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    properties.add( Property( "position", position ) );
    properties.add( Property( "model", modelName ) );

    LitWorldNode::getProperties( properties );
}

void StaticMeshWn::render( StormCraftFrame* frame, bool isPicking )
{
    if ( model == nullptr )
        model = getRenderModel( world, name, modelName );

    if ( model == nullptr )
        return;

    Transform transform( Transform::translate, position );

    if ( !isPicking )
        model->render( &transform, 1 );
    else
        model->pick( &transform, 1 );
}

bool StaticMeshWn::setProperty( Property& property )
{
    if ( property.name == "position" && property.type == Property::vector )
        position = property.vectorValue;
    else if ( property.name == "model" && property.type == Property::text )
    {
        modelName = property.textValue;
        model.release();
    }
    else if ( !LitWorldNode::setProperty( property ) )
        return WorldNode::setProperty( property );

    return true;
}

TerrainWorldNode::TerrainWorldNode( World* world, Resources* res, const char* name )
        : WorldNode( world, res, "Terrain", name ), anchor( world->getObjectPlacePosition() ), size( 20.0f, 20.0f, 2.0f ), heightMapResolution( 20, 20 ), wireframe( false ),
        needRebuild( false )
{
}

TerrainWorldNode::TerrainWorldNode( World* world, Resources* res, cfx2::Node node )
        : WorldNode( world, res, "Terrain" ), needRebuild( false )
{
    load( node );
}

TerrainWorldNode::~TerrainWorldNode()
{
    printf( "destroying Terrain World Node\n" );
}

void TerrainWorldNode::buildGeometry( Bsp* bsp, List<BspPolygon>& polygons )
{
    Object<MaterialStaticProperties> materialProperties = new MaterialStaticProperties;

    if ( !getMaterial( material, materialProperties ) )
        return;

    LightMapping lightMapping;

    setUpLighting( world, materialProperties, &lightMapping );

    /*Reference<IMaterial> terrainMaterial = getMaterial( world, name, material );

    if ( terrainMaterial == nullptr )
        return;

    unsigned materialIndex = bsp->getMaterialIndex( terrainMaterial );*/

    TerrainBuildInfo buildInfo;

    buildInfo.resolution = heightMapResolution + 1;

    buildInfo.pos = anchor;
    buildInfo.origin = size.getXy() / 2;
    buildInfo.size = size;

    buildInfo.uv[0][0] = Vector2<>();
    buildInfo.uv[0][1] = size.getXy();

    buildInfo.lightUv[0] = lightMapping.uv[0];
    buildInfo.lightUv[1] = lightMapping.uv[1];

    buildInfo.materialIndex = bsp->registerMaterial( name + ".material", materialProperties.detach() );

    size_t listOffset = polygons.getLength();

    heightMap->buildTerrain( &buildInfo, polygons );

    // Generate the lightmap
    if ( lightingMode == 1 )
    {
        const Vector2<unsigned> maxVertex = buildInfo.resolution - 1;

        if ( lightMapping.lightMap != nullptr )
        {
            Image* lightMapImage = lightMapping.lightMap->image;

            SG_assert( lightMapImage->format == Image::Format::rgb )

            const Vector2<> range( lightMapping.coords[1].x - lightMapping.coords[0].x, lightMapping.coords[1].y - lightMapping.coords[0].y );
            const Vector2<> polygonSize = buildInfo.size.getXy() / maxVertex;

            for ( unsigned y = lightMapping.coords[0].y; y < lightMapping.coords[1].y; y++ )
            {
                size_t index = ( y * lightMapImage->size.x + lightMapping.coords[0].x ) * 3;

                for ( unsigned x = lightMapping.coords[0].x; x < lightMapping.coords[1].x; x++ )
                {
                    const Vector2<> terrainUv( ( x - lightMapping.coords[0].x ) / range.x, ( y - lightMapping.coords[0].y ) / range.y );
                    Vector2<unsigned> poly = ( terrainUv * maxVertex ).floor().minimum( maxVertex );

                    if ( poly.x >= ( unsigned ) heightMapResolution.x )
                        poly.x--;

                    if ( poly.y >= ( unsigned ) heightMapResolution.y )
                        poly.y--;

                    const Vector2<> xyInMesh = terrainUv * buildInfo.size.getXy();

                    if ( Vector2<unsigned>( x, y ) == lightMapImage->size.getXy() / 2 || Vector2<unsigned>( x, y ) == lightMapImage->size.getXy() / 4
                            || Vector2<unsigned>( x, y ) == lightMapImage->size.getXy() * 3 / 4
                            || Vector2<unsigned>( x, y ) == lightMapImage->size.getXy() - 1 )
                    {
                        printf( "%u, %u: terrainUv = %s\n", x, y, terrainUv.toString().c_str() );
                        printf( "\tpoly = %s\n", poly.toString().c_str() );
                        printf( "\tpolygonSize = %s\n", polygonSize.toString().c_str() );
                        printf( "\txyInMesh = %s\n", xyInMesh.toString().c_str() );
                    }

                    unsigned polyIndex = poly.y * maxVertex.x + poly.x;

                    const Vector2<> xyInPoly = xyInMesh - poly * polygonSize;
                    const Vector2<> tuInPoly = xyInPoly / polygonSize;

                    BspPolygon& tri = polygons[listOffset + polyIndex];

                    Vector<> normalSamples[2][2];

                    normalSamples[0][0] = tri.v[0].normal;
                    normalSamples[0][1] = tri.v[1].normal;
                    normalSamples[1][0] = tri.v[3].normal;
                    normalSamples[1][1] = tri.v[2].normal;

                    const Vector<> normal = sample( normalSamples, tuInPoly );
                    const Vector<> pos;

                    Colour luxel = Lighting::calculateAt( pos, normal, world );

                    lightMapImage->data[index++] = luxel.getR();
                    lightMapImage->data[index++] = luxel.getG();
                    lightMapImage->data[index++] = luxel.getB();
                }
            }
        }
    }
/*
        for ( int y = 0; y < lightMapResolution.y; y++ )
            for ( int x = 0; x < lightMapResolution.x; x++ )
            {
                const Vector2<float> lightMapUv = Vector2<float>( x, y ) / ( lightMapResolution - 1 );
                Vector2<unsigned> poly = ( lightMapUv * maxVertex ).floor<unsigned>().minimum( maxVertex );

                if ( poly.x >= ( unsigned ) heightMapResolution.x )
                    poly.x--;

                if ( poly.y >= ( unsigned ) heightMapResolution.y )
                    poly.y--;

                const Vector2<float> polygonSize = buildInfo.size.getXy() / maxVertex;
                const Vector2<float> xyInMesh = lightMapUv * buildInfo.size.getXy();

                if ( Vector2<unsigned>( x, y ) == lightMapResolution / 2 || Vector2<unsigned>( x, y ) == lightMapResolution / 4
                        || Vector2<unsigned>( x, y ) == lightMapResolution * 3 / 4
                        || Vector2<unsigned>( x, y ) == lightMapResolution - 1 )
                {
                    printf( "%u, %u: lightMapUv = %s\n", x, y, lightMapUv.toString().c_str() );
                    printf( "\tpoly = %s\n", poly.toString().c_str() );
                    printf( "\tpolygonSize = %s\n", polygonSize.toString().c_str() );
                    printf( "\txyInMesh = %s\n", xyInMesh.toString().c_str() );
                }

                //unsigned polyIndex = ( poly.y * maxVertex.x + poly.x ) * 2;
                unsigned polyIndex = poly.y * maxVertex.x + poly.x;

                const Vector2<float> xyInPoly = xyInMesh - poly * polygonSize;
                const Vector2<float> tuInPoly = xyInPoly / polygonSize;

                BspPolygon& tri = polygons[listOffset + polyIndex];
                //BspPolygon& tri2 = polygons[listOffset + polyIndex + 1];

                Vector<float> samples[2][2];

                samples[0][0] = tri.v[0].normal;
                samples[0][1] = tri.v[1].normal;
                //samples[1][0] = tri.v[2].normal;
                //samples[1][1] = tri2.v[2].normal;
                samples[1][0] = tri.v[3].normal;
                samples[1][1] = tri.v[2].normal;

                Vector<float> normal = sample( samples, tuInPoly );

                Colour luxel = Lighting::calculateAt( Vector<>(), normal, world );

                lightMap.data[index++] = luxel.getR();
                lightMap.data[index++] = luxel.getG();
                lightMap.data[index++] = luxel.getB();
            }

        Reference<OutputStream> output = File::open( world->tmpDir + "/lightmap_" + lightMapName, true );
        ImageWriter::save( &lightMap, output, Image::StorageFormat::dxt1 );
    }*/
}

void TerrainWorldNode::drag( Vector<float>& vec )
{
    float height = heightMap->get( selectedPoint.x, selectedPoint.y ) + vec.z / size.z;
    heightMap->set( selectedPoint.x, selectedPoint.y, height );

    /*unsigned vertexOffset = selectedPoint.y * heightMapResolution.x + selectedPoint.x;
    Vertex vertex;

    terrain->retrieveVertices( 0, vertexOffset, &vertex, 1 );
    vertex.z = height * size.z;
    terrain->updateVertices( 0, vertexOffset, &vertex, 1 );*/
}

void TerrainWorldNode::getProperties( List<Property>& properties )
{
    WorldNode::getProperties( properties );

    properties.add( Property( "anchor", anchor ) );
    properties.add( Property( "size", size ) );
    //properties.add( Property( "heightMap", heightMap ) );
    properties.add( Property( "heightMapResolution", heightMapResolution ) );
    properties.add( Property( "wireframe", wireframe ) );
    properties.add( Property( "material", material ) );

    LitWorldNode::getProperties( properties );
}

void TerrainWorldNode::onPickingFinished( unsigned id, StormCraftFrame* frame )
{
    pickingIds.begin();

    for ( int y = 0; y < heightMapResolution.y; y++ )
        for ( int x = 0; x < heightMapResolution.x; x++ )
        {
            if ( pickingIds.current() == id )
            {
                Vector<> position( x * size.x / ( heightMapResolution.x - 1 ), y * size.y / ( heightMapResolution.y - 1 ),
                        heightMap->get( x, y ) * size.z );

                bool visible[3] = { false, false, true };

                frame->showGizmo( anchor - Vector<>( size.x / 2, size.y / 2 ) + position, this, visible );

                selectedPoint = Vector<unsigned>( x, y );
                return;
            }

            pickingIds.next();
        }

    if ( id == pickingId )
    {
        bool visible[3] = { true, true, true };

        frame->showGizmo( anchor + Vector<>( 0.0f, 0.0f, size.z ), this, visible );
    }
}

void TerrainWorldNode::rebuild()
{
    if ( heightMapResolution < Vector2<int>( 2, 2 ) )
        return;

    if ( heightMap == nullptr )
    {
        heightMap = sg->createHeightMap( heightMapResolution );

        for ( int y = 0; y < heightMapResolution.y; y++ )
            for ( int x = 0; x < heightMapResolution.x; x++ )
                heightMap->set( x, y, ( rand() % 100 ) / 100.0f );
    }

    Reference<IMaterial> terrainMaterial = getRenderMaterial( material );


////////////////////////////////////////////////////////////
/*
    List<float> coords;

    Vector2<float> spacing( size.getXy() / ( heightMapResolution - 1 ) );

    for ( int y = 0; y < heightMapResolution.y - 1; y++ )
    {
        for ( int x = 0; x < heightMapResolution.x - 1; x++ )
        {
            coords.add( x * spacing.x - size.x / 2 );
            coords.add( y * spacing.y - size.y / 2 );
            coords.add( heightMap->get(  x,y ) * size.z - size.z / 2 );

            coords.add( x * spacing.x - size.x / 2 );
            coords.add( (y+1) * spacing.y - size.y / 2 );
            coords.add( heightMap->get( x, y+1 ) * size.z - size.z / 2 );

            coords.add( (x+1) * spacing.x - size.x / 2 );
            coords.add( y * spacing.y - size.y / 2 );
            coords.add( heightMap->get(x+1,y ) * size.z - size.z / 2 );



            coords.add( (x+1) * spacing.x - size.x / 2 );
            coords.add( (y+1) * spacing.y - size.y / 2 );
            coords.add( heightMap->get( x+1, y+1 ) * size.z - size.z / 2 );

            coords.add( (x+1) * spacing.x - size.x / 2 );
            coords.add( y * spacing.y - size.y / 2 );
            coords.add( heightMap->get(  x+1,y ) * size.z - size.z / 2 );

            coords.add( x * spacing.x - size.x / 2 );
            coords.add( (y+1) * spacing.y - size.y / 2 );
            coords.add( heightMap->get(  x,y+1 ) * size.z - size.z / 2 );
        }
    }

    List<BspTri> tris;

    for ( unsigned i = 0; i < coords.getLength(); i += 9 )
    {
        BspTri triangle;

        triangle.v[0].pos = { coords[i], coords[i+1], coords[i+2] };
        triangle.v[1].pos = { coords[i+3], coords[i+4], coords[i+5] };
        triangle.v[2].pos = { coords[i+6], coords[i+7], coords[i+8] };

        triangle.v[0].uv[0] =  {coords[i], coords[i+1]};
        triangle.v[1].uv[0] =  {coords[i+3], coords[i+4]};
        triangle.v[2].uv[0] =  {coords[i+6], coords[i+7]};

        tris.add( triangle );
    }

    Bsp bsp( 100, Vector<float>( 4.0f, 4.0f, 2.0f ) );

    Object<BspTree> tree = bsp.generate( tris.getPtr(), tris.getLength() );
    tree->material = terrainMaterial;

    terrain = driver->createModelFromBsp( "World_1", tree );*/
/////////////////////////////////////////////////////////////////////////

    TerrainCreationInfo terrainInfo( heightMap, size, Vector<float>( size.x / 2, size.y / 2 ), heightMapResolution, Vector2<>(), Vector2<>( size.getXy() ),
            false, false, wireframe, terrainMaterial->reference() );
    terrain = graphicsDriver->createTerrain( name + ".terrain", &terrainInfo, IModel::dynamicVertices );

    needRebuild = false;
}

void TerrainWorldNode::render( StormCraftFrame* frame, bool isPicking )
{
    if ( needRebuild )
        rebuild();

    Transform transforms[3];

    transforms[0] = Transform( Transform::translate, anchor );

    if ( !isPicking )
    {
        if ( world->gizmoObject == this )
        {
            terrain->render( transforms, 1, selectionBlend );
            renderBoundingBox( anchor + Vector<>( 0.0f, 0.0f, size.z / 2 ), size );
        }
        else
            terrain->render( transforms, 1 );
    }
    else
    {
        pickingId = terrain->pick( transforms, 1 );
        pickingIds.clear( true );
    }

        /*transforms.add( Transform( Transform::scale, size ) );
        transforms.add( Transform( Transform::translate, anchor + Vector<float>( 0.0f, 0.0f, size.z / 2 ) ) );
        res->boundingBox->render( transforms );
        transforms.clear();*/

    transforms[0] = Transform( Transform::scale, Vector<>( 0.05f, 0.05f, 0.05f ) );
    transforms[1] = Transform( Transform::translate, anchor - Vector<float>( size.x / 2, size.y / 2 ) );
    transforms[2] = Transform( Transform::translate, Vector<>() );

    Vector2<float> maxSample( heightMapResolution - 1 );
    Vector2<float> spacing( size.getXy() / maxSample );

    for ( int y = 0; y < heightMapResolution.y; y++ )
        for ( int x = 0; x < heightMapResolution.x; x++ )
        {
            transforms[2].vector = Vector<>( x * spacing.x, y * spacing.y, heightMap->get( x, y ) * size.z );

            if ( !isPicking )
                frame->res->diamond->render( transforms, 3 );
            else
                pickingIds.add( frame->res->diamond->pick( transforms, 3 ) );
        }
}

bool TerrainWorldNode::setProperty( Property& property )
{
    if ( property.name == "anchor" && property.type == Property::vector )
        anchor = property.vectorValue;
    else if ( property.name == "size" && property.type == Property::vector )
    {
        size = property.vectorValue;
        needRebuild = true;
    }
    /*else if ( property.name == "heightMap" && property.type == Property::text )
    {
        heightMap = property.textValue;
        mustRebuild = true;
    }*/
    else if ( property.name == "heightMapResolution" && property.type == Property::vector2i )
    {
        heightMapResolution = property.vector2iValue.maximum( 2 );
        needRebuild = true;
    }
    else if ( property.name == "wireframe" && property.type == Property::boolean )
    {
        wireframe = property.boolValue;
        needRebuild = true;
    }
    else if ( property.name == "material" && property.type == Property::text )
    {
        material = property.textValue;
        needRebuild = true;
    }
    else if ( !LitWorldNode::setProperty( property ) )
        return WorldNode::setProperty( property );

    return true;
}

void TerrainWorldNode::startup()
{
    rebuild();
}
