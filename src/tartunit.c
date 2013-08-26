/*

tartunit.c -- Tiny Actor Run-Time

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

#define  TARTUNIT    /* turn tests on (#define) and off (#undef) */

#include "tartunit.h"

void
halt(char * msg)
{
    TRACE(fprintf(stderr, "%s **** HALTED ****\n", msg));
    assert(0);
    for (;;)    // loop forever
        ;
}

inline TUnitConfig
tunit_config_new()
{
    TUnitConfig tunit_cfg = NEW(TUNIT_CONFIG);
    BEH(tunit_cfg) = beh_config;
    Config cfg = (Config)tunit_cfg;
    cfg->events = deque_new();
    cfg->actors = list_new();
    tunit_cfg->history = list_new();
    tunit_cfg->expectations = list_new();
    return tunit_cfg;
}
inline void
tunit_config_enqueue(TUnitConfig tunit_cfg, Actor e)
{
    if (beh_event != BEH(e)) { halt("tunit_config_enqueue: event actor required"); }
    deque_give(((Config)tunit_cfg)->events, (Actor)e);
}
inline void
tunit_config_enlist(TUnitConfig tunit_cfg, Actor a)
{
    ((Config)tunit_cfg)->actors = list_push(((Config)tunit_cfg)->actors, a);
}
inline void
tunit_config_send(TUnitConfig tunit_cfg, Actor target, Actor msg)
{
    if (beh_config != BEH(tunit_cfg)) { halt("tunit_config_send: tunit_config actor required"); }
    TRACE(fprintf(stderr, "tunit_config_send: actor=%p, msg=%p\n", target, msg));
    tunit_config_enqueue(tunit_cfg, event_new((Config)tunit_cfg, target, msg));
}
Actor
tunit_config_dispatch(TUnitConfig tunit_cfg)
{
    Actor a_event = config_dispatch((Config)tunit_cfg);
    if (a_event == NOTHING) { 
        return a_event; 
    }
    
    /* test to see if the event is expected */
    // TRACE(fprintf(stderr, "tunit_config_dispatch: expectations=%p\n", tunit_cfg->expectations));
    Actor expectations = tunit_cfg->expectations;
    // TODO: abusing lists here due to lack of Set as a data structure
    Pair pair;
    Pair previous = (Pair)0; // to facilitate element removal
    // TRACE(fprintf(stderr, "tunit_config_dispatch: previous=%p\n", previous));
    while (expectations != a_empty_list) {
        pair = list_pop(expectations);
        Actor a_expectation = pair->h;
        // TODO: rewrite to use single customer... 
        Actor a_runner = tunit_runner_new();
        Config config = config_new();
        // TODO: think of sending a_event between machines... don't share memory
        config_send(config, a_expectation, PR(a_runner, a_event));
        while (config_dispatch(config) != NOTHING)
            ;

        // if expectation was met and did not fail, event is expected
        Pair data = (Pair)DATA(a_runner);
        if (data->h /*ok*/ == a_true && data->t /*fail*/ != a_true) {
            // TRACE(fprintf(stderr, "tunit_config_dispatch: met expectation\n"));
            /* remove matched expectation from the list */
            if (previous) { // in the middle of the list
                // TRACE(fprintf(stderr, "tunit_config_dispatch: removing from middle of list\n"));
                previous->t = pair->t;
            } else { // first item in list
                // TRACE(fprintf(stderr, "tunit_config_dispatch: removing from front of list\n"));
                tunit_cfg->expectations = pair->t;
            }
            break;
        } 
        previous = pair;
        expectations = (Actor)pair->t;
        // TODO: clean up memory from this internal configuration run
    }

    tunit_cfg->history = list_push(tunit_cfg->history, a_event); // PLACE PROCESSED EVENT IN HISTORY
    return a_event;
}

void
val_tunit_runner(Event e)
{
    TRACE(fprintf(stderr, "val_tunit_runner{event=%p}\n", e));
    Pair pair = DATA(SELF(e));
    if (MSG(e) == a_true) {
        pair->h = a_true;
    } else if (MSG(e) == a_false) {
        pair->t = a_true;
    } else {
        expr_value(e);
    }
}
Actor
tunit_runner_new()
{
    Value val = NEW(VALUE);
    BEH(val) = val_tunit_runner;
    DATA(val) = PR(NOTHING, NOTHING);
    return (Actor)val;
}

void
beh_tunit_expected_event(Event e)
{

}

static void
val_event_targets_equal(Event e)
{
    TRACE(fprintf(stderr, "val_event_targets_equal{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (beh_pair != BEH(MSG(e))) { halt("val_event_targets_equal: pair msg required"); }
    Pair msg = (Pair)MSG(e);
    Actor a_cust = msg->h;
    Actor expected = (Actor)DATA(SELF(e));
    Event actual = (Event)msg->t;
    if (expected == SELF(actual)) {
        config_send(SPONSOR(e), a_cust, a_true);
    } else {
        config_send(SPONSOR(e), a_cust, a_false);
    }
}

static void
val_event_target_and_message_equal(Event e)
{
    TRACE(fprintf(stderr, "val_event_target_and_message_equal{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (beh_pair != BEH(MSG(e))) { halt("val_event_target_and_message_equal: pair msg required"); }
    Pair msg = (Pair)MSG(e);
    Actor a_cust = msg->h;
    Event expected = (Event)DATA(SELF(e));
    Event actual = (Event)msg->t;
    if (expected->target == actual->target && expected->message == actual->message) {
        config_send(SPONSOR(e), a_cust, a_true);
    } else {
        config_send(SPONSOR(e), a_cust, a_false);
    }
}

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

static void
val_forward(Event e)
{
    TRACE(fprintf(stderr, "val_forward{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    config_send(SPONSOR(e), DATA(SELF(e)), MSG(e));
}

/*
 *  Main entry-point
 */

int
main()
{
    /* tunit unit tests */    
    TRACE(fprintf(stderr, "a_true: %p, a_false: %p, NOTHING: %p\n", a_true, a_false, NOTHING));
    
    /* TEST macro test */
    TRACE(fprintf(stderr, "--- TEST macro test ---\n"));
    TUnitConfig __tunit_config = tunit_config_new();
    while (tunit_config_dispatch(__tunit_config) != NOTHING)
        ;
    assert(__tunit_config->expectations == a_empty_list);

    /* EXPECT_EVENT(expectation) macro test - (event targets equal) */
    TRACE(fprintf(stderr, "--- <expanded> EXPECT_EVENT(expectation) macro test - (event targets equal) ---\n"));
    Actor a_my_ignore = actor_new(beh_ignore);
    Actor a_expectation = value_new(val_event_targets_equal, a_my_ignore);
    Actor already_occurred = a_false;
    Actor history = __tunit_config->history;
    Pair pair;
    while (history != a_empty_list) {
        pair = list_pop(history);
        Event e = (Event)pair->h;
        Actor a_runner = tunit_runner_new();
        Config config = config_new();
        config_send(config, a_expectation, PR(a_runner, (Actor)e));
        while (config_dispatch(config) != NOTHING)
            ;

        /* if expectation against history is met, event already occurred */
        Pair data = (Pair)DATA(a_runner);
        if (data->h /*ok*/ == a_true && data->t /*fail*/ != a_true) {
            already_occurred = a_true;
            break;
        }
        history = (Actor)pair->t;
    }
    if (already_occurred == a_false) {
        __tunit_config->expectations =
            list_push(__tunit_config->expectations, a_expectation);
    }
    while (tunit_config_dispatch(__tunit_config) != NOTHING)
        ;
    assert(__tunit_config->expectations != a_empty_list);

    /* Actual macro usage test */
    TRACE(fprintf(stderr, "--- EXPECT_EVENT(expectation) macro test - (event targets equal) ---\n"));
    TEST(
        EXPECT_EVENT(a_expectation);
        TEST_SEND(a_my_ignore, (Actor)0);
    );

    /* EXPECT_EVENT(expectation) macro test -- (target and message equal) */
    TRACE(fprintf(stderr, "--- EXPECT_EVENT(expectation) macro test - (target and message equal) ---\n"));
    Actor a_expected_event = event_new((Config)__tunit_config, a_my_ignore, (Actor)0);
    Actor a_expect_target_and_msg_equal_expectation = value_new(val_event_target_and_message_equal, a_expected_event);
    TEST(
        EXPECT_EVENT(a_expect_target_and_msg_equal_expectation);
        TEST_SEND(a_my_ignore, (Actor)0);
    );

    TRACE(fprintf(stderr, "--- indirect EXPECT_EVENT(expectation) macro test ---\n"));
    Actor a_second_ignore = actor_new(beh_ignore);
    Actor a_first_forward = value_new(val_forward, a_second_ignore);
    Actor a_expect_target_is_first_forward = value_new(val_event_targets_equal, a_first_forward);
    Actor a_expect_target_is_second_ignore = value_new(val_event_targets_equal, a_second_ignore);
    TEST(
        EXPECT_EVENT(a_expect_target_is_second_ignore);
        TEST_SEND(a_first_forward, (Actor)0);
    );

    TRACE(fprintf(stderr, "--- multiple EXPECT_EVENT(expectation) macro test ---\n"));
    TEST(
        EXPECT_EVENT(a_expect_target_is_second_ignore);
        TEST_SEND(a_first_forward, (Actor)0);
        EXPECT_EVENT(a_expect_target_is_first_forward);
    );

    return 0;
}