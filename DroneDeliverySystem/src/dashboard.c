#include "dashboard.h"

#include <util.h>
#include <pthread.h>

static cc_Color main_bg_color = BLACK;
static cc_Color main_fg_color = WHITE;

#define DASHBOARD_ELEMENT_STATE_NUMBER (unsigned int)(D_UNKNOWN+1)
#define DASHBOARD_ELEMENT_TEXT_MAX_SIZE 22

#define DASHBOARD_ELEMENTS_TABLE(ENTRY)\
    ENTRY(D_DRONE_WAITING,                     YELLOW,  WHITE,   "Waiting")\
    ENTRY(D_DRONE_CHARGING,                    BLUE,    WHITE,   "Charging")\
    ENTRY(D_DRONE_FLYING_MTC,                  CYAN,    WHITE,   "flying to client")\
    ENTRY(D_DRONE_FLYING_CTM_DELIVERY_SUCCESS, GREEN,   WHITE,   "flying back")\
    ENTRY(D_DRONE_FLYING_CTM_DELIVERY_FAIL,    RED,     WHITE,   "flying back")\
    ENTRY(D_DRONE_FINISHED,                    WHITE,   BLACK,   "Finished")\
    ENTRY(D_CLIENT_WAITING,                    YELLOW,  WHITE,   "Waiting")\
    ENTRY(D_CLIENT_TARGET_OUT,                 CYAN,    WHITE,   "Target out")\
    ENTRY(D_CLIENT_ABSENT,                     BLUE,    WHITE,   "Client absent")\
    ENTRY(D_CLIENT_FINISHED,                   GREEN,   WHITE,   "Finished")\
    ENTRY(D_PACKAGE_WAITING,                   YELLOW,  WHITE,   "Waiting")\
    ENTRY(D_PACKAGE_FLYING,                    CYAN,    WHITE,   "Flying")\
    ENTRY(D_PACKAGE_SUCCESS,                   GREEN,   WHITE,   "Delivery success")\
    ENTRY(D_PACKAGE_FAIL,                      RED,     WHITE,   "Delivery fail")\
    ENTRY(D_UNKNOWN,                           MAGENTA, WHITE,   "unknown")\

#define _EXPAND_AS_STATE(state, bg_color, fg_color, text) state,
#define _EXPAND_AS_BG_COLOR(state, bg_color, fg_color, text) bg_color,
#define _EXPAND_AS_FG_COLOR(state, bg_color, fg_color, text) fg_color,
#define _EXPAND_AS_TEXT(state, bg_color, fg_color, text) text,

static char package_name_format[] = "Package %03u:";
static char drone_name_format[] = "  Drone %03u:";
static char client_name_format[] = " Client %03u:";

static unsigned int package_name_length = 13;
static unsigned int drone_name_length = 13;
static unsigned int client_name_length = 13;
static unsigned int state_length = 24;

/*static DashboardElementState de_states[] = {
	DASHBOARD_ELEMENTS_TABLE(_EXPAND_AS_STATE)
};*/
static cc_Color de_bg_colors[] = {
	DASHBOARD_ELEMENTS_TABLE(_EXPAND_AS_BG_COLOR)
};
static cc_Color de_fg_colors[] = {
	DASHBOARD_ELEMENTS_TABLE(_EXPAND_AS_FG_COLOR)
};
static char de_text[][DASHBOARD_ELEMENT_TEXT_MAX_SIZE] = {
	DASHBOARD_ELEMENTS_TABLE(_EXPAND_AS_TEXT)
};

static cc_Vector2 nullpos = {0, 0};

static bool updateVars(Dashboard* dashboard);

static void printElementName(Dashboard* dashboard, DashboardElementType elementType, unsigned int number);

static void printElementState(Dashboard* dashboard, DashboardElementType elementType, unsigned int number);

static void printAll(Dashboard* dashboard);

static bool process_message(Dashboard* dashboard, DashboardMessage* message);

bool updateVars(Dashboard* dashboard) {
	unsigned int width = (unsigned int) cc_getWidth();
	unsigned int height = (unsigned int) cc_getHeight();
	if((width != dashboard->lastWidth) || (height != dashboard->lastHeight)) {
		unsigned int packagesPosX;
		unsigned int dronesPosX;
		unsigned int clientsPosX;
		unsigned int notmultiColumns;
		if(package_name_length + drone_name_length + client_name_length + 3 * state_length + 9 > width) {
			// all in 1 column
			packagesPosX = 3;
			dronesPosX = 3;
			clientsPosX = 3;
			notmultiColumns = 1;
		} else {
			// 3 columns
			packagesPosX = 3;
			dronesPosX = package_name_length + state_length + 6;
			clientsPosX = package_name_length + drone_name_length + 2 * state_length + 9;
			notmultiColumns = 0;
		}
		unsigned int i = 0;
		for(unsigned int j = 0; j < dashboard->packagesNumber; ++j) {
			if(i < height) {
				dashboard->packagesPos[j].x = (cc_type) packagesPosX;
				dashboard->packagesPos[j].y = (cc_type) i;
			} else {
				dashboard->packagesPos[j] = nullpos;
			}
			++i;
		}
		i *= notmultiColumns;
		for(unsigned int j = 0; j < dashboard->dronesNumber; ++j) {
			if(i < height) {
				dashboard->dronesPos[j].x = (cc_type) dronesPosX;
				dashboard->dronesPos[j].y = (cc_type) i;
			} else {
				dashboard->dronesPos[j] = nullpos;
			}
			++i;
		}
		i *= notmultiColumns;
		for(unsigned int j = 0; j < dashboard->clientsNumber; ++j) {
			if(i < height) {
				dashboard->clientsPos[j].x = (cc_type) clientsPosX;
				dashboard->clientsPos[j].y = (cc_type) i;
			} else {
				dashboard->clientsPos[j] = nullpos;
			}
			++i;
		}

		dashboard->lastWidth = width;
		dashboard->lastHeight = height;

		return true;
	}
	return false;
}

void printElementName(Dashboard* dashboard, DashboardElementType elementType, unsigned int number) {
	switch(elementType) {
		case D_PACKAGE:
			if(dashboard->packagesPos[number].x != nullpos.x) {
				cc_setCursorPosition(dashboard->packagesPos[number]);
				cc_setColors(main_bg_color, main_fg_color);
				printf(package_name_format, number);
			}
			break;
		case D_DRONE:
			if(dashboard->dronesPos[number].x != nullpos.x) {
				cc_setCursorPosition(dashboard->dronesPos[number]);
				cc_setColors(main_bg_color, main_fg_color);
				printf(drone_name_format, number);
			}
			break;
		case D_CLIENT:
			if(dashboard->clientsPos[number].x != nullpos.x) {
				cc_setCursorPosition(dashboard->clientsPos[number]);
				cc_setColors(main_bg_color, main_fg_color);
				printf(client_name_format, number);
			}
			break;
		case D_EXIT:
		default:
			break;
	}
	fflush(stdout);
}

void printElementState(Dashboard* dashboard, DashboardElementType elementType, unsigned int number) {
	cc_Vector2 pos;
	unsigned int j;
	switch(elementType) {
		case D_PACKAGE:
			if(dashboard->packagesPos[number].x == nullpos.x) {
				return;
			}
			pos = dashboard->packagesPos[number];
			pos.x += (cc_type) package_name_length;
			j = (unsigned int) dashboard->packagesStates[number];
			break;
		case D_DRONE:
			if(dashboard->dronesPos[number].x == nullpos.x) {
				return;
			}
			pos = dashboard->dronesPos[number];
			pos.x += (cc_type) drone_name_length;
			j = (unsigned int) dashboard->dronesStates[number];
			break;
		case D_CLIENT:
			if(dashboard->clientsPos[number].x == nullpos.x) {
				return;
			}
			pos = dashboard->clientsPos[number];
			pos.x += (cc_type) client_name_length;
			j = (unsigned int) dashboard->clientsStates[number];
			break;
		case D_EXIT:
		default:
			return;
	}

	unsigned int len = (unsigned int) strlen(de_text[j]);
	unsigned int st = 1;

	cc_setCursorPosition(pos);
	cc_setColors(de_bg_colors[j], de_fg_colors[j]);
	putchar('[');
	while(st < (state_length - len) / 2) {
		putchar(' ');
		++st;
	}
	printf(de_text[j]);
	st += len;
	while(st < state_length - 1) {
		putchar(' ');
		++st;
	}
	putchar(']');
	fflush(stdout);
}

void printAll(Dashboard* dashboard) {

	cc_setColors(main_bg_color, main_fg_color);
	cc_clean();

	for(unsigned int i = 0; i < dashboard->packagesNumber; ++i) {
		printElementName(dashboard, D_PACKAGE, i);
		printElementState(dashboard, D_PACKAGE, i);
	}
	for(unsigned int i = 0; i < dashboard->dronesNumber; ++i) {
		printElementName(dashboard, D_DRONE, i);
		printElementState(dashboard, D_DRONE, i);
	}
	for(unsigned int i = 0; i < dashboard->clientsNumber; ++i) {
		printElementName(dashboard, D_CLIENT, i);
		printElementState(dashboard, D_CLIENT, i);
	}
}

bool process_message(Dashboard* dashboard, DashboardMessage* message) {
	switch(message->type) {
		case D_PACKAGE:
			dashboard->packagesStates[message->number] = message->state;
			break;
		case D_DRONE:
			dashboard->dronesStates[message->number] = message->state;
			break;
		case D_CLIENT:
			dashboard->clientsStates[message->number] = message->state;
			break;
		case D_EXIT:
			return true;
		default:
			break;
	}
	if(updateVars(dashboard)) {
		printAll(dashboard);
	}
	printElementState(dashboard, message->type, message->number);
	return false;
}

Dashboard* dashboard_constructor(unsigned int packagesNumber, unsigned int dronesNumber, unsigned int clientsNumber) {
	Dashboard* dashboard = (Dashboard*) malloc(sizeof(Dashboard));
	struct mq_attr attr;
	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(DashboardMessage);
	check((dashboard->msgQueueID = mq_open("/dashboard", O_RDWR | O_CREAT, 0660, &attr)), "Dashboard: mq_open failed");
	check(mq_unlink("/dashboard"), "Dashboard: mq_unlink failed");

	dashboard->packagesNumber = packagesNumber;
	dashboard->dronesNumber = dronesNumber;
	dashboard->clientsNumber = clientsNumber;
	dashboard->packagesStates = malloc(packagesNumber * sizeof(DashboardElementState));
	dashboard->dronesStates = malloc(dronesNumber * sizeof(DashboardElementState));
	dashboard->clientsStates = malloc(clientsNumber * sizeof(DashboardElementState));

	for(unsigned int i = 0; i < dashboard->packagesNumber; ++i) {
		dashboard->packagesStates[i] = D_UNKNOWN;
	}
	for(unsigned int i = 0; i < dashboard->dronesNumber; ++i) {
		dashboard->dronesStates[i] = D_UNKNOWN;
	}
	for(unsigned int i = 0; i < dashboard->clientsNumber; ++i) {
		dashboard->clientsStates[i] = D_UNKNOWN;
	}

	dashboard->packagesPos = malloc(packagesNumber * sizeof(cc_Vector2));
	dashboard->dronesPos = malloc(dronesNumber * sizeof(cc_Vector2));
	dashboard->clientsPos = malloc(clientsNumber * sizeof(cc_Vector2));
	dashboard->lastWidth = 0;
	dashboard->lastHeight = 0;

	return dashboard;
}

void* dashboard_launch(void* dashboard) {
	cc_displayInputs(false);
	cc_setCursorVisibility(false);

	updateVars(dashboard);
	printAll(dashboard);

	Dashboard* dashboard1 = (Dashboard*) dashboard;
	DashboardMessage dashboardMessage;
	struct timespec time;
	time.tv_sec = 1;
	time.tv_nsec = 0;
	ssize_t result;
	bool ended = false;

	while(!ended) {
		result = mq_timedreceive(dashboard1->msgQueueID, (char*) &dashboardMessage, sizeof(DashboardMessage), 0, &time);
		if(result < 0) {
			if(errno != ETIMEDOUT) {
				check((int) result, "dashboard: mq_receive failed\n");
			} else {
				if(updateVars(dashboard)) {
					printAll(dashboard);
				}
			}
		} else {
			ended = process_message(dashboard1, &dashboardMessage);
		}
	}

	cc_Vector2 pos = {0, cc_getHeight() - 2};
	cc_setCursorPosition(pos);
	cc_setColors(BLACK, WHITE);
	cc_setCursorVisibility(true);
	while(cc_waitingInput()){
		cc_getInput();
	}
	printf("Please press any key to exit...");
	fflush(stdout);
	cc_getInput();
	putchar('\n');
	cc_displayInputs(true);
	pthread_exit(0);
}

void dashboard_free(Dashboard* dashboard) {
	free(dashboard->packagesStates);
	free(dashboard->dronesStates);
	free(dashboard->clientsStates);
	free(dashboard->packagesPos);
	free(dashboard->dronesPos);
	free(dashboard->clientsPos);
	check(mq_close(dashboard->msgQueueID), "Dashboard: mq_close failed\n");
	free(dashboard);
}

void dashboard_sendMessage(Dashboard* dashboard, DashboardMessage* message) {
	check(mq_send(dashboard->msgQueueID, (const char*) message, sizeof(DashboardMessage), 0), "mq_send failed");
}
