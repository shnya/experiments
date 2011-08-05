/*
 * WRITTEN BY Masahiko Higashiyama in 2010.
 *
 * THIS CODE IS IN PUBLIC DOMAIN.
 * THIS SOFTWARE IS COMPLETELY FREE TO COPY, MODIFY AND/OR RE-DISTRIBUTE.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fstream>
#include <string>
#include "double_array.hpp"
#include "profile.hpp"
using namespace std;


void print(const DoubleArray &da, const char *str){
  vector<int> v1;
  vector<int> v2;
  cout << "searching " << str << endl;
  da.commonPrefixSearch(str,v1,v2);
  if(v1.size() == 0) cout << "Not Found" << endl;
  for(size_t i = 0; i < v1.size(); i++)
    cout << v1[i] << " " << v2[i] << endl;
}

void read(vector<string> &v){
  string line;
  while(getline(cin,line)){
    v.push_back(line);
  }
  stable_sort(v.begin(),v.end());
}

void build(DoubleArray &da, const vector<string> &v){
  double t1,t2;
  t1 = GetusageSec();
  for(size_t i = 0; i < v.size(); i++){
    da.insert(v[i].c_str());
    //cout << i << " " << v[i] << endl;
  }
  t2 = GetusageSec();
  PrintTime(t1,t2);
}


void main2(int argc, char *argv[], DoubleArray &da){

  if(argc < 1 || argv[1] == NULL){
    print(da,"bisons");
    da.erase("bisons");
    cout << "erase bisons" << endl;
    da.erase("bison");
    da.erase("bison");
    cout << "erase bison" << endl;
    print(da,"bisons");
    print(da,"bison");
    da.insert("bisons");
    print(da,"bisons");
    print(da,"bison");
    da.insert("bison");
    print(da,"bison");
    da.erase("ARPANET");
    cout << "erase ARPANET" << endl;
    print(da,"ARPA");
    print(da,"ARPANET");
  }else{
    print(da,argv[1]);
  }
}

void main3(int argc, char *argv[], DoubleArray &da, vector<string> &v){
  double t1,t2;
  t1 = GetusageSec();
  for(size_t i = 0; i < v.size(); i++){
    int a;
    if((a = da.exactMatch(v[i].c_str())) == -1){
      cout << "error " << v[i] << " " << a << endl;
    }else{
      //cout << v[i] << " " << a << endl;
    }
  }
  t2 = GetusageSec();
  PrintTime(t1,t2);
}

void main4(int argc, char *argv[]){
  vector<string> lines;
  read(lines);
  DoubleArray da,dda;
  build(da,lines);


  ofstream ofs("test2.idx");
  dda.save(ofs);
  ofs.close();
  ifstream ifs("test2.idx");
  da.load(ifs);
  ifs.close();

  random_shuffle(lines.begin(),lines.end());
  main3(argc,argv,da,lines);

}

int main(int argc, char *argv[]){

  vector<string> lines;
  read(lines);
  DoubleArray da;
  build(da,lines);
  main3(argc,argv,da,lines);

  return 0;
}
