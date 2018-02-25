
#include "Inventory.hpp"
#include "UI.hpp"

namespace GameUI
{
    struct ItemPanelLayout
    {
        uint8_t cols, rows;
    };

    class InventoryWindow : public Window, EventListener
    {
        EventListener* onClose;

        public:
            InventoryWindow( UI* ui, int x, int y, GameClient::Bag* bag );
            virtual ~InventoryWindow();

            void setOnClose( EventListener* listener );
            void uiEvent( Widget* widget, const li::String& event );
    };

    class ItemDrag : public Widget
    {
        UI* ui;

        GameClient::Item item;
        GameClient::Slot* sourceSlot;

        public:
            ItemDrag( UI* ui );
            ~ItemDrag();

            virtual bool mouseMove( int x, int y );
            void pick( GameClient::Slot* from );
            void put( GameClient::Slot* where );
            virtual void render();
    };

    class ItemPanel : public Widget
    {
        UI* ui;
        bool ownsTooltip;

        GameClient::Bag* bag;
        StormGraph::Texture* slotTexture;

        ItemPanelLayout layout;

        public:
            ItemPanel( UI* ui, int x, int y, GameClient::Bag* bag );
            virtual ~ItemPanel();

            virtual bool mouseDown( int x, int y );
            virtual bool mouseMove( int x, int y );
            virtual void render();
    };
}
