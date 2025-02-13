## Overview - Windows

How is the agent installed?

### Static initialization
1. Start the target process in a suspended state
2. Start a new thread (the only one that won't be suspended at that moment in time)
3. Init runs and starts waiting for modules that should be intercepted according to the current monitoring configuration
4. All threads are unsuspended

### Dynamic initialization
1. Process is assumed to be unsuspended
2. Suspend every Thread
3. Go to step 2 of the static initialization
  - downside of Dynamic init: You cannot monitor objects created prior to loading the Agent.