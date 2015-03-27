#include <utility>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <iostream>

#include "micter.hpp"

class MailParser {
  typedef std::pair<std::string, std::string> Header;
  typedef std::vector<Header> MailHeaders;
  typedef std::map<std::string, int> WordCount;

  static void AddHeader(MailHeaders &mh, const std::string &line){
    size_t pos = line.find(':');
    if(pos == std::string::npos) return;
    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    mh.push_back(make_pair(name, value));
  }

  // RFCに従っていない場合も多いので大雑把にメールヘッダをパース
  // RFC822チェックはしない
  static MailHeaders ParseMailHeader(const std::string &maildata){
    MailHeaders mh;

    size_t pos = maildata.find("\r\n\r\n");
    if(pos == std::string::npos)
      return mh;

    std::istringstream iss(maildata.substr(0, pos + 1));
    std::string line, current;
    while(!std::getline(iss, line)){
      if(line[0] == '\t' || line[0] == ' '){
        //改行されたfield-bodyはくっつける
        current += "\r\n";
        current += line;
        continue;
      }
      if(!current.empty())
        AddHeader(mh, current);
      current = line;
    }
    if(!current.empty())
      AddHeader(mh, current);

    return mh;
  }

  //micterを使ってBodyテキストの分かち書きをする
  static WordCount ParseMailBody(micter::micter &mic,
                                 const std::string &maildata){
    WordCount wc;

    size_t pos = maildata.find("\r\n\r\n");
    if(pos == std::string::npos)
      return wc;

    std::string body = maildata.substr(pos + 4);

    std::vector<std::string> res;
    mic.split(body, &res);
    for(std::vector<std::string>::iterator itr = res.begin();
        itr != res.end(); ++itr){
      wc[*itr]++;
    }

    return wc;
  }

};
