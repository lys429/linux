
#include<ncurses.h>
#include<string>
#include<deque>
#include<unordered_set>
namespace client{
  class Window
  {
    public:
      Window();
      ~Window();

      void DrawHeader();
      void DrawInput();
      void DrawOutput();
      void DrawFriendList();
      void PutStrTowin(WINDOW* win, int y, int x, const std::string&str);
      void GetStrFromWin(WINDOW* win, std::string*str);
      void AddMsg(const std::string&msg);
      void AddFriend(const std::string& friend_info);
      void DelFriend(const std::string& friend_info);

      WINDOW* header_win_;
      WINDOW* input_win_;
      WINDOW* output_win_;
      WINDOW* friend_list_win_;

    private:
      std::deque<std::string> msgs_;//适配器
      std::unordered_set<std::string> friend_list_;

  };

} 
