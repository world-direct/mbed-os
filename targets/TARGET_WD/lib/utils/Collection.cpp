/*
 * Collection.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created:	22.06.2017
 * Author:	Simon Pfeifhofer
 * EMail:	simon.pfeifhofer@world-direct.at
 *
 * Description:
 *   Provides a generic collection.
 *
 */ 

#include "Collection.h"
#include <cassert>
 
template<typename T>
	Collection<T>::Collection() {
		cap = 0;
		length = 0;
		arr = 0;
	}
 
template<typename T>
	Collection<T>::Collection(const Collection& d)
		: cap(d.cap)
		, length(d.length)
		, arr(0) {
		arr = new T[cap];
		for (int i = 0; i < length; ++i)
			arr[i] = d.arr[i];
	}
 
template<typename T>
	Collection<T>::~Collection() {
		delete[] this->arr;
		cap = length = 0;
		arr = 0;
	}
 
template<typename T>
	const T& Collection<T>::operator [](unsigned int idx) const {
		assert(idx < length);
		return arr[idx];
	}
 
template<typename T>
	T& Collection<T>::operator [](unsigned int idx) {
		assert(idx < length);
		return arr[idx];
	}
 
template<typename T>
	void Collection<T>::pop_back() {
		assert(length > 0);
		--length;
	}
 
template<typename T>
	void Collection<T>::push_back(const T& entry) {
		if (length < cap)
			arr[length++] = entry;
		else {
			T* temp = new T[length + 5];
        
			for (int i = 0; i < length; ++i)
				temp[i] = arr[i];
            
			temp[length] = entry;
			delete[] arr;
			arr = temp;
			cap += 5;
			++length;
			std::cout << "nAddress of Arr: " << &arr << '\n';
		}
	}
 
template<typename T>
	Collection<T>& Collection<T>::operator =(const Collection &d) {
		if (this != &d) {
		    //delete[] this;
			this = d;
		}
		return *this;
	}
 
 
template<typename T>
	void Collection<T>::reserve(unsigned newcap) {
		if (cap >= newcap)
			return;
    
		T* temp = new T[newcap];
		for (int i = 0; i < length; ++i)
			temp[i] = arr[i];
    
		delete[] arr;
		arr = temp;
		cap = newcap;
	}
 
	 //operator += ()
 
	  /*
	  darray operator + (const darray& d1, const darray& d2)
	  {
	      darray temp;
	      temp += d1;
	      temp += d2;
	      return temp;
	      }
	      */
 
	       /*
	       std::ostream& operator << (std::ostream& out, const darray& d)
	       {
	           out << '(';
	           int i;
	           for( i = 0; i < (d.length-1); ++i )
	               out << d.arr[i] << ' ';
	           return out << d.arr[i] << ')';
	           }
	           */
 
template <typename T>
	void Collection<T>::sort(Collection<T>& elems) {
		for (int top = elems.size() - 1; top > 0; --top)
			for (int k = 0; k < top; ++k)
				if (elems[k] > elems[k + 1])
					swap(elems[k], elems[k + 1]);
	}
 
template <typename T>
	void Collection<T>::swap(T& elem1, T& elem2) {
		T temp = elem1;
		elem1 = elem2;
		elem2 = temp;
	}
