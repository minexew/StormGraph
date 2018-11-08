#pragma once

#include <wx/app.h>

#include "StormCraftMain.hpp"

class StormCraftApp : public wxApp
{
    Object<IEngine> sg;

    public:
        virtual ~StormCraftApp();

        virtual bool OnInit();
};
