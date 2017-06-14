#pragma once
#include <list>
#include <iterator>

template <typename T, int size>
class MeasurementBuffer {

public:
	
	MeasurementBuffer(){};
	
	void clear() {
		buf.clear();
	};
	
	void add(T v){
		buf.push_back(v);
		if (buf.size() > size) buf.pop_front();
	};
	
	T get() {
		std::list<T> bufCpy (buf);
		
		if (bufCpy.size() > 0) {
			
			
			bufCpy.sort();
		
			if (bufCpy.size() % 2 == 0 ) {
				typename std::list<T>::const_iterator it1 = bufCpy.begin();
				std::advance(it1, (bufCpy.size() / 2) - 1);
				typename std::list<T>::const_iterator it2 = bufCpy.begin();
				std::advance(it2, bufCpy.size() / 2);
				
				return (*it1 + *it2) / 2;
			} else {
				typename std::list<T>::const_iterator it = bufCpy.begin();
				std::advance(it, (bufCpy.size() - 1) / 2);
				return *it;
			}
		}
		
		return T();
	};

private:
	std::list<T> buf;
	
};