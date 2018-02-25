
#include "launcher.hpp"

#ifndef __li_MSW
//#include <sys/stat.h>
//#include <sys/types.h>
#endif

class UpdateListener : public HttpRequestListener
{
    String fileName, localName;
    Reference<File> file;
    unsigned dataLength, receivedLength;

    Object<Transfer> transfer;
    unsigned mirror;

    public:
        UpdateListener( const String& fileName, unsigned mirror );
        virtual ~UpdateListener();

        virtual bool onDataReady( HttpRequest* request, uint64_t length );
        virtual void onData( HttpRequest* request, const void* data, uint64_t length );
        virtual void onStatusChange( HttpRequest* request );
};

static HttpClient* client;
static bool updateFailed = false;

static List<String> createdDirs, filesToUpdate, mirrors;

static Mutex ftuMutex;
static File* logFile;

Mutex transfersMutex;
List<Transfer*> transfers;
String base;

static void checkVersions();

static void createDir( const String& name )
{
    iterate ( createdDirs )
        if ( createdDirs.current() == name )
            return;

    createdDirs.add( name );

#ifdef __li_MSW
    CreateDirectory( name.replaceAll( "/", "\\" ), 0 );
#else
    mkdir( name, S_IRWXU | S_IRWXG | S_IRWXO );
#endif
}

bool fetch( unsigned mirror )
{
    ftuMutex.enter();
    if ( filesToUpdate.isEmpty() )
    {
        ftuMutex.leave();
        return false;
    }

    String fileName = filesToUpdate[0];
    filesToUpdate.remove( 0 );
    ftuMutex.leave();

    HttpRequest* request = new HttpRequest( mirrors[mirror] + "/" + fileName, new UpdateListener( fileName, mirror ) );
    client->request( request );

    return true;
}

UpdateListener::UpdateListener( const String& fileName, unsigned mirror )
        : fileName( fileName ), transfer( 0 ), mirror( mirror )
{
    int index = 0;

    localName = base + "/" + fileName;

    while ( ( unsigned ) index < localName.getNumBytes() )
    {
        index = localName.findChar( '/', index );

        if ( index < 0 )
            break;

        createDir( localName.leftPart( index ) );

        index += 1;
    }
}

UpdateListener::~UpdateListener()
{
}

bool UpdateListener::onDataReady( HttpRequest* request, uint64_t length )
{
    if ( updateFailed )
        return false;

    logFile->writeLine( "[UpdateListener " + fileName + "] Data ready: " + ( unsigned ) length + " bytes" );

    if ( !file )
        file = File::open( localName, true );

    if ( !file )
        accessDenied();

    dataLength = length;
    receivedLength = 0;
    return true;
}

void UpdateListener::onData( HttpRequest* request, const void* data, uint64_t length )
{
    logFile->writeLine( "[UpdateListener " + fileName + "] Data receive: " + ( unsigned ) length + " bytes" );

    if ( file )
        file->write( data, length );

    receivedLength += length;

    transfersMutex.enter();

    if ( transfer )
        transfer->progress = receivedLength * 100 / dataLength;

    transfersMutex.leave();

    refresh();
}

void UpdateListener::onStatusChange( HttpRequest* request )
{
    transfersMutex.enter();

    logFile->writeLine( "[UpdateListener " + fileName + "] Status=" + request->getStatus() );

    if ( request->getStatus() == HttpRequest::processing )
    {
        if ( !transfer )
        {
            transfer = new Transfer;
            transfer->fileName = fileName;
            transfer->progress = 0;
            transfers.add( transfer );
        }
    }
    else
    {
        transfers.removeItem( transfer );
        transfer.release();
    }

    transfersMutex.leave();

    if ( request->getStatus() == HttpRequest::successful )
        fetch( mirror );
    else if ( request->getStatus() == HttpRequest::failed )
    {
        updateFailed = true;
        client->cancelAllRequests();

        transfersMutex.enter();
        transfers.clear();
        transfersMutex.leave();

        changeStatus( "update failed: " + request->getFailReason() );
        MessageBox( 0, fileName + " - update failed: " + request->getFailReason(), window_title, MB_ICONWARNING );
    }
}

class VersionListener : public HttpRequestListener
{
    Reference<OutputStream> updateList;

    public:
        VersionListener()
        {
        }

        virtual ~VersionListener()
        {
        }

        virtual bool onDataReady( HttpRequest* request, uint64_t length )
        {
            logFile->writeLine( ( String ) "[VersionListener] Data ready: " + ( unsigned ) length + " bytes" );

            if ( !updateList )
            {
                createDir( base );
                updateList = File::open( versionFileName, true );
            }

            if ( !updateList )
                accessDenied();

            return true;
        }

        virtual void onData( HttpRequest* request, const void* data, uint64_t length )
        {
            logFile->writeLine( ( String ) "[VersionListener] Data receive: " + ( unsigned ) length + " bytes" );

            if ( updateList )
                updateList->write( data, length );
        }

        virtual void onStatusChange( HttpRequest* request )
        {
            logFile->writeLine( ( String ) "[VersionListener] Status=" + request->getStatus() );

            if ( request->getStatus() == HttpRequest::processing )
                changeStatus( "checking for updates..." );
            else if ( request->getStatus() == HttpRequest::successful )
            {
                updateList.release();

                checkVersions();
            }
            else if ( request->getStatus() == HttpRequest::failed )
            {
                updateList.release();

                changeStatus( "game update failed: " + request->getFailReason() );
                MessageBox( 0, "Game update failed: " + request->getFailReason(), window_title, MB_ICONWARNING );
                runGame();
            }
        }
};

class UpdateWaiter : public Thread
{
    public:
        UpdateWaiter()
        {
            destroyOnExit();
        }

        virtual ~UpdateWaiter()
        {
        }

        virtual void run()
        {
            if ( client->isRunning() )
                changeStatus( "update in progress..." );

            client->waitFor();

            if ( !updateFailed )
            {
                changeStatus( "launching game" );
                runGame();
            }
        }
};

static bool testFile( uint8_t mode, const String& fileName, InputStream* version )
{
    Reference<File> file = File::open( fileName );

    logFile->writeLine( "[testFile] Testing " + File::formatFileName( fileName ) );

    if ( mode == 0x01 )
        return isReadable( file );
    else/* if ( mode == 0x02 )*/
    {
        uint8_t hash[16];
        version->read( hash, sizeof( hash ) );

        if ( !isReadable( file ) )
            return false;

        md5_t state;
        md5_init( &state );

        while ( file->isReadable() )
        {
            uint8_t buffer[buffer_size];

            unsigned read = file->read( buffer, buffer_size );
            md5_process( &state, buffer, read );
        }

        uint8_t signature[16];
        md5_finish( &state, signature );

        if ( memcmp( signature, hash, 16 ) != 0 )
            return false;

        return true;
    }
}

static void checkVersions()
{
    InputStream* version = GZFileStream::open( versionFileName );

    if ( !isReadable( version ) )
    {
        changeStatus( "failed to open file" );
        return;
    }

    uint16_t formatVersion = version->read<uint16_t>();

    if ( formatVersion != 0x0002 )
        launcherOutdated();

    uint16_t numMirrors = version->read<uint16_t>();
    version->read<int16_t>();

    for ( unsigned i = 0; i < numMirrors && version->isReadable(); i++ )
        mirrors.add( version->readString() );

    // List files to update
    unsigned numStarted = 0;

    while ( version->isReadable() )
    {
        uint8_t mode = version->read<uint8_t>();
        String fileName = version->readString();

        if ( !testFile( mode, base + "/" + fileName, version ) )
        {
            ftuMutex.enter();
            filesToUpdate.add( fileName );
            ftuMutex.leave();

            if ( numStarted < numMirrors )
                fetch( numStarted++ );
        }
    }

    version->release();
    remove( versionFileName );

    ( new UpdateWaiter() )->start();
}

void checkForUpdates( const String& baseDir )
{
    base = baseDir;

    logFile = File::open( baseDir + "/launcher.log", true );
    logFile->writeLine( "Checking for updates." );

    client = new HttpClient();
    HttpRequest* request = new HttpRequest( version_file_uri, new VersionListener() );

    client->request( request );
}
