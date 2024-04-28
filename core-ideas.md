# Ada - Automated dynamic analysis

The task is split up into 3 different components
- Agent - platform specific Component responsible for monitoring the process.
- Handler - Cross platform component that  collects info from the Agent, coordinates what to do and acts as a Bridge between the GUI and the Agent
- GUI - Cross platform Component that communicates with the handler.
  - reference implementation is implemented as a Web UI

# Overview - Windows

How is the agent installed?

## Static initialization
1. Start the target process in a suspended state
2. Start a new thread (the only one that won't be suspended at that moment in time)
3. Init runs and starts waiting for modules that should be intercepted according to the current monitoring configuration
4. All threads are unsuspended

## Dynamic initialization
1. Process is assumed to be unsuspended
2. Suspend every Thread
3. Go to step 2 of the static initialization
  - downside of Dynamic init: You cannot monitor objects created prior to loading the Agent.