/*

tartunit.h -- Tiny Actor Run-Time Unit Test Framework

"MIT License"

Copyright (c) 2013 Dale Schumacher, Tristan Slominski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef _TARTUNIT_H_
#define _TARTUNIT_H_

#include <tart.h>
#include <actor.h>
#include <expr.h>
#include <number.h>

typedef struct tunit_config TUNIT_CONFIG, *TUnitConfig;
typedef struct tunit_expected_event TUNIT_EXPECTED_EVENT, *TUnitExpectedEvent;

typedef Actor (*TUnitEventExpectation)(Event expected, Event actual);

struct tunit_config {
    CONFIG      config;
    Actor       history;            // list of events already processed
    Actor       expected_events;    // list of expected events
};

struct tunit_expected_event {
    ACTOR                    _act;
    Event                    event;
    TUnitEventExpectation    expectation;
};

extern TUnitConfig  tunit_config_new();
extern void         tunit_config_enqueue(TUnitConfig cfg, Actor e);
extern void         tunit_config_enlist(TUnitConfig cfg, Actor a);
extern void         tunit_config_send(TUnitConfig cfb, Actor target, Actor msg);
extern Actor        tunit_config_dispatch(TUnitConfig cfg);

extern Actor        tunit_event_new(TUnitConfig cfg, Actor target, Actor msg);

extern TUnitExpectedEvent tunit_expected_event_new(Event event, TUnitEventExpectation expectation);

extern Actor        tunit_event_targets_equal(Event expected, Event actual);

#ifdef  TARTUNIT 

#define TEST(test_body) ({ \
    /* create a new isolated configuration for the test */ \
    TUnitConfig __tunit_config = tunit_config_new(); \
    /* insert the test body */ \
    ({test_body}); \
    /* dispatch events until empty */ \
    while (tunit_config_dispatch(__tunit_config) == a_true) \
        ; \
    /* check that there are no left-over expected events */ \
    /* TODO: make this perhaps more meaningful instead of simple assertion fail */ \
    assert(__tunit_config->expected_events == a_empty_list); \
    /* TODO: clean up memory */ \
})

#define TEST_SEND(target, message) ({ \
    tunit_config_send(__tunit_config, target, message); \
})

#define EXPECT_EVENT(target) ({ \
    /* check configuration history to see if the event already occurred */ \
    Actor history = __tunit_config->history; \
    Actor already_occurred = a_false; \
    Pair pair; \
    while (history != a_empty_list) { \
        pair = list_pop(history); \
        Event e = (Event)pair->h; \
        /* TODO: elaborate this after mechanism validation */ \
        if (SELF(e) == target) { \
            already_occurred = a_true; \
            break; \
        } \
        history = (Actor)pair->t; \
    } \
    /* if event not found in history, add to expected events */ \
    if (already_occurred == a_false) { \
        __tunit_config->expected_events = \
            list_push( \
                __tunit_config->expected_events, \
                (Actor)tunit_expected_event_new( \
                    (Event)event_new((Config)__tunit_config, target, (Any)0), \
                    tunit_event_targets_equal)); \
    } \
})

#else /* TARTUNIT */

#define TEST(test_body)
#define TEST_SEND(target, message)
#define EXPECT_EVENT(target)

#endif /* TARTUNIT */

#endif /* _TARTUNIT_H_ */