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
#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <sys/resource.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <cstdio>

double GetusageSec()
{
  struct rusage t;
  struct timeval tv;
  getrusage(RUSAGE_SELF, &t);
  tv = t.ru_utime;
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void PrintTime(double before, double after)
{
  printf("  time = %10.30f\n", after-before);
}

#endif /* _PROFILE_H_ */
