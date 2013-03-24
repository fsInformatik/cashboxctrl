/* Name: main.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include "usbdrv.h"
#include "../communication.h"

#define OPENER_OUTPUT       PORTB
#define OPENER_DDR          DDRB
#define OPENER_PIN          0

#define BUTTON_INPUT        PORTB
#define BUTTON_DDR          DDRB
#define BUTTON_PIN

#define DRAWER_STATE_INPUT  PORTB
#define DRAWER_STATE_DDR    DDRB
#define DRAWER_STATE_PIN

void cash_box_open(void)
{
    OPENER_OUTPUT |= _BV(OPENER_PIN);
    // set up timer with prescaler = 64 and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS11)|(1 << CS10);
    // initialize counter
    TCNT1 = 0;
    // initialize compare value
    OCR1A = 100;
    // enable compare interrupt
    TIMSK |= (1 << OCIE1A);
}

ISR (TIMER1_COMPA_vect)
{
    OPENER_OUTPUT &= _BV(OPENER_PIN);
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;
    switch(rq->bRequest)
    {
        case RQ_CASH_BOX_OPEN:
            cash_box_open();
            return 0;
    }
    return 0;
}

int __attribute__((noreturn)) main(void)
{
    uint8_t i;

    OPENER_DDR |= _BV(OPENER_PIN);

    OPENER_OUTPUT |= _BV(OPENER_PIN);
    _delay_ms(100);
    OPENER_OUTPUT &= ~_BV(OPENER_PIN);

    wdt_enable(WDTO_60MS);
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    for(i = 0; i < 250; i++)
    {             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(2);
    }

    usbDeviceConnect();

    sei();

    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
    }
}

