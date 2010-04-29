#ifndef __GAME_EXCEPTION__
#define __GAME_EXCEPTION__
namespace cj
{
//**************** GAME EXCEPTION
class GameException
{
   public:
      GameException(const char *msg);
      virtual ~GameException();
      virtual const char *Message(void) const;

   private:
      const char *description;
};// 

class PathfindException : public GameException
{
   public:
      PathfindException(const char * msg);
};// 

}// cj
#endif// inc

