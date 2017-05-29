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

#include <log.h>
#include <util/LinkedList.h>

////////////////////////////////////////////////////////////
List* createList(size_t elementSize) {
	List* list = (List*) malloc(sizeof(List));
	list->valueSize = elementSize;
	list->sentinel = (ListNode*) malloc(sizeof(ListNode));
	list->sentinel->next = list->sentinel;
	list->sentinel->previous = list->sentinel;
	list->size = 0;
	return list;
}

////////////////////////////////////////////////////////////
void deleteList(List* list) {
	ListNode* tmpNode = list->sentinel->next;
	while (tmpNode != list->sentinel) {
		tmpNode = tmpNode->next;
		free(tmpNode->previous->value);
		free(tmpNode->previous);
	}
	free(list->sentinel);
	free(list);
}

////////////////////////////////////////////////////////////
bool isEmpty(List* list) {
	return !(list->size);
}

////////////////////////////////////////////////////////////
size_t getSize(List* list) {
	return list->size;
}

////////////////////////////////////////////////////////////
void insertFirst(List* list, void* element) {
	ListNode* newNode = (ListNode*) malloc(sizeof(ListNode));
	newNode->value = element;
	newNode->previous = list->sentinel;
	newNode->next = list->sentinel->next;
	list->sentinel->next->previous = newNode;
	list->sentinel->next = newNode;
	++list->size;
}

////////////////////////////////////////////////////////////
void insertLast(List* list, void* element) {
	ListNode* newNode = (ListNode*) malloc(sizeof(ListNode));
	newNode->value = element;
	newNode->previous = list->sentinel->previous;
	newNode->next = list->sentinel;
	list->sentinel->previous->next = newNode;
	list->sentinel->previous = newNode;
	++list->size;
}

////////////////////////////////////////////////////////////
void insertElement(List* list, size_t elementPosition, void* element) {
	ListNode* listNode = getListNode(list, elementPosition);
	if (listNode) {
		ListNode* newNode = (ListNode*) malloc(sizeof(ListNode));
		newNode->value = element;
		newNode->next = listNode;
		newNode->previous = listNode->previous;
		listNode->previous->next = newNode;
		listNode->previous = newNode;
		++list->size;
	}
	else {
		insertLast(list, element);
	}
}

////////////////////////////////////////////////////////////
bool removeFirst(List* list) {
	if (list->size) {
		ListNode* listNode = list->sentinel->next;
		list->sentinel->next = listNode->next;
		listNode->next->previous = list->sentinel;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	LOG("Try to delete first element of an empty list");
	return false;
}

////////////////////////////////////////////////////////////
bool removeLast(List* list) {
	if (list->size) {
		ListNode* listNode = list->sentinel->previous;
		list->sentinel->previous = listNode->previous;
		listNode->previous->next = list->sentinel;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	LOG("Try to delete last element of an empty list");
	return false;
}

////////////////////////////////////////////////////////////
bool removeElement(List* list, size_t elementPosition) {
	ListNode* listNode = getListNode(list, elementPosition);
	if (listNode) {
		listNode->previous->next = listNode->next;
		listNode->next->previous = listNode->previous;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	char msg[100];
	sprintf(msg, "Try to delete element %zu in list of size %zu",elementPosition,list->size);
	LOG(msg);
	return false;
}

////////////////////////////////////////////////////////////
void* getFirst(List* list) {
	return list->sentinel->next->value;
}

////////////////////////////////////////////////////////////
void* getLast(List* list) {
	return list->sentinel->previous->value;
}

////////////////////////////////////////////////////////////
void* getElement(List* list, size_t elementPosition) {
	ListNode* listNode = getListNode(list, elementPosition);
	if (listNode) {
		return listNode->value;
	}
	char msg[100];
	sprintf(msg, "Try to get element %zu in list of size %zu",elementPosition,list->size);
	LOG(msg);
	return NULL;
}

////////////////////////////////////////////////////////////
bool contains(List* list, void* element, int (* compare)(void*, void*)) {
	list->sentinel->value = element;
	ListNode* tmpNode = list->sentinel->next;
	while ((*compare)(tmpNode->value, element)) {
		tmpNode = tmpNode->next;
	}
	return (tmpNode != list->sentinel);
}

////////////////////////////////////////////////////////////
unsigned int getElementPosition(List* list, void* element, int (* compare)(void*, void*)) {
	list->sentinel->value = element;
	ListNode* tmpNode = list->sentinel->next;
	unsigned int i = 0;
	while ((*compare)(tmpNode->value, element)) {
		tmpNode = tmpNode->next;
		++i;
	}
	return i; // if not found i is the size of the list
}

////////////////////////////////////////////////////////////
ListNode* getListNode(List* list, size_t listElementPosition) {
	if (listElementPosition < list->size) {
		ListNode* tmpNode = list->sentinel->next;
		for (size_t i = listElementPosition; i--;) {
			tmpNode = tmpNode->next;
		}
		return tmpNode;
	}
	return NULL;
}

////////////////////////////////////////////////////////////
ListIterator* firstIterator(List* list) {
	ListIterator* listIterator = (ListIterator*) malloc(sizeof(ListIterator));
	listIterator->list = list;
	if (isEmpty(list)) {
		listIterator->listNode = list->sentinel;
		//LOG("Iterator requested on an empty list");
	} else {
		listIterator->listNode = list->sentinel->next;
	}
	return listIterator;
}

////////////////////////////////////////////////////////////
ListIterator* lastIterator(List* list) {
	ListIterator* listIterator = (ListIterator*) malloc(sizeof(ListIterator));
	listIterator->list = list;
	if (isEmpty(list)) {
		listIterator->listNode = list->sentinel;
		//LOG("Iterator requested on an empty list");
	} else {
		listIterator->listNode = list->sentinel->previous;
	}
	return listIterator;
}

////////////////////////////////////////////////////////////
bool hasNext(ListIterator* listIterator) {
	return (listIterator->listNode != listIterator->list->sentinel);
}

////////////////////////////////////////////////////////////
void* next(ListIterator* listIterator) {
	void* value = listIterator->listNode->value;
	listIterator->listNode = listIterator->listNode->next;
	return value;
}

////////////////////////////////////////////////////////////
bool hasPrev(ListIterator* listIterator) {
	return (listIterator->listNode != listIterator->list->sentinel);
}

////////////////////////////////////////////////////////////
void* prev(ListIterator* listIterator) {
	void* value = listIterator->listNode->value;
	listIterator->listNode = listIterator->listNode->previous;
	return value;
}

////////////////////////////////////////////////////////////
void deleteIterator(ListIterator* listIterator) {
	free(listIterator);
}
