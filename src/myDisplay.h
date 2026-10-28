#include "structs.h"
#include "DWIN.h"

#ifndef mydisplay_H
#define mydisplay_H

class myDisplay: public LoopableClass, public IDWINHandler
{
    public: 
        myDisplay(DWIN* hmi);
        void displayEventHandler(long address, int lastByte, int message, String response);
        void Init();
        void Loop();

        DWIN* getHmi(){return _hmi;};
        void InitDisplayVariables();
        void UpdateDisplayVariables();
        void drawOilGraphValues();
        bool isHomePage();
        
        enum DwinAddress VpAddress;
        enum DwinPage CurrentPage;

        void SetPage(DwinPage page);
        

    private:
        DWIN* _hmi;
        unsigned long _lastUpdateMillis = 0;
        const unsigned long _updateInterval = 200; //display values refresh (ms)
};

#endif