#ifndef GENERAL_OBJECT_HPP
 #define GENERAL_OBJECT_HPP

#include <map>
#include <vector>
#include <string>

namespace ft
{
  typedef enum e_objectType
  {
    TYPE_STRING,
    TYPE_OBJECT,
    TYPE_LIST
  } ObjectType;

  class BaseObject
  {
  protected:
      ObjectType _type;
  public:
      BaseObject(ObjectType type);
      virtual ~BaseObject();
  };

  class ObjectString : virtual public BaseObject, virtual public std::string
  {
  public:
      ObjectString(std::string& str);
      virtual ~ObjectString();
  };

  class ObjectVector : virtual public BaseObject, virtual public std::vector<std::string>
  {
  public:
      ObjectVector(std::vector<std::string>& vec);
      virtual ~ObjectVector();
  };

  class Object : virtual public BaseObject, virtual public std::map<std::string, BaseObject>
  {
  public:
      Object();
      virtual ~Object();
  };
};


#endif
