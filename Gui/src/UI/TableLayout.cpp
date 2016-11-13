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

#include "GuiDriver.hpp"

namespace GuiDriver
{
    TableLayout::TableLayout( Gui* gui, size_t numColumns )
            : numColumns( numColumns ), spacing( 4.0f, 4.0f )
    {
    }

    TableLayout::~TableLayout()
    {
    }

    void TableLayout::add( IChildWidget* widget )
    {
        ISizableWidget* sizable = dynamic_cast<ISizableWidget*>( widget );

        if ( sizable != nullptr )
            sizable->setFreeFloat( false );

        Container::add( widget );

        layout();
    }

    Vector<float> TableLayout::getMinSize()
    {
        if ( widgets.isEmpty() )
            return Vector2<>();

        SG_assert( numColumns > 0 )

        // Determine the table height
        size_t numRows = widgets.getLength() / numColumns;

        if ( widgets.getLength() % numColumns != 0 )
            numRows++;

        Array<float> rowHeights( numRows ), columnWidths( numColumns );

        size_t i = 0;

        for ( size_t row = 0; i < widgets.getLength(); row++ )
        {
            for ( size_t column = 0; column < numColumns && i < widgets.getLength(); column++ )
            {
                Vector<float> cellMinSize = static_cast<IChildWidget*>( ( IWidget* ) widgets[i] )->getMinSize();

                if ( cellMinSize.x > columnWidths[column] )
                    columnWidths[column] = cellMinSize.x;

                if ( cellMinSize.y > rowHeights[row] )
                    rowHeights[row] = cellMinSize.y;

                i++;
            }
        }

        Vector2<float> minSize;

        for ( size_t column = 0; column < numColumns; column++ )
            minSize.x += columnWidths[column];

        for ( size_t row = 0; row < numRows; row++ )
            minSize.y += rowHeights[row];

        minSize.x += ( numColumns - 1 ) * spacing.x;
        minSize.y += ( numRows - 1 ) * spacing.y;

        return minSize;
    }

    void TableLayout::layout()
    {
        if ( widgets.isEmpty() )
            return;

        SG_assert( numColumns > 0 )

        // Determine the table height
        size_t numRows = widgets.getLength() / numColumns;

        if ( widgets.getLength() % numColumns != 0 )
            numRows++;

        Array<float> rowHeights( numRows ), columnWidths( numColumns );

        size_t i = 0;

        for ( size_t row = 0; i < widgets.getLength(); row++ )
        {
            for ( size_t column = 0; column < numColumns && i < widgets.getLength(); column++ )
            {
                Vector<float> cellMinSize = static_cast<IChildWidget*>( ( IWidget* ) widgets[i] )->getMinSize();

                if ( !columnsGrowable[column] && cellMinSize.x > columnWidths[column] )
                    columnWidths[column] = cellMinSize.x;

                if ( !rowsGrowable[row] && cellMinSize.y > rowHeights[row] )
                    rowHeights[row] = cellMinSize.y;

                i++;
            }
        }

        float fixedWidth = 0.0f;
        float fixedHeight = 0.0f;

        size_t numGrowableColumns = 0;
        size_t numGrowableRows = 0;

        for ( size_t column = 0; column < numColumns; column++ )
            if ( !columnsGrowable[column] )
                fixedWidth += columnWidths[column];
            else
                numGrowableColumns++;

        for ( size_t row = 0; row < numRows; row++ )
            if ( !rowsGrowable[row] )
                fixedHeight += rowHeights[row];
            else
                numGrowableRows++;

        fixedWidth += ( numColumns - 1 ) * spacing.x;
        fixedHeight += ( numRows - 1 ) * spacing.y;

        float cellWidth = 0.0f;
        float cellHeight = 0.0f;

        if ( numGrowableColumns > 0 )
            cellWidth = ( areaSize.x - fixedWidth ) / numGrowableColumns;

        if ( numGrowableRows > 0 )
            cellHeight = ( areaSize.y - fixedHeight ) / numGrowableRows;

        float y = 0.0f;

        i = 0;

        for ( size_t row = 0; i < widgets.getLength(); row++ )
        {
            float rowHeight = ( !rowsGrowable[row] ) ? rowHeights[row] : cellHeight;

            float x = 0.0f;

            for ( size_t column = 0; column < numColumns && i < widgets.getLength(); column++ )
            {
                float columnWidth = ( !columnsGrowable[column] ) ? columnWidths[column] : cellWidth;

                static_cast<IChildWidget*>( ( IWidget* ) widgets[i] )->setBounds( areaPos + Vector2<float>( x, y ), Vector2<float>( columnWidth, rowHeight ) );

                x += columnWidth + spacing.x;
                i++;
            }

            y += rowHeight + spacing.y;
        }
    }

    void TableLayout::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        Container::onKeyState( key, state, character );
    }

    bool TableLayout::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        return Container::onMouseButton( button, pressed, mouse );
    }

    void TableLayout::onMouseMoveTo( const Vector2<int>& mouse )
    {
        Container::onMouseMoveTo( mouse );
    }

    void TableLayout::render()
    {
        Container::render();
    }

    void TableLayout::setBounds( const Vector<float>& pos, const Vector<float>& size )
    {
        areaPos = pos.getXy();
        areaSize = size.getXy();

        layout();
    }

    void TableLayout::setColumnGrowable( size_t column, bool growable )
    {
        columnsGrowable[column] = growable;
    }

    void TableLayout::setRowGrowable( size_t row, bool growable )
    {
        rowsGrowable[row] = growable;
    }

    void TableLayout::update( double delta )
    {
        Container::update( delta );
    }
}

