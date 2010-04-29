#include "Exception.hpp"

namespace cj
{

GameException::GameException(const char *msg)
: description(msg)
{}

GameException::~GameException()
{}

const char *GameException::Message(void) const
{  return description;  }

PathfindException::PathfindException(const char *msg): GameException(msg)
{}

}// cj

