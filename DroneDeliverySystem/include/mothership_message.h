#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_MESSAGE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_MESSAGE_H

typedef enum mothership_message_type {
	DRONE_BACK_TO_MOTHERSHIP,
	DRONE_DONE_CHARGING,
	DRONE_PACKAGE_DELIVERED_SUCCESS,
	DRONE_PACKAGE_DELIVERED_FAIL,
	DRONE_DEAD
} MothershipMessageType;

struct mothership_message {
	MothershipMessageType type;
	unsigned int sender_id;
};

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_MESSAGE_H
