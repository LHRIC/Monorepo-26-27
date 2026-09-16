#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Utils.hpp>
#include <touchgfx/Texts.hpp>
#include <touchgfx/hal/Types.hpp>

extern "C" uint32_t HAL_GetTick(void);

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    //initialize CAN value buffer pointers
    coolant.textAreaPtr.wildcard = &CoolantValue;
    coolant.bufferPtr = CoolantValueBuffer;
    coolant.bufferSize = COOLANTVALUE_SIZE;
	coolant.wildcard = true;

    throttle.textAreaPtr.wildcard = &ThrottleValue;
    throttle.bufferPtr = ThrottleValueBuffer;
    throttle.bufferSize = THROTTLEVALUE_SIZE;
	throttle.wildcard = true;

    battery.textAreaPtr.wildcard = &BatteryValue;
    battery.bufferPtr = BatteryValueBuffer;
    battery.bufferSize = BATTERYVALUE_SIZE;
	battery.wildcard = true;

    gear.textAreaPtr.wildcard = &GearValue;
    gear.bufferPtr = GearValueBuffer;
    gear.bufferSize = GEARVALUE_SIZE;
	gear.wildcard = true;

	fault.textAreaPtr.noWildcard = &SHUTDOWNWARNING;
	fault.wildcard = false;

	rpm.textAreaPtr.wildcard = &SpeedValue_1;
	rpm.bufferPtr = SpeedValue_1Buffer;
	rpm.bufferSize = SPEEDVALUE_1_SIZE;
	rpm.wildcard = true;

	speed.textAreaPtr.wildcard = &SpeedValue;
	speed.bufferPtr = SpeedValueBuffer;
	speed.bufferSize = SPEEDVALUE_SIZE;
	speed.wildcard = true;

	FPSCOUNTER.setVisible(false);//remove or set to true if we want fps counter
	ShiftLight.setVisible(false);

    lastMs = HAL_GetTick();
    frameCount = 0;
	flashState = true;
	shiftFlash = false;
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}
void Screen1View::handleTickEvent()
{
	Screen1ViewBase::handleTickEvent();

	    // Count how many frames (ticks that rendered) occur
	    //frameCount++;

	    uint32_t now = HAL_GetTick();
	    uint32_t elapsed = now - lastMs;
		//flashing element handler
		if(elapsed >= 250U){
			for(int i = 0; i < NUM_OF_CAN_VALUES; i++){
				if(CAN_value_ptrs[i]->flashing){
					if(CAN_value_ptrs[i]->wildcard){
						CAN_value_ptrs[i]->textAreaPtr.wildcard->setVisible(flashState);
						CAN_value_ptrs[i]->textAreaPtr.wildcard->invalidate();
					}
					else{
						CAN_value_ptrs[i]->textAreaPtr.noWildcard->setVisible(flashState);
						CAN_value_ptrs[i]->textAreaPtr.noWildcard->invalidate();
					}

				}
			}
			if(shiftFlash){
				ShiftLight.setVisible(flashState);
				ShiftLight.invalidate();
			}
			flashState = !flashState;
			lastMs = now;
		}
	    // Update once per second
	    // if (elapsed >= 1000U)
	    // {
		// 	////fps counter if we want that
		// 	//float fps = frameCount / (elapsed / 1000.0f);
	    //     //uint8_t fpsI = (uint8_t)fps;
	    //     //Unicode::snprintfFloat(FPSCOUNTERBuffer, FPSCOUNTER_SIZE, "%3.1f", fps);
	    //     //FPSCOUNTER.invalidateContent();   // redraw the label

	    //     // Reset window

	    //     frameCount = 0;
	    //     lastMs = now;
	    // }
}

void Screen1View::updateDisplayValue(CAN_value_t* CAN_val)
{
    //Update text area or progress bar (make a case for anything with special functionality, default is for text)
	switch(CAN_val->valueIdentifier){
	case RPM://rpm progress bar
		imageProgress1.setValue(CAN_val->value.u16);
		imageProgress1.invalidate();
		//fill the buffer based on updated value
		CAN_value_updateTextBuffer(CAN_val);
		//invalidate text buffer
		CAN_val->textAreaPtr.wildcard->invalidateContent();
		if(CAN_val->value.u16 > 12500){
			shiftFlash = true;
		}
		else{
			shiftFlash = false;
			ShiftLight.setVisible(false);
			ShiftLight.invalidate();
		}
		break;
	case FAULT://warnings
		CAN_val->textAreaPtr.noWildcard->setVisible((bool)(CAN_val->value.i8 != 0));//nonzero means warning!
		CAN_val->flashing = (bool)(CAN_val->value.i8 != 0);
		CAN_val->textAreaPtr.noWildcard->invalidate();
		break;
	default://text update
		//fill the buffer based on updated value
		CAN_value_updateTextBuffer(CAN_val);
		//invalidate text buffer
		CAN_val->textAreaPtr.wildcard->invalidateContent();
		break;
	}

}