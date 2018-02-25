
#include "ClientUI.hpp"

namespace GameUI
{
    static const unsigned slotWidth = 48, slotHeight = 48, itemIconWidth = 32, itemIconHeight = 32, itemDragWidth = 48, itemDragHeight = 48;

    static const ItemPanelLayout itemPanelLayouts[] =
    {
        // 1 - 3 items: 1 row
        { 0, 0 }, { 1, 1 }, { 2, 1 }, { 3, 1 },

        // 4 - 8 items: 2 rows
        { 2, 2 }, { 0, 0 }, { 3, 2 }, { 0, 0 }, { 4, 2 },

        // 9 - 15 items: 3 rows
        { 3, 3 }, { 5, 2 }, { 0, 0 }, { 4, 3 }, { 0, 0 }, { 0, 0 }, { 5, 3 },

        // 16 - 24 items: 4 rows
        { 4, 4 }, { 0, 0 }, { 6, 3 }, { 0, 0 }, { 5, 4 }, { 7, 3 }, { 0, 0 }, { 6, 4 }
    };

    InventoryWindow::InventoryWindow( UI* ui, int x, int y, GameClient::Bag* bag )
            : Window( ui, "Inventory", x, y, 512, 320 ), onClose( 0 )
    {
        GameUI::Button* closeButton = new GameUI::Button( 488, 8, 16, 16, "x" );
        closeButton->setName( "close_btn" );
        closeButton->setOnPush( this );
        add( closeButton );

        GameUI::ItemPanel* items = new GameUI::ItemPanel( ui, 16, 16, bag );
        add( items );
    }

    InventoryWindow::~InventoryWindow()
    {
    }

    void InventoryWindow::setOnClose( EventListener* listener )
    {
        onClose = listener;
    }

    void InventoryWindow::uiEvent( Widget* widget, const li::String& event )
    {
        if ( widget->getName() == "close_btn" && event == "push" && onClose )
            onClose->uiEvent( this, "close" );
    }

    ItemDrag::ItemDrag( UI* ui )
            : Widget( 0, 0 ), ui( ui ), sourceSlot( 0 )
    {
    }

    ItemDrag::~ItemDrag()
    {
    }

    bool ItemDrag::mouseMove( int x, int y )
    {
        this->x = x;
        this->y = y;

        return false;
    }

    void ItemDrag::pick( GameClient::Slot* from )
    {
        item = from->get();
        from->set( GameClient::Item() );

        if ( ui )
            ui->focus( this );
    }

    void ItemDrag::put( GameClient::Slot* where )
    {
        where->set( item );
        item = GameClient::Item();
    }

    void ItemDrag::render()
    {
        if ( item.itemId != 0 )
        {
            StormGraph::Texture* icon = GameClient::globalItemIconMgr->get( item.itemId );

            icon->render2D( x - itemDragWidth / 2, y - itemDragHeight / 2, itemDragWidth, itemDragHeight );
        }
    }

    ItemPanel::ItemPanel( UI* ui, int x, int y, GameClient::Bag* bag )
            : Widget( x, y ), ui( ui ), ownsTooltip( false ), bag( bag )
    {
        layout = itemPanelLayouts[bag->getNumSlots()];

        slotTexture = GameClient::globalResMgr->getTexture( "tolcl/gfx/item/inv_empty.png" );
    }

    ItemPanel::~ItemPanel()
    {
    }

    bool ItemPanel::mouseDown( int x, int y )
    {
        if ( x >= this->x && y >= this->y && x < ( int )( this->x + layout.cols * slotWidth ) && y < ( int )( this->y + layout.rows * slotHeight ) )
        {
            unsigned col = ( x - this->x ) / slotWidth;
            unsigned row = ( y - this->y ) / slotHeight;

            GameClient::Slot* slot = bag->get( row * layout.cols + col );

            if ( slot )
            {
                if ( !slot->isEmpty() )
                {
                    ItemDrag* drag = ( ItemDrag* )ui->findWidget( "item_drag" );

                    if ( drag )
                        drag->pick( slot );
                }
                else
                {
                    ItemDrag* drag = ( ItemDrag* )ui->findWidget( "item_drag" );

                    if ( drag )
                        drag->put( slot );
                }
            }

            return true;
        }

        return false;
    }

    bool ItemPanel::mouseMove( int x, int y )
    {
        if ( x >= this->x && y >= this->y && x < ( int )( this->x + layout.cols * slotWidth ) && y < ( int )( this->y + layout.rows * slotHeight ) )
        {
            unsigned col = ( x - this->x ) / slotWidth;
            unsigned row = ( y - this->y ) / slotHeight;

            GameClient::Slot* slot = bag->get( row * layout.cols + col );

            if ( slot && !slot->isEmpty() )
            {
                ui->showTooltip( x + 40, y, slot->get().getName() );
                ownsTooltip = true;
                return false;
            }
        }

        if ( ownsTooltip )
        {
            ui->hideTooltip();
            ownsTooltip = false;
        }

        return false;
    }

    void ItemPanel::render()
    {
        for ( unsigned row = 0; row < layout.rows; row++ )
            for ( unsigned col = 0; col < layout.cols; col++ )
            {
                GameClient::Slot* slot = bag->get( row * layout.cols + col );

                if ( slot )
                {
                    slotTexture->render2D( x + col * slotWidth, y + row * slotHeight, 48.0f, 48.0f );

                    if ( !slot->isEmpty() )
                    {
                        unsigned itemId = slot->getItemId();

                        StormGraph::Texture* icon = GameClient::globalItemIconMgr->get( itemId );

                        icon->render2D( x + col * slotWidth + ( slotWidth - itemIconWidth ) / 2,
                                y + row * slotHeight + ( slotHeight - itemIconHeight ) / 2,
                                itemIconWidth, itemIconHeight );
                    }
                }
            }
    }
}
