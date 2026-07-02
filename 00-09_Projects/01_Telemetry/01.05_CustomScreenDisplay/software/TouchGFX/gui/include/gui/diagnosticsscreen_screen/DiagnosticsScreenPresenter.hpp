#ifndef DIAGNOSTICSSCREENPRESENTER_HPP
#define DIAGNOSTICSSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class DiagnosticsScreenView;

class DiagnosticsScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    DiagnosticsScreenPresenter(DiagnosticsScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~DiagnosticsScreenPresenter() {}

private:
    DiagnosticsScreenPresenter();

    DiagnosticsScreenView& view;
};

#endif // DIAGNOSTICSSCREENPRESENTER_HPP
