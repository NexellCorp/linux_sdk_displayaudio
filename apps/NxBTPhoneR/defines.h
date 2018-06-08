#ifndef DEFINES_H
#define DEFINES_H

namespace Nexell {
    namespace BTPhone {
        enum Menu {
            Menu_Select,
            Menu_Call,
            Menu_Message,
            Menu_Calling // in comming, out going
        };

        enum PlayStatus {
            PlayStatus_Playing,
            PlayStatus_Paused,
            PlayStatus_Stopped,
            PlayStatus_Stopped_After_Play
        };
    } /* namespace BTPhone */
} /* namespace Nexell  */

#endif // DEFINES_H
