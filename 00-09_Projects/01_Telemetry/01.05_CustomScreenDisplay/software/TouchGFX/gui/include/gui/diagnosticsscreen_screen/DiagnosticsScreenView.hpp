#ifndef DIAGNOSTICSSCREENVIEW_HPP
#define DIAGNOSTICSSCREENVIEW_HPP

#include <gui_generated/diagnosticsscreen_screen/DiagnosticsScreenViewBase.hpp>
#include <gui/diagnosticsscreen_screen/DiagnosticsScreenPresenter.hpp>

class DiagnosticsScreenView : public DiagnosticsScreenViewBase
{
public:
    DiagnosticsScreenView();
    virtual ~DiagnosticsScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // DIAGNOSTICSSCREENVIEW_HPP
