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

#include <LinkedList.h>

////////////////////////////////////////////////////////////
LinkedList* ll_createList() {
	LinkedList* list = (LinkedList*) malloc(sizeof(LinkedList));
	list->sentinel = (LinkedListNode*) malloc(sizeof(LinkedListNode));
	list->sentinel->next = list->sentinel;
	list->sentinel->previous = list->sentinel;
	list->size = 0;
	return list;
}

////////////////////////////////////////////////////////////
void ll_deleteList(LinkedList* list) {
	LinkedListNode* tmpNode = list->sentinel->next;
	while(tmpNode != list->sentinel) {
		tmpNode = tmpNode->next;
		free(tmpNode->previous->value);
		free(tmpNode->previous);
	}
	free(list->sentinel);
	free(list);
}

void ll_deleteListNoClean(LinkedList* list) {
	LinkedListNode* tmpNode = list->sentinel->next;
	while(tmpNode != list->sentinel) {
		tmpNode = tmpNode->next;
		free(tmpNode->previous);
	}
	free(list->sentinel);
	free(list);
}

////////////////////////////////////////////////////////////
bool ll_isEmpty(LinkedList* list) {
	return !(list->size);
}

////////////////////////////////////////////////////////////
unsigned int ll_getSize(LinkedList* list) {
	return list->size;
}

////////////////////////////////////////////////////////////
void ll_insertFirst(LinkedList* list, void* element) {
	LinkedListNode* newNode = (LinkedListNode*) malloc(sizeof(LinkedListNode));
	newNode->value = element;
	newNode->previous = list->sentinel;
	newNode->next = list->sentinel->next;
	list->sentinel->next->previous = newNode;
	list->sentinel->next = newNode;
	++list->size;
}

////////////////////////////////////////////////////////////
void ll_insertLast(LinkedList* list, void* element) {
	LinkedListNode* newNode = (LinkedListNode*) malloc(sizeof(LinkedListNode));
	newNode->value = element;
	newNode->previous = list->sentinel->previous;
	newNode->next = list->sentinel;
	list->sentinel->previous->next = newNode;
	list->sentinel->previous = newNode;
	++list->size;
}

////////////////////////////////////////////////////////////
void ll_insertElement(LinkedList* list, unsigned int elementPosition, void* element) {
	LinkedListNode* listNode = ll_getListNode(list, elementPosition);
	if(listNode) {
		LinkedListNode* newNode = (LinkedListNode*) malloc(sizeof(LinkedListNode));
		newNode->value = element;
		newNode->next = listNode;
		newNode->previous = listNode->previous;
		listNode->previous->next = newNode;
		listNode->previous = newNode;
		++list->size;
	}
	else {
		ll_insertLast(list, element);
	}
}

////////////////////////////////////////////////////////////
void ll_insertSorted(LinkedList* list, void* element, int (* comparator)(void*, void*)) {
	if (list->size == 0) {
		LinkedListNode* toInsert = (LinkedListNode*)malloc(sizeof(LinkedListNode));
		toInsert->value = element;
		toInsert->previous = list->sentinel;
		toInsert->next = list->sentinel;

		list->sentinel->next = toInsert;
		list->sentinel->previous = toInsert;

		++(list->size);
		return;
	}

	LinkedListNode* node = list->sentinel->next;
	while (node != list->sentinel) {
		if ((*comparator)(element, node->value) < 0) {
			LinkedListNode* toInsert = (LinkedListNode*)malloc(sizeof(LinkedListNode));
			toInsert->value = element;
			toInsert->next = node;
			toInsert->previous = node->previous;

			node->previous->next = toInsert;
			node->previous = toInsert;

			++(list->size);
			return;
		}
		node = node->next;
	}

	ll_insertLast(list, element);
}

////////////////////////////////////////////////////////////
bool ll_removeFirst(LinkedList* list) {
	if(list->size) {
		LinkedListNode* listNode = list->sentinel->next;
		list->sentinel->next = listNode->next;
		listNode->next->previous = list->sentinel;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	SLOG_WARN("Try to delete first element of an empty list");
	return false;
}

////////////////////////////////////////////////////////////
bool ll_removeLast(LinkedList* list) {
	if(list->size) {
		LinkedListNode* listNode = list->sentinel->previous;
		list->sentinel->previous = listNode->previous;
		listNode->previous->next = list->sentinel;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	SLOG_WARN("Try to delete last element of an empty list");
	return false;
}

////////////////////////////////////////////////////////////
bool ll_removeElement(LinkedList* list, unsigned int elementPosition) {
	LinkedListNode* listNode = ll_getListNode(list, elementPosition);
	if(listNode) {
		listNode->previous->next = listNode->next;
		listNode->next->previous = listNode->previous;
		free(listNode->value);
		free(listNode);
		--list->size;
		return true;
	}
	LOG_WARN("Try to delete element %u in list of size %u", elementPosition, list->size);
	return false;
}

////////////////////////////////////////////////////////////
void* ll_getFirst(LinkedList* list) {
	return list->sentinel->next->value;
}

////////////////////////////////////////////////////////////
void* ll_getLast(LinkedList* list) {
	return list->sentinel->previous->value;
}

////////////////////////////////////////////////////////////
void* ll_getElement(LinkedList* list, unsigned int elementPosition) {
	LinkedListNode* listNode = ll_getListNode(list, elementPosition);
	if(listNode) {
		return listNode->value;
	}
	LOG_WARN("Try to get element %u in list of size %u", elementPosition, list->size);
	return NULL;
}

////////////////////////////////////////////////////////////
bool ll_contains(LinkedList* list, void* element, int (* compare)(void*, void*)) {
	list->sentinel->value = element;
	LinkedListNode* tmpNode = list->sentinel->next;
	while((*compare)(tmpNode->value, element)) {
		tmpNode = tmpNode->next;
	}
	return (tmpNode != list->sentinel);
}

////////////////////////////////////////////////////////////
LinkedListIterator* ll_findElement(LinkedList* list, void* descriptor, int (* check)(void*, void*)) {
	LinkedListNode* tmpNode = list->sentinel->next;
	while(tmpNode != list->sentinel){
		if((*check)(descriptor, tmpNode->value)){
			LinkedListIterator* listIterator = (LinkedListIterator*) malloc(sizeof(LinkedListIterator));
			listIterator->list = list;
			listIterator->listNode = tmpNode;

			return listIterator;
		}

		tmpNode = tmpNode->next;
	}
	return NULL; // not found
}

////////////////////////////////////////////////////////////
unsigned int ll_getElementPosition(LinkedList* list, void* element, int (* compare)(void*, void*)) {
	list->sentinel->value = element;
	LinkedListNode* tmpNode = list->sentinel->next;
	unsigned int i = 0;
	while((*compare)(tmpNode->value, element)) {
		tmpNode = tmpNode->next;
		++i;
	}
	return i; // if not found i is the size of the list
}

////////////////////////////////////////////////////////////
LinkedListNode* ll_getListNode(LinkedList* list, unsigned int listElementPosition) {
	if(listElementPosition < list->size) {
		LinkedListNode* tmpNode = list->sentinel->next;
		for(unsigned int i = listElementPosition; i--;) {
			tmpNode = tmpNode->next;
		}
		return tmpNode;
	}
	return NULL;
}

////////////////////////////////////////////////////////////
LinkedListIterator* ll_firstIterator(LinkedList* list) {
	LinkedListIterator* listIterator = (LinkedListIterator*) malloc(sizeof(LinkedListIterator));
	listIterator->list = list;
	if(ll_isEmpty(list)) {
		listIterator->listNode = list->sentinel;
		SLOG_INFO("Iterator requested on an empty list");
	}
	else {
		listIterator->listNode = list->sentinel->next;
	}
	return listIterator;
}

////////////////////////////////////////////////////////////
LinkedListIterator* ll_lastIterator(LinkedList* list) {
	LinkedListIterator* listIterator = (LinkedListIterator*) malloc(sizeof(LinkedListIterator));
	listIterator->list = list;
	if(ll_isEmpty(list)) {
		listIterator->listNode = list->sentinel;
		SLOG_INFO("Iterator requested on an empty list");
	}
	else {
		listIterator->listNode = list->sentinel->previous;
	}
	return listIterator;
}

////////////////////////////////////////////////////////////
bool ll_hasNext(LinkedListIterator* listIterator) {
	return (listIterator->listNode != listIterator->list->sentinel);
}

////////////////////////////////////////////////////////////
void* ll_next(LinkedListIterator* listIterator) {
	void* value = listIterator->listNode->value;
	listIterator->listNode = listIterator->listNode->next;
	return value;
}

////////////////////////////////////////////////////////////
bool ll_hasPrev(LinkedListIterator* listIterator) {
	return (listIterator->listNode != listIterator->list->sentinel);
}

////////////////////////////////////////////////////////////
void* ll_prev(LinkedListIterator* listIterator) {
	void* value = listIterator->listNode->value;
	listIterator->listNode = listIterator->listNode->previous;
	return value;
}

void ll_removeIt(LinkedListIterator* listIterator) {
	LinkedListNode* node = listIterator->listNode;
	node->previous->next = node->next;
	node->next->previous = node->previous;

	listIterator->listNode = listIterator->listNode->next;

	--(listIterator->list->size);

	free(node);
}

////////////////////////////////////////////////////////////
void ll_deleteIterator(LinkedListIterator* listIterator) {
	free(listIterator);
}

void* ll_getValue(LinkedListIterator* it) {
	return it->listNode->value;
}
