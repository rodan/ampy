#ifndef __SYS_MESSAGEBUS_H__
#define __SYS_MESSAGEBUS_H__

#include "proj.h"

/*!
	\brief List of possible message types for the message bus.
	\sa sys_messagebus_register()
*/
/* WARNING: the enum values are optimized to work with some drivers.
	If you need to add a new entry, append it to the end! */
enum sys_message {
    // TIMER0
    SYS_MSG_TIMER0_CCR0 = BIT0,
    SYS_MSG_TIMER0_CCR1 = BIT1,
    SYS_MSG_TIMER0_CCR2 = BIT2, // timer_a0_delay_noblk
    SYS_MSG_TIMER0_CCR3 = BIT3,
    SYS_MSG_TIMER0_CCR4 = BIT4,
    SYS_MSG_TIMER0_IFG = BIT5,  // timer0 overflow
    // TIMER1A
    SYS_MSG_TIMER1_CRR0 = BIT7, // infrared remote sequence decoding
    SYS_MSG_TIMER1_CRR1 = BIT8, // PWM for intertechno OOK module
    SYS_MSG_TIMER1_CRR2 = BIT9, // unused
    // UART
    SYS_MSG_UART0_RX = BITA,
    // sound presence detection
    SYS_MSG_PORT_TRIG = BITB
};

/*!
	\brief Linked list of nodes listening to the message bus.
*/
struct sys_messagebus {
    /*! callback for receiving messages from the system bus */
    void (*fn) (enum sys_message);
    /*! bitfield of message types that the node wishes to receive */
    enum sys_message listens;
    /*! pointer to the next node in the list */
    struct sys_messagebus *next;
};

//static 
struct sys_messagebus *messagebus;

/*!
	\brief Registers a node in the message bus.
	\details Registers (add) a node to the message bus. A node can filter what message(s) are to be received by setting the bitfield \b listens.
	\sa sys_message, sys_messagebus, sys_messagebus_unregister
*/
void sys_messagebus_register(
                                // callback to receive messages from the message bus
                                void (*callback) (enum sys_message),
                                // only receive messages of this type
                                enum sys_message listens);

/*!
	\brief Unregisters a node from the message bus.
	\sa sys_messagebus_register
*/
void sys_messagebus_unregister(
                                  // the same callback used on sys_messagebus_register()
                                  void (*callback) (enum sys_message)
    );

#endif
