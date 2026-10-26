/**
 * CAN Handler Interface  
 * Copyright (c) Dixtone @2025. All rights reserved.
 * Based on Can library from Sandeep Mistry
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H
class CANHandler{
    public:
        virtual ~CANHandler(){}
        virtual void onReceivePacket(int packetSize);
};
#endif