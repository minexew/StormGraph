#pragma once

#include "GameClient.hpp"

namespace GameClient
{
    enum ItemClass
    {
        Item_Devel
    };

    enum ItemSubClass
    {
        Item_Devel_Test
    };

    class Item
    {
        public:
            unsigned itemId;

        public:
            Item();
            Item( unsigned itemId );

            ItemClass getClass() const;
            const char* getName() const;
            ItemSubClass getSubClass() const;
    };

    class Slot
    {
        Item item;

        public:
            Item& get() { return item; }
            unsigned getItemId() const { return item.itemId; }
            bool isEmpty() const { return item.itemId == 0; }
            void set( const Item& item ) { this->item = item; }
    };

    class Bag
    {
        unsigned numSlots;
        Array<Slot> slots;

        public:
            Bag( unsigned numSlots );

            Slot* get( unsigned slot );
            bool getEmptySlot( unsigned& slot );
            unsigned getNumSlots() const;
            void set( unsigned slot, const Item& item );
    };

    class Inventory
    {
        List<Bag*> bags;

        public:
            Inventory();
            ~Inventory();

            Bag* getBag( unsigned bag ) { return bags[bag]; }
            bool getEmptySlot( unsigned& bag, unsigned& slot );
            unsigned getNumBags() const;
            unsigned getNumSlots( unsigned bag ) const;
            Slot* get( unsigned bag, unsigned slot );
            void set( unsigned bag, unsigned slot, const Item& item );
    };
}
