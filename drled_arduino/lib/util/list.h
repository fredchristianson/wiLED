#ifndef DR_LIST_H
#define DR_LIST_H

#include "../log/interface.h"

namespace DevRelief {


template<class T>
struct ListNode
{
	T data;
	ListNode<T> *next;
};


template <typename T>
class LinkedList{

public:
	LinkedList();
	virtual ~LinkedList();


	virtual int size() const;

	virtual bool insertAt(int index, T);

	virtual bool add(T);
    virtual T get(int index) const;
    virtual void removeAt(int index);
    virtual void removeFirst(T t);
    virtual void removeAll(T t);
    virtual int firstIndexOf(T t,int start=0) const;
    virtual T last() const;
    virtual void clear();
    T operator[](int index) const  { return this->get(index); }
	//T operator[](size_t& i)  const { return this->get(i); }
  	//const T operator[](const size_t& i) const { return this->get(i); }
    
    // remove all items where the lambda return true;
    void removeMatch(auto&&lambda);  
    
    void each(auto&& lambda) const;
    T* first(auto&& lambda) const;

protected:
    virtual void deleteNode(ListNode<T>* node);
    virtual ListNode<T>* getNode(int index) const;
	int m_size;
	ListNode<T> *m_root;
	ListNode<T>	*m_last;

};



template<typename T>
LinkedList<T>::LinkedList()
{
	m_root=NULL;
	m_last=NULL;
	m_size=0;
}

template<typename T>
LinkedList<T>::~LinkedList()
{

	ListNode<T>* tmp;
	while(m_root!=NULL)
	{
		tmp=m_root;
		m_root=m_root->next;
		deleteNode(tmp);
	}

	m_size=0;

}

template<typename T>
void LinkedList<T>::each(auto&& lambda) const {
  ListNode<T>* node = m_root;
  while(node != NULL) {
      ListNode<T>*next = node->next;
      lambda(node->data);
      node = next;
  }  
}


template<typename T>
void LinkedList<T>::removeMatch(auto&& lambda)  {
  ListNode<T>* node = m_root;
  ListNode<T>* prev = NULL;
  while(node != NULL) {
      if (lambda(node->data)) {
        if (prev != NULL) {
            prev->next = node->next;
            deleteNode(node);
            node = prev->next;
        } else {
            m_root = node->next;
            deleteNode(node);
            node = m_root;
        }
        m_size -= 1;
      } else {
        prev = node;

        node = node->next;
      }

  }  
}


template<typename T>
T* LinkedList<T>::first(auto&& lambda) const {
    ListNode<T>* node = m_root;
    ListNode<T>* matchNode = NULL;
    while(node != NULL && matchNode == NULL) {
        if (lambda(node->data)) {
            matchNode = node;
        }
        node = node->next;
    }  
    return matchNode ? &(matchNode->data) : NULL;
}

template<typename T>
T LinkedList<T>::get(int index) const{
    ListNode<T>* node = getNode(index);
    if (node != NULL) {
        return node->data;
    }
    return NULL;
}

template<typename T>
ListNode<T>* LinkedList<T>::getNode(int index) const{
    if (index < 0 || index >= size()){
        return NULL;
    }
	int pos = 0;
	ListNode<T>* current = m_root;

	while(pos < index && current){
		current = current->next;

		pos++;
	}

	// Check if the object index got is the same as the required
	if(pos == index){
		return current;
	}

	return NULL;
}

template<typename T>
int LinkedList<T>::size() const{
	return m_size;
}

template<typename T>
bool LinkedList<T>::insertAt(int index, T item){

	ListNode<T> *tmp = new ListNode<T>();
	tmp->data = item;
	tmp->next = NULL;
	
    if (m_root == NULL) {
        m_root = tmp;
    } else if (index == 0) {
        tmp->next = m_root;
        m_root = tmp;
    } else {
        ListNode<T>* prev = getNode(index-1);
        if (prev == NULL) {
            prev = getNode(m_size-1);
        }
        tmp->next = prev->next;
        prev->next = tmp;
    }

    m_size++;
	return true;
}

template<typename T>
bool LinkedList<T>::add(T item){
	ListNode<T> *tmp = new ListNode<T>();
    tmp->data = item;
	tmp->next = NULL;
	
    if (m_root == NULL) {
        m_root = tmp;
    } else {
        ListNode<T>* node = m_root;
        while(node->next != NULL) {
            node = node->next;
        }
        node->next = tmp;
    }

	m_size++;

	return true;
}

template<typename T>
T LinkedList<T>::last() const{
    if (m_root == NULL) {
        return T();
    }
    ListNode<T>* node = m_root;
    while(node->next != NULL) {
        node = node->next;
    }
    return node->data;
}

template<typename T>
void LinkedList<T>::clear(){
    while(m_root != NULL){
        ListNode<T>*next = m_root->next;
        deleteNode(m_root);
        m_root = next;
    }
    m_size = 0;
}

template<typename T>
void LinkedList<T>::removeAt(int index){
	if (index < 0 || index >= m_size)
	{
		return;
	}
    if (index == 0) {
        ListNode<T>*next = m_root->next;
        deleteNode(m_root);
        m_root = next;
        m_size--;
        return;
    }


	ListNode<T>*prev = getNode(index-1);
    ListNode<T>*tmp = prev->next;
    prev->next = tmp->next;
    m_size --;
    deleteNode(tmp);
}

template<typename T>
void LinkedList<T>::removeFirst(T t) {
    removeAt(firstIndexOf(t));
}

template<typename T>
void LinkedList<T>::removeAll(T t){
    int idx = firstIndexOf(t);
    while(idx>=0) {
        LinkedList<T>::removeAt(idx);
        idx = firstIndexOf(t);
    }
}

template<typename T>
int LinkedList<T>::firstIndexOf(T t,int start) const{
    int idx = start;
    ListNode<T>*node = m_root;
    while(idx<m_size&&node != NULL && node->data != t) {
        node = node->next;
        idx++;
    }
    return node != NULL ? idx : -1;
}

template<typename T>
void LinkedList<T>::deleteNode(ListNode<T>* t){
    delete t;
    
}

template<typename T>
class PtrList : public LinkedList<T> {
    public:
    	PtrList();
	    virtual ~PtrList();

    protected:
       // virtual ListNode<T> getNodePtr(int idx);
        virtual void deleteNode(ListNode<T>*node);
};


template<typename T>
PtrList<T>::~PtrList(){
    ListNode<T>*node=LinkedList<T>::getNode(0);
    while(node != NULL) {
        ListNode<T>*next = node->next;
        deleteNode(node);
        node = next;
    }
    LinkedList<T>::m_root = NULL;

}

template<typename T>
PtrList<T>::PtrList(){

}
/*
template<typename T>
ListNode<T> PtrList<T>::getNodePtr(int idx) {
    return LinkedList<T>::getNode(idx);
}
*/
template<typename T>
void PtrList<T>::deleteNode(ListNode<T>*node) {
    if (node == 0) {
    } else {
        //delete node->data;
        node->data->destroy();
        delete node;
    }
}

};
#endif