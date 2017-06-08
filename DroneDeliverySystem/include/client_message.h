#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_MESSAGE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_MESSAGE_H

typedef enum client_message_type{
	DRONE_PUT_TARGET,
	DRONE_DELIVERY_SUCCESS,
	DRONE_DELIVERY_FAILURE, // no target, will try again
	DRONE_DELIVERY_FINAL_FAILURE // no target, no retry
} ClientMessageType;

struct client_message{
	ClientMessageType type;
};

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_MESSAGE_H
