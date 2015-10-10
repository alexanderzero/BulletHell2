#pragma once

class EntitySystem;
class NameIndex;

struct EntitySystemView
{
   //actual system
   EntitySystem* system;

   //managers
   NameIndex* nameIndex;
};
