//
// List.cc
//
// Implementation of the List class
//
//
#if RELEASE
static char	RCSid[] = "$Id: List.cc,v 1.1.1.1.2.1 1999/03/23 00:34:19 grdetil Exp $";
#endif

#include "List.h"

//*********************************************************************
// List::List()
//   Constructor
//
List::List()
{
    head = tail = current = 0;
    number = 0;
    current_index = -1;
}


//*********************************************************************
// List::~List()
//   Destructor
//
List::~List()
{
    Destroy();
}


//*********************************************************************
// void List::Release()
//   Release all the objects from our list.
//
void List::Release()
{
    listnode		*node;
    while (head)
    {
	node = head;
	head = head->next;
	delete node;
    }
    head = tail = current = 0;
    number = 0;
    current_index = -1;
}


//*********************************************************************
// void List::Destroy()
//   Delete all the objects from our list.
//
void List::Destroy()
{
    listnode		*node;
    while (head)
    {
	node = head;
	head = head->next;
	delete node->object;
	delete node;
    }
    head = tail = current = 0;
    number = 0;
    current_index = -1;
}


//*********************************************************************
// void List::Add(Object *object)
//   Add an object to the list.
//
void List::Add(Object *object)
{
    listnode		*node = new listnode;
    node->next = 0;
    node->prev = tail;
    node->object = object;
    if (tail)
    {
	tail->next = node;
	tail = node;
    }
    else
    {
	head = tail = node;
    }

    number++;
}


//*********************************************************************
// void List::Insert(Object *object, int position)
//   Add an object to the list.
//
void List::Insert(Object *object, int position)
{
    listnode		*node = new listnode;
    node->next = 0;
    node->prev = 0;
    node->object = object;

    listnode		*ln = head;

    for (int i = 0; i < position && ln; i++, ln = ln->next)
	;
    if (!ln)
    {
	node->prev = tail;
	if (tail)
	    tail->next = node;
	tail = node;

	//
	// The list is empty.  This is a simple case, then.
	//
	if (!head)
	    head = node;
    }
    else
    {
	if (ln == head)
	{
	    node->next = head;
	    node->next->prev = node;
	    head = node;
	}
	else
	{
	    node->next = ln;
	    node->prev = ln->prev;
	    node->prev->next = node;
	    node->next->prev = node;
	}
    }

    current_index = -1;;
    number++;
}


//*********************************************************************
// void List::Assign(Object *object, int position)
//   Assign a new value to an index.
//
void List::Assign(Object *object, int position)
{
    //
    // First make sure that there is something there!
    //
    while (number < position + 1)
    {
	Add(0);
    }

    //
    // Now find the listnode to put the new object in
    //
    listnode	*temp = head;

    for (int i = 0; temp && i < position; i++)
    {
	temp = temp->next;
    }

    current_index = -1;
    delete temp->object;
    temp->object = object;
}


//*********************************************************************
// int List::Remove(Object *object)
//   Remove an object from the list.
//
int List::Remove(Object *object)
{
    listnode		*node = head;
    while (node)
    {
	if (node->object == object)
	{
	    //
	    // Found it!
	    //
	    //
	    // If we are in the middle of a Get_Next() sequence, we need to
	    // fix up any problems with the current node.
	    //
	    if (current == node)
	    {
		current = node->next;
	    }

	    if (head == tail)
	    {
		head = tail = 0;
	    }
	    else if (head == node)
	    {
		head = head->next;
		head->prev = 0;
	    }
	    else if (tail == node)
	    {
		tail = tail->prev;
		tail->next = 0;
	    }
	    else
	    {
		node->next->prev = node->prev;
		node->prev->next = node->next;
	    }

	    delete node;
	    number--;
	    current_index = -1;
	    return 1;
	}
	node = node->next;
    }
    return 0;
}


//*********************************************************************
// Object *List::Get_Next()
//   Return the next object in the list.
//
Object *List::Get_Next()
{
    listnode	*temp = current;

    if (current)
    {
	current = current->next;
	if (current_index >= 0)
	    current_index++;
    }
    else
	return 0;
    return temp->object;
}


//*********************************************************************
// Object *List::Get_First()
//   Return the first object in the list.
//
Object *List::Get_First()
{
    if (head)
	return head->object;
    else
	return 0;
}


//*********************************************************************
// int List::Index(Object *obj)
//   Return the index of an object in the list.
//
int List::Index(Object *obj)
{
    listnode	*temp = head;
    int			index = 0;

    while (temp && temp->object != obj)
    {
	temp = temp->next;
	index++;
    }
    if (index >= number)
	return -1;
    else
	return index;
}


//*********************************************************************
// Object *List::Next(Object *prev)
//   Return the next object in the list.  Using this, the list will
//   appear as a circular list.
//
Object *List::Next(Object *prev)
{
    listnode	*node = head;
    while (node)
    {
	if (node->object == prev)
	{
	    node = node->next;
	    if (!node)
		return head->object;
	    else
		return node->object;
	}
	node = node->next;
    }
	
    return 0;
}


//*********************************************************************
// Object *List::Previous(Object *prev)
//   Return the previous object in the list.  Using this, the list will
//   appear as a circular list.
//
Object *List::Previous(Object *prev)
{
    listnode	*node = tail;
    while (node)
    {
	if (node->object == prev)
	{
	    node = node->prev;
	    if (!node)
		return tail->object;
	    else
		return node->object;
	}
	node = node->prev;
    }
	
    return 0;
}


//*********************************************************************
// Object *List::Nth(int n)
//   Return the nth object in the list.
//
Object *List::Nth(int n)
{
    if (n < 0 || n >= number)
        return 0;

    listnode	*temp = head;

    if (current_index == n)
	return current->object;

    if (current && current_index >= 0 && n == current_index + 1)
    {
	current = current->next;
	if (!current)
	{
	    current_index = -1;
	    return 0;
	}
	current_index = n;
	return current->object;
    }

    for (int i = 0; temp && i < n; i++)
    {
	temp = temp->next;
    }

    if (temp)
    {
	current_index = n;
	current = temp;
	return temp->object;
    }
    else
	return 0;
}


//*********************************************************************
// Object *List::Last()
//   Return the last object inserted.
//
Object *List::Last()
{
    if (tail)
    {
	return tail->object;
    }

    return 0;
}


//*********************************************************************
// Object *List::Copy()
//   Return a deep copy of the list.
//
Object *List::Copy()
{
    List	*list = new List;

    Start_Get();
    Object	*obj;
    while ((obj = Get_Next()))
    {
	list->Add(obj->Copy());
    }
    return list;
}


//*********************************************************************
// List &List::operator=(List &list)
//   Return a deep copy of the list.
//
List &List::operator=(List &list)
{
    Destroy();
    list.Start_Get();
    Object	*obj;
    while ((obj = list.Get_Next()))
    {
	Add(obj->Copy());
    }
    return *this;
}


