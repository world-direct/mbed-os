#ifndef COLLECTION_H_
#define COLLECTION_H_

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
 *	 Modified version of https://developer.mbed.org/users/Fuzball/code/DynamicArray/
 *
 */ 

#include <iostream>
 
template<typename T>
	class Collection {
	public:
	     
		// Constructors
        
	    // Pre: none
	    // Post: default constructor is called and default Collection results
		Collection();
    
		// Pre: darray passed to the copy constructor must have = operator
		// Post: darray passed is copied into the new Collection
		Collection(const Collection &);
 
	        // Destructor
        
	    // Pre: computer is turned on
	    // Post: Collection is tilded and destroyed
		~Collection();
    
		// Constant members
    
		// Pre: none
		// Post: length of Collection is returned
		unsigned size() const { return length; }
    
		// Pre: none
		// Post: total capacity of Collection is returned
		unsigned capacity() const { return cap; }
    
		// Pre: index accessed must exist or cassert will fail
		// Post: a reference to the value at that index is returned
		const T& operator [](unsigned int) const;
 
		// Modifiers
        
	    // Pre: Collection must have a length > 0, otherwise cassert will fail
	    // Post: length of Collection is decreased by one
		void pop_back();
    
		// Pre: Value passed must be a valid type and have = operator assigned
		// Post: Collection is expanded and the type, T, is added to the end of the Collection
		void push_back(const T &);
    
		// Pre: index accessed must exist or cassert will fail
		// Post: a non-constant reference to the value at that index is returned
		T& operator [](unsigned int);
    
		// Pre: none
		// Post: amount of type * unsigned is reserved in memory
		void reserve(unsigned);
    
		// Pre: darray must have more than one item or memory-out-of-bounds thingy will go nuts on you
		// Post: list is sorted... ... ...backwards
		void sort(Collection&);
    
		// Pre: Parameters one and two should most functionally not equal each other...
		// Post: Values at param 1 and param 2 are exchanged and proper locations in the darray are exchanged as well
		void swap(T&, T&);
 
	    // Operators
    
	    // Pre: both sides must be valid darrays?
	    // Post: Collection1 now ='s Collection2. Huzzah!
		Collection& operator =(const Collection&);
 
	    // Friends
	    // Collection have no friends, instead, they hold work-relationships with various types
    
	private:
 
	    // length and capacity of the Collection
		unsigned length, cap;
		// Pointer to the Collection of types, T.
		T* arr;
    
	};
 
// template include
// #include "Collection.cpp"

#endif // COLLECTION_H_