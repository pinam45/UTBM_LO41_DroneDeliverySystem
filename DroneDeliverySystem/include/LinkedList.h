/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    UTBM_LP25_S2016_Project, school project                                  //
//    Copyright (C) 2016  Cortier Benoît & Pinard Maxime                       //
//                                                                             //
//    This program is free software: you can redistribute it and/or modify     //
//    it under the terms of the GNU General Public License as published by     //
//    the Free Software Foundation, either version 3 of the License, or        //
//    (at your option) any later version.                                      //
//                                                                             //
//    This program is distributed in the hope that it will be useful,          //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
//    GNU General Public License for more details.                             //
//                                                                             //
//    You should have received a copy of the GNU General Public License        //
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @file LinkedList.h
 * @brief      Implementation of a doubly linked list which use sentinel nodes
 * @author     Pinard Maxime
 * @author     Cortier Benoît
 * @version    0.1
 * @date       19/05/16
 */

#ifndef UTBM_LP25_S2016_PROJECT_LINKEDLIST_H
#define UTBM_LP25_S2016_PROJECT_LINKEDLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*//**
 * @struct ListNode
 *
 * @brief      Node used in the doubly linked list List
 */
typedef struct elem {
	void* value; /**< Pointer on the ListNode value */
	struct elem* next; /**< Pointer on the next ListNode of the List (or the tail sentinel node) */
	struct elem* previous; /**< Pointer on the previous ListNode of the List (or the head sentinel node) */
} ListNode;

/*-------------------------------------------------------------------------*//**
 * @struct List
 *
 * @brief      Doubly linked list which use sentinel nodes and store the list
 */
typedef struct {
	ListNode* sentinel; /**< Pointer on sentinel node */
	size_t size; /**< List size */
	size_t valueSize; /**< Size of the value parameter of ListNode */
} List;

/*-------------------------------------------------------------------------*//**
 * @struct ListIterator
 *
 * @brief      Iterator used with the doubly linked list List
 */
typedef struct {
	List* list; /**< Pointer on the list */
	ListNode* listNode; /**< Pointer on the list node */
} ListIterator;

/*-------------------------------------------------------------------------*//**
 * @brief      Create a new List.
 *
 * @param[in]  elementSize  The size of the elements of the list used in dynamic
 *                          allocation (use sizeof(ElementType) to get the size
 *                          of an element of type ElementType).
 *
 * @return     The newly created List.
 */
List* createList(size_t elementSize);

/*-------------------------------------------------------------------------*//**
 * @brief      Delete the given list and free the List pointer.
 *
 * @param      list  The list
 *
 * @details    After calling this function the List pointer will no longer point
 *             on a List.
 */
void deleteList(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Determines if the list is empty.
 *
 * @param      list  The list
 *
 * @return     True if empty, False otherwise.
 */
bool isEmpty(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the list size.
 *
 * @param      list  The list
 *
 * @return     The size.
 */
size_t getSize(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element at the first position in the list.
 *
 * @param      list     The list
 * @param      element  The element
 */
void insertFirst(List* list, void* element);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element at the last position in the list.
 *
 * @param      list     The list
 * @param      element  The element
 */
void insertLast(List* list, void* element);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element at the given position in the list.
 *
 * @param      list             The list
 * @param[in]  elementPosition  The element position
 * @param      element          The element
 *
 * @details    If the given element position is greater or equals to the list
 *             size, it will be inserted at the last position
 */
void insertElement(List* list, size_t elementPosition, void* element);

/*-------------------------------------------------------------------------*//**
 * @brief      Remove the first element of the list.
 *
 * @param      list  The list
 *
 * @return     True if the first element was deleted, False if there is no first
 *             element (the list is empty).
 */
bool removeFirst(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Remove the last element of the list.
 *
 * @param      list  The list
 *
 * @return     True if the last element was deleted, False if there is no last
 *             element (the list is empty).
 */
bool removeLast(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Remove the element at the given position
 *
 * @param      list             The list
 * @param[in]  elementPosition  The element position
 *
 * @return     True if the element was deleted, False if the given position is
 *             out of the list range (or the list is empty) and no element was
 *             deleted.
 */
bool removeElement(List* list, size_t elementPosition);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the first element of the list.
 *
 * @param      list  The list
 *
 * @return     The first element of the list.
 */
void* getFirst(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the last element of the list.
 *
 * @param      list  The list
 *
 * @return     The last element of the list.
 */
void* getLast(List* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the element at the given position in the list.
 *
 * @param      list             The list
 * @param[in]  elementPosition  The element position
 *
 * @return     The element or NULL if the given position is out of the list
 *             range (or the list is empty).
 */
void* getElement(List* list, size_t elementPosition);

/*-------------------------------------------------------------------------*//**
 * @brief      Check if the list contains the given elements
 *
 * @param      list     The list
 * @param      element  The element
 * @param      compare  The function used to compare the elements
 *
 * @return     True if the list contains the element, False otherwise.
 *
 * @details    The function compare must return 0 if the 2 elements passed in
 *             parameters are equals.
 */
bool contains(List* list, void* element, int (* compare)(void*, void*));

/*-------------------------------------------------------------------------*//**
 * @brief      Get the given element position.
 *
 * @param      list     The list
 * @param      element  The element
 * @param      compare  The function used to compare the elements
 *
 * @return     The element position if the list contains the element, the list
 *             size otherwise.
 *
 * @details    The function compare must return 0 if the 2 elements passed in
 *             parameters are equals.
 */
unsigned int getElementPosition(List* list, void* element, int (* compare)(void*, void*));

/*-------------------------------------------------------------------------*//**
 * @brief      Get the ListNode at the given position in the list
 *
 * @param      list                 The list
 * @param[in]  listElementPosition  The ListNode position
 *
 * @return     The ListNode or NULL if the given position is out of the list
 *             range (or the list is empty).
 */
ListNode* getListNode(List* list, size_t listElementPosition);

/**
 * @brief      Create a ListIterator at the head of the list
 *
 * @param      list  The list
 *
 * @return     The ListIterator if the list isn't empty, NULL otherwise.
 */
ListIterator* firstIterator(List* list);

/**
 * @brief      Create a ListIterator at the tail of the list
 *
 * @param      list  The list
 *
 * @return     The ListIterator if the list isn't empty, NULL otherwise.
 */
ListIterator* lastIterator(List* list);

/**
 * @brief      Determines if the ListIterator related ListNode has a next
 *             ListNode.
 *
 * @param      listIterator  The list iterator
 *
 * @return     True if has next, False otherwise.
 */
bool hasNext(ListIterator* listIterator);

/**
 * @brief      Returns the next value of the iterator.
 *
 * @param      listIterator  The list iterator
 *
 * @return     the next value.
 */
void* next(ListIterator* listIterator);

/**
 * @brief      Determines if the ListIterator related ListNode has a previous
 *             ListNode.
 *
 * @param      listIterator  The list iterator
 *
 * @return     True if has previous, False otherwise.
 */
bool hasPrev(ListIterator* listIterator);

/**
 * @brief      Returns the previous value of the iterator.
 *
 * @param      listIterator  The list iterator
 *
 * @return     the previous value.
 */
void* prev(ListIterator* listIterator);

/*-------------------------------------------------------------------------*//**
 * @brief      Free the listIterator.
 *
 * @param      listIterator  The list iterator
 *
 * @details    After calling this function the ListIterator pointer will no
 *             longer point on a ListIterator.
 */
void deleteIterator(ListIterator* listIterator);

#endif //UTBM_LP25_S2016_PROJECT_LINKEDLIST_H
