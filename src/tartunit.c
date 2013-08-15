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

#define TARTUNIT    /* turn tests on and off */
#include "tart.h"
#include "actor.h"
#include "expr.h"
#include "number.h"
#include "tartunit.h"

void
halt(char * msg)
{
    TRACE(fprintf(stderr, "%s **** HALTED ****\n", msg));
    assert(0);
    for (;;)    // loop forever
        ;
}

inline TartunitConfig
tartunit_config_new()
{
    TartunitConfig tartunit_cfg = NEW(TARTUNIT_CONFIG);
    BEH(tartunit_cfg) = beh_config;
    Config cfg = (Config)tartunit_cfg;
    cfg->events = deque_new();
    cfg->actors = list_new();
    tartunit_cfg->history = list_new();
    tartunit_cfg->expected_events = list_new();
    return tartunit_cfg;
}
inline void
tartunit_config_enqueue(TartunitConfig tartunit_cfg, Event e)
{
    if (beh_event != BEH(e)) { halt("tartunit_config_enqueue: event actor required"); }
    deque_give(((Config)tartunit_cfg)->events, e);
}
inline void
tartunit_config_enlist(TartunitConfig tartunit_cfg, Actor a)
{
    ((Config)tartunit_cfg)->actors = list_push(((Config)tartunit_cfg)->actors, a);
}
inline void
tartunit_config_send(TartunitConfig tartunit_cfg, Actor target, Any msg)
{
    if (beh_config != BEH(tartunit_cfg)) { halt("tartunit_config_send: tartunit_config actor required"); }
    TRACE(fprintf(stderr, "tartunit_config_send: actor=%p, msg=%p\n", target, msg));
    tartunit_config_enqueue(tartunit_cfg, event_new((Config)tartunit_cfg, target, msg));
}
Boolean
tartunit_config_dispatch(TartunitConfig tartunit_cfg)
{
    if (beh_config != BEH(tartunit_cfg)) { halt("tartunit_config_dispatch: tartunit_config actor required"); }
    if (deque_empty_p(((Config)tartunit_cfg)->events) != a_false) {
        TRACE(fprintf(stderr, "tartunit_config_dispatch: <EMPTY>\n"));
        return a_false;
    }
    Event e = deque_take(((Config)tartunit_cfg)->events);
    TRACE(fprintf(stderr, "tartunit_config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
    
    /* test to see if the event is expected */
    // TRACE(fprintf(stderr, "tartunit_config_dispatch: expected_events=%p\n", tartunit_cfg->expected_events));
    Actor expected_events = tartunit_cfg->expected_events;
    Boolean expected = a_false;
    Pair pair;
    Pair previous = (Pair)0; // to facilitate element removal
    // TRACE(fprintf(stderr, "tartunit_config_dispatch: previous=%p\n", previous));
    while (expected_events != a_empty_list) {
        pair = list_pop(expected_events);
        TartunitExpectedEvent expected_event = (TartunitExpectedEvent)pair->h;
        if (expected_event->expectation(expected_event->event, e) == a_true) {
            // TRACE(fprintf(stderr, "tartunit_config_dispatch: met expectation\n"));
            /* remove matched event from the list */
            if (previous) { // in the middle of the list
                // TRACE(fprintf(stderr, "tartunit_config_dispatch: removing from middle of list\n"));
                previous->t = pair->t;
            } else { // first item in list
                // TRACE(fprintf(stderr, "tartunit_config_dispatch: removing from front of list\n"));
                tartunit_cfg->expected_events = pair->t;
            }  
            break;
        }
        previous = pair;
        expected_events = (Actor)pair->t;
    }

    (CODE(SELF(e)))(e); // INVOKE BEHAVIOR
    tartunit_cfg->history = list_push(tartunit_cfg->history, e); // PLACE PROCESSED EVENT IN HISTORY
    return a_true;
}

void
beh_tartunit_expected_event(Event e)
{

}

inline TartunitExpectedEvent
tartunit_expected_event_new(Event event, TartunitEventExpectation expectation)
{
    TartunitExpectedEvent expected_event = NEW(TARTUNIT_EXPECTED_EVENT);
    BEH(expected_event) = beh_tartunit_expected_event;
    expected_event->event = event;
    expected_event->expectation = expectation;
    return expected_event;
}

Boolean
tartunit_event_targets_equal(Event expected, Event actual)
{
    if (SELF(expected) == SELF(actual)) { return a_true; }
    return a_false;
}

/*
 *  Main entry-point
 */

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

int
main()
{
    /* tartunit unit tests */    
    
    /* TEST macro test */
    TartunitConfig test_tartunit_config = tartunit_config_new();
    while (tartunit_config_dispatch(test_tartunit_config) == a_true)
        ;
    assert(test_tartunit_config->expected_events == a_empty_list);

    /* EXPECT_EVENT macro test */
    Actor a_my_ignore = actor_new(beh_ignore);
    Actor history = test_tartunit_config->history;
    Boolean already_occurred = a_false;
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
        test_tartunit_config->expected_events =
            list_push(
                test_tartunit_config->expected_events,
                tartunit_expected_event_new(
                    event_new((Config)test_tartunit_config, a_my_ignore, (Any)0),
                    tartunit_event_targets_equal));
    }
    while (tartunit_config_dispatch(test_tartunit_config) == a_true)
        ;
    assert(test_tartunit_config->expected_events != a_empty_list);

    /* Actual macro usage test */
    TEST(
        EXPECT_EVENT(a_my_ignore);
        TEST_SEND(a_my_ignore, (Any)0);
    );

    return 0;
}