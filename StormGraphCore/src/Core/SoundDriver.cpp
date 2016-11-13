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

#include <StormGraph/Engine.hpp>
#include <StormGraph/SoundDriver.hpp>

#include <al.h>
#include <alc.h>
#include <vorbis/vorbisfile.h>

#define testErrors() SG_assert3( alGetError() == AL_NO_ERROR, "StormGraph.SoundDriver.?" )

namespace StormGraph
{
    class SoundSource : public ISoundSource
    {
        protected:
            enum { stopped, playing } state;

            ALuint source, buffers[2];
            ALenum format;
            unsigned frequency;

            Array<uint8_t> streamBuffer;
            size_t bufferSize;

            Reference<ISoundStream> stream;

            virtual ~SoundSource();

            void refill( ALuint buffer );

        public:
            li_ReferencedClass_override( SoundSource )

            SoundSource( ISoundStream* stream );

            virtual void onFrameEnd();
            virtual void play();
    };

    class SoundDriver : public ISoundDriver
    {
        protected:
            ALCdevice* device;
            ALCcontext* context;

            List<SoundSource*> sources;

        public:
            SoundDriver();
            virtual ~SoundDriver();

            virtual ISoundSource* createSoundSource( ISoundStream* stream ) override;
            virtual void onFrameEnd() override;
    };

    class VorbisSoundStream : public ISoundStream
    {
        protected:
            String name;

            Reference<InputStream> input;

            OggVorbis_File oggVorbisFile;
            Info info;

            static size_t read_func( void* output, size_t size, size_t count, void* vss );

        public:
            VorbisSoundStream( InputStream* input, const char* name );
            virtual ~VorbisSoundStream();

            virtual const char* getClassName() const { return "StormGraph.VorbisSoundStream"; }
            virtual void getInfo( Info& output );
            virtual const char* getName() const { return name; }
            virtual size_t read( void* output, size_t numSamples );
    };

    SoundDriver::SoundDriver() : device( nullptr ), context( nullptr )
    {
        device = alcOpenDevice( 0 );

        SG_assert3( device != 0, "StormGraph.SoundDriver.SoundDriver" )

        context = alcCreateContext( device, 0 );
        alcMakeContextCurrent( context );

        bool haveEax = alIsExtensionPresent( "EAX2.0" ) != 0;


#define test( value_ ) ( ( value_ ) ? "<span style=\"color: #080\">yes</span>" : "<b style=\"color: #f00\">no</b>" )

        Common::logEvent( "StormGraph.SoundDriver", ( String ) "Initializing SoundDriver!\n"
                + "&nbsp;&nbsp;<b>OpenAL renderer</b>: " +/* ( const char* )*/ alGetString( AL_RENDERER ) + "\n"
                + "&nbsp;&nbsp;<b>OpenAL version</b>: "+ ( const char* ) alGetString( AL_VERSION ) + "\n"
                + "&nbsp;&nbsp;<b>Renderer vendor</b>: " + ( const char* ) alGetString( AL_VENDOR ) + "\n"
                + "&nbsp;&nbsp;<b>Have EAX 2.0</b>: " + test( haveEax ) + "\n" );

        alGetError();

        //alSourcei( source, AL_BUFFER, buffer );
        //testErrors()

        /*ALfloat listenerPos[] = { 0.0, 0.0, 0.0 };
        ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
        ALfloat listenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

        alListenerfv( AL_POSITION, listenerPos );
        testErrors()

        alListenerfv( AL_VELOCITY, listenerVel );
        testErrors()

        alListenerfv( AL_ORIENTATION, listenerOri );
        testErrors()

        alSourcef( source, AL_PITCH, 1.0f );
        testErrors()

        alSourcef( source, AL_GAIN, 1.0f );
        testErrors()

        alSource3f( source, AL_POSITION, 100.0, 0.0, 0.0 );
        testErrors()

        alSourcei( source, AL_SOURCE_STATE, AL_PLAYING );
        testErrors()

        alSourcefv( source, AL_VELOCITY, source0Vel );
        testErrors()

        alSourcei( source, AL_LOOPING, AL_FALSE );
*/
    }

    SoundDriver::~SoundDriver()
    {
        iterate ( sources )
            sources.current()->release();

        context = alcGetCurrentContext();
        device = alcGetContextsDevice( context );
        alcMakeContextCurrent( 0 );
        alcDestroyContext( context );
        alcCloseDevice( device );
    }

    ISoundSource* SoundDriver::createSoundSource( ISoundStream* stream )
    {
        Reference<SoundSource> source = new SoundSource( stream );

        sources.add( source->reference() );

        return source.detach();
    }

    void SoundDriver::onFrameEnd()
    {
        iterate ( sources )
            sources.current()->onFrameEnd();
    }

    SoundSource::SoundSource( ISoundStream* stream )
            : state( stopped ), stream( stream )
    {
        alGenSources( 1, &source );
        testErrors()

        alSourcef( source, AL_GAIN, 0.45f );
        testErrors()

        alGenBuffers( 2, buffers );
        testErrors()

        ISoundStream::Info info;
        stream->getInfo( info );

        if ( info.numChannels == 1 && info.bitsPerSample == 8 )
            format = AL_FORMAT_MONO8;
        else if ( info.numChannels == 1 && info.bitsPerSample == 16 )
            format = AL_FORMAT_MONO16;
        else if ( info.numChannels == 2 && info.bitsPerSample == 8 )
            format = AL_FORMAT_STEREO8;
        else if ( info.numChannels == 2 && info.bitsPerSample == 16 )
            format = AL_FORMAT_STEREO16;

        // 200ms buffers
        bufferSize = info.numChannels * info.frequency * info.bitsPerSample / 8 / 5;
        streamBuffer.resize( bufferSize );
        frequency = info.frequency;

        refill( buffers[0] );
        refill( buffers[1] );
    }

    SoundSource::~SoundSource()
    {
        alSourceStop( source );
        alSourcei( source, AL_BUFFER, 0 );

        alDeleteSources( 1, &source );
        alDeleteBuffers( 2, buffers );
    }

    void SoundSource::onFrameEnd()
    {
        if ( state != playing )
            return;

        ALint buffersProcessed = 0;
        alGetSourcei( source, AL_BUFFERS_PROCESSED, &buffersProcessed );

        while ( buffersProcessed-- )
        {
            ALuint removedBuffer = 0;
            alSourceUnqueueBuffers( source, 1, &removedBuffer );

            refill( removedBuffer );
        }

        ALint state;
        alGetSourcei( source, AL_SOURCE_STATE, &state );

        if ( state != AL_PLAYING )
        {
            ALint queuedBuffers;
            alGetSourcei( source, AL_BUFFERS_QUEUED, &queuedBuffers );

            if ( queuedBuffers )
                alSourcePlay( source );
        }
    }

    void SoundSource::play()
    {
        alSourcePlay( source );
        testErrors()

        state = playing;
    }

    void SoundSource::refill( ALuint buffer )
    {
        size_t got = stream->read( streamBuffer.getPtr(), bufferSize );

        if ( got )
        {
            alBufferData( buffer, format, streamBuffer.getPtr(), got, frequency );
            alSourceQueueBuffers( source, 1, &buffer );
        }
    }

    VorbisSoundStream::VorbisSoundStream( InputStream* input, const char* name )
            : name( name ), input( input )
    {
        SG_assert3( input != nullptr, "StormGraph.VorbisSoundStream.VorbisSoundStream" )

        ov_callbacks callbacks;
	    callbacks.read_func = read_func;
	    callbacks.seek_func = 0;
	    callbacks.close_func = 0;
	    callbacks.tell_func = 0;

	    SG_assert3( ov_open_callbacks( this, &oggVorbisFile, 0, 0, callbacks ) == 0, "StormGraph.VorbisSoundStream.VorbisSoundStream" )

        vorbis_info* vorbisInfo = ov_info( &oggVorbisFile, -1 );
        SG_assert3( vorbisInfo != nullptr, "StormGraph.VorbisSoundStream.VorbisSoundStream" )

        info.numChannels = vorbisInfo->channels;
        info.frequency = vorbisInfo->rate;
        info.bitsPerSample = 16;

        printf( "Opened VorbisSoundStream: [version=%i, %ux f=%ux%u, bitrate=%li-%li-%li]\n", vorbisInfo->version,
                info.numChannels, info.frequency, info.bitsPerSample,
                vorbisInfo->bitrate_lower, vorbisInfo->bitrate_nominal, vorbisInfo->bitrate_upper );
    }

    VorbisSoundStream::~VorbisSoundStream()
    {
        ov_clear( &oggVorbisFile );
    }

    void VorbisSoundStream::getInfo( Info& output )
    {
        output.numChannels = info.numChannels;
        output.frequency = info.frequency;
        output.bitsPerSample = info.bitsPerSample;
    }

    size_t VorbisSoundStream::read( void* output, size_t numSamples )
    {
        int currentSection;

        size_t bytesWritten = 0;

        while ( true )
        {
            long decodeSize = ov_read( &oggVorbisFile, ( char* ) output + bytesWritten, numSamples - bytesWritten, 0, 2, 1, &currentSection );

            if ( decodeSize > 0 )
            {
                bytesWritten += decodeSize;

                if ( bytesWritten >= numSamples )
                    break;
            }
            else
                break;
        }

        return bytesWritten;
    }

    size_t VorbisSoundStream::read_func( void* output, size_t size, size_t count, void* vss )
    {
        return ( ( VorbisSoundStream* ) vss )->input->read( output, size * count ) / size;
    }

    ISoundDriver* createSoundDriver( IEngine* engine )
    {
        return new SoundDriver();
    }
}
