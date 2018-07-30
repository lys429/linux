
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

      void DrawHeader();////绘制标题窗
      void DrawInput();//绘制输入栏
      void DrawOutput();//绘制输出消息栏
      void DrawFriendList();//绘制好友列表栏
      void PutStrTowin(WINDOW* win, int y, int x, const std::string& str);//将字符串写进窗口
      void GetStrFromWin(WINDOW* win, std::string*str);//从窗口得到一个信息
      void AddMsg(const std::string& msg);//添加消息到好友列表
      void AddFriend(const std::string& friend_info);//添加好友
      void DelFriend(const std::string& friend_info);//删除好友

      WINDOW* header_win_;
      WINDOW* input_win_;
      WINDOW* output_win_;
      WINDOW* friend_list_win_;//好友列表

    private:
      std::deque<std::string> msgs_;//适配器
      std::unordered_set<std::string> friend_list_;

  };

} 
