#pragma once

extern "C"
{
#include "md5.h"
}

#define littl_GZFileStream
#include <littl.hpp>
using namespace li;

#include "config.hpp"

// launcher.cpp Entries
void accessDenied();
void changeStatus( const char* status );
void launcherOutdated();
void refresh();
void runGame();

// update.cpp Entries
struct Transfer
{
    String fileName;
    unsigned progress;
};

extern List<Transfer*> transfers;
extern Mutex transfersMutex;
extern String base;

void checkForUpdates( const String& baseDir );
