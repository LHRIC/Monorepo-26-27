/*
 * can_types.c
 *
 *  Created on: Oct 21, 2025
 *      Author: ayaan
 */
#include "can_types.hpp"

//ALL TEXT BUFFERS AND BUFFER SIZES ARE INITIALIZED IN SCREEN1VIEW.cpp (bc the buffers in SCREEN1VIEWBASE generated code)
CAN_value_t rpm = {
    CAN_ID_RPM_THROTTLE,
	{.u16 = 9877}, //magic default value I decided to set in the UI for literally no reason and should match here now 
    0,
	CAN_TYPE_UINT16,
	RPM,
	1.0,
	0.0,
	true,
	false,
	{.u16 = 0},
	{.u16 = 0}
};
CAN_value_t coolant = {
    CAN_ID_COOLANT,
	{.u16 = 0},
    0,
	CAN_TYPE_UINT16,
	COOLANT,
	0.1,
	-273.0,
	true,
	false,
	{.u16 = 89},
	{.u16 = 95}
};
//-1-6 (-1 is an error/no value from ECU)
CAN_value_t gear = {
	CAN_ID_GEAR,
	{.i8 = 0},
    6,
	CAN_TYPE_INT8,
	GEAR,
	1.0,
	0.0,
	true,
	false,
	{.i8 = 0},
	{.i8 = 0}
};

CAN_value_t throttle = {
	CAN_ID_RPM_THROTTLE,
	{.f = 0.0},   // float signal (ECU handles conversion)
    4,
	CAN_TYPE_FLOAT16,
	THROTTLE,
	0.1,//10
	0,
	true,
	false,
	{.f = 0.0},
	{.f = 0.0}
};

CAN_value_t battery = {
	CAN_ID_BATTERY,
	{.f = 0.0},
    0,
	CAN_TYPE_FLOAT16,
	BATTERY,
	0.1,
	0.0,
	true,
	false,
	{.f = 0.0},
	{.f = 0.0}
};

CAN_value_t fault = {
	CAN_ID_FAULT,
	{.i8 = 1},
    0,
	CAN_TYPE_INT8,
	FAULT,
	1.0,
	0.0,
	true,
	true,
	{.i8 = 0},
	{.i8 = 0}
};
CAN_value_t speed = {
	CAN_ID_SPEED,
	{.u16 = 0},
	0,
	CAN_TYPE_UINT16,
	SPEED,
	0.1,
	0.0,
	true,
	false,
	{.u16 = 0},
	{.u16 = 0}
};


CAN_value_t* CAN_value_ptrs[NUM_OF_CAN_VALUES] = {&rpm, &throttle, &coolant, &battery, &gear, &fault, &speed};

void CAN_value_updateTextBuffer(CAN_value_t* CAN_val){
	switch(CAN_val->type){
			case CAN_TYPE_FLOAT16:
				touchgfx::Unicode::snprintfFloat(CAN_val->bufferPtr, CAN_val->bufferSize, "%4.1f", CAN_val->value.f);
				break;
			case CAN_TYPE_INT8:
				touchgfx::Unicode::snprintf(CAN_val->bufferPtr, CAN_val->bufferSize, "%d", CAN_val->value.i8);
				break;
			case CAN_TYPE_UINT16:
				touchgfx::Unicode::snprintf(CAN_val->bufferPtr, CAN_val->bufferSize, "%u", CAN_val->value.u16);
				break;
			}
}

void CAN_value_updateColor(CAN_value_t* CAN_val, touchgfx::colortype color){
	if(CAN_val->wildcard){
		CAN_val->textAreaPtr.wildcard->setColor(color);
		CAN_val->textAreaPtr.wildcard->invalidate();
	}
	else{
		CAN_val->textAreaPtr.noWildcard->setColor(color);
		CAN_val->textAreaPtr.noWildcard->invalidate(); 
	}
}

void CAN_value_updateVisibility(CAN_value_t* CAN_val, bool visible){
	if(CAN_val->wildcard){
		CAN_val->textAreaPtr.wildcard->setVisible(visible);
		CAN_val->textAreaPtr.wildcard->invalidate();
	}
	else{
		CAN_val->textAreaPtr.noWildcard->setVisible(visible);
		CAN_val->textAreaPtr.noWildcard->invalidate(); 
	}
}

bool CAN_value_updateValue(CAN_value_t* CAN_val, uint16_t CAN_data){
	//update value if different (need to do this differently per value)
	//returns whether or not the value was updated
	switch(CAN_val->type){
			case CAN_TYPE_FLOAT16:{
				float fVal = CAN_data * CAN_val->scale + CAN_val->offset;
				if(CAN_val->value.f != fVal){
					CAN_val->value.f = fVal;
					if(CAN_val->redThreshold.f == 0.0) return false; //no colors
					if(CAN_val->value.f < CAN_val->yellowThreshold.f){
						CAN_value_updateColor(CAN_val, WHITE); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else if(CAN_val->value.f < CAN_val->redThreshold.f){
						CAN_value_updateColor(CAN_val, YELLOW); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else{
						CAN_value_updateColor(CAN_val, RED); 
						CAN_val->flashing = true;
					}
					return false;//text is no longer updated
				}
				break;}
			case CAN_TYPE_INT8:{
				int8_t iVal = (int8_t)(CAN_data);
				if(CAN_val->value.i8 != iVal){
					CAN_val->value.i8 = iVal;
					if(CAN_val->redThreshold.i8 == 0) return false; //no colors
					if(CAN_val->value.i8 < CAN_val->yellowThreshold.i8){
						CAN_value_updateColor(CAN_val, WHITE); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else if(CAN_val->value.i8 < CAN_val->redThreshold.i8){
						CAN_value_updateColor(CAN_val, YELLOW); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else{
						CAN_value_updateColor(CAN_val, RED); 
						CAN_val->flashing = true;
					}
					return false;
				}
				break;}
			case CAN_TYPE_UINT16:{
				uint16_t uVal = (uint16_t)(CAN_data * CAN_val->scale + CAN_val->offset);
				if(CAN_val->value.u16 != uVal){
					CAN_val->value.u16 = uVal;
					if(CAN_val->redThreshold.u16 == 0) return false; //no colors
					if(CAN_val->value.u16 < CAN_val->yellowThreshold.u16){
						CAN_value_updateColor(CAN_val, WHITE); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else if(CAN_val->value.u16 < CAN_val->redThreshold.u16){
						CAN_value_updateColor(CAN_val, YELLOW); 
						CAN_val->flashing = false;
						CAN_value_updateVisibility(CAN_val, true);
					}
					else{
						CAN_value_updateColor(CAN_val, RED); 
						CAN_val->flashing = true;
					}
					return false;
				}
				break;}
	}
	return true;//text is still updated

}
