#pragma once

#include <stdint.h>
#include <littl.hpp>

namespace StormGraph
{
    uint64_t getRelativeMicroseconds();

    class Timer
    {
        uint64_t startTicks, pauseTicks;
        bool paused, started;

        public:
            Timer();
            uint64_t getMicros() const;
            uint64_t getMilis() const;
            bool isStarted() const { return started; }
            bool isPaused() const { return paused; }
            void pause();
            void reset();
            void start();
            void stop();
            void unpause();
    };

    class FpsMeter: public Timer
    {
        unsigned frame;
        uint64_t frameStartTime, frameTotalTime;

        public:
            FpsMeter() : frame( 0 ), frameStartTime( 0 ), frameTotalTime( 0 )
            {
            }

            uint64_t getLastMicros() const
            {
                return frameTotalTime;
            }

            unsigned getLastMilis() const
            {
                return ( unsigned )( frameTotalTime / 1000 );
            }

            unsigned getRate() const
            {
                if ( frameTotalTime )
                    return ( unsigned )( 1000000 / frameTotalTime );
                else
                    return 1234;
            }

            void newFrame()
            {
                frame++;
                frameStartTime = getMicros();
            }

            void endFrame()
            {
                frameTotalTime = getMicros() - frameStartTime;
            }

            void restart()
            {
                start();
                frame = 0;
            }
    };

}
