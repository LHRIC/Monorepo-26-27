#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <fdcan.h>

extern FDCAN_HandleTypeDef hfdcan1;  // get the can object
Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	for(int i = 0; i < NUM_OF_CAN_VALUES; i++){
		if(!(CAN_value_ptrs[i]->textUpdated)){
		modelListener->onCanMessageReceived(CAN_value_ptrs[i]);
		CAN_value_ptrs[i]->textUpdated = true;
		}
	}
}
