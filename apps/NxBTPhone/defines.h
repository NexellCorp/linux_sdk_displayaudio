#ifndef DEFINES_H
#define DEFINES_H

enum Menu {
	Menu_Select,
	Menu_Call,
	Menu_Message,
	Menu_Calling, // in comming, out going
	Menu_CallingEnd
};

enum PlayStatus {
	PlayStatus_Playing,
	PlayStatus_Paused,
	PlayStatus_Stopped,
	PlayStatus_Stopped_After_Play
};

#endif // DEFINES_H
