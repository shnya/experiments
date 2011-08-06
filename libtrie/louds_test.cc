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
#include "louds.hpp"
#include "profile.hpp"
using namespace std;


int main(int argc, char *argv[])
{
  string line;
  double t1,t2;
  vector<string> lines;
  while(getline(cin, line)) {
    lines.push_back(line);
  }
  t1 = GetusageSec();
  std::stable_sort(lines.begin(), lines.end());
  LOUDS dic(lines);
  t2 = GetusageSec();
  PrintTime(t1,t2);

  ofstream ofs("test2.idx");
  dic.save(ofs);
  ofs.close();

  LOUDS dic2;
  ifstream ifs("test2.idx");
  dic2.load(ifs);
  ifs.close();


  t1 = GetusageSec();
  vector<string> vec;
  //int n = 0;
  for(vector<string>::iterator itr = lines.begin();  itr != lines.end(); ++itr){
    dic2.common_prefix_search(itr->c_str(), vec);
    for(size_t i = 0; i < vec.size(); i++){
      cout << *itr << " " << vec[i] << endl;
    }
    //if(!dic2.exact_match(*itr)){
    //cout << "error" << endl;
    //}
    //if(n++ % 100000 == 0){
    //cout << n << endl;
    //}
  }
  t2 = GetusageSec();
  PrintTime(t1,t2);

  return 0;
}
