#include "input.hpp"


static std::vector<KeyEvent> g_EventQueueHack;

void inputPushEvent(KeyEvent event)
{
   g_EventQueueHack.push_back(event);
}
std::vector<KeyEvent> inputFlushQueue()
{
   std::vector<KeyEvent> out;
   std::swap(g_EventQueueHack, out);
   return out;
}