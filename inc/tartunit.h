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

/**
Expectation [*|*]
             | +--> DATA
             V
            CODE
Note: Expectation is simply a Value. Expectation message protocol consists of
accepting only Request actor: request_new(ok, fail, req), where `ok` is success
actor, `fail` is fail actor, and `req` is the event to test expectations against.
**/

typedef struct tunit_config TUNIT_CONFIG, *TUnitConfig;

struct tunit_config {
    CONFIG          config;
    Actor           history;        // list of events already processed
    Actor           expectations;   // list of expected events
};

extern void         val_tunit_runner(Event e);
extern Actor        tunit_runner_new();

extern TUnitConfig  tunit_config_new();
extern void         tunit_config_enqueue(TUnitConfig cfg, Actor e);
extern void         tunit_config_enlist(TUnitConfig cfg, Actor a);
extern void         tunit_config_send(TUnitConfig cfb, Actor target, Actor msg);
extern Actor        tunit_config_dispatch(TUnitConfig cfg);

#ifdef  TARTUNIT 

#define TEST(test_body) ({ \
    /* create a new isolated configuration for the test */ \
    TUnitConfig __tunit_config = tunit_config_new(); \
    /* insert the test body */ \
    ({test_body}); \
    /* dispatch events until empty */ \
    while (tunit_config_dispatch(__tunit_config) != NOTHING) \
        ; \
    /* check that there are no left-over expected events */ \
    /* TODO: make this perhaps more meaningful instead of simple assertion fail */ \
    assert(__tunit_config->expectations == a_empty_list); \
    /* TODO: clean up memory */ \
})

#define TEST_SEND(target, message) ({ \
    tunit_config_send(__tunit_config, target, message); \
})

#define EXPECT_EVENT(expectation) ({ \
    /* check configuration history to see if the event already occurred */ \
    Actor already_occurred = a_false; \
    Actor history = __tunit_config->history; \
    Pair pair; \
    while (history != a_empty_list) { \
        pair = list_pop(history); \
        Event e = (Event)pair->h; \
        Actor a_runner = tunit_runner_new(); \
        Config config = config_new(); \
        config_send(config, expectation, PR(a_runner, (Actor)e)); \
        while (config_dispatch(config) != NOTHING) \
            ; \
        \
        /* if expectation against history is met, event already occurred */ \
        Pair data = (Pair)DATA(a_runner); \
        if (data->h /*ok*/ == a_true && data->t /*fail*/ != a_true) { \
            already_occurred = a_true; \
            break; \
        } \
        history = (Actor)pair->t; \
    } \
    /* if event not found in history, add to expected events */ \
    if (already_occurred == a_false) { \
        __tunit_config->expectations = \
            list_push(__tunit_config->expectations, expectation); \
    } \
})

#else /* TARTUNIT */

#define TEST(test_body)
#define TEST_SEND(target, message)
#define EXPECT_EVENT(target)

#endif /* TARTUNIT */

#endif /* _TARTUNIT_H_ */