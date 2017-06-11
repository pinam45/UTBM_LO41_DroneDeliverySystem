#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_DASHBOARD_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_DASHBOARD_H

#include <mqueue.h>
#include <ConsoleControl.h>

#include "typedefs.h"

typedef enum {
	D_DRONE_WAITING = 0,
	D_DRONE_CHARGING,
	D_DRONE_FLYING_MTC,
	D_DRONE_FLYING_CTM_DELIVERY_SUCCESS,
	D_DRONE_FLYING_CTM_DELIVERY_FAIL,
	D_DRONE_DEAD,
	D_DRONE_FINISHED,
	D_CLIENT_WAITING,
	D_CLIENT_TARGET_OUT,
	D_CLIENT_ABSENT,
	D_CLIENT_FINISHED,
	D_PACKAGE_WAITING,
	D_PACKAGE_FLYING,
	D_PACKAGE_SUCCESS,
	D_PACKAGE_FAIL,
	D_UNKNOWN,
} DashboardElementState;

typedef enum {
	D_PACKAGE,
	D_DRONE,
	D_CLIENT,
	D_EXIT
} DashboardElementType;

struct dashboard_message {
	DashboardElementState state;
	DashboardElementType type;
	unsigned int number;
};

struct dashboard {
	mqd_t msgQueueID;
	unsigned int packagesNumber;
	unsigned int dronesNumber;
	unsigned int clientsNumber;
	DashboardElementState* packagesStates;
	DashboardElementState* dronesStates;
	DashboardElementState* clientsStates;
	cc_Vector2* packagesPos;
	cc_Vector2* dronesPos;
	cc_Vector2* clientsPos;
	unsigned int lastWidth;
	unsigned int lastHeight;
};

Dashboard* global_dashboard;

Dashboard* dashboard_constructor(unsigned int packagesNumber, unsigned int dronesNumber, unsigned int clientsNumber);

void* dashboard_launch(/* Dashboard* */ void* dashboard);

void dashboard_free(Dashboard* dashboard);

void dashboard_sendMessage(Dashboard* dashboard, DashboardMessage* message);


#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_DASHBOARD_H
