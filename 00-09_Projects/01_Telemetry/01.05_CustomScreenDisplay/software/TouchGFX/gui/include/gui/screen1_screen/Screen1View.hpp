#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    void updateDisplayValue(CAN_value_t* CAN_val);//updates speed number with CAN value
protected:
    uint32_t lastMs;
    uint16_t frameCount;
    bool flashState;
    bool shiftFlash;
};

#endif // SCREEN1VIEW_HPP
