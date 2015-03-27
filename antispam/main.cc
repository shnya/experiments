#include "micter.hpp"

using namespace std;




int main(int argc, char *argv[])
{
  micter::micter mic;
  mic.load("micter.model");
  vector<string> res;
  mic.split("本日は晴天なり。", &res);

  for(vector<string>::iterator itr = res.begin();
      itr != res.end(); ++itr){
    cout << *itr << endl;
  }
  return 0;
}
