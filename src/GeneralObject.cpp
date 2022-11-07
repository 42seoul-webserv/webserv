#include "GenericObject.hpp"

ft::BaseObject::BaseObject(ft::ObjectType type):
  _type(type)
{
}

ft::ObjectString::ObjectString(std::string& str):
  BaseObject(TYPE_STRING),
  std::string(str)
{
}

ft::ObjectVector::ObjectVector(std::vector<std::string>& vec):
  BaseObject(TYPE_LIST),
  std::vector<std::string>(vec)
{
}

ft::Object::Object():
  BaseObject(TYPE_OBJECT),
  std::map<std::string, BaseObject>()
{
}
