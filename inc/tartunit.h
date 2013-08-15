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

#include "tart.h"

typedef struct tartunit_config TARTUNIT_CONFIG, *TartunitConfig;
typedef struct tartunit_expected_event TARTUNIT_EXPECTED_EVENT, *TartunitExpectedEvent;

typedef Boolean (*TartunitEventExpectation)(Event expected, Event actual);

struct tartunit_config {
    CONFIG      config;
    Actor       history;            // list of events already processed
    Actor       expected_events;    // list of expected events
};

struct tartunit_expected_event {
    ACTOR                       _act;
    Event                       event;
    TartunitEventExpectation    expectation;
};

extern TartunitConfig   tartunit_config_new();
extern void             tartunit_config_enqueue(TartunitConfig cfg, Event e);
extern void             tartunit_config_enlist(TartunitConfig cfg, Actor a);
extern void             tartunit_config_send(TartunitConfig cfb, Actor target, Any msg);
extern Boolean          tartunit_config_dispatch(TartunitConfig cfg);

extern Event    tartunit_event_new(TartunitConfig cfg, Actor target, Any msg);

extern TartunitExpectedEvent tartunit_expected_event_new(Event event, TartunitEventExpectation expectation);

extern Boolean  tartunit_event_targets_equal(Event expected, Event actual);

#ifdef  TARTUNIT 

#define TEST(test_body) ({ \
    /* create a new isolated configuration for the test */ \
    TartunitConfig __tartunit_config = tartunit_config_new(); \
    /* insert the test body */ \
    ({test_body}); \
    /* dispatch events until empty */ \
    while (tartunit_config_dispatch(__tartunit_config) == a_true) \
        ; \
    /* check that there are no left-over expected events */ \
    /* TODO: make this perhaps more meaningful instead of simple assertion fail */ \
    assert(__tartunit_config->expected_events == a_empty_list); \
    /* TODO: clean up memory */ \
})

#define TEST_SEND(target, message) ({ \
    tartunit_config_send(__tartunit_config, target, message); \
})

#define EXPECT_EVENT(target) ({ \
    /* check configuration history to see if the event already occurred */ \
    Actor history = __tartunit_config->history; \
    Boolean already_occurred = a_false; \
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
        __tartunit_config->expected_events = \
            list_push( \
                __tartunit_config->expected_events, \
                tartunit_expected_event_new( \
                    event_new((Config)__tartunit_config, target, (Any)0), \
                    tartunit_event_targets_equal)); \
    } \
})

#else /* TARTUNIT */

#define TEST(test_body)
#define TEST_SEND(target, message)
#define EXPECT_EVENT(target)

#endif /* TARTUNIT */

#endif /* _TARTUNIT_H_ */