
#include <StormGraph/StormGraph.hpp>

#include <SDL_image.h>

namespace StormGraph
{
    HeightMap::HeightMap( const char* fileName, const Vector<float>& dimensions, float heightMin )
            : width( 0 ), height( 0 ), dimensions( dimensions )
    {
        SDL_Surface* map = Texture::load( fileName );

        if ( !map || map->w < 2 || map->h < 2 )
            throw Exception( "StormGraph.HeightMap", "HeightMap", "HeightMapLoadError",
                    ( String )"Failed to load `" + fileName + "`.\n\nThe file probably either doesn't exist at all or is not a well-formed image. (minimal size is 2x2 pixels)" );

        width = map->w;
        height = map->h;

        if ( SDL_MUSTLOCK( map ) )
            SDL_LockSurface( map );

        int numColorChannels = map->format->BytesPerPixel;
        data = new float [map->w * map->h];
        heights = new float* [map->w];

        float* ptr = data;

        if ( numColorChannels == 4 )
        {
            if ( map->format->Rmask == 0x000000FF )
            {
                for ( int x = 0; x < map->w; x++ )
                    for ( int y = 0; y < map->h; y++ )
                        *( ptr++ ) = ( ( uint8_t* )map->pixels )[y * map->pitch + x * 4] / 255.0f * dimensions.z + heightMin;
            }
            else
            {
                for ( int x = 0; x < map->w; x++ )
                    for ( int y = 0; y < map->h; y++ )
                        *( ptr++ ) = ( ( uint8_t* )map->pixels )[y * map->pitch + x * 4 + 2] / 255.0f * dimensions.z + heightMin;
            }
        }
        else if ( numColorChannels == 3 )
        {
            if ( map->format->Rmask == 0x000000FF )
            {
                for ( int x = 0; x < map->w; x++ )
                    for ( int y = 0; y < map->h; y++ )
                        *( ptr++ ) = ( ( uint8_t* )map->pixels )[y * map->pitch + x * 3] / 255.0f * dimensions.z + heightMin;
            }
            else
            {
                for ( int x = 0; x < map->w; x++ )
                    for ( int y = 0; y < map->h; y++ )
                        *( ptr++ ) = ( ( uint8_t* )map->pixels )[y * map->pitch + x * 3 + 2] / 255.0f * dimensions.z + heightMin;
            }
        }
        else
        {
            printf( "[Fatal Error] Mesh::loadTerrain(numColorChannels = %i)\n", numColorChannels );
            return;
        }

        for ( int x = 0; x < map->w; x++ )
            heights[x] = data + x * map->h;

        if ( SDL_MUSTLOCK( map ) )
            SDL_UnlockSurface( map );

        SDL_FreeSurface( map );
    }

    HeightMap::~HeightMap()
    {
        delete[] heights;
        delete[] data;
    }

    float HeightMap::get( unsigned x, unsigned y )
    {
        if ( x >= 0 && x < width && y >= 0 && y < height )
            return heights[x][y];
        else
            return 0.0f;
    }

    float HeightMap::get( float x, float y )
    {
        x = x / dimensions.x * ( width - 1 );
        y = y / dimensions.y * ( height - 1 );

        float x0 = floor( x ), y0 = floor( y );
        float x1 = ceil( x ), y1 = ceil( y );

        if ( x1 == x0 && y1 == y0 )
            return get( ( unsigned )x0, ( unsigned )y0 );
        else if ( x1 == x0 )
            return get( ( unsigned )x0, ( unsigned )y0 ) + ( get( ( unsigned )x0, ( unsigned )y1 ) - get( ( unsigned )x0, ( unsigned )y0 ) ) / ( y1 - y0 ) * ( y - y0 );
        else if ( y1 == y0 )
            return get( ( unsigned )x0, ( unsigned )y0 ) + ( get( ( unsigned )x1, ( unsigned )y0 ) - get( ( unsigned )x0, ( unsigned )y0 ) ) / ( x1 - x0 ) * ( x - x0 );
        else
        {
            float divisor = ( x1 - x0 ) * ( y1 - y0 );

            return get( ( unsigned )x0, ( unsigned )y0 ) / divisor * ( x1 - x ) * ( y1 - y )
                    + get( ( unsigned )x1, ( unsigned )y0 ) / divisor * ( x - x0 ) * ( y1 - y )
                    + get( ( unsigned )x0, ( unsigned )y1 ) / divisor * ( x1 - x ) * ( y - y0 )
                    + get( ( unsigned )x1, ( unsigned )y1 ) / divisor * ( x - x0 ) * ( y - y0 );
        }
    }
}
