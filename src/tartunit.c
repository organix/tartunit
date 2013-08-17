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
    tunit_cfg->expected_events = list_new();
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
    if (beh_config != BEH(tunit_cfg)) { halt("tunit_config_dispatch: tunit_config actor required"); }
    if (deque_empty_p(((Config)tunit_cfg)->events) != a_false) {
        TRACE(fprintf(stderr, "tunit_config_dispatch: <EMPTY>\n"));
        return a_false;
    }
    Event e = (Event)deque_take(((Config)tunit_cfg)->events);
    TRACE(fprintf(stderr, "tunit_config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
    
    /* test to see if the event is expected */
    // TRACE(fprintf(stderr, "tunit_config_dispatch: expected_events=%p\n", tunit_cfg->expected_events));
    Actor expected_events = tunit_cfg->expected_events;
    Actor expected = a_false;
    Pair pair;
    Pair previous = (Pair)0; // to facilitate element removal
    // TRACE(fprintf(stderr, "tunit_config_dispatch: previous=%p\n", previous));
    while (expected_events != a_empty_list) {
        pair = list_pop(expected_events);
        TUnitExpectedEvent expected_event = (TUnitExpectedEvent)pair->h;
        if (expected_event->expectation(expected_event->event, e) == a_true) {
            // TRACE(fprintf(stderr, "tunit_config_dispatch: met expectation\n"));
            /* remove matched event from the list */
            if (previous) { // in the middle of the list
                // TRACE(fprintf(stderr, "tunit_config_dispatch: removing from middle of list\n"));
                previous->t = pair->t;
            } else { // first item in list
                // TRACE(fprintf(stderr, "tunit_config_dispatch: removing from front of list\n"));
                tunit_cfg->expected_events = pair->t;
            }  
            break;
        }
        previous = pair;
        expected_events = (Actor)pair->t;
    }

    (CODE(SELF(e)))(e); // INVOKE BEHAVIOR
    tunit_cfg->history = list_push(tunit_cfg->history, (Actor)e); // PLACE PROCESSED EVENT IN HISTORY
    return a_true;
}

void
beh_tunit_expected_event(Event e)
{

}

inline TUnitExpectedEvent
tunit_expected_event_new(Event event, TUnitEventExpectation expectation)
{
    TUnitExpectedEvent expected_event = NEW(TUNIT_EXPECTED_EVENT);
    BEH(expected_event) = beh_tunit_expected_event;
    expected_event->event = event;
    expected_event->expectation = expectation;
    return expected_event;
}

Actor
tunit_event_targets_equal(Event expected, Event actual)
{
    if (SELF(expected) == SELF(actual)) { return a_true; }
    return a_false;
}

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

/*
 *  Main entry-point
 */

int
main()
{
    /* tunit unit tests */    
    
    /* TEST macro test */
    TUnitConfig __tunit_config = tunit_config_new();
    while (tunit_config_dispatch(__tunit_config) == a_true)
        ;
    assert(__tunit_config->expected_events == a_empty_list);

    /* EXPECT_EVENT macro test */
    Actor a_my_ignore = actor_new(beh_ignore);
    Actor history = __tunit_config->history;
    Actor already_occurred = a_false;
    Pair pair;
    while (history != a_empty_list) {
        pair = list_pop(history);
        Event e = (Event)pair->h;
        if (SELF(e) == a_my_ignore) {
            already_occurred = a_true;
            break;
        }
        history = (Actor)pair->t;
    }

    if (already_occurred == a_false) {
        __tunit_config->expected_events =
            list_push(
                __tunit_config->expected_events,
                (Actor)tunit_expected_event_new(
                    (Event)event_new((Config)__tunit_config, a_my_ignore, (Actor)0),
                    tunit_event_targets_equal));
    }
    while (tunit_config_dispatch(__tunit_config) == a_true)
        ;
    assert(__tunit_config->expected_events != a_empty_list);

    /* Actual macro usage test */
    TEST(
        EXPECT_EVENT(a_my_ignore);
        TEST_SEND(a_my_ignore, (Actor)0);
    );

    return 0;
}