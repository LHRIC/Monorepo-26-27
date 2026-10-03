#ifndef DIAGNOSTICSPRESENTER_HPP
#define DIAGNOSTICSPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class diagnosticsView;

class diagnosticsPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    diagnosticsPresenter(diagnosticsView& v);

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

    virtual ~diagnosticsPresenter() {}

private:
    diagnosticsPresenter();

    diagnosticsView& view;
};

#endif // DIAGNOSTICSPRESENTER_HPP
