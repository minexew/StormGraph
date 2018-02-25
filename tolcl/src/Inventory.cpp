
#include "Inventory.hpp"

namespace GameClient
{
    static const char* itemNames[] = { "", "Trollface" };

    Item::Item() : itemId( 0 )
    {
    }

    Item::Item( unsigned itemId ) : itemId( itemId )
    {
    }

    const char* Item::getName() const
    {
        return itemNames[itemId];
    }

    Bag::Bag( unsigned numSlots ) : numSlots( numSlots )
    {
    }

    Slot* Bag::get( unsigned slot )
    {
        if ( slot < numSlots )
            return &slots[slot];
        else
            return 0;
    }

    bool Bag::getEmptySlot( unsigned& slot )
    {
        for ( unsigned i = 0; i < numSlots; i++ )
            if ( slots[i].isEmpty() )
            {
                slot = i;
                return true;
            }

        return false;
    }

    unsigned Bag::getNumSlots() const
    {
        return numSlots;
    }

    void Bag::set( unsigned slot, const Item& item )
    {
        if ( slot < numSlots )
            slots[slot].set( item );
    }

    Inventory::Inventory()
    {
        bags.add( new Bag( 16 ) );
    }

    Inventory::~Inventory()
    {
        delete bags.current();
    }

    bool Inventory::getEmptySlot( unsigned& bag, unsigned& slot )
    {
        iterate ( bags )
            if ( bags.current() && bags.current()->getEmptySlot( slot ) )
            {
                bag = bags.iter();
                return true;
            }

        return false;
    }

    unsigned Inventory::getNumBags() const
    {
        return bags.getLength();
    }

    unsigned Inventory::getNumSlots( unsigned bag ) const
    {
        if ( bags[bag] )
            return bags[bag]->getNumSlots();
        else
            return 0;
    }

    Slot* Inventory::get( unsigned bag, unsigned slot )
    {
        if ( bags[bag] )
            return bags[bag]->get( slot );
        else
            return 0;
    }

    void Inventory::set( unsigned bag, unsigned slot, const Item& item )
    {
        if ( bags[bag] )
            bags[bag]->set( slot, item );
    }
}
