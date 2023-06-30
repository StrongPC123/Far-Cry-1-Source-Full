/*
 *
 * Copyright (c) 2002, NVIDIA Corporation.
 * 
 *  
 * 
 * NVIDIA Corporation("NVIDIA") supplies this software to you in consideration 
 * of your agreement to the following terms, and your use, installation, 
 * modification or redistribution of this NVIDIA software constitutes 
 * acceptance of these terms.  If you do not agree with these terms, please do 
 * not use, install, modify or redistribute this NVIDIA software.
 * 
 *  
 * 
 * In consideration of your agreement to abide by the following terms, and 
 * subject to these terms, NVIDIA grants you a personal, non-exclusive license,
 * under NVIDIA’s copyrights in this original NVIDIA software (the "NVIDIA 
 * Software"), to use, reproduce, modify and redistribute the NVIDIA 
 * Software, with or without modifications, in source and/or binary forms; 
 * provided that if you redistribute the NVIDIA Software, you must retain the 
 * copyright notice of NVIDIA, this notice and the following text and 
 * disclaimers in all such redistributions of the NVIDIA Software. Neither the 
 * name, trademarks, service marks nor logos of NVIDIA Corporation may be used 
 * to endorse or promote products derived from the NVIDIA Software without 
 * specific prior written permission from NVIDIA.  Except as expressly stated 
 * in this notice, no other rights or licenses express or implied, are granted 
 * by NVIDIA herein, including but not limited to any patent rights that may be 
 * infringed by your derivative works or by other works in which the NVIDIA 
 * Software may be incorporated. No hardware is licensed hereunder. 
 * 
 *  
 * 
 * THE NVIDIA SOFTWARE IS BEING PROVIDED ON AN "AS IS" BASIS, WITHOUT 
 * WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING 
 * WITHOUT LIMITATION, WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR ITS USE AND OPERATION 
 * EITHER ALONE OR IN COMBINATION WITH OTHER PRODUCTS.
 * 
 *  
 * 
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * EXEMPLARY, CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOST 
 * PROFITS; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) OR ARISING IN ANY WAY OUT OF THE USE, 
 * REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE NVIDIA SOFTWARE, 
 * HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 * NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF NVIDIA HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */ 

/*********************************************************************NVMH2****
File:  cgTemplates.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

cgTemplates - cg utility templates


******************************************************************************/

#ifndef CG_TEMPLATE_H
#define CG_TEMPLATE_H

#include <stdlib.h>


//////////////////////////////////////////
//
//       templated cg_vector class
//
//////////////////////////////////////////


template <class T>
class cg_vector{
public:
	typedef int size_type;

	cg_vector();
    cg_vector( cg_vector<T>& v );
	~cg_vector();

	void clear();
	int size() const;
	T& operator [] ( int i );
	void push_back( const T& t );

private:
	T* data;
	int num;
	int capacity;
	const static int initSize;
};


//definitions of cg_vector methods

template <class T>
const int cg_vector<T>::initSize = 16;

template <class T>
cg_vector<T>::cg_vector(){
	data = NULL;
	clear();
}

template <class T>
cg_vector<T>::cg_vector( cg_vector<T>& v ){
	data = NULL;
	clear();
    for( int i=0; i<v.size(); i++ ){
        push_back( v[i] );
    }
}

template <class T>
cg_vector<T>::~cg_vector(){
	delete[] data;
}

template <class T>
void cg_vector<T>::clear(){
	if( data ){
		delete[] data;
	}
	data = (T*)new T[initSize];
	num = 0;
	capacity = initSize;
}

template <class T>
int cg_vector<T>::size() const{
	return num;
}

template <class T>
void cg_vector<T>::push_back( const T& t ){
	if( num >= capacity ){
		T* temp = new T[ 2 * capacity ];
        for( int i = 0; i < num; i++ ){
            temp[i] = data[i];
        }
		delete[] data;
		data = temp;
		capacity *= 2;
	}
	data[num] = t;
	num++;
}

template <class T>
T& cg_vector<T>::operator [] ( int i ){
	if( i>=0 && i < num ){
		return data[i];
	}else{
		return data[0];
	}
}




//////////////////////////////////////////
//
//       templated cg_list class
//
//////////////////////////////////////////


template <class T>
class cg_list{
public:
	class Node{
	public:
		Node( const T& _data, Node* _next, Node* _prev ){
			data = _data;
			next = _next;
			prev = _prev;
		}

		T data;
		Node* next;
		Node* prev;
	};

	class iterator{
	public:
		iterator( Node* _node ){
			node = _node;
		}
		iterator(){
			node = NULL;
		}

		bool operator == ( const iterator& it ) const{
			return node == it.node;
		}
		bool operator != ( const iterator& it ) const{
			return !( *this == it );
		}
		T& operator * (){
			return node -> data;
		}
		void operator ++ (){
			node = node -> next;
		}
		void operator ++ (int){
			node = node -> next;
		}

		Node* node;
	};

	cg_list();
    cg_list( const cg_list<T>& l );
	~cg_list();
	iterator begin();
	iterator end();
	void push_back( const T& t );
	void clear();
	void remove( const T& value );

private:
	Node* head;
	Node* tail;

	void insert( const T& t, Node* node );
	void remove( Node* node );
};


// definitions of cg_list methods

template <class T>
cg_list<T>::cg_list(){
	head = tail = new Node( T(), NULL, NULL );
}

template <class T>
cg_list<T>::~cg_list(){
	clear();
	delete head;
}

template <class T>
cg_list<T>::cg_list( const cg_list<T>& l ){
    head = tail = new Node( T(), NULL, NULL );
    cg_list<T>::iterator it = l.begin();
    while( it != l.end() ){
        push_back( *it );
        it++;
    }
}

template <class T>
cg_list<T>::iterator cg_list<T>::begin(){
	return head;
}

template <class T>
cg_list<T>::iterator cg_list<T>::end(){
	return tail;
}

template <class T>
void cg_list<T>::push_back( const T& t ){
	insert( t, tail );
}

template <class T>
void cg_list<T>::clear(){
	while( head != tail ){
		remove( head );
	}
}

template <class T>
void cg_list<T>::remove( const T& value ){
	Node* curr = head;
	Node* temp;
	while( curr != tail ){
		temp = curr;
		curr = curr -> next;
		if( temp -> data == value ){
			remove( temp );
		}
	}
}

template <class T>
void cg_list<T>::insert( const T& t, cg_list<T>::Node* node ){
	Node* newNode = new Node( t, node, node -> prev );
	if( node -> prev == NULL ){
		head = newNode;
	}else{
		node -> prev -> next = newNode;
	}
	node -> prev = newNode;
}

template <class T>
void cg_list<T>::remove( cg_list<T>::Node* node ){
	if( node == tail ){
		return;
	}
	node -> next -> prev = node -> prev;
	if( node -> prev == NULL ){
		head = node -> next;
	}else{
		node -> prev -> next = node -> next;
	}
	delete node;
}



//////////////////////////////////////////
//
//             cg_string class
//
//////////////////////////////////////////


class cg_string{
public:
	typedef int size_type;
	cg_string();
	cg_string( const char* s);
	cg_string( const cg_string& s );
	~cg_string();

	int size() const;
	const char* c_str() const;
	void resize( int n, char c = '\0' );
	cg_string& erase( int pos = 0, int n = npos );
	int find_last_of( const cg_string& s, int pos = npos ) const;
	cg_string substr( int pos = 0, int len = npos ) const;

	char& operator [] ( int i );
	const char& operator [] ( int i ) const;
	cg_string& operator = ( const cg_string& s );
	cg_string& operator += ( const cg_string& s );
	cg_string operator + ( const cg_string& s ) const;

	const static int npos;

private:
	char* data;
	int num;					//length of the string, not counting the trailing '\0'.
	int capacity;
	const static int initSize;

	void set( const char* s, int n );
};

#endif 
