
#pragma once

namespace StormRender
{
    // ############
    // Event System
    // ############

    enum EventType
    {
        EV_DUMMY            = 0,
        EV_VKEY             = 1,
    };

    //
    // The VKey system maps various inputs (kb, m, joy, gamepad) to virtual IDs
    // VKeys > 0 are fixed, < 0 are assigned dynamically
    //
    enum
    {
        VKEY_PRESSED        = 1,
        VKEY_RELEASED       = 2,
        VKEY_TRIG           = 4,
        VKEY_PARAMCHANGEX   = 8,
        VKEY_PARAMCHANGEY   = 16,
    };

    enum VKEY_Fixed
    {
        V_CLOSE = 1,
    };

    struct VKeyEvent_t
    {
        int flags, vk, x, y;

        bool triggered() { return flags & (VKEY_PRESSED | VKEY_TRIG); }
    };

    //
    // Local event structure
    //
    struct Event_t
    {
        EventType type;

        union
        {
            VKeyEvent_t vkey;
        };
    };

    class IEventReceiver
    {
        public:
            virtual void ReceiveEvent( const Event_t& event ) = 0;
    };

    class IEventInput
    {
        public:
            virtual ~IEventInput() {}

            virtual void ReceiveEvents( IEventReceiver* receiver ) = 0;
    };

    // #####################
    // Main Visual Interface
    // #####################
    class IR
    {
        public:
            // Events, Frame
            virtual void beginFrame() = 0;
            virtual void endFrame() = 0;
            virtual Event_t* getEvent() = 0;
    };
}
