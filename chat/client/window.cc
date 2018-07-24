#include"window.h"
#include<locale.h>
#include "../common/util.hpp"
namespace  client
{
  Window::Window()
  {
    setlocale(LC_ALL, "");//设置字符集
    initscr();
    curs_set(0);

  }
  Window::~Window()
  {
    endwin();//销毁窗

  }
  void Window::DrawHeader()
  {
    int h = LINES / 5;
    int w = COLS;
    int y = 0;
    int x = 0;
    header_win_ = newwin(h, w, y, x);
    std::string title = "畅所欲言";
    PutStrTowin(header_win_, h / 2, w / 2 - title.size() / 2, title.c_str());
    box(header_win_, '|', '-');
    wrefresh(header_win_);

  }
  void Window::DrawInput()
  {
    int h = LINES / 5;
    int w = COLS;
    int y = LINES * 4 / 5;
    int x = 0;
    input_win_ = newwin(h, w, y, x);
    std::string title = "请输入消息";

    box(input_win_, '|', '-');
    PutStrTowin(input_win_, 1, 2, title.c_str());
    wrefresh(input_win_);

  }
  void Window::DrawOutput()
  {
    int h = LINES * 3 / 5;
    int w = COLS * 3 / 4;
    int y = LINES / 5;
    int x = 0;
    output_win_ = newwin(h, w, y, x);
    box(output_win_, '|', '-');

    for (size_t i = 0; i < msgs_.size(); ++i)
    {
      PutStrTowin(output_win_, i + 1, 2, msgs_[i]);

    }
    wrefresh(output_win_);

  }
  void Window::DrawFriendList()
  {
    int h = LINES * 3 / 5;
    int w = COLS / 4;
    int y = LINES / 5;
    int x = COLS * 3 / 4;
    friend_list_win_ = newwin(h, w, y, x);
    box(friend_list_win_, '|', '-');
    size_t i = 0;
    for (auto item : friend_list_)
    {
      PutStrTowin(friend_list_win_, i + 1, 1, item);
      ++i;

    }
    wrefresh(friend_list_win_);

  }
  void Window::PutStrTowin(WINDOW* win, int y, int x, const std::string&str)
  {
    mvwaddstr(win, y, x, str.c_str());

  }
  void Window::GetStrFromWin(WINDOW* win, std::string*str)
  {
    char buf[1024 * 10] = { 0  };
    wgetnstr(win, buf, sizeof(buf)-1);//读取
    *str = buf;

  }
  void Window::AddMsg(const std::string&msg)
  {
    msgs_.push_back(msg);
    int max_line = LINES * 3 / 5 - 2;
    if (max_line < 3)
    {
      LOG(ERROR) << "max_line too small\n";
      exit(0);

    }
    if (msgs_.size() > max_line)
    {
      msgs_.pop_front();

    }

  }
  void Window::AddFriend(const std::string& friend_info)
  {
    friend_list_.insert(friend_info);

  }
  void Window::DelFriend(const std::string& friend_info)
  {
    friend_list_.erase(friend_info);

  }

}//end client 

//#include<unistd.h>
//int main()
//{
//  client::Window win;
//  win.DrawHeader();
//  win.DrawInput();
//  win.AddMsg("chan1:haha");
//  win.AddMsg("chan2:haha");
//  win.AddMsg("chan3:haha");
//  win.AddMsg("chan4:haha");
//  win.AddMsg("chan5:haha");
//  win.DrawOutput();
//  win.AddFriend("chan1|陕科大");
//  win.AddFriend("chan2|陕科大");
//  win.AddFriend("chan3|陕科大");
//  win.AddFriend("chan4|陕科大");
//  win.DrawFriendList();
//  sleep(5);
//  return 0;
//
//}
//
