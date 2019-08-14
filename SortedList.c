//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373

//SortedList.c


//includes
#include "SortedList.h"
#include <pthread.h> //sched_yield()
#include <stdlib.h>  //free()
#include <string.h>  //strcmp()
#include <stdio.h>   //printf()

// SortedList_insert ... insert an element into a sorted list
//
//The specified element will be inserted in to
//the specified list, which will be kept sorted
//in ascending order based on associated keys
//
// @param SortedList_t *list ... header for the list
// @param SortedListElement_t *element ... element to be added to the list

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  //insert in ascending order
  //accessing the list is the critical section
  if (opt_yield & INSERT_YIELD){
    sched_yield();
  }
  //get the first pointer
  SortedListElement_t * head = list;
  //see if empty list
  //key == NULL only in list pointer
  if (head->next->key == NULL){
    //insert the element
    list->next = element;
    list->prev = element;
    element->next = list;
    element->prev = list;
    return;
  }
  //traverse the list
  //compare elements
  //head-key > element->key
  head = head->next;
  while((head->key != NULL) && (strcmp(head->key, element->key) > 0)){
    //increment pointer
    head = head->next;
  }
  //INSERT before head
  element->next = head;
  element->prev = head->prev;
  head->prev->next = element;
  head->prev = element;
  return;
}


// SortedList_delete ... remove an element from a sorted list
//
//The specified element will be removed from whatever
//list it is currently in.
//
//Before doing the deletion, we check to make sure that
//next->prev and prev->next both point to this node
//
// @param SortedListElement_t *element ... element to be removed
//
// @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 
int SortedList_delete( SortedListElement_t *element){
  //critical section is accessing the list, not deleting the element
  if (opt_yield & DELETE_YIELD){
    sched_yield();
  }
  //check that the element is not corrupted
  if((element->prev->next == element) && (element->next->prev == element)){
    //not corrupted
    element->prev->next = element->next;
    element->next->prev = element->prev;
    //free(element);
    return 0;
  } else {
    //corrupted
    return 1;
  }
}


// SortedList_lookup ... search sorted list for a key
//
//The specified list will be searched for an
//element with the specified key.
//
// @param SortedList_t *list ... header for the list
// @param const char * key ... the desired key
//
// @return pointer to matching element, or NULL if none is found

SortedListElement_t * SortedList_lookup(SortedList_t *list, const char *key){
  //critical section is accessing the list
  if (opt_yield & LOOKUP_YIELD){
    sched_yield();
  }
  //get first pointer in the list to traverse the linked list
  SortedListElement_t * head = list->next;
  //traverse the rest of the linked list
  //head->key == NULL only if its the list pointer
  while(head->key != NULL){
    //compare keys
    if (strcmp(head->key,key) == 0){
      //found the element
      return head;
    }
    //increment the pointer
    head = head->next;
  }
  //element is not in the list
  return NULL;
}


// SortedList_length ... count elements in a sorted list
//While enumeratign list, it checks all prev/next pointers
//
// @param SortedList_t *list ... header for the list
//
// @return int number of elements in list (excluding head)
//   -1 if the list is corrupted

int SortedList_length(SortedList_t *list){
  //same algorithm as searching a list
  //critical section is accessing the list, not the counter
  if (opt_yield & LOOKUP_YIELD){
    sched_yield();
  }
  //check for a corrupted list
  if (list == NULL){
    return -1;
  }
  //get head pointer
  SortedListElement_t * head = list->next;
  //initialize a length counter
  int length = 0;
  //traverse the linked list
  while(head->key != NULL){
    length++;
    head = head->next;
  }
  //return the length of the list
  return length;
}
