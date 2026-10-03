#ifndef DIAGNOSTICSVIEW_HPP
#define DIAGNOSTICSVIEW_HPP

#include <gui_generated/diagnostics_screen/diagnosticsViewBase.hpp>
#include <gui/diagnostics_screen/diagnosticsPresenter.hpp>

class diagnosticsView : public diagnosticsViewBase
{
public:
    diagnosticsView();
    virtual ~diagnosticsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // DIAGNOSTICSVIEW_HPP
