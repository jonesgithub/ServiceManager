#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <sys/types.h>
extern "C" {
#include <sys/event.h>
}
#include <sys/wait.h>
#include <vector>

#include "s16.h"
#include "s16db.h"
#include "s16rr.h"
#include "state.h"

class SvcManager
{
    SvcStateFactory m_state_factory;
    std::vector<pid_t> m_pids;
    std::vector<std::shared_ptr<SvcState> > m_state_stack;
    /* Here, we store a pair of timer ID and the callback function. */
    std::vector<std::pair<unsigned int, std::function<bool(unsigned int)> > >
        m_timers;
    /* This is the service we're running on. */
    svc_t * m_svc;
    /* The type of the service - simple, forking, etc. */
    SvcTypes m_type;
    /* A reference to the System Dr. manager object. */
    class SystemDr & m_sd;

  public:
    /* The present primary PID we're monitoring. */
    pid_t main_pid;
    /* The start and stop timeouts. 90 by default. */
    unsigned int timeout_start, timeout_stop;

    int fork_register_exec (const char * exe);
    void launch ();
    void register_timer (unsigned int sec,
                         std::function<bool(unsigned int)> cb);
    void register_pid (pid_t pid);
    void deregister_pid (pid_t pid);
    int pids_relevant (pid_t one, pid_t two)
    {
        for (std::vector<pid_t>::iterator it = m_pids.begin ();
             it != m_pids.end (); it++)
        {
            if (*it == one | *it == two)
            {
                return 1;
            }
        }
        return 0;
    }
    void process_event (pt_info_t * pt)
    {
        m_state_stack.back ()->process_event (pt);
    }
    SvcManager (SystemDr & sd, svc_t * svc);
};

class SystemDr
{
    friend class SvcManager;
    std::vector<SvcManager> m_managers;
    CLIENT * m_clnt;
    int m_kq;
    process_tracker_t * m_ptrack;

  public:
    SystemDr (CLIENT * clnt);
    ~SystemDr ();
    void main_loop ();
    void add_svc (svc_t * svc)
    {
        m_managers.push_back (SvcManager (*this, svc));
    }
};

#endif
