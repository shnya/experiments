#ifndef _PAGE_GROUP_H_
#define _PAGE_GROUP_H_

#include <vector>
#include <stdint.h>
#include "page.h"

enum PageGroupInfo {
  FIX_IN_MEMORY = 1
};

class PageGroup {
  std::vector<Page *> pages;
  int group_info_;

  void LoadPages(File &file, off_t offset, size_t idx, int scale){
    off_t pagesize = scale * Page::GetPageSize();
    while(1){
      pages.push_back(new Page(file, offset + pagesize * idx, scale));
      uint32_t *pdata = pages.back()->data<uint32_t>();
      if(pdata[0] & 1UL << 31){
        idx = pdata[0] & ((1UL << 31) - 1);
      }else{
        break;
      }
    }
  }

public:
  PageGroup(File &file, off_t offset, size_t idx, int group_info, int scale = 1)
    : group_info_(group_info){
    LoadPages(file, offset, idx, scale);
  }

  ~PageGroup(){
    for(size_t i = 0; i < pages.size(); i++){
      delete pages[i];
    }
  }

  bool is_fix_in_memory(){
    return group_info_ & FIX_IN_MEMORY;
  }

  void unmap(){
    for(size_t i = 0; i < pages.size(); i++){
      pages[i]->munmap();
    }
  }

  void sync(){
    for(size_t i = 0; i < pages.size(); i++){
      pages[i]->msync();
    }
  }

  size_t size() const {
    return pages.size();
  }

  template <class T>
  T* data(size_t idx){
    return pages[idx]->data<T>();
  }

};


#endif /* _PAGE_GROUP_H_ */
