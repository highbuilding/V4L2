#ifndef __VIDEODATA_LIST__H
#define __VIDEODATA_LIST__H
#include "locker.h"
#include <list>

template<typename T>
class videodata_list{
  private:
  	int max_framecnt;
    std::list<T*> m_datalist;
	locker m_queuelocker;
  public:
  	videodata_list(void){
		max_framecnt=120;
	}
  	void add(T* p_data){
		m_queuelocker.lock();
		if(m_datalist.size()>=max_framecnt){
            m_queuelocker.unlock();
			return;
		}
        m_datalist.push_back(p_data);
		m_queuelocker.unlock();
  	}
    T* get(void){
		m_queuelocker.lock();
		if (m_datalist.empty()) {
            m_queuelocker.unlock();
			return null;
        }
	    T *result = m_datalist.front();
		m_datalist.pop_front();
		m_queuelocker.unlock();
		return  result;
    }
		
	 
  	

};

#endif




