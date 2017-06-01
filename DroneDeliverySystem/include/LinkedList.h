/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    UTBM_LP25_S2016_Project, school project                                  //
//    Copyright (C) 2016  Cortier Benoît & Pinard Maxime & Boulmier Jérôme     //
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
 * @brief      Definition of a doubly linked list and its related functions.
 * @author     Pinard Maxime
 * @author     Cortier Benoît
 * @author	   Boulmier Jérôme
 * @version    0.3
 * @date       1/06/2017
 */

#ifndef UTBM_LP25_S2016_PROJECT_LINKEDLIST_H
#define UTBM_LP25_S2016_PROJECT_LINKEDLIST_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <log.h>

/*-------------------------------------------------------------------------*//**
 * @struct LinkedListNode
 *
 * @brief      Node used in the doubly linked list LinkedList
 */
typedef struct elem {
	void* value; /**< Pointer on the LinkedListNode value */
	struct elem* next; /**< Pointer on the next LinkedListNode of the LinkedList (or the tail sentinel node) */
	struct elem* previous; /**< Pointer on the previous LinkedListNode of the LinkedList (or the head sentinel node) */
} LinkedListNode;

/*-------------------------------------------------------------------------*//**
 * @struct LinkedList
 *
 * @brief      Doubly linked list which use sentinel nodes and store the list
 */
typedef struct {
	LinkedListNode* sentinel; /**< Pointer on sentinel node */
	unsigned int size; /**< LinkedList size */
} LinkedList;

/*-------------------------------------------------------------------------*//**
 * @struct LinkedListIterator
 *
 * @brief      Iterator used with the doubly linked list LinkedList
 */
typedef struct {
	LinkedList* list; /**< Pointer on the list */
	LinkedListNode* listNode; /**< Pointer on the list node */
} LinkedListIterator;

/*-------------------------------------------------------------------------*//**
 * @brief      Create a new LinkedList.
 *
 * @return     The newly created LinkedList.
 */
LinkedList* ll_createList();

/*-------------------------------------------------------------------------*//**
 * @brief      Delete the given list and free the LinkedList pointer.
 *
 * @param      list  The list
 *
 * @details    After calling this function the LinkedList pointer will no longer point
 *             on a LinkedList.
 */
void ll_deleteList(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Determines if the list is empty.
 *
 * @param      list  The list
 *
 * @return     True if empty, False otherwise.
 */
bool ll_isEmpty(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the list size.
 *
 * @param      list  The list
 *
 * @return     The size.
 */
unsigned int ll_getSize(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element at the first position in the list.
 *
 * @param      list     The list
 * @param      element  The element
 */
void ll_insertFirst(LinkedList* list, void* element);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element at the last position in the list.
 *
 * @param      list     The list
 * @param      element  The element
 */
void ll_insertLast(LinkedList* list, void* element);

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
void ll_insertElement(LinkedList* list, unsigned int elementPosition, void* element);

/*-------------------------------------------------------------------------*//**
 * @brief      Insert the given element in a sorted linked list
 *
 * @param      list        The list
 * @param      element     The element
 * @param      comparator  The function used to compare the elements
 *
 * @since      0.3
 */
void ll_insertSorted(LinkedList* list, void* element, int (* comparator)(void*, void*));

/*-------------------------------------------------------------------------*//**
 * @brief      Remove the first element of the list.
 *
 * @param      list  The list
 *
 * @return     True if the first element was deleted, False if there is no first
 *             element (the list is empty).
 */
bool ll_removeFirst(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Remove the last element of the list.
 *
 * @param      list  The list
 *
 * @return     True if the last element was deleted, False if there is no last
 *             element (the list is empty).
 */
bool ll_removeLast(LinkedList* list);

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
bool ll_removeElement(LinkedList* list, unsigned int elementPosition);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the first element of the list.
 *
 * @param      list  The list
 *
 * @return     The first element of the list.
 */
void* ll_getFirst(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the last element of the list.
 *
 * @param      list  The list
 *
 * @return     The last element of the list.
 */
void* ll_getLast(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Get the element at the given position in the list.
 *
 * @param      list             The list
 * @param[in]  elementPosition  The element position
 *
 * @return     The element or NULL if the given position is out of the list
 *             range (or the list is empty).
 */
void* ll_getElement(LinkedList* list, unsigned int elementPosition);

/*-------------------------------------------------------------------------*//**
 * @brief      Check if the list contains the given element.
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
bool ll_contains(LinkedList* list, void* element, int (* compare)(void*, void*));

/*-------------------------------------------------------------------------*//**
 * @brief      Find an element in a list.
 *
 * @param      list        The list
 * @param      descriptor  Descriptor, passed as first argument of the
 *                         comparison function
 * @param[in]  check       The function used to check the elements with the
 *                         descriptor (descriptor as first argument, element as
 *                         second), must return true for a wanted, false
 *                         otherwise
 *
 * @return     The wanted element if found, NULL otherwise
 */
void* ll_findElement(LinkedList* list, void* descriptor, int (* check)(void*, void*));

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
unsigned int ll_getElementPosition(LinkedList* list, void* element, int (* compare)(void*, void*));

/*-------------------------------------------------------------------------*//**
 * @brief      Get the LinkedListNode at the given position in the list
 *
 * @param      list                 The list
 * @param[in]  listElementPosition  The LinkedListNode position
 *
 * @return     The LinkedListNode or NULL if the given position is out of the list
 *             range (or the list is empty).
 */
LinkedListNode* ll_getListNode(LinkedList* list, unsigned int listElementPosition);

/*-------------------------------------------------------------------------*//**
 * @brief      Create a LinkedListIterator at the head of the list
 *
 * @param      list  The list
 *
 * @return     The LinkedListIterator if the list isn't empty, NULL otherwise.
 */
LinkedListIterator* ll_firstIterator(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Create a LinkedListIterator at the tail of the list
 *
 * @param      list  The list
 *
 * @return     The LinkedListIterator if the list isn't empty, NULL otherwise.
 */
LinkedListIterator* ll_lastIterator(LinkedList* list);

/*-------------------------------------------------------------------------*//**
 * @brief      Determines if the LinkedListIterator related LinkedListNode has a next
 *             LinkedListNode.
 *
 * @param      listIterator  The list iterator
 *
 * @return     True if has next, False otherwise.
 */
bool ll_hasNext(LinkedListIterator* listIterator);

/*-------------------------------------------------------------------------*//**
 * @brief      Returns the next value of the iterator.
 *
 * @param      listIterator  The list iterator
 *
 * @return     the next value.
 */
void* ll_next(LinkedListIterator* listIterator);

/*-------------------------------------------------------------------------*//**
 * @brief      Determines if the LinkedListIterator related LinkedListNode has a previous
 *             LinkedListNode.
 *
 * @param      listIterator  The list iterator
 *
 * @return     True if has previous, False otherwise.
 */
bool ll_hasPrev(LinkedListIterator* listIterator);

/*-------------------------------------------------------------------------*//**
 * @brief      Returns the previous value of the iterator.
 *
 * @param      listIterator  The list iterator
 *
 * @return     the previous value.
 */
void* ll_prev(LinkedListIterator* listIterator);

/*-------------------------------------------------------------------------*//**
 * @brief      Free the listIterator.
 *
 * @param      listIterator  The list iterator
 *
 * @details    After calling this function the LinkedListIterator pointer will no
 *             longer point on a LinkedListIterator.
 */
void ll_deleteIterator(LinkedListIterator* listIterator);

#ifdef __cplusplus
}
#endif


#endif //UTBM_LP25_S2016_PROJECT_LINKEDLIST_H
